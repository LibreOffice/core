/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
#define OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include <com/sun/star/graphic/XGraphic.hpp>
#include <boost/shared_ptr.hpp>
#include "oox/helper/helper.hxx"
#include "tokens.hxx"
#include <vector>
#include <map>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#ifndef __com_sun_star_beans_PropertyValues_hpp__
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/XShape.hpp>

namespace oox { namespace drawingml {

class CustomShapeProperties;

typedef boost::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

struct CustomShapeGuide
{
    rtl::OUString	maName;
    rtl::OUString	maFormula;
};

struct AdjustHandle
{
    sal_Bool								polar;
    com::sun::star::drawing::EnhancedCustomShapeParameterPair
                                            pos;

    // depending to the type (polar or not):
    OptValue< rtl::OUString >				gdRef1;	// gdRefX	or gdRefR
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            min1;	// minX		or minR
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            max1;	// maxX		or maxR
    OptValue< rtl::OUString >				gdRef2;	// gdRefY	or gdRefAng
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            min2;	// minX		or minAng
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            max2;	// maxY		or maxAng

    AdjustHandle( sal_Bool bPolar ) : polar( bPolar ) {};
};

struct ConnectionSite
{
    com::sun::star::drawing::EnhancedCustomShapeParameterPair
                                pos;
    com::sun::star::drawing::EnhancedCustomShapeParameter
                                ang;
};

struct GeomRect
{
    com::sun::star::drawing::EnhancedCustomShapeParameter	l;
    com::sun::star::drawing::EnhancedCustomShapeParameter	t;
    com::sun::star::drawing::EnhancedCustomShapeParameter	r;
    com::sun::star::drawing::EnhancedCustomShapeParameter	b;
};

struct Path2D
{
    sal_Int64	w;
    sal_Int64	h;
    sal_Int32	fill;
    sal_Bool	stroke;
    sal_Bool	extrusionOk;
    std::vector< com::sun::star::drawing::EnhancedCustomShapeParameterPair > parameter;

    Path2D() : w( 0 ), h( 0 ), fill( XML_norm ), stroke( sal_True ), extrusionOk( sal_True ) {};
};

class CustomShapeProperties
{
public:

    CustomShapeProperties();
    virtual ~CustomShapeProperties();

    void apply( const CustomShapePropertiesPtr& );
    void pushToPropSet( const ::oox::core::FilterBase& rFilterBase,
            const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape > & xShape) const;

    void setShapePresetType( const rtl::OUString& rShapePresetType ){ maShapePresetType = rShapePresetType; };

    std::vector< CustomShapeGuide >&	getAdjustmentGuideList(){ return maAdjustmentGuideList; };
    std::vector< CustomShapeGuide >&	getGuideList(){ return maGuideList; };
    std::vector< AdjustHandle >&		getAdjustHandleList(){ return maAdjustHandleList; };
    std::vector< ConnectionSite >&		getConnectionSiteList(){ return maConnectionSiteList; };
    OptValue< GeomRect >&				getTextRect(){ return maTextRect; };
    std::vector< Path2D >&				getPath2DList(){ return maPath2DList; };
    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >& getSegments(){ return maSegments; };
    void								setMirroredX( sal_Bool bMirroredX ) { mbMirroredX = bMirroredX; };
    void								setMirroredY( sal_Bool bMirroredY ) { mbMirroredY = bMirroredY; };

    double getValue( const std::vector< CustomShapeGuide >&, sal_uInt32 nIndex ) const;
    static sal_Int32 SetCustomShapeGuideValue( std::vector< CustomShapeGuide >& rGuideList, const CustomShapeGuide& rGuide );
    static sal_Int32 GetCustomShapeGuideValue( const std::vector< CustomShapeGuide >& rGuideList, const rtl::OUString& rFormulaName );

private:

    rtl::OUString					maShapePresetType;
    std::vector< CustomShapeGuide >	maAdjustmentGuideList;
    std::vector< CustomShapeGuide >	maGuideList;
    std::vector< AdjustHandle >		maAdjustHandleList;
    std::vector< ConnectionSite >	maConnectionSiteList;
    OptValue< GeomRect >			maTextRect;
    std::vector< Path2D >			maPath2DList;

    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >
                                    maSegments;
    sal_Bool						mbMirroredX;
    sal_Bool						mbMirroredY;
};

} }

#endif  //  OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
