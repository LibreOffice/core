#pragma once

#include <com/sun/star/rendering/XBitmap.hpp>
#include "canvas.hxx"
#include "customsprite.hxx"
#include "sal/types.h"
namespace vcl_canvas
{

class SpriteCanvas : public Canvas
{
public:
    virtual CustomSpriteSharedPtr createCustomSprite( const ::css::geometry::RealSize2D& spriteSize ) = 0;
    // virtual SpriteSharedPtr createClonedSprite( const SpriteBase& original ) = 0;
    virtual sal_Bool updateScreen( sal_Bool bUpdateAll ) = 0;
};

typedef std::shared_ptr< SpriteCanvas > SpriteCanvasSharedPtr;

}
