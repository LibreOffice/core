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

#include "drawingml/customshapeproperties.hxx"
#include "oox/helper/helper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/tokenmap.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::drawing;

namespace oox { namespace drawingml {

CustomShapeProperties::CustomShapeProperties()
: mnShapePresetType ( -1 )
, mbShapeTypeOverride(false)
, mbMirroredX   ( false )
, mbMirroredY   ( false )
, mnTextRotateAngle ( 0 )
, mnArcNum ( 0 )
{
}
CustomShapeProperties::~CustomShapeProperties()
{
}

uno::Sequence< sal_Int8 > CustomShapeProperties::getShapePresetTypeName() const
{
    return StaticTokenMap::get().getUtf8TokenName( mnShapePresetType );
}

sal_Int32 CustomShapeProperties::SetCustomShapeGuideValue( std::vector< CustomShapeGuide >& rGuideList, const CustomShapeGuide& rGuide )
{
    sal_uInt32 nIndex = 0;
    for( ; nIndex < rGuideList.size(); nIndex++ )
    {
        if ( rGuideList[ nIndex ].maName == rGuide.maName )
            break;
    }
    if ( nIndex == rGuideList.size() )
        rGuideList.push_back( rGuide );
    return static_cast< sal_Int32 >( nIndex );
}

// returns the index into the guidelist for a given formula name,
// if the return value is < 0 then the guide value could not be found
sal_Int32 CustomShapeProperties::GetCustomShapeGuideValue( const std::vector< CustomShapeGuide >& rGuideList, const OUString& rFormulaName )
{
    // traverse the list from the end, because guide names can be reused
    // and current is the last one
    // see a1 guide in gear6 custom shape preset as example
    sal_Int32 nIndex = static_cast< sal_Int32 >( rGuideList.size() ) - 1;
    for( ; nIndex >= 0; nIndex-- )
    {
        if ( rGuideList[ nIndex ].maName == rFormulaName )
            break;
    }

    return nIndex;
}

CustomShapeProperties::PresetDataMap CustomShapeProperties::maPresetDataMap;

static OUString GetConnectorShapeType( sal_Int32 nType )
{
    SAL_INFO(
        "oox.drawingml", "preset: " << nType << " " << XML_straightConnector1);

    OUString sType;
    switch( nType )
    {
        case XML_straightConnector1:
            sType = "mso-spt32";
            break;
        default:
            break;
    }
    return sType;
}

void CustomShapeProperties::pushToPropSet( const ::oox::core::FilterBase& /* rFilterBase */,
    const Reference < XPropertySet >& xPropSet, const Reference < XShape > & xShape, const awt::Size &aSize )
{
    if ( mnShapePresetType >= 0 )
    {
        SAL_INFO("oox.drawingml", "preset: " << mnShapePresetType);

        if (maPresetDataMap.empty())
            initializePresetDataMap();

        PropertyMap aPropertyMap;
        PropertySet aPropSet( xPropSet );

        OUString sConnectorShapeType = GetConnectorShapeType( mnShapePresetType );

        if (sConnectorShapeType.getLength() > 0)
        {
            SAL_INFO(
                "oox.drawingml",
                "connector shape: " << sConnectorShapeType << " ("
                    << mnShapePresetType << ")");
            //const uno::Reference < drawing::XShape > xShape( xPropSet, UNO_QUERY );
            Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY );
            if( xDefaulter.is() ) {
                xDefaulter->createCustomShapeDefaults( sConnectorShapeType );
                aPropertyMap.setProperty( PROP_Type, sConnectorShapeType );
            }
        }
        else if (maPresetDataMap.find(mnShapePresetType) != maPresetDataMap.end())
        {
            SAL_INFO(
                "oox.drawingml",
                "found property map for preset: " << mnShapePresetType);

            aPropertyMap = maPresetDataMap[mnShapePresetType];
#ifdef DEBUG
            aPropertyMap.dumpCode();
#endif
        }

        aPropertyMap.setProperty( PROP_MirroredX, mbMirroredX );
        aPropertyMap.setProperty( PROP_MirroredY, mbMirroredY );
        aPropertyMap.setProperty( PROP_TextPreRotateAngle, mnTextRotateAngle );
        aPropertyMap.setProperty( PROP_IsPostRotateAngle, true); // For OpenXML Imports
        Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
        aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );

        const OUString sCustomShapeGeometry("CustomShapeGeometry");
        uno::Any aGeoPropSet = xPropSet->getPropertyValue( sCustomShapeGeometry );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;

        sal_Int32 i, nCount = 0;
        if (aGeoPropSet >>= aGeoPropSeq)
        {
            nCount = aGeoPropSeq.getLength();
            for ( i = 0; i < nCount; i++ )
            {
                const OUString sAdjustmentValues("AdjustmentValues");
                if ( aGeoPropSeq[ i ].Name.equals( sAdjustmentValues ) )
                {
                    OUString presetTextWarp;
                    if ( aGeoPropSeq[ i ].Value >>= presetTextWarp )
                    {
                        aPropertyMap.setProperty( PROP_PresetTextWarp, Any( presetTextWarp ) );
                    }
                }
            }
        }

