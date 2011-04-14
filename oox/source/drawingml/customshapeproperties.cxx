/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::drawing;

# define USS(x) OUStringToOString( x, RTL_TEXTENCODING_UTF8 ).getStr()

namespace oox { namespace drawingml {

CustomShapeProperties::CustomShapeProperties()
: mbMirroredX   ( sal_False )
, mbMirroredY   ( sal_False )
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

void CustomShapeProperties::pushToPropSet( const ::oox::core::FilterBase& /* rFilterBase */,
    const Reference < XPropertySet >& xPropSet, const Reference < XShape > & xShape ) const
{
    if ( maShapePresetType.getLength() )
    {
        static OUString sRightArrow = CREATE_OUSTRING("right-arrow");
        if ( maShapePresetType.equals( sRightArrow ) ) {

            PropertyMap aPropertyMap;

    Sequence< EnhancedCustomShapeAdjustmentValue > aAdjSequence (2);
    {
        Any aAny ((sal_Int32) 18000);
        aAdjSequence [0].Value = aAny;
    }
    {
        Any aAny ((sal_Int32) 10000);
        aAdjSequence [1].Value = aAny;
    }
    aPropertyMap [PROP_AdjustmentValues] <<= aAdjSequence;

    Sequence< OUString > aStringSequence (15);
    aStringSequence[0] = CREATE_OUSTRING ("min(width,height)");
    aStringSequence[1] = CREATE_OUSTRING ("20000*width/?0 ");
    aStringSequence[2] = CREATE_OUSTRING ("if(0-$0 ,0,if(20000-$0 ,$0 ,20000))");
    aStringSequence[3] = CREATE_OUSTRING ("if(0-$1 ,0,if(?1 -$1 ,$1 ,?1 ))");
    aStringSequence[4] = CREATE_OUSTRING ("?0 *?3 /20000");
    aStringSequence[5] = CREATE_OUSTRING ("width+0-?4 ");
    aStringSequence[6] = CREATE_OUSTRING ("height*?2 /40000");
    aStringSequence[7] = CREATE_OUSTRING ("height/2");
    aStringSequence[8] = CREATE_OUSTRING ("?7 +0-?6 ");
    aStringSequence[9] = CREATE_OUSTRING ("?7 +?6 -0");
    aStringSequence[10] = CREATE_OUSTRING ("height/2");
    aStringSequence[11] = CREATE_OUSTRING ("?8 *?4 /?10 ");
    aStringSequence[12] = CREATE_OUSTRING ("?5 +?11 -0");
    aStringSequence[13] = CREATE_OUSTRING ("height");
    aStringSequence[14] = CREATE_OUSTRING ("width");
    aPropertyMap [PROP_Equations] <<= aStringSequence;

    Sequence< Sequence < PropertyValue > > aPropSequenceSequence (2);
    {
        Sequence< PropertyValue > aPropSequence (4);
        {
            aPropSequence [0].Name = CREATE_OUSTRING ("Position");
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 8);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.Second = aParameter;
            }
            aPropSequence [0].Value = makeAny (aParameterPair);
        }
        {
            aPropSequence [1].Name = CREATE_OUSTRING ("RangeYMaximum");
            EnhancedCustomShapeParameter aParameter;
            Any aAny ((sal_Int32) 20000);
            aParameter.Value = aAny;
            aParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
            aPropSequence [1].Value = makeAny (aParameter);
        }
        {
            aPropSequence [2].Name = CREATE_OUSTRING ("RangeYMinimum");
            EnhancedCustomShapeParameter aParameter;
            Any aAny ((sal_Int32) 0);
            aParameter.Value = aAny;
            aParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
            aPropSequence [2].Value = makeAny (aParameter);
        }
        {
            aPropSequence [3].Name = CREATE_OUSTRING ("RefY");
            Any aAny ((sal_Int32) 0);
            aPropSequence [3].Value = makeAny (aAny);
        }
        aPropSequenceSequence [0] = aPropSequence;
    }
    {
        Sequence< PropertyValue > aPropSequence (4);
        {
            aPropSequence [0].Name = CREATE_OUSTRING ("Position");
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 5);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                aParameterPair.Second = aParameter;
            }
            aPropSequence [0].Value = makeAny (aParameterPair);
        }
        {
            aPropSequence [1].Name = CREATE_OUSTRING ("RangeXMaximum");
            EnhancedCustomShapeParameter aParameter;
            Any aAny ((sal_Int32) 1);
            aParameter.Value = aAny;
            aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
            aPropSequence [1].Value = makeAny (aParameter);
        }
        {
            aPropSequence [2].Name = CREATE_OUSTRING ("RangeXMinimum");
            EnhancedCustomShapeParameter aParameter;
            Any aAny ((sal_Int32) 0);
            aParameter.Value = aAny;
            aParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
            aPropSequence [2].Value = makeAny (aParameter);
        }
        {
            aPropSequence [3].Name = CREATE_OUSTRING ("RefX");
            Any aAny ((sal_Int32) 1);
            aPropSequence [3].Value = makeAny (aAny);
        }
        aPropSequenceSequence [1] = aPropSequence;
    }
    aPropertyMap [PROP_Handles] <<= aPropSequenceSequence;

    aPropertyMap [PROP_MirroredX] <<= Any ((sal_Bool) sal_False);

    aPropertyMap [PROP_MirroredY] <<= Any ((sal_Bool) sal_False);

