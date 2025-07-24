#pragma once

#include <memory>
namespace vcl_canvas
{
class Canvas
{
public:
    virtual void clear() = 0;
    // NO DRAWING FUNCTIONS NEEDED :)
};

typedef std::shared_ptr<Canvas> CanvasSharedPtr;
}
