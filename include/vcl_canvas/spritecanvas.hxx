#pragma once

#include <com/sun/star/rendering/XAnimation.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include "canvas.hxx"
#include "customsprite.hxx"
namespace vcl_canvas
{

class SpriteCanvas : public Canvas
{
public:
    virtual CustomSpriteSharedPtr createCustomSprite( const ::css::geometry::RealSize2D& spriteSize ) = 0;
    virtual SpriteSharedPtr createClonedSprite( const SpriteBase& original ) = 0;
    virtual bool updateScreen( bool bUpdateAll ) = 0;
};

}
