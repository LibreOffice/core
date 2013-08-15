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

#include "oox/drawingml/customshapeproperties.hxx"
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

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::drawing;

# define USS(x) OUStringToOString( x, RTL_TEXTENCODING_UTF8 ).getStr()

namespace oox { namespace drawingml {

CustomShapeProperties::CustomShapeProperties()
: mnShapePresetType ( -1 )
, mbMirroredX   ( sal_False )
, mbMirroredY   ( sal_False )
, mnTextRotateAngle ( 0 )
, mnArcNum ( 0 )
{
}
CustomShapeProperties::~CustomShapeProperties()
{
}

OUString CustomShapeProperties::getShapePresetTypeName() const
{
    return StaticTokenMap::get().getUnicodeTokenName( mnShapePresetType );
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

CustomShapeProperties::PresetsMap CustomShapeProperties::maPresetsMap;

static OUString GetConnectorShapeType( sal_Int32 nType )
{
    OSL_TRACE("GetConnectorShapeType preset: %d %d", nType, XML_straightConnector1);

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
    const Reference < XPropertySet >& xPropSet, const Reference < XShape > & xShape )
{
    if ( mnShapePresetType >= 0 )
    {
        OSL_TRACE("preset: %d", mnShapePresetType);

        if (maPresetsMap.empty())
            initializePresetsMap();

        PropertyMap aPropertyMap;
        PropertySet aPropSet( xPropSet );

        OUString sConnectorShapeType = GetConnectorShapeType( mnShapePresetType );

        if (sConnectorShapeType.getLength() > 0)
        {
            OSL_TRACE("connector shape: %s (%d)", USS(sConnectorShapeType), mnShapePresetType);
            //const uno::Reference < drawing::XShape > xShape( xPropSet, UNO_QUERY );
            Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY );
            if( xDefaulter.is() ) {
                xDefaulter->createCustomShapeDefaults( sConnectorShapeType );
                aPropertyMap[ PROP_Type ] <<= Any( sConnectorShapeType );
            }
        }
        else if (maPresetsMap.find(mnShapePresetType) != maPresetsMap.end())
        {
            OSL_TRACE("found property map for preset: %s (%d)", USS(getShapePresetTypeName()), mnShapePresetType);

            CustomShapeProvider *pProvider = maPresetsMap[ mnShapePresetType ];
            if (pProvider)
                aPropertyMap = pProvider->getProperties();
#ifdef DEBUG
            aPropertyMap.dumpCode();
#endif
        }

        aPropertyMap[ PROP_MirroredX ] <<= Any( mbMirroredX );
        aPropertyMap[ PROP_MirroredY ] <<= Any( mbMirroredY );
        aPropertyMap[ PROP_TextPreRotateAngle ] <<= Any( mnTextRotateAngle );
        aPropertyMap[ PROP_IsPostRotateAngle ] <<= true; // For OpenXML Imports
        Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
        aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );

        if ( maAdjustmentGuideList.size() )
        {
            const OUString sType = "Type";
            const OUString sCustomShapeGeometry("CustomShapeGeometry");
            uno::Any aGeoPropSet = xPropSet->getPropertyValue( sCustomShapeGeometry );
            uno::Sequence< beans::PropertyValue > aGeoPropSeq;
            if ( aGeoPropSet >>= aGeoPropSeq )
            {
                sal_Int32 i, nCount = aGeoPropSeq.getLength();
                for ( i = 0; i < nCount; i++ )
                {
                    const OUString sAdjustmentValues("AdjustmentValues");
                    if ( aGeoPropSeq[ i ].Name.equals( sAdjustmentValues ) )
                    {
                        uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                        if ( aGeoPropSeq[ i ].Value >>= aAdjustmentSeq )
                        {
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
                                    aAdjustmentSeq[ 0 ] = aAdjustmentVal;
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
        aPropertyMap[ PROP_Type ] <<= OUString( "ooxml-non-primitive" );
        aPropertyMap[ PROP_MirroredX ] <<= Any( mbMirroredX );
        aPropertyMap[ PROP_MirroredY ] <<= Any( mbMirroredY );
        awt::Size aSize;
        awt::Rectangle aViewBox( 0, 0, aSize.Width * 360, aSize.Height * 360 );
        aPropertyMap[ PROP_ViewBox ] <<= aViewBox;

        Sequence< EnhancedCustomShapeAdjustmentValue > aAdjustmentValues( maAdjustmentGuideList.size() );
        for ( i = 0; i < maAdjustmentGuideList.size(); i++ )
        {
            EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
            aAdjustmentVal.Value <<= maAdjustmentGuideList[ i ].maFormula.toInt32();
            aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
            aAdjustmentVal.Name = maAdjustmentGuideList[ i ].maName;
            aAdjustmentValues[ i ] = aAdjustmentVal;
        }
        aPropertyMap[ PROP_AdjustmentValues ] <<= aAdjustmentValues;

        PropertyMap aPath;

        Sequence< EnhancedCustomShapeSegment > aSegments( maSegments.size() );
        for ( i = 0; i < maSegments.size(); i++ )
            aSegments[ i ] = maSegments[ i ];
        aPath[ PROP_Segments ] <<= aSegments;

        if ( maTextRect.has() ) {
            Sequence< EnhancedCustomShapeTextFrame > aTextFrames(1);
            aTextFrames[0].TopLeft.First = maTextRect.get().l;
            aTextFrames[0].TopLeft.Second = maTextRect.get().t;
            aTextFrames[0].BottomRight.First = maTextRect.get().r;
            aTextFrames[0].BottomRight.Second = maTextRect.get().b;
            aPath[ PROP_TextFrames ] <<= aTextFrames;
        }

        sal_uInt32 j, k, nParameterPairs = 0;
        for ( i = 0; i < maPath2DList.size(); i++ )
            nParameterPairs += maPath2DList[ i ].parameter.size();

        Sequence< EnhancedCustomShapeParameterPair > aParameterPairs( nParameterPairs );
        for ( i = 0, k = 0; i < maPath2DList.size(); i++ )
            for ( j = 0; j < maPath2DList[ i ].parameter.size(); j++ )
                aParameterPairs[ k++ ] = maPath2DList[ i ].parameter[ j ];
        aPath[ PROP_Coordinates ] <<= aParameterPairs;

        if ( maPath2DList.size() )
        {
            sal_Bool bAllZero = sal_True;
            for ( i=0; i < maPath2DList.size(); i++ )
            {
                if ( maPath2DList[i].w || maPath2DList[i].h ) {
                    bAllZero = sal_False;
                    break;
                }
            }

            if ( !bAllZero ) {
                Sequence< awt::Size > aSubViewSize( maPath2DList.size() );
                for ( i=0; i < maPath2DList.size(); i++ )
                {
                    aSubViewSize[i].Width = static_cast< sal_Int32 >( maPath2DList[i].w );
                    aSubViewSize[i].Height = static_cast< sal_Int32 >( maPath2DList[i].h );
                    OSL_TRACE("set subpath %d size: %d x %d", i, maPath2DList[i].w, maPath2DList[i].h);
                }
                aPath[ PROP_SubViewSize ] <<= aSubViewSize;
            }
        }

        Sequence< PropertyValue > aPathSequence = aPath.makePropertyValueSequence();
        aPropertyMap[ PROP_Path ] <<= aPathSequence;

        Sequence< OUString > aEquations( maGuideList.size() );
        for ( i = 0; i < maGuideList.size(); i++ )
            aEquations[ i ] = maGuideList[ i ].maFormula;
        aPropertyMap[ PROP_Equations ] <<= aEquations;

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
                aHandle[ PROP_Position ] <<= maAdjustHandleList[ i ].pos;
                if ( maAdjustHandleList[ i ].min1.has() )
                    aHandle[ PROP_RadiusRangeMinimum ] <<= maAdjustHandleList[ i ].min1.get();
                if ( maAdjustHandleList[ i ].max1.has() )
                    aHandle[ PROP_RadiusRangeMaximum ] <<= maAdjustHandleList[ i ].max1.get();

                /* TODO: AngleMin & AngleMax
                if ( maAdjustHandleList[ i ].min2.has() )
                    aHandle[ PROP_ ] = maAdjustHandleList[ i ].min2.get();
                if ( maAdjustHandleList[ i ].max2.has() )
                    aHandle[ PROP_ ] = maAdjustHandleList[ i ].max2.get();
                */
            }
            else
            {
                aHandle[ PROP_Position ] <<= maAdjustHandleList[ i ].pos;
                if ( maAdjustHandleList[ i ].gdRef1.has() )
                {
                    // TODO: PROP_RefX and PROP_RefY are not yet part of our file format,
                    // so the handles will not work after save/reload
                    sal_Int32 nIndex = GetCustomShapeGuideValue( maAdjustmentGuideList, maAdjustHandleList[ i ].gdRef1.get() );
                    if ( nIndex >= 0 )
                        aHandle[ PROP_RefX ] <<= nIndex;
                }
                if ( maAdjustHandleList[ i ].gdRef2.has() )
                {
                    sal_Int32 nIndex = GetCustomShapeGuideValue( maAdjustmentGuideList, maAdjustHandleList[ i ].gdRef2.get() );
                    if ( nIndex >= 0 )
                        aHandle[ PROP_RefY ] <<= nIndex;
                }
                if ( maAdjustHandleList[ i ].min1.has() )
                    aHandle[ PROP_RangeXMinimum ] <<= maAdjustHandleList[ i ].min1.get();
                if ( maAdjustHandleList[ i ].max1.has() )
                    aHandle[ PROP_RangeXMaximum ] <<= maAdjustHandleList[ i ].max1.get();
                if ( maAdjustHandleList[ i ].min2.has() )
                    aHandle[ PROP_RangeYMinimum ] <<= maAdjustHandleList[ i ].min2.get();
                if ( maAdjustHandleList[ i ].max2.has() )
                    aHandle[ PROP_RangeYMaximum ] <<= maAdjustHandleList[ i ].max2.get();
            }
            aHandles[ i ] = aHandle.makePropertyValueSequence();
        }
        aPropertyMap[ PROP_Handles ] <<= aHandles;

#ifdef DEBUG
        SAL_INFO("oox.cscode", "==cscode== begin");
        aPropertyMap.dumpCode();
        SAL_INFO("oox.cscode", "==cscode== end");
#endif
        // converting the vector to a sequence
        Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
        PropertySet aPropSet( xPropSet );
        aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );
    }
}

Any CustomShapeProvider::createStringSequence( size_t nStrings, const char **pStrings )
{
    Sequence< OUString > aStringSequence( nStrings );
    for (size_t i = 0; i < nStrings; i++)
        aStringSequence[i] = OUString::intern(
                                pStrings[i], strlen( pStrings[i] ),
                                RTL_TEXTENCODING_ASCII_US );
    return makeAny( aStringSequence );
}

com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment >
CustomShapeProvider::createSegmentSequence( size_t nElems, const sal_uInt16 *pValues )
{
    Sequence< EnhancedCustomShapeSegment > aSequence( (nElems + 1) / 2 );
    for (size_t i = 0, j = 0; i < nElems / 2; i++)
    {
        aSequence[i].Command = pValues[j++];
        aSequence[i].Count = pValues[j++];
    }
    return aSequence;
}

com::sun::star::drawing::EnhancedCustomShapeParameterPair
CustomShapeProvider::createParameterPair( const ParameterPairData *pData )
{
    EnhancedCustomShapeParameterPair aParameterPair;
    aParameterPair.First.Type = pData->nFirstType;
    aParameterPair.First.Value = makeAny(pData->nFirstValue);
    aParameterPair.Second.Type = pData->nSecondType;
    aParameterPair.Second.Value = makeAny(pData->nSecondValue);
    return aParameterPair;
}

com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair >
CustomShapeProvider::createParameterPairSequence( size_t nElems, const ParameterPairData *pData )
{
    Sequence< EnhancedCustomShapeParameterPair > aSequence( nElems );
    for (size_t i = 0; i < nElems; i++)
        aSequence[i] = createParameterPair( pData + i );
    return aSequence;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