        if ( maAdjustmentGuideList.size() )
        {
            const OUString sType = "Type";
            if ( aGeoPropSet >>= aGeoPropSeq )
            {
                nCount = aGeoPropSeq.getLength();
                for ( i = 0; i < nCount; i++ )
                {
                    const OUString sAdjustmentValues("AdjustmentValues");
                    if ( aGeoPropSeq[ i ].Name.equals( sAdjustmentValues ) )
                    {
                        uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                        if ( aGeoPropSeq[ i ].Value >>= aAdjustmentSeq )
                        {
                            int nIndex=0;
                            for (std::vector< CustomShapeGuide >::const_iterator aIter( maAdjustmentGuideList.begin() ), aEnd(maAdjustmentGuideList.end());
                             aIter != aEnd; ++aIter)
                            {
                                if ( (*aIter).maName.getLength() > 3 )
                                {
                                    sal_Int32 nAdjustmentIndex = (*aIter).maName.copy( 3 ).toInt32() - 1;
                                    if ( ( nAdjustmentIndex >= 0 ) && ( nAdjustmentIndex < aAdjustmentSeq.getLength() ) )
                                    {
                                        EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
                                        aAdjustmentVal.Value <<= (*aIter).maFormula.toInt32();
                                        aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
                                        aAdjustmentVal.Name = (*aIter).maName;
                                        aAdjustmentSeq[ nAdjustmentIndex ] = aAdjustmentVal;
                                    }
                                } else if ( aAdjustmentSeq.getLength() > 0 ) {
                                    EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
                                    aAdjustmentVal.Value <<= (*aIter).maFormula.toInt32();
                                    aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
                                    aAdjustmentVal.Name = (*aIter).maName;
                                    aAdjustmentSeq[ nIndex++ ] = aAdjustmentVal;
                                }
                            }
                            aGeoPropSeq[ i ].Value <<= aAdjustmentSeq;
                            xPropSet->setPropertyValue( sCustomShapeGeometry, Any( aGeoPropSeq ) );
                        }
                    }
                    else if ( aGeoPropSeq[ i ].Name.equals( sType ) )
                    {
                        if ( sConnectorShapeType.getLength() > 0 )
                            aGeoPropSeq[ i ].Value <<= sConnectorShapeType;
                        else
                            aGeoPropSeq[ i ].Value <<= OUString( "ooxml-CustomShape" );
                    }
                }
            }
        }
    }
    else
    {
        sal_uInt32 i;
        PropertyMap aPropertyMap;
        aPropertyMap.setProperty( PROP_Type, OUString( "ooxml-non-primitive" ));
        aPropertyMap.setProperty( PROP_MirroredX, mbMirroredX );
        aPropertyMap.setProperty( PROP_MirroredY, mbMirroredY );
        // Note 1: If Equations are defined - they are processed using internal div by 360 coordinates
        // while if they are not, standard ooxml coordinates are used.
        // This size specifically affects scaling.
        // Note 2: Width and Height are set to 0 to force scaling to 1.
        awt::Rectangle aViewBox( 0, 0, aSize.Width, aSize.Height );
        if( maGuideList.size() )
            aViewBox = awt::Rectangle( 0, 0, 0, 0 );
        aPropertyMap.setProperty( PROP_ViewBox, aViewBox);

        Sequence< EnhancedCustomShapeAdjustmentValue > aAdjustmentValues( maAdjustmentGuideList.size() );
        for ( i = 0; i < maAdjustmentGuideList.size(); i++ )
        {
            EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
            aAdjustmentVal.Value <<= maAdjustmentGuideList[ i ].maFormula.toInt32();
            aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
            aAdjustmentVal.Name = maAdjustmentGuideList[ i ].maName;
            aAdjustmentValues[ i ] = aAdjustmentVal;
        }
        aPropertyMap.setProperty( PROP_AdjustmentValues, aAdjustmentValues);

        PropertyMap aPath;

        aPath.setProperty( PROP_Segments, comphelper::containerToSequence(maSegments) );

        if ( maTextRect.has() ) {
            Sequence< EnhancedCustomShapeTextFrame > aTextFrames(1);
            aTextFrames[0].TopLeft.First = maTextRect.get().l;
            aTextFrames[0].TopLeft.Second = maTextRect.get().t;
            aTextFrames[0].BottomRight.First = maTextRect.get().r;
            aTextFrames[0].BottomRight.Second = maTextRect.get().b;
            aPath.setProperty( PROP_TextFrames, aTextFrames);
        }

        sal_uInt32 j, k, nParameterPairs = 0;
        for ( i = 0; i < maPath2DList.size(); i++ )
            nParameterPairs += maPath2DList[ i ].parameter.size();

        Sequence< EnhancedCustomShapeParameterPair > aParameterPairs( nParameterPairs );
        for ( i = 0, k = 0; i < maPath2DList.size(); i++ )
            for ( j = 0; j < maPath2DList[ i ].parameter.size(); j++ )
                aParameterPairs[ k++ ] = maPath2DList[ i ].parameter[ j ];
        aPath.setProperty( PROP_Coordinates, aParameterPairs);

        if ( maPath2DList.size() )
        {
            bool bAllZero = true;
            for ( i=0; i < maPath2DList.size(); i++ )
            {
                if ( maPath2DList[i].w || maPath2DList[i].h ) {
                    bAllZero = false;
                    break;
                }
            }

            if ( !bAllZero ) {
                Sequence< awt::Size > aSubViewSize( maPath2DList.size() );
                for ( i=0; i < maPath2DList.size(); i++ )
                {
                    aSubViewSize[i].Width = static_cast< sal_Int32 >( maPath2DList[i].w );
                    aSubViewSize[i].Height = static_cast< sal_Int32 >( maPath2DList[i].h );
                    SAL_INFO(
                        "oox.cscode",
                        "set subpath " << i << " size: " << maPath2DList[i].w
                            << " x " << maPath2DList[i].h);
                }
                aPath.setProperty( PROP_SubViewSize, aSubViewSize);
            }
        }

        Sequence< PropertyValue > aPathSequence = aPath.makePropertyValueSequence();
        aPropertyMap.setProperty( PROP_Path, aPathSequence);

        Sequence< OUString > aEquations( maGuideList.size() );
        for ( i = 0; i < maGuideList.size(); i++ )
            aEquations[ i ] = maGuideList[ i ].maFormula;
        aPropertyMap.setProperty( PROP_Equations, aEquations);

        Sequence< PropertyValues > aHandles( maAdjustHandleList.size() );
        for ( i = 0; i < maAdjustHandleList.size(); i++ )
        {
            PropertyMap aHandle;
            // maAdjustmentHandle[ i ].gdRef1 ... maAdjustmentHandle[ i ].gdRef2 ... :(
            // gdRef1 && gdRef2 -> we do not offer such reference, so it is difficult
            // to determine the correct adjustment handle that should be updated with the adjustment
            // position. here is the solution: the adjustment value that is used within the position
            // has to be updated, in case the position is a formula the first usage of a
            // adjustment value is decisive
            if ( maAdjustHandleList[ i ].polar )
            {
                aHandle.setProperty( PROP_Position, maAdjustHandleList[ i ].pos);
                if ( maAdjustHandleList[ i ].min1.has() )
                    aHandle.setProperty( PROP_RadiusRangeMinimum, maAdjustHandleList[ i ].min1.get());
                if ( maAdjustHandleList[ i ].max1.has() )
                    aHandle.setProperty( PROP_RadiusRangeMaximum, maAdjustHandleList[ i ].max1.get());

                /* TODO: AngleMin & AngleMax
                if ( maAdjustHandleList[ i ].min2.has() )
                    aHandle.setProperty( PROP_ ] = maAdjustHandleList[ i ].min2.get());
                if ( maAdjustHandleList[ i ].max2.has() )
                    aHandle.setProperty( PROP_ ] = maAdjustHandleList[ i ].max2.get());
                */
            }
            else
            {
                aHandle.setProperty( PROP_Position, maAdjustHandleList[ i ].pos);
                if ( maAdjustHandleList[ i ].gdRef1.has() )
                {
                    // TODO: PROP_RefX and PROP_RefY are not yet part of our file format,
                    // so the handles will not work after save/reload
                    sal_Int32 nIndex = GetCustomShapeGuideValue( maAdjustmentGuideList, maAdjustHandleList[ i ].gdRef1.get() );
                    if ( nIndex >= 0 )
                        aHandle.setProperty( PROP_RefX, nIndex);
                }
                if ( maAdjustHandleList[ i ].gdRef2.has() )
                {
                    sal_Int32 nIndex = GetCustomShapeGuideValue( maAdjustmentGuideList, maAdjustHandleList[ i ].gdRef2.get() );
                    if ( nIndex >= 0 )
                        aHandle.setProperty( PROP_RefY, nIndex);
                }
                if ( maAdjustHandleList[ i ].min1.has() )
                    aHandle.setProperty( PROP_RangeXMinimum, maAdjustHandleList[ i ].min1.get());
                if ( maAdjustHandleList[ i ].max1.has() )
                    aHandle.setProperty( PROP_RangeXMaximum, maAdjustHandleList[ i ].max1.get());
                if ( maAdjustHandleList[ i ].min2.has() )
                    aHandle.setProperty( PROP_RangeYMinimum, maAdjustHandleList[ i ].min2.get());
                if ( maAdjustHandleList[ i ].max2.has() )
                    aHandle.setProperty( PROP_RangeYMaximum, maAdjustHandleList[ i ].max2.get());
            }
            aHandles[ i ] = aHandle.makePropertyValueSequence();
        }
        aPropertyMap.setProperty( PROP_Handles, aHandles);

#ifdef DEBUG
        SAL_INFO("oox.cscode", "==cscode== begin");
        aPropertyMap.dumpCode();
        SAL_INFO("oox.cscode", "==cscode== end");
        SAL_INFO("oox.csdata", "==csdata== begin");
        aPropertyMap.dumpData();
        SAL_INFO("oox.csdata", "==csdata== end");
#endif
        // converting the vector to a sequence
        Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
        PropertySet aPropSet( xPropSet );
        aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
