#pragma once

#include "sprite.hxx"
#include "canvas.hxx"
#include <memory>

namespace vcl_canvas
{
class CustomSprite : public SpriteBase
{
public:
    virtual Canvas* getContentCanvas() = 0;
};

typedef std::shared_ptr<CustomSprite> CustomSpriteSharedPtr;
}
