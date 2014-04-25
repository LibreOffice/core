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



#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/helper/helper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <basegfx/numeric/ftools.hxx>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::drawing;

namespace oox { namespace drawingml {

CustomShapeProperties::CustomShapeProperties()
: mbMirroredX   ( sal_False )
, mbMirroredY   ( sal_False )
, mnTextRotation(0) // #119920# Add missing extra text rotation
{
}
CustomShapeProperties::~CustomShapeProperties()
{
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
sal_Int32 CustomShapeProperties::GetCustomShapeGuideValue( const std::vector< CustomShapeGuide >& rGuideList, const rtl::OUString& rFormulaName )
{
    sal_Int32 nIndex = 0;
    for( ; nIndex < static_cast< sal_Int32 >( rGuideList.size() ); nIndex++ )
    {
        if ( rGuideList[ nIndex ].maName == rFormulaName )
            break;
    }
    if ( nIndex == static_cast< sal_Int32 >( rGuideList.size() ) )
        nIndex = -1;
    return nIndex;
}

void CustomShapeProperties::apply( const CustomShapePropertiesPtr& /* rSourceCustomShapeProperties */ )
{
    // not sure if this needs to be implemented
}

bool setOrCreatePropertyValue(
    uno::Sequence< beans::PropertyValue >& rPropSeq,
    const OUString& rName,
    const uno::Any& rAny)
{
    const sal_Int32 nCount(rPropSeq.getLength());

    for(sal_Int32 a(0); a < nCount; a++)
    {
        beans::PropertyValue& rEntry = rPropSeq[a];

        if(rEntry.Name.equals(rName))
        {
            rEntry.Value = rAny;
            return false;
        }
    }

    beans::PropertyValue aNewValue;

    aNewValue.Name = rName;
    aNewValue.Value = rAny;

    rPropSeq.realloc(nCount + 1);
    rPropSeq[nCount] = aNewValue;

    return true;
}

void CustomShapeProperties::pushToPropSet( const ::oox::core::FilterBase& /* rFilterBase */,
    const Reference < XPropertySet >& xPropSet, const Reference < XShape > & xShape ) const
{
    if ( maShapePresetType.getLength() )
    {
        //const uno::Reference < drawing::XShape > xShape( xPropSet, UNO_QUERY );
        Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY );
        const OUString sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeGeometry" ) );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;
        uno::Any aGeoPropSet;
        bool bValuesAdded(false);

        if( xDefaulter.is() )
        {
            xDefaulter->createCustomShapeDefaults( maShapePresetType );
        }

        if(mbMirroredX || mbMirroredY || mnTextRotation)
        {
            // #121371# set these values, but do *not* set a completely new
            // "CustomShapeGeometry", this would reset the evtl. already created
            // "Type" entry
            aGeoPropSet = xPropSet->getPropertyValue(sCustomShapeGeometry);

            if(aGeoPropSet >>= aGeoPropSeq)
            {
                uno::Any aAny;

                if(mbMirroredX) // TTTT: remove again after aw080, make it part of object transformation
                {
                    const rtl::OUString sMirroredX(RTL_CONSTASCII_USTRINGPARAM("MirroredX"));
                    aAny <<= mbMirroredX;
                    bValuesAdded = setOrCreatePropertyValue(aGeoPropSeq, sMirroredX, aAny);
                }

                if(mbMirroredY) // TTTT: remove again after aw080, make it part of object transformation
                {
                    const rtl::OUString sMirroredY(RTL_CONSTASCII_USTRINGPARAM("MirroredY"));
                    aAny <<= mbMirroredY;
                    bValuesAdded = setOrCreatePropertyValue(aGeoPropSeq, sMirroredY, aAny);
                }

                if(mnTextRotation)
                {
                    const rtl::OUString sTextRotateAngle(RTL_CONSTASCII_USTRINGPARAM("TextRotateAngle"));
                    aAny <<= (double)mnTextRotation;
                    bValuesAdded = setOrCreatePropertyValue(aGeoPropSeq, sTextRotateAngle, aAny);
                }
            }
        }

        if ( maAdjustmentGuideList.size() )
        {
            if(!aGeoPropSeq.getLength())
            {
                aGeoPropSet = xPropSet->getPropertyValue( sCustomShapeGeometry );
                aGeoPropSet >>= aGeoPropSeq;
            }

            sal_Int32 i, nCount = aGeoPropSeq.getLength();

            for ( i = 0; i < nCount; i++ )
            {
                const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM( "AdjustmentValues" ) );
                const OUString sType = CREATE_OUSTRING( "Type" );

                if ( aGeoPropSeq[ i ].Name.equals( sAdjustmentValues ) )
                {
                    uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                    if ( aGeoPropSeq[ i ].Value >>= aAdjustmentSeq )
                    {
                        std::vector< CustomShapeGuide >::const_iterator aIter( maAdjustmentGuideList.begin() );
                        while( aIter != maAdjustmentGuideList.end() )
                        {
                            if ( (*aIter).maName.getLength() > 3 )
                            {
                                sal_Int32 nAdjustmentIndex = (*aIter).maName.copy( 3 ).toInt32() - 1;
                                if ( ( nAdjustmentIndex >= 0 ) && ( nAdjustmentIndex < aAdjustmentSeq.getLength() ) )
                                {
                                    EnhancedCustomShapeAdjustmentValue aAdjustmentVal;

                                    sal_Int32 nValue((*aIter).maFormula.toInt32());

                                    // #124703# The ms control point coordinates are relative to the
                                    // object center in the range [-50000 .. 50000] while our customshapes
                                    // use a range from [0 .. 21600], so adapt the value as needed
                                    nValue = basegfx::fround((double(nValue) + 50000.0) * (21600.0 / 100000.0));

                                    aAdjustmentVal.Value <<= nValue;
                                    aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
                                    aAdjustmentSeq[ nAdjustmentIndex ] = aAdjustmentVal;
                                }
                            }
                            aIter++;
                        }
                        aGeoPropSeq[ i ].Value <<= aAdjustmentSeq;
                        xPropSet->setPropertyValue( sCustomShapeGeometry, Any( aGeoPropSeq ) );
                    }
                }
                else if ( aGeoPropSeq[ i ].Name.equals( sType ) )
                {
                    aGeoPropSeq[ i ].Value <<= maShapePresetType;
                }
            }
        }

        if(bValuesAdded)
        {
            aGeoPropSet <<= aGeoPropSeq;
            xPropSet->setPropertyValue(sCustomShapeGeometry, aGeoPropSet);
        }
    }
    else
    {
        sal_uInt32 i;
        PropertyMap aPropertyMap;
        aPropertyMap[ PROP_Type ] <<= CREATE_OUSTRING( "non-primitive" );
        aPropertyMap[ PROP_MirroredX ] <<= Any( mbMirroredX );
        aPropertyMap[ PROP_MirroredY ] <<= Any( mbMirroredY );

        if(mnTextRotation)
        {
            aPropertyMap[ PROP_TextRotation ] <<= Any(mnTextRotation);
        }

        awt::Size aSize( xShape->getSize() );
        awt::Rectangle aViewBox( 0, 0, aSize.Width * 360, aSize.Height * 360 );
        if ( maPath2DList.size() )
        {   // TODO: each polygon may have its own size, but I think it is rather been used
            // so we are only taking care of the first
            if ( maPath2DList[ 0 ].w )
                aViewBox.Width = static_cast< sal_Int32 >( maPath2DList[ 0 ].w );
            if ( maPath2DList[ 0 ].h )
                aViewBox.Height = static_cast< sal_Int32 >( maPath2DList[ 0 ].h );
        }
        aPropertyMap[ PROP_ViewBox ] <<= aViewBox;

        Sequence< EnhancedCustomShapeAdjustmentValue > aAdjustmentValues( maAdjustmentGuideList.size() );
        for ( i = 0; i < maAdjustmentGuideList.size(); i++ )
        {
            EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
            aAdjustmentVal.Value <<= maAdjustmentGuideList[ i ].maFormula.toInt32();
            aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
            aAdjustmentValues[ i ] = aAdjustmentVal;
        }
        aPropertyMap[ PROP_AdjustmentValues ] <<= aAdjustmentValues;

        Sequence< rtl::OUString > aEquations( maGuideList.size() );
        for ( i = 0; i < maGuideList.size(); i++ )
            aEquations[ i ] = maGuideList[ i ].maFormula;
        aPropertyMap[ PROP_Equations ] <<= aEquations;

        PropertyMap aPath;
        Sequence< EnhancedCustomShapeSegment > aSegments( maSegments.size() );
        for ( i = 0; i < maSegments.size(); i++ )
            aSegments[ i ] = maSegments[ i ];
        aPath[ PROP_Segments ] <<= aSegments;
        sal_uInt32 j, k, nParameterPairs = 0;
        for ( i = 0; i < maPath2DList.size(); i++ )
            nParameterPairs += maPath2DList[ i ].parameter.size();
        Sequence< EnhancedCustomShapeParameterPair > aParameterPairs( nParameterPairs );
        for ( i = 0, k = 0; i < maPath2DList.size(); i++ )
            for ( j = 0; j < maPath2DList[ i ].parameter.size(); j++ )
                aParameterPairs[ k++ ] = maPath2DList[ i ].parameter[ j ];
        aPath[ PROP_Coordinates ] <<= aParameterPairs;
        Sequence< PropertyValue > aPathSequence = aPath.makePropertyValueSequence();
        aPropertyMap[ PROP_Path ] <<= aPathSequence;

        Sequence< PropertyValues > aHandles( maAdjustHandleList.size() );
        for ( i = 0; i < maAdjustHandleList.size(); i++ )
        {
            PropertyMap aHandle;
            // maAdjustmentHandle[ i ].gdRef1 ... maAdjustmentHandle[ i ].gdRef2 ... :(
            // gdRef1 && gdRef2 -> we do not offer such reference, so it is difficult
            // to determine the correct adjustment handle that should be updated with the adjustment
            // position. here is the solution: the adjustment value that is used within the position
            // has to be updated, in case the position is a formula the first usage of a
            // adjument value is decisive
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

        // converting the vector to a sequence
        Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
        PropertySet aPropSet( xPropSet );
        aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );
    }
}

double CustomShapeProperties::getValue( const std::vector< CustomShapeGuide >& rGuideList, sal_uInt32 nIndex ) const
{
    double fRet = 0.0;
    if ( nIndex < rGuideList.size() )
    {

    }
    return fRet;
}

} }