    Sequence< PropertyValue > aPropSequence (2);
    {
        aPropSequence [0].Name = CREATE_OUSTRING ("Coordinates");
        Sequence< EnhancedCustomShapeParameterPair > aParameterPairSeq (7);
        {
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 8);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.Second = aParameter;
            }
            aParameterPairSeq [0] = aParameterPair;
        }
        {
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 5);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 8);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.Second = aParameter;
            }
            aParameterPairSeq [1] = aParameterPair;
        }
        {
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 5);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                aParameterPair.Second = aParameter;
            }
            aParameterPairSeq [2] = aParameterPair;
        }
        {
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 14);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 7);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.Second = aParameter;
            }
            aParameterPairSeq [3] = aParameterPair;
        }
        {
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 5);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 13);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.Second = aParameter;
            }
            aParameterPairSeq [4] = aParameterPair;
        }
        {
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 5);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 9);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.Second = aParameter;
            }
            aParameterPairSeq [5] = aParameterPair;
        }
        {
            EnhancedCustomShapeParameterPair aParameterPair;
            {
                EnhancedCustomShapeParameter aParameter;
                aParameterPair.First = aParameter;
            }
            {
                EnhancedCustomShapeParameter aParameter;
                Any aAny ((sal_Int32) 9);
                aParameter.Value = aAny;
                aParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
                aParameterPair.Second = aParameter;
            }
            aParameterPairSeq [6] = aParameterPair;
        }
        aPropSequence [0].Value = makeAny (aParameterPairSeq);
    }
    {
        aPropSequence [1].Name = CREATE_OUSTRING ("Segments");
        Sequence< EnhancedCustomShapeSegment > aSegmentSeq (4);
        {
            EnhancedCustomShapeSegment aSegment;
            aSegment.Command = 1;
            aSegment.Count = 1;
            aSegmentSeq [0] = aSegment;
        }
        {
            EnhancedCustomShapeSegment aSegment;
            aSegment.Command = 2;
            aSegment.Count = 6;
            aSegmentSeq [1] = aSegment;
        }
        {
            EnhancedCustomShapeSegment aSegment;
            aSegment.Command = 4;
            aSegment.Count = 0;
            aSegmentSeq [2] = aSegment;
        }
        {
            EnhancedCustomShapeSegment aSegment;
            aSegment.Command = 5;
            aSegment.Count = 0;
            aSegmentSeq [3] = aSegment;
        }
        aPropSequence [1].Value = makeAny (aSegmentSeq);
    }
    aPropertyMap [PROP_Path] <<= aPropSequence;

    awt::Rectangle aRectangle;
    aRectangle.X = 0;
    aRectangle.Y = 0;
    aRectangle.Width = 1050480;
    aRectangle.Height = 456840;
    aPropertyMap [PROP_ViewBox] <<= aRectangle;


            aPropertyMap [ PROP_Type ] <<= CREATE_OUSTRING("ooxml-right-arrow");
            //aPropertyMap[ PROP_Type ] <<= CREATE_OUSTRING( "non-primitive" );

            Sequence< PropertyValue > aSeq = aPropertyMap.makePropertyValueSequence();
            PropertySet aPropSet( xPropSet );
            aPropSet.setProperty( PROP_CustomShapeGeometry, aSeq );
            OSL_TRACE("created ooxml preset");

            aPropertyMap.dump();
            aPropertyMap.dumpCode();
        } else {
            //const uno::Reference < drawing::XShape > xShape( xPropSet, UNO_QUERY );
            Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY );
            if( xDefaulter.is() )
                xDefaulter->createCustomShapeDefaults( maShapePresetType );
        }

        if ( maAdjustmentGuideList.size() )
        {
            const OUString sType = CREATE_OUSTRING( "Type" );
            const OUString sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeGeometry" ) );
            uno::Any aGeoPropSet = xPropSet->getPropertyValue( sCustomShapeGeometry );
            uno::Sequence< beans::PropertyValue > aGeoPropSeq;
            if ( aGeoPropSet >>= aGeoPropSeq )
            {
                sal_Int32 i, nCount = aGeoPropSeq.getLength();
                for ( i = 0; i < nCount; i++ )
                {
                    const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM( "AdjustmentValues" ) );
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
                                        aAdjustmentVal.Value <<= (*aIter).maFormula.toInt32();
                                        aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
                                        aAdjustmentSeq[ nAdjustmentIndex ] = aAdjustmentVal;
                                    }
                                } else if ( aAdjustmentSeq.getLength() > 0 ) {
                                    EnhancedCustomShapeAdjustmentValue aAdjustmentVal;
                                    aAdjustmentVal.Value <<= (*aIter).maFormula.toInt32();
                                    aAdjustmentVal.State = PropertyState_DIRECT_VALUE;
                                    aAdjustmentSeq[ 0 ] = aAdjustmentVal;
                                }
                                aIter++;
                            }
                            aGeoPropSeq[ i ].Value <<= aAdjustmentSeq;
                            xPropSet->setPropertyValue( sCustomShapeGeometry, Any( aGeoPropSeq ) );
                        }
                    }
                    else if ( aGeoPropSeq[ i ].Name.equals( sType ) && !maShapePresetType.equals ( CREATE_OUSTRING ( "right-arrow" ) ) )
                    {
                        aGeoPropSeq[ i ].Value <<= maShapePresetType;
                    }
                }
            }
        }
    }
    else
    {
        sal_uInt32 i;
        PropertyMap aPropertyMap;
        aPropertyMap[ PROP_Type ] <<= CREATE_OUSTRING( "non-primitive" );
        aPropertyMap[ PROP_MirroredX ] <<= Any( mbMirroredX );
        aPropertyMap[ PROP_MirroredY ] <<= Any( mbMirroredY );
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
        // aPropertyMap.dump();
        // aPropertyMap.dumpCode();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
