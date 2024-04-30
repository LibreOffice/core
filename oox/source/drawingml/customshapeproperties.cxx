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

#include <drawingml/customshapeproperties.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokenmap.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;

namespace oox::drawingml {

void CustomShapeGuideContainer::ActualizeLookupMap() const
{
    if ( mbLookupMapStale )
    {
        // maGuideListLookupMap maps guide name to index in maGuideList
        // guides were added since last actualization, need to update map based on those
        // guide names can be reused, and current is the latest one
        // (see a1 guide in gear6 custom shape preset as example):
        //  go backwards and update if index is higher than previously
        for ( sal_Int32 nIndex = static_cast<sal_Int32>( maGuideList.size() ) - 1; nIndex >= mnPreviousActSize; --nIndex )
        {
            const auto it = maGuideListLookupMap.find( maGuideList[ nIndex ].maName );
            if ( it != maGuideListLookupMap.end() )
            {
                if ( nIndex > it->second )
                    it->second = nIndex;
            }
            else
                maGuideListLookupMap[ maGuideList[ nIndex ].maName ] = nIndex;
        }
        mbLookupMapStale = false;
        mnPreviousActSize = static_cast<sal_Int32>( maGuideList.size() );
    }
}

void CustomShapeGuideContainer::push_back( const CustomShapeGuide& rGuide )
{
    if ( !mbLookupMapStale )
    {
        mbLookupMapStale = true;
        mnPreviousActSize = static_cast<sal_Int32>( maGuideList.size() );
    }
    maGuideList.push_back( rGuide );
}

// returns the index into the guidelist for a given formula name,
// if the return value is < 0 then the guide value could not be found
sal_Int32 CustomShapeGuideContainer::GetCustomShapeGuideValue( const OUString &rFormulaName ) const
{
    ActualizeLookupMap();
    const auto it = maGuideListLookupMap.find( rFormulaName );
    if ( it != maGuideListLookupMap.end() )
        return it->second;

    return -1;
}

sal_Int32 CustomShapeGuideContainer::SetCustomShapeGuideValue( const CustomShapeGuide& rGuide )
{
    ActualizeLookupMap();
    // change from previous SetCustomShapeGuideValue behavior: searching using cache traverses backwards
    const auto it = maGuideListLookupMap.find( rGuide.maName );
    if ( it != maGuideListLookupMap.end() )
        return it->second;

    maGuideList.push_back( rGuide );
    maGuideListLookupMap[ rGuide.maName ] = mnPreviousActSize;
    mnPreviousActSize++;
    return mnPreviousActSize - 1;
}

CustomShapeProperties::CustomShapeProperties()
: mnShapePresetType ( -1 )
, mbShapeTypeOverride(false)
, mbMirroredX   ( false )
, mbMirroredY   ( false )
, mnTextPreRotateAngle ( 0 )
, mnTextCameraZRotateAngle ( 0 )
, mnArcNum ( 0 )
{
}

OUString CustomShapeProperties::getShapePresetTypeName() const
{
    return StaticTokenMap().getUnicodeTokenName(mnShapePresetType);
}

bool CustomShapeProperties::representsDefaultShape() const
{
    return !((getShapePresetType() >= 0 || maPath2DList.size() > 0) &&
             getShapePresetType() != XML_Rect &&
             getShapePresetType() != XML_rect);
}

CustomShapeProperties::PresetDataMap CustomShapeProperties::maPresetDataMap;

void CustomShapeProperties::pushToPropSet(
    const Reference < XPropertySet >& xPropSet, const awt::Size &aSize )
{
    if ( mnShapePresetType >= 0 )
    {
        SAL_INFO("oox.drawingml", "preset: " << mnShapePresetType);

        if (maPresetDataMap.empty())
            initializePresetDataMap();

        PropertyMap aPropertyMap;
        PropertySet aPropSet( xPropSet );

        if (maPresetDataMap.contains(mnShapePresetType))
        {
            SAL_INFO(
                "oox.drawingml",
                "found property map for preset: " << mnShapePresetType);

            aPropertyMap = maPresetDataMap[mnShapePresetType];
#if OSL_DEBUG_LEVEL >= 2
            aPropertyMap.dumpCode( aPropertyMap.makePropertySet() );
#endif
        }

        aPropertyMap.setProperty( PROP_MirroredX, mbMirroredX );
        aPropertyMap.setProperty( PROP_MirroredY, mbMirroredY );
        aPropertyMap.setProperty( PROP_TextPreRotateAngle, mnTextPreRotateAngle );
        aPropertyMap.setProperty( PROP_TextCameraZRotateAngle, mnTextCameraZRotateAngle );
        if (moTextAreaRotateAngle.has_value())
            aPropertyMap.setProperty(PROP_TextRotateAngle, moTextAreaRotateAngle.value());
        if (!maExtrusionPropertyMap.empty())
        {
            Sequence< PropertyValue > aExtrusionSequence = maExtrusionPropertyMap.makePropertyValueSequence();
            aPropertyMap.setAnyProperty( PROP_Extrusion, css::uno::Any(aExtrusionSequence));
        }
        Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
        aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );

        if ( !maAdjustmentGuideList.empty() )
        {
            static constexpr OUString sCustomShapeGeometry(u"CustomShapeGeometry"_ustr);
            static constexpr OUStringLiteral sAdjustmentValues(u"AdjustmentValues");
            uno::Any aGeoPropSet = xPropSet->getPropertyValue( sCustomShapeGeometry );
            uno::Sequence< beans::PropertyValue > aGeoPropSeq;
            if ( aGeoPropSet >>= aGeoPropSeq )
            {
                for ( auto& rGeoProp : asNonConstRange(aGeoPropSeq) )
                {
                    if ( rGeoProp.Name == sAdjustmentValues )
                    {
                        uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                        if ( rGeoProp.Value >>= aAdjustmentSeq )
                        {
                            auto aAdjustmentSeqRange = asNonConstRange(aAdjustmentSeq);
                            int nIndex=0;
                            for (auto const& adjustmentGuide : maAdjustmentGuideList)
                            {
                                if ( adjustmentGuide.maName.getLength() > 3 )
                                {
                                    sal_Int32 nAdjustmentIndex = o3tl::toInt32(adjustmentGuide.maName.subView( 3 )) - 1;
                                    if ( ( nAdjustmentIndex >= 0 ) && ( nAdjustmentIndex < aAdjustmentSeq.getLength() ) )
                                    {
                                        EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
                                        aAdjustmentVal.Value <<= adjustmentGuide.maFormula.toInt32();
                                        aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
                                        aAdjustmentVal.Name = adjustmentGuide.maName;
                                        aAdjustmentSeqRange[ nAdjustmentIndex ] = aAdjustmentVal;
                                    }
                                } else if ( aAdjustmentSeq.hasElements() ) {
                                    EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
                                    aAdjustmentVal.Value <<= adjustmentGuide.maFormula.toInt32();
                                    aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
                                    aAdjustmentVal.Name = adjustmentGuide.maName;
                                    if (nIndex < aAdjustmentSeq.getLength())
                                    {
                                        aAdjustmentSeqRange[nIndex] = aAdjustmentVal;
                                        ++nIndex;
                                    }
                                }
                            }
                            rGeoProp.Value <<= aAdjustmentSeq;
                            xPropSet->setPropertyValue( sCustomShapeGeometry, Any( aGeoPropSeq ) );
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        PropertyMap aPropertyMap;
        aPropertyMap.setProperty( PROP_Type, OUString( "ooxml-non-primitive" ));
        aPropertyMap.setProperty( PROP_MirroredX, mbMirroredX );
        aPropertyMap.setProperty( PROP_MirroredY, mbMirroredY );
        if( mnTextPreRotateAngle )
            aPropertyMap.setProperty( PROP_TextPreRotateAngle, mnTextPreRotateAngle );
        if (moTextAreaRotateAngle.has_value())
            aPropertyMap.setProperty(PROP_TextRotateAngle, moTextAreaRotateAngle.value());
        // Note 1: If Equations are defined - they are processed using internal div by 360 coordinates
        // while if they are not, standard ooxml coordinates are used.
        // This size specifically affects scaling.
        // Note 2: Width and Height are set to 0 to force scaling to 1.
        awt::Rectangle aViewBox( 0, 0, aSize.Width, aSize.Height );
        // tdf#113187 Each ArcTo introduces two additional equations for converting start and swing
        // angles. They do not count for test for use of standard ooxml coordinates
        if (maGuideList.size() - 2 * countArcTo() > 0)
            aViewBox = awt::Rectangle( 0, 0, 0, 0 );
        aPropertyMap.setProperty( PROP_ViewBox, aViewBox);

        Sequence< EnhancedCustomShapeAdjustmentValue > aAdjustmentValues( maAdjustmentGuideList.size() );
        auto aAdjustmentValuesRange = asNonConstRange(aAdjustmentValues);
        for ( std::vector<CustomShapeGuide>::size_type i = 0; i < maAdjustmentGuideList.size(); i++ )
        {
            EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
            aAdjustmentVal.Value <<= maAdjustmentGuideList[ i ].maFormula.toInt32();
            aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
            aAdjustmentVal.Name = maAdjustmentGuideList[ i ].maName;
            aAdjustmentValuesRange[ i ] = aAdjustmentVal;
        }
        aPropertyMap.setProperty( PROP_AdjustmentValues, aAdjustmentValues);

        PropertyMap aPath;

        aPath.setProperty( PROP_Segments, comphelper::containerToSequence(maSegments) );

        if ( maTextRect.has_value() ) {
            Sequence< EnhancedCustomShapeTextFrame > aTextFrames{
                { /* tl */ { maTextRect.value().l, maTextRect.value().t },
                  /* br */ { maTextRect.value().r, maTextRect.value().b } }
            };
            aPath.setProperty( PROP_TextFrames, aTextFrames);
        }

        if (!maConnectionSiteList.empty())
        {
            css::uno::Sequence<EnhancedCustomShapeParameterPair> seqGluePoints;
            seqGluePoints.realloc(maConnectionSiteList.size());
            sal_Int32 nId = 0;
            for (auto& rGluePoint : asNonConstRange(seqGluePoints))
            {
                rGluePoint.First.Value = maConnectionSiteList[nId].pos.First.Value;
                rGluePoint.First.Type = maConnectionSiteList[nId].pos.First.Type;
                rGluePoint.Second.Value = maConnectionSiteList[nId].pos.Second.Value;
                rGluePoint.Second.Type = maConnectionSiteList[nId].pos.Second.Type;
                nId++;
            }
            aPath.setProperty(PROP_GluePoints, seqGluePoints);
        }

        sal_uInt32 nParameterPairs = 0;
        for ( auto const & i: maPath2DList )
            nParameterPairs += i.parameter.size();

        Sequence< EnhancedCustomShapeParameterPair > aParameterPairs( nParameterPairs );
        auto aParameterPairsRange = asNonConstRange(aParameterPairs);
        sal_uInt32 k = 0;
        for ( auto const & i: maPath2DList )
            for ( auto const & j: i.parameter )
                aParameterPairsRange[ k++ ] = j;
        aPath.setProperty( PROP_Coordinates, aParameterPairs);

        if ( !maPath2DList.empty() )
        {
            bool bAllZero = true;
            for ( auto const & i: maPath2DList )
            {
                if ( i.w || i.h ) {
                    bAllZero = false;
                    break;
                }
            }

            if ( !bAllZero ) {
                Sequence< awt::Size > aSubViewSize( maPath2DList.size() );
                std::transform(maPath2DList.begin(), maPath2DList.end(), aSubViewSize.getArray(),
                               [](const auto& p2d)
                               {
                                   SAL_INFO("oox.cscode",
                                            "set subpath; size: " << p2d.w << " x " << p2d.h);
                                   return awt::Size(p2d.w, p2d.h);
                               });
                aPath.setProperty( PROP_SubViewSize, aSubViewSize);
            }
        }

        Sequence< PropertyValue > aPathSequence = aPath.makePropertyValueSequence();
        aPropertyMap.setProperty( PROP_Path, aPathSequence);

        Sequence< OUString > aEquations( maGuideList.size() );
        std::transform(maGuideList.begin(), maGuideList.end(), aEquations.getArray(),
                       [](const auto& g) { return g.maFormula; });
        aPropertyMap.setProperty( PROP_Equations, aEquations);

        Sequence< PropertyValues > aHandles( maAdjustHandleList.size() );
        auto aHandlesRange = asNonConstRange(aHandles);
        for ( std::vector<AdjustHandle>::size_type i = 0; i < maAdjustHandleList.size(); i++ )
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
                // Polar handles in DrawingML
                // 1. don't have reference center, so PROP_Polar isn't needed.
                // 2. position always use planar coordinates.
                // 3. use RefAngle and RefR to specify adjustment value to be updated.
                // 4. The unit of angular adjustment values are 6000th degree.

                aHandle.setProperty( PROP_Position, maAdjustHandleList[ i ].pos);
                if ( maAdjustHandleList[ i ].gdRef1.has_value() )
                {
                    sal_Int32 nIndex = maAdjustmentGuideList.GetCustomShapeGuideValue( maAdjustHandleList[ i ].gdRef1.value() );
                    if ( nIndex >= 0 )
                        aHandle.setProperty( PROP_RefR, nIndex);
                }
                if ( maAdjustHandleList[ i ].gdRef2.has_value() )
                {
                    sal_Int32 nIndex = maAdjustmentGuideList.GetCustomShapeGuideValue( maAdjustHandleList[ i ].gdRef2.value() );
                    if ( nIndex >= 0 )
                        aHandle.setProperty( PROP_RefAngle, nIndex);
                }
                if ( maAdjustHandleList[ i ].min1.has_value() )
                    aHandle.setProperty( PROP_RadiusRangeMinimum, maAdjustHandleList[ i ].min1.value());
                if ( maAdjustHandleList[ i ].max1.has_value() )
                    aHandle.setProperty( PROP_RadiusRangeMaximum, maAdjustHandleList[ i ].max1.value());

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
                if ( maAdjustHandleList[ i ].gdRef1.has_value() )
                {
                    // TODO: PROP_RefX and PROP_RefY are not yet part of our file format,
                    // so the handles will not work after save/reload
                    sal_Int32 nIndex = maAdjustmentGuideList.GetCustomShapeGuideValue( maAdjustHandleList[ i ].gdRef1.value() );
                    if ( nIndex >= 0 )
                        aHandle.setProperty( PROP_RefX, nIndex);
                }
                if ( maAdjustHandleList[ i ].gdRef2.has_value() )
                {
                    sal_Int32 nIndex = maAdjustmentGuideList.GetCustomShapeGuideValue( maAdjustHandleList[ i ].gdRef2.value() );
                    if ( nIndex >= 0 )
                        aHandle.setProperty( PROP_RefY, nIndex);
                }
                if ( maAdjustHandleList[ i ].min1.has_value() )
                    aHandle.setProperty( PROP_RangeXMinimum, maAdjustHandleList[ i ].min1.value());
                if ( maAdjustHandleList[ i ].max1.has_value() )
                    aHandle.setProperty( PROP_RangeXMaximum, maAdjustHandleList[ i ].max1.value());
                if ( maAdjustHandleList[ i ].min2.has_value() )
                    aHandle.setProperty( PROP_RangeYMinimum, maAdjustHandleList[ i ].min2.value());
                if ( maAdjustHandleList[ i ].max2.has_value() )
                    aHandle.setProperty( PROP_RangeYMaximum, maAdjustHandleList[ i ].max2.value());
            }
            aHandlesRange[ i ] = aHandle.makePropertyValueSequence();
        }
        aPropertyMap.setProperty( PROP_Handles, aHandles);
        if (!maExtrusionPropertyMap.empty())
        {
            Sequence< PropertyValue > aExtrusionSequence = maExtrusionPropertyMap.makePropertyValueSequence();
            aPropertyMap.setProperty( PROP_Extrusion, aExtrusionSequence);
        }

#if OSL_DEBUG_LEVEL >= 2
        // Note that the script oox/source/drawingml/customshapes/generatePresetsData.pl looks
        // for these ==cscode== and ==csdata== markers, so don't "clean up" these SAL_INFOs.
        SAL_INFO("oox.cscode", "==cscode== begin");
        aPropertyMap.dumpCode( aPropertyMap.makePropertySet() );
        SAL_INFO("oox.cscode", "==cscode== end");
        SAL_INFO("oox.csdata", "==csdata== begin");
        aPropertyMap.dumpData( aPropertyMap.makePropertySet() );
        SAL_INFO("oox.csdata", "==csdata== end");
#endif
        // converting the vector to a sequence
        Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
        PropertySet aPropSet( xPropSet );
        aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
