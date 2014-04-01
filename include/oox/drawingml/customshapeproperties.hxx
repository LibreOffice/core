/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
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
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/helper/helper.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/token/tokens.hxx>

namespace oox { namespace drawingml {

class CustomShapeProperties;

typedef boost::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

struct CustomShapeGuide
{
    OUString   maName;
    OUString   maFormula;
};

struct AdjustHandle
{
    bool                                    polar;
    com::sun::star::drawing::EnhancedCustomShapeParameterPair
                                            pos;

    // depending to the type (polar or not):
    OptValue< OUString >               gdRef1; // gdRefX   or gdRefR
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            min1;   // minX     or minR
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            max1;   // maxX     or maxR
    OptValue< OUString >               gdRef2; // gdRefY   or gdRefAng
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            min2;   // minX     or minAng
    OptValue< com::sun::star::drawing::EnhancedCustomShapeParameter >
                                            max2;   // maxY     or maxAng

    AdjustHandle( bool bPolar ) : polar( bPolar ) {};
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
    bool        stroke;
    bool        extrusionOk;
    std::vector< com::sun::star::drawing::EnhancedCustomShapeParameterPair > parameter;

    Path2D() : w( 0 ), h( 0 ), fill( XML_norm ), stroke( true ), extrusionOk( true ) {};
};


class CustomShapeProvider {
protected:
    struct ParameterPairData {
        sal_uInt16 nFirstType;
        sal_uInt16 nSecondType;
        sal_uInt32 nFirstValue;
        sal_uInt32 nSecondValue;
    };
    static com::sun::star::uno::Any createStringSequence( size_t nStrings, const char **pStrings );
    static com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > createSegmentSequence( size_t nElems, const sal_uInt16 *pValues );
    static com::sun::star::drawing::EnhancedCustomShapeParameterPair createParameterPair( const ParameterPairData *pData );
    static com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > createParameterPairSequence( size_t nElems, const ParameterPairData *pData );
public:
    virtual ~CustomShapeProvider() {}
    virtual PropertyMap getProperties() = 0;
};

class CustomShapeProperties
{
public:

    CustomShapeProperties();
    virtual ~CustomShapeProperties();

    void pushToPropSet( const ::oox::core::FilterBase& rFilterBase,
            const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape > & xShape, const ::com::sun::star::awt::Size &aSize );

    sal_Int32 getShapePresetType() const { return mnShapePresetType; }
    OUString getShapePresetTypeName() const;
    void setShapePresetType( sal_Int32 nShapePresetType ){ mnShapePresetType = nShapePresetType; };
    bool                                getShapeTypeOverride(){ return mbShapeTypeOverride; };
    void                                setShapeTypeOverride( bool bShapeTypeOverride ) { mbShapeTypeOverride = bShapeTypeOverride; };

    std::vector< CustomShapeGuide >&    getAdjustmentGuideList(){ return maAdjustmentGuideList; };
    std::vector< CustomShapeGuide >&    getGuideList(){ return maGuideList; };
    std::vector< AdjustHandle >&        getAdjustHandleList(){ return maAdjustHandleList; };
    std::vector< ConnectionSite >&      getConnectionSiteList(){ return maConnectionSiteList; };
    OptValue< GeomRect >&               getTextRect(){ return maTextRect; };
    std::vector< Path2D >&              getPath2DList(){ return maPath2DList; };
    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >& getSegments(){ return maSegments; };
    void                                setMirroredX( bool bMirroredX ) { mbMirroredX = bMirroredX; };
    void                                setMirroredY( bool bMirroredY ) { mbMirroredY = bMirroredY; };
    void                                setTextRotateAngle( sal_Int32 nAngle ) { mnTextRotateAngle = nAngle; };

    static sal_Int32 SetCustomShapeGuideValue( std::vector< CustomShapeGuide >& rGuideList, const CustomShapeGuide& rGuide );
    static sal_Int32 GetCustomShapeGuideValue( const std::vector< CustomShapeGuide >& rGuideList, const OUString& rFormulaName );

    sal_Int32 getArcNum() { return mnArcNum++; }

private:

    sal_Int32                       mnShapePresetType;
    bool                            mbShapeTypeOverride;
    std::vector< CustomShapeGuide > maAdjustmentGuideList;
    std::vector< CustomShapeGuide > maGuideList;
    std::vector< AdjustHandle >     maAdjustHandleList;
    std::vector< ConnectionSite >   maConnectionSiteList;
    OptValue< GeomRect >            maTextRect;
    std::vector< Path2D >           maPath2DList;

    std::vector< com::sun::star::drawing::EnhancedCustomShapeSegment >
                                    maSegments;
    bool                            mbMirroredX;
    bool                            mbMirroredY;
    sal_Int32                       mnTextRotateAngle;

    typedef boost::unordered_map< sal_Int32, CustomShapeProvider * > PresetsMap;

    static PresetsMap maPresetsMap;
    static void initializePresetsMap();
    static void initializePresetsMap1();
    static void initializePresetsMap2();
    static void initializePresetsMap3();
    static void initializePresetsMap4();
    static void initializePresetsMap5();
    static void initializePresetsMap6();

    sal_Int32 mnArcNum;
};

} }

#endif // INCLUDED_OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
