#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>

typedef sf::Vector2<double> Point;

struct Ray
{
    Point start, end;
};

struct Segment
{
    Segment(const Point p1, const Point p2)
        : p1(p1)
          , p2(p2)
    {}
    Point p1, p2;
};

struct Wall
{

    Segment* segments[4];
    Point points[4];

    Wall(float x, float y, float width, float height)
    {
        segments[0] = new Segment(Point(x, y), Point(x + width, y));
        segments[1] = new Segment(Point(x + width, y), Point(x + width, y + height));
        segments[2] = new Segment(Point(x + width, y + height), Point(x, y + height));
        segments[3] = new Segment(Point(x, y + height), Point(x, y));

        for(std::size_t i = 0; i < 4; i++)
        {
            points[i] = segments[i]->p1;
        }
    }

    void debugDraw(sf::RenderTarget* window)
    {
        for(std::size_t i = 0; i < 4; i++)
        {
            sf::Vertex line[2];
            line[0].position = sf::Vector2f(segments[i]->p1.x, segments[i]->p1.y);
            line[0].color = sf::Color::Green;
            line[1].position = sf::Vector2f(segments[i]->p2.x, segments[i]->p2.y);
            line[1].color = sf::Color::Green;
            window->draw(line, 2, sf::Lines);

            sf::CircleShape pointNode(2);
            pointNode.setOrigin(pointNode.getRadius(), pointNode.getRadius());
            pointNode.setFillColor(sf::Color::Blue);
            pointNode.setPosition(sf::Vector2f(points[i].x, points[i].y));
            window->draw(pointNode);

        }
    }

};

double dot(const Point& a, const Point& b)     { return (a.x * b.x) + (a.y * b.y); }
double perpDot(const Point& a, const Point& b) { return (a.y * b.x) - (a.x * b.y); }

bool getIntersection(Ray ray, Segment* segment, Point& intersect)
{
    Point a(ray.end - ray.start);
    Point b(segment->p2 - segment->p1);

    double f = perpDot(a, b);
    if(!f)
        return false;

    Point c(segment->p2 - ray.end);
    double aa = perpDot(a,c);
    double bb = perpDot(b,c);

    if(f < 0)
    {
        if(aa > 0 || aa < f) return false;
        if(bb > 0 || bb < f) return false;
    }
    else
    {
        if(aa < 0 || aa > f) return false;
        if(bb < 0 || bb > f) return false;
    }

    double out = 1.0 - (aa / f);
    intersect = ((segment->p2 - segment->p1) * out) + segment->p1;
    return true;
}

struct RayLine
{
    Point point;
    double angle;
    bool boundary;
    bool operator<(const RayLine& r2)
    {
        return (angle < r2.angle);
    }

};

bool isSegmentFacing(Ray ray, Segment* segment, int i)
{
    switch(i)
    {
        case 0:
            if(ray.start.y < segment->p1.y) return true;
            break;
        case 1:
            if(ray.start.x > segment->p1.x) return true;
            break;
        case 2:
            if(ray.start.y > segment->p1.y) return true;
            break;
        case 3:
            if(ray.start.x < segment->p1.x) return true;
            break;
    }
    return false;
};

bool isPointBoundary(Ray ray, Wall* wall, int pn)
{
    int prevPoint = (pn == 0) ? 3 : pn - 1;
    bool s1 = isSegmentFacing(ray, wall->segments[prevPoint], prevPoint);
    bool s2 = isSegmentFacing(ray, wall->segments[pn], pn);
    if(s1 != s2)
        return true;
    return false;
}

void drawCircle(float radius, Point pos, sf::Color colour, sf::RenderTarget* window)
{
    sf::CircleShape circle(radius);
    circle.setOrigin(circle.getRadius(), circle.getRadius());
    circle.setFillColor(colour);
    circle.setPosition(pos.x, pos.y);
    window->draw(circle);
}

void drawLine(Point p1, Point p2, sf::Color colour, sf::RenderTarget* window)
{
    sf::Vertex line[2];
    line[0].position = sf::Vector2f(p1.x, p1.y);
    line[1].position = sf::Vector2f(p2.x, p2.y);
    line[0].color = colour;
    line[1].color = colour;
    window->draw(line, 2, sf::Lines);
}

