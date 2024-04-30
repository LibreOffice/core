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

#include <memory>
#include <unordered_map>
#include <vector>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <oox/helper/helper.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml {

class CustomShapeProperties;

typedef std::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

struct CustomShapeGuide
{
    OUString   maName;
    OUString   maFormula;
};

class CustomShapeGuideContainer
{
public:
    sal_Int32 GetCustomShapeGuideValue( const OUString& rFormulaName ) const;
    sal_Int32 SetCustomShapeGuideValue( const CustomShapeGuide& rGuide );

    void push_back( const CustomShapeGuide& rGuide );
    size_t size() const { return maGuideList.size(); };
    bool empty() const { return maGuideList.empty(); };
    std::vector< CustomShapeGuide >::const_iterator begin() const { return maGuideList.begin(); };
    std::vector< CustomShapeGuide >::const_iterator end() const { return maGuideList.end(); };
    const CustomShapeGuide& operator[](size_t nIndex) const { return maGuideList[ nIndex ]; };

private:
    std::vector< CustomShapeGuide > maGuideList;
    mutable std::unordered_map< OUString, sal_Int32 > maGuideListLookupMap;
    mutable bool mbLookupMapStale = false;
    mutable sal_Int32 mnPreviousActSize = 0;

    void ActualizeLookupMap() const;
};

struct AdjustHandle
{
    bool                                    polar;
    css::drawing::EnhancedCustomShapeParameterPair
                                            pos;

    // depending to the type (polar or not):
    std::optional< OUString >               gdRef1; // gdRefX   or gdRefR
    std::optional< css::drawing::EnhancedCustomShapeParameter >
                                            min1;   // minX     or minR
    std::optional< css::drawing::EnhancedCustomShapeParameter >
                                            max1;   // maxX     or maxR
    std::optional< OUString >               gdRef2; // gdRefY   or gdRefAng
    std::optional< css::drawing::EnhancedCustomShapeParameter >
                                            min2;   // minX     or minAng
    std::optional< css::drawing::EnhancedCustomShapeParameter >
                                            max2;   // maxY     or maxAng

    AdjustHandle( bool bPolar ) : polar( bPolar ) {};
};

struct ConnectionSite
{
    css::drawing::EnhancedCustomShapeParameterPair
                                pos;
    css::drawing::EnhancedCustomShapeParameter
                                ang;
};

struct GeomRect
{
    css::drawing::EnhancedCustomShapeParameter   l;
    css::drawing::EnhancedCustomShapeParameter   t;
    css::drawing::EnhancedCustomShapeParameter   r;
    css::drawing::EnhancedCustomShapeParameter   b;
};

struct Path2D
{
    sal_Int64   w;
    sal_Int64   h;
    sal_Int32   fill;
    bool        stroke;
    bool        extrusionOk;
    std::vector< css::drawing::EnhancedCustomShapeParameterPair > parameter;

    Path2D() : w( 0 ), h( 0 ), fill( XML_norm ), stroke( true ), extrusionOk( true ) {};
};


class CustomShapeProperties final
{
public:
    CustomShapeProperties();

    void pushToPropSet( const css::uno::Reference < css::beans::XPropertySet > & xPropSet,
                        const css::awt::Size &aSize );

    sal_Int32 getShapePresetType() const { return mnShapePresetType; }
    OUString getShapePresetTypeName() const;
    void setShapePresetType( sal_Int32 nShapePresetType ){ mnShapePresetType = nShapePresetType; };
    bool                                getShapeTypeOverride() const { return mbShapeTypeOverride; };
    void                                setShapeTypeOverride( bool bShapeTypeOverride ) { mbShapeTypeOverride = bShapeTypeOverride; };

    CustomShapeGuideContainer&          getAdjustmentGuideList(){ return maAdjustmentGuideList; };
    CustomShapeGuideContainer&          getGuideList(){ return maGuideList; };
    std::vector< AdjustHandle >&        getAdjustHandleList(){ return maAdjustHandleList; };
    std::vector< ConnectionSite >&      getConnectionSiteList(){ return maConnectionSiteList; };
    std::optional< GeomRect >&          getTextRect(){ return maTextRect; };
    std::vector< Path2D >&              getPath2DList(){ return maPath2DList; };
    std::vector< css::drawing::EnhancedCustomShapeSegment >& getSegments(){ return maSegments; };
    void                                setMirroredX( bool bMirroredX ) { mbMirroredX = bMirroredX; };
    void                                setMirroredY( bool bMirroredY ) { mbMirroredY = bMirroredY; };
    void                                setTextPreRotateAngle( sal_Int32 nAngle ) { mnTextPreRotateAngle = nAngle; };
    void                                setTextCameraZRotateAngle( sal_Int32 nAngle ) { mnTextCameraZRotateAngle = nAngle; };
    void                                setTextAreaRotateAngle(sal_Int32 nAngle) { moTextAreaRotateAngle = nAngle; };

    sal_Int32 getArcNum() { return mnArcNum++; }
    sal_Int32 countArcTo() { return mnArcNum; }
    PropertyMap& getExtrusionPropertyMap() { return maExtrusionPropertyMap; }

    /**
       Returns whether or not the current CustomShapeProperties
       represent a default shape preset that is rectangular.
    */
    bool representsDefaultShape() const;

private:

    sal_Int32                       mnShapePresetType;
    bool                            mbShapeTypeOverride;
    CustomShapeGuideContainer       maAdjustmentGuideList;
    CustomShapeGuideContainer       maGuideList;
    std::vector< AdjustHandle >     maAdjustHandleList;
    std::vector< ConnectionSite >   maConnectionSiteList;
    std::optional< GeomRect >       maTextRect;
    std::vector< Path2D >           maPath2DList;

    std::vector< css::drawing::EnhancedCustomShapeSegment >
                                    maSegments;
    bool                            mbMirroredX;
    bool                            mbMirroredY;
    sal_Int32                       mnTextPreRotateAngle; // TextPreRotateAngle
    sal_Int32                       mnTextCameraZRotateAngle;
    std::optional< sal_Int32 >      moTextAreaRotateAngle; // TextRotateAngle

    typedef std::unordered_map< sal_Int32, PropertyMap > PresetDataMap;

    static PresetDataMap maPresetDataMap;
    static void initializePresetDataMap();

    sal_Int32 mnArcNum;
    PropertyMap maExtrusionPropertyMap;
};

}

#endif // INCLUDED_OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
