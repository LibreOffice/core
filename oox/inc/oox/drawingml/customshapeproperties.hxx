/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
#define OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX

#include <boost/shared_ptr.hpp>
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
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/color.hxx"
#include "oox/helper/helper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/token/tokens.hxx"

namespace oox { namespace drawingml {

class CustomShapeProperties;

typedef boost::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

struct CustomShapeGuide
{
    rtl::OUString   maName;
    rtl::OUString   maFormula;
};

struct AdjustHandle
{
    sal_Bool                                polar;
    com::sun::star::drawing::EnhancedCustomShapeParameterPair
                                            pos;

    // depending to the type (polar or not):
    OptValue< rtl::OUString >               gdRef1; // gdRefX   or gdRefR
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            min1;   // minX     or minR
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            max1;   // maxX     or maxR
    OptValue< rtl::OUString >               gdRef2; // gdRefY   or gdRefAng
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            min2;   // minX     or minAng
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            max2;   // maxY     or maxAng

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
    com::sun::star::drawing::EnhancedCustomShapeParameter   l;
    com::sun::star::drawing::EnhancedCustomShapeParameter   t;
    com::sun::star::drawing::EnhancedCustomShapeParameter   r;
    com::sun::star::drawing::EnhancedCustomShapeParameter   b;
};

struct Path2D
{
    sal_Int64   w;
    sal_Int64   h;
    sal_Int32   fill;
    sal_Bool    stroke;
    sal_Bool    extrusionOk;
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

    std::vector< CustomShapeGuide >&    getAdjustmentGuideList(){ return maAdjustmentGuideList; };
    std::vector< CustomShapeGuide >&    getGuideList(){ return maGuideList; };
    std::vector< AdjustHandle >&        getAdjustHandleList(){ return maAdjustHandleList; };
    std::vector< ConnectionSite >&      getConnectionSiteList(){ return maConnectionSiteList; };
    OptValue< GeomRect >&               getTextRect(){ return maTextRect; };
    std::vector< Path2D >&              getPath2DList(){ return maPath2DList; };
    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >& getSegments(){ return maSegments; };
    void                                setMirroredX( sal_Bool bMirroredX ) { mbMirroredX = bMirroredX; };
    void                                setMirroredY( sal_Bool bMirroredY ) { mbMirroredY = bMirroredY; };

    // #119920# Add missing extra text rotation
    void setTextRotation(sal_Int32 nTextRotation) { mnTextRotation = nTextRotation; }

    double getValue( const std::vector< CustomShapeGuide >&, sal_uInt32 nIndex ) const;
    static sal_Int32 SetCustomShapeGuideValue( std::vector< CustomShapeGuide >& rGuideList, const CustomShapeGuide& rGuide );
    static sal_Int32 GetCustomShapeGuideValue( const std::vector< CustomShapeGuide >& rGuideList, const rtl::OUString& rFormulaName );

private:

    rtl::OUString                   maShapePresetType;
    std::vector< CustomShapeGuide > maAdjustmentGuideList;
    std::vector< CustomShapeGuide > maGuideList;
    std::vector< AdjustHandle >     maAdjustHandleList;
    std::vector< ConnectionSite >   maConnectionSiteList;
    OptValue< GeomRect >            maTextRect;
    std::vector< Path2D >           maPath2DList;

    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >
                                    maSegments;
    sal_Bool                        mbMirroredX;
    sal_Bool                        mbMirroredY;

    // #119920# Add missing extra text rotation
    sal_Int32                       mnTextRotation;
};

} }

#endif  //  OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
