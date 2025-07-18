#pragma once

#include "com/sun/star/geometry/AffineMatrix2D.hdl"
#include "com/sun/star/geometry/RealPoint2D.hdl"
#include "com/sun/star/rendering/RenderState.hdl"
#include "com/sun/star/rendering/ViewState.hdl"
#include <memory>
namespace vcl_canvas
{
class SpriteBase
{
public:
    virtual void setAlpha( double nAlpha ) = 0;
    virtual void move( const ::css::geometry::RealPoint2D& aNewPos, const ::css::rendering::ViewState& aViewState, const ::css::rendering::RenderState& aRenderState ) = 0;
    virtual void transform( const ::css::geometry::AffineMatrix2D& aTransformation ) = 0;
    virtual void clip( const ::css::uno::Reference< ::css::rendering::XPolyPolygon2D >& aClip ) = 0;
    virtual void setPriority( double nPriority ) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
};

typedef std::shared_ptr<SpriteBase> SpriteSharedPtr;
}
