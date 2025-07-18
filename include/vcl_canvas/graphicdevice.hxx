#pragma once

#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/rendering/XLinePolyPolygon2D.hpp>
#include <com/sun/star/rendering/XBezierPolyPolygon2D.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XVolatileBitmap.hpp>
namespace vcl_canvas
{
class GraphicDevice
{
public:
    virtual ::css::uno::Reference< ::css::rendering::XColorSpace > getDeviceColorSpace() = 0;
    virtual ::css::geometry::RealSize2D getPhysicalResolution() = 0;
    virtual ::css::geometry::RealSize2D getPhysicalSize() = 0;
    virtual ::css::uno::Reference< ::css::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon( const ::css::uno::Sequence< ::css::uno::Sequence< ::css::geometry::RealPoint2D > >& points ) = 0;
    virtual ::css::uno::Reference< ::css::rendering::XBezierPolyPolygon2D > createCompatibleBezierPolyPolygon( const ::css::uno::Sequence< ::css::uno::Sequence< ::css::geometry::RealBezierSegment2D > >& points ) = 0;
    virtual ::css::uno::Reference< ::css::rendering::XBitmap > createCompatibleBitmap( const ::css::geometry::IntegerSize2D& size ) = 0;
    virtual ::css::uno::Reference< ::css::rendering::XVolatileBitmap > createVolatileBitmap( const ::css::geometry::IntegerSize2D& size ) = 0;
    virtual ::css::uno::Reference< ::css::rendering::XBitmap > createCompatibleAlphaBitmap( const ::css::geometry::IntegerSize2D& size ) = 0;
    virtual ::css::uno::Reference< ::css::rendering::XVolatileBitmap > createVolatileAlphaBitmap( const ::css::geometry::IntegerSize2D& size ) = 0;
    virtual ::sal_Bool hasFullScreenMode() = 0;
    virtual ::sal_Bool enterFullScreenMode( ::sal_Bool bEnter ) = 0;
};
}