double distance(Point p1, Point p2)
{
    double dx = p1.x - p2.x;
    double dy = p1.y = p2.y;
    return std::sqrt(dx * dx + dy * dy);
}

int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(800,600), "GameWindow", sf::Style::Default, settings);

    // Add walls - Wall(position, dimensions)
    std::vector<Wall*> walls;
    walls.emplace_back(new Wall(0,0,800,600));
    walls.emplace_back(new Wall(120,100,150,250));

    Ray ray;

    // Max length line needed to reach all points of screen (hyp of right angle triangle)
    float lineMax = std::sqrt((window.getSize().x * window.getSize().x) + (window.getSize().y * window.getSize().y)) + 1;

    // Circle to show the start of the ray
    sf::CircleShape rayStart(3);
    rayStart.setOrigin(rayStart.getRadius(), rayStart.getRadius());
    rayStart.setFillColor(sf::Color::Yellow);

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        // Set ray.start to mouse position
        ray.start.x = (double)sf::Mouse::getPosition(window).x;
        ray.start.y = (double)sf::Mouse::getPosition(window).y;

        window.clear(sf::Color(64,64,64));

        for(Wall* wall : walls)
        {
            wall->debugDraw(&window);
        }

        std::vector<RayLine> raylines;
        for(Wall* wall : walls)
        {
            for(int i = 0; i < 4; i++)
            {
                RayLine rayline;
                rayline.point = wall->points[i];
                rayline.angle = std::atan2(rayline.point.y - ray.start.y, rayline.point.x - ray.start.x);
                rayline.boundary = isPointBoundary(ray, wall, i);
                raylines.push_back(rayline);
            }
        }
        std::sort(raylines.begin(), raylines.end());

        // THE MAIN EVENT! GET THE INTERSECTIONS
        std::vector<Point> convexPoints;
        for(RayLine& rayline : raylines)
        {
            ray.end.x = ray.start.x + lineMax * std::cos(rayline.angle);
            ray.end.y = ray.start.y + lineMax * std::sin(rayline.angle);

            drawLine(ray.start, ray.end, sf::Color::White, &window);

            //calc nearest intersection
            //if boundary 
            //  if nearest closer that point
            //      add nearest
            //  else
            //      if nearest is point
            //         add second nearest
            //      else
            //        add nearest
            //      add point

            std::vector<Point> intersects;
            for(Wall* wall : walls)
            {
                for(Segment* segment : wall->segments)
                {
                    Point intersect;
                    getIntersection(ray, segment, intersect);
                    intersects.push_back(intersect);
                    drawCircle(5, intersect, sf::Color::Magenta, &window);
                }
            }
            std::sort(intersects.begin(), intersects.end(),
                    [ray] (Point& p1, Point& p2) { return distance(p1, ray.start) < distance(p2, ray.start); });

            if(rayline.boundary)
            {
                if(distance(intersects[0], ray.start) < distance(rayline.point, ray.start))
                {
                    convexPoints.push_back(intersects[0]);
                } 
                else
                {
                    if(intersects[0] == rayline.point)
                    {
                        convexPoints.push_back(intersects[1]);
                        convexPoints.push_back(intersects[0]); 
                    } 
                    else
                    {
                        convexPoints.push_back(intersects[0]);
                        convexPoints.push_back(rayline.point);
                    }
                }
            }
            else
            {
                convexPoints.push_back(intersects.front());
            } 
        }

        /* sf::ConvexShape light(convexPoints.size()); */
        /* for(size_t i = 0; i < convexPoints.size(); i++) */
        /*     light.setPoint(i, sf::Vector2f(convexPoints[i].x, convexPoints[i].y)); */
        /* light.setFillColor(sf::Color::White); */
        /* window.draw(light); */

        for(Point& p : convexPoints)
            drawCircle(5, p, sf::Color::Yellow, &window);

        rayStart.setPosition(ray.start.x, ray.start.y);
        window.draw(rayStart);

        window.display();
    }
}
