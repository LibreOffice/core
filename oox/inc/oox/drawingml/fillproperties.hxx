/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fillproperties.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_FILLPROPERTIES_HXX
#define OOX_DRAWINGML_FILLPROPERTIES_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include "tokens.hxx"
#include <vector>
#include <map>

namespace oox { namespace drawingml {

class FillProperties;
class BlipFillPropertiesContext;

struct GradientStop
{
    ::oox::drawingml::ColorPtr  maColor;
    double                      mnPosition; // ST_PositiveFixedPercentage
    GradientStop() : maColor( new Color ), mnPosition( 0 ) {};
};

typedef boost::shared_ptr< FillProperties > FillPropertiesPtr;

class FillProperties
{
    friend class BlipFillPropertiesContext;

public:

    FillProperties( sal_Int32 nContext = XML_spPr );
    virtual ~FillProperties();

    void apply( const FillProperties& );
    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
            const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet, sal_Int32 nShapeRotation ) const;

    ::boost::optional< ::com::sun::star::drawing::FillStyle >&          getFillStyle(){ return moFillStyle; };
    ::boost::optional< ::com::sun::star::drawing::BitmapMode >&         getBitmapMode(){ return moBitmapMode; };
    ::boost::optional< sal_Bool >&                                      getRotateWithShape(){ return moRotateWithShape; };
    ::boost::optional< sal_Bool >&                                      getShadeScaled(){ return moShadeScaled; };
    ::boost::optional< sal_Int32 >&                                     getShadeAngle(){ return moShadeAngle; };
    ::boost::optional< sal_Int32 >&                                     getShadeTypeToken(){ return moShadeTypeToken; };
    ::boost::optional< sal_Int32 >&                                     getFlipModeToken(){ return moFlipModeToken; };
    ::boost::optional< com::sun::star::geometry::IntegerRectangle2D >&  getFillToRect(){ return moFillToRect; };
    ::boost::optional< com::sun::star::geometry::IntegerRectangle2D >&  getTileRect(){ return moTileRect; };
    ::boost::optional< sal_Int32 >&                                     getTileAlign(){ return moTileAlign; };
    ::boost::optional< sal_Int32 >&                                     getTileX(){ return moTileX; };
    ::boost::optional< sal_Int32 >&                                     getTileY(){ return moTileY; };
    ::boost::optional< sal_Int32 >&                                     getTileSX(){ return moTileSX; };
    ::boost::optional< sal_Int32 >&                                     getTileSY(){ return moTileSY; };
    std::vector< GradientStop >&                                        getGradientStops(){ return mvGradientStops; };
    ::oox::drawingml::ColorPtr&                                         getFillColor(){ return maFillColor; };
    ::oox::drawingml::ColorPtr&                                         getColorChangeFrom(){ return maColorChangeFrom; };
    ::oox::drawingml::ColorPtr&                                         getColorChangeTo(){ return maColorChangeTo; };
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > getXGraphic(){ return mxGraphic; };

private:

    // creates the graphic for a graphic object and its corresponding properties
    void createTransformedGraphic( const oox::core::XmlFilterBase& rFilterBase,
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet ) const;

    sal_Int32                                                               mnContext;
    ::boost::optional< ::com::sun::star::drawing::FillStyle >               moFillStyle;
    ::boost::optional< ::com::sun::star::drawing::BitmapMode >              moBitmapMode;
    ::boost::optional< sal_Bool >                                           moRotateWithShape;
    ::boost::optional< sal_Bool >                                           moShadeScaled;
    ::boost::optional< sal_Int32 >                                          moShadeAngle;
    ::boost::optional< sal_Int32 >                                          moShadeTypeToken;
    ::boost::optional< sal_Int32 >                                          moFlipModeToken;
    ::boost::optional< com::sun::star::geometry::IntegerRectangle2D >       moFillToRect;
    ::boost::optional< com::sun::star::geometry::IntegerRectangle2D >       moTileRect;
    ::boost::optional< sal_Int32 >                                          moTileAlign;
    ::boost::optional< sal_Int32 >                                          moTileX;
    ::boost::optional< sal_Int32 >                                          moTileY;
    ::boost::optional< sal_Int32 >                                          moTileSX;
    ::boost::optional< sal_Int32 >                                          moTileSY;
    std::vector< GradientStop >                                             mvGradientStops;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > mxGraphic;
    ::oox::drawingml::ColorPtr                                              maFillColor;
    ::oox::drawingml::ColorPtr                                              maColorChangeFrom;
    ::oox::drawingml::ColorPtr                                              maColorChangeTo;
};

} }

#endif  //  OOX_DRAWINGML_FILLPROPERTIES_HXX
