/*************************************************************************
 *
 *  $RCSfile: AxisItemConverter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-04 12:37:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "AxisItemConverter.hxx"
#include "ItemPropertyMap.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "SchSfxItemIds.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "Scaling.hxx"

#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <drafts/com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XBOUNDEDCOORDINATESYSTEM_HPP_
#include <drafts/com/sun/star/chart2/XBoundedCoordinateSystem.hpp>
#endif

// #ifndef _COMPHELPER_PROCESSFACTORY_HXX_
// #include <comphelper/processfactory.hxx>
// #endif
// for SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// for SvxDoubleItem
#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_DOUBLE 0
#include <svx/chrtitem.hxx>
#endif
// for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetAxisPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aAxisPropertyMap(
        ::comphelper::MakeItemPropertyMap
        ( SCHATTR_AXIS_SHOWDESCR, C2U( "DisplayLabels" ))
        ( SCHATTR_AXIS_TICKS,     C2U( "MajorTickmarks" ))
        ( SCHATTR_AXIS_HELPTICKS, C2U( "MinorTickmarks" ))
        ( SCHATTR_TEXT_ORDER,     C2U( "ArrangeOrder" ))
        ( SCHATTR_TEXT_STACKED,   C2U( "StackCharacters" ))
        ( SCHATTR_TEXTBREAK,      C2U( "TextBreak" ))
        ( SCHATTR_TEXT_OVERLAP,   C2U( "TextOverlap" ))
        );

    return aAxisPropertyMap;
};
} // anonymous namespace

namespace chart
{
namespace wrapper
{

AxisItemConverter::AxisItemConverter(
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    NumberFormatterWrapper * pNumFormatter,
    chart2::ExplicitScaleData * pScale /* = NULL */,
    chart2::ExplicitIncrementData * pIncrement /* = NULL */,
    double * pExplicitOrigin /* = NULL */ ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_pNumberFormatterWrapper( pNumFormatter ),
        m_pExplicitScale( NULL ),
        m_pExplicitIncrement( NULL ),
        m_pExplicitOrigin( NULL )
{
    if( pScale )
        m_pExplicitScale = new chart2::ExplicitScaleData( *pScale );
    if( pIncrement )
        m_pExplicitIncrement = new chart2::ExplicitIncrementData( *pIncrement );
    if( pExplicitOrigin )
        m_pExplicitOrigin = new double( *pExplicitOrigin );

    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
                                 GraphicPropertyItemConverter::LINE_PROPERTIES ));
    m_aConverters.push_back( new CharacterPropertyItemConverter( rPropertySet, rItemPool ));

    uno::Reference< chart2::XAxis > xAxis( rPropertySet, uno::UNO_QUERY );
    OSL_ASSERT( xAxis.is());
    if( xAxis.is())
    {
        m_xAxis = xAxis;

        m_xCoordinateSystem.set( xAxis->getCoordinateSystem());
        if( m_xCoordinateSystem.is())
        {
            m_nDimension = xAxis->getRepresentedDimension();
        }
        else
        {
            OSL_ENSURE( false, "No coordinate system found at axis!" );
        }
    }
}

AxisItemConverter::~AxisItemConverter()
{
    delete m_pExplicitScale;
    delete m_pExplicitIncrement;
    delete m_pExplicitOrigin;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::DeleteItemConverterPtr() );
}

void AxisItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::FillItemSetFunc( rOutItemSet ));

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool AxisItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const USHORT * AxisItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nAxisWhichPairs;
}

bool AxisItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    ::comphelper::ItemPropertyMapType & rMap( lcl_GetAxisPropertyMap());
    ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

//     OSL_TRACE( "Which-Id: %d", nWhichId );

    if( aIt == rMap.end())
        return false;

    rOutName =(*aIt).second;
//     OSL_TRACE( "Which-Id: %d, Entry found in map: %s.", nWhichId,
//                ::rtl::OUStringToOString( rOutName, RTL_TEXTENCODING_ASCII_US ).getStr());

    return true;
}

void AxisItemConverter::FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    if( ! m_xAxis.is() ||
        ! m_xCoordinateSystem.is() )
        return;

    uno::Reference< chart2::XScale > xScale( m_xCoordinateSystem->getScaleByDimension( m_nDimension ));
    OSL_ASSERT( xScale.is());
    uno::Reference< chart2::XIncrement > xIncrement( m_xAxis->getIncrement() );
    OSL_ASSERT( xIncrement.is());
    if( ! xScale.is() ||
        ! xIncrement.is() )
        return;

    const chart2::ScaleData     aScale( xScale->getScaleData() );
    const chart2::IncrementData aInc( xIncrement->getIncrementData() );
    const uno::Sequence< chart2::SubIncrement > aSubIncs( xIncrement->getSubIncrements() );

    switch( nWhichId )
    {
        case SCHATTR_AXIS_AUTO_MAX:
            // if the any has no value => auto is on
            rOutItemSet.Put( SfxBoolItem( nWhichId, ( ! aScale.Maximum.hasValue())));
            break;

        case SCHATTR_AXIS_MAX:
            if( aScale.Maximum.hasValue())
            {
                OSL_ASSERT( aScale.Maximum.getValueTypeClass() == uno::TypeClass_DOUBLE );
                rOutItemSet.Put(
                    SvxDoubleItem(
                        *reinterpret_cast< const double * >( aScale.Maximum.getValue()), nWhichId ));
            }
            else
            {
                double fExplicitMax = 10.0;
                if( m_pExplicitScale )
                    fExplicitMax = m_pExplicitScale->Maximum;

                rOutItemSet.Put(
                    SvxDoubleItem( fExplicitMax, nWhichId ));
            }
            break;

        case SCHATTR_AXIS_AUTO_MIN:
            // if the any has no value => auto is on
            rOutItemSet.Put( SfxBoolItem( nWhichId, ( ! aScale.Minimum.hasValue())));
            break;

        case SCHATTR_AXIS_MIN:
            if( aScale.Minimum.hasValue())
            {
                OSL_ASSERT( aScale.Minimum.getValueTypeClass() == uno::TypeClass_DOUBLE );
                rOutItemSet.Put(
                    SvxDoubleItem(
                        *reinterpret_cast< const double * >( aScale.Minimum.getValue()), nWhichId ));
            }
            else
            {
                if( m_pExplicitScale )
                    rOutItemSet.Put( SvxDoubleItem( m_pExplicitScale->Minimum, nWhichId ));
            }
            break;

        case SCHATTR_AXIS_LOGARITHM:
        {
            uno::Reference< lang::XServiceName > xServiceName( aScale.Scaling, uno::UNO_QUERY );
            BOOL bValue =
                // if the following is true, we have logarithmic scaling,
                // otherwise not (per definition)
                ( xServiceName.is() &&
                  (xServiceName->getServiceName()).equals(
                      C2U( "com.sun.star.chart2.LogarithmicScaling" )) );

            rOutItemSet.Put( SfxBoolItem( nWhichId, bValue ));
        }
        break;

        // Increment
        case SCHATTR_AXIS_AUTO_STEP_MAIN:
            // if the any has no value => auto is on
            rOutItemSet.Put( SfxBoolItem( nWhichId, ( ! aInc.Distance.hasValue())));
            break;

        case SCHATTR_AXIS_STEP_MAIN:
            if( aInc.Distance.hasValue())
            {
                OSL_ASSERT( aInc.Distance.getValueTypeClass() == uno::TypeClass_DOUBLE );
                rOutItemSet.Put(
                    SvxDoubleItem(
                        *reinterpret_cast< const double * >( aInc.Distance.getValue()), nWhichId ));
            }
            else
            {
                if( m_pExplicitIncrement )
                    rOutItemSet.Put( SvxDoubleItem( m_pExplicitIncrement->Distance, nWhichId ));
            }
            break;

        // SubIncrement
        case SCHATTR_AXIS_AUTO_STEP_HELP:
        {
            // if the any has no value => auto is on
            rOutItemSet.Put(
                SfxBoolItem(
                    nWhichId,
                    ! ( aSubIncs.getLength() > 0 &&
                        aSubIncs[0].IntervalCount.hasValue() )));
        }
        break;

        case SCHATTR_AXIS_STEP_HELP:
        {
            if( aSubIncs.getLength() > 0 &&
                aSubIncs[0].IntervalCount.hasValue())
            {
                OSL_ASSERT( aSubIncs[0].IntervalCount.getValueTypeClass() == uno::TypeClass_LONG );
                rOutItemSet.Put(
                    SfxInt32Item(
                        nWhichId,
                        *reinterpret_cast< const sal_Int32 * >(
                            aSubIncs[0].IntervalCount.getValue()) ));
            }
            else
            {
                if( m_pExplicitIncrement &&
                    m_pExplicitIncrement->SubIncrements.getLength() > 0 )
                {
                    rOutItemSet.Put(
                        SfxInt32Item(
                            nWhichId,
                            m_pExplicitIncrement->SubIncrements[0].IntervalCount ));
                }
            }
        }
        break;

        case SCHATTR_AXIS_AUTO_ORIGIN:
        {
            uno::Sequence< uno::Any > aCoord( m_xCoordinateSystem->getOrigin() );
            OSL_ASSERT( m_nDimension <= aCoord.getLength());
            rOutItemSet.Put(
                SfxBoolItem(
                    nWhichId,
                    ! ( aCoord.getLength() > m_nDimension &&
                        aCoord[ m_nDimension ].hasValue() ) ));
        }
        break;

        case SCHATTR_AXIS_ORIGIN:
        {
            bool bIsAuto = true;
            uno::Sequence< uno::Any > aCoord( m_xCoordinateSystem->getOrigin() );
            if( aCoord.getLength() > m_nDimension &&
                aCoord[ m_nDimension ].hasValue() )
            {
                OSL_ASSERT( aCoord[ m_nDimension ].getValueTypeClass() == uno::TypeClass_DOUBLE );
                rOutItemSet.Put(
                    SvxDoubleItem(
                        *reinterpret_cast< const double * >(aCoord[ m_nDimension ].getValue()),
                        nWhichId ));
                bIsAuto = false;
            }

            if( bIsAuto && m_pExplicitOrigin )
                rOutItemSet.Put( SvxDoubleItem( *m_pExplicitOrigin, nWhichId ));
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            // convert double to int (times 100)
            double fVal;

            if( GetPropertySet()->getPropertyValue( C2U( "TextRotation" )) >>= fVal )
            {
                rOutItemSet.Put( SfxInt32Item( nWhichId, static_cast< sal_Int32 >(
                                                   ::rtl::math::round( fVal * 100.0 ) ) ));
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
//         case SCHATTR_AXIS_NUMFMT:
        {
            if( m_pNumberFormatterWrapper )
            {
                chart2::NumberFormat aNumFmt;
                if( GetPropertySet()->getPropertyValue( C2U( "NumberFormat" )) >>= aNumFmt )
                {
                    sal_Int32 nKey = m_pNumberFormatterWrapper->getKeyForNumberFormat( aNumFmt );

                    OSL_TRACE( "NumberFormat: ""%s"", Locale: %s_%s@%s, Key: %d",
                               U2C( aNumFmt.aFormat ),
                               U2C( aNumFmt.aLocale.Language ),
                               U2C( aNumFmt.aLocale.Country ),
                               U2C( aNumFmt.aLocale.Variant ),
                               nKey );
                    rOutItemSet.Put( SfxUInt32Item( nWhichId, nKey ));
                }
            }
            else
            {
                OSL_ENSURE( false, "No NumberFormatterWrapper !" );
            }
        }
        break;

//         case SID_ATTR_NUMBERFORMAT_SOURCE:
    }
}

bool AxisItemConverter::ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    if( ! ( m_xAxis.is() &&
            m_xCoordinateSystem.is()))
        return false;

    uno::Reference< chart2::XScale > xScale( m_xCoordinateSystem->getScaleByDimension( m_nDimension ));
    OSL_ASSERT( xScale.is());
    uno::Reference< chart2::XIncrement > xIncrement( m_xAxis->getIncrement() );
    OSL_ASSERT( xIncrement.is());
    if( ! xScale.is() ||
        ! xIncrement.is() )
        return false;

    chart2::ScaleData     aScale( xScale->getScaleData() );
    chart2::IncrementData aInc( xIncrement->getIncrementData() );
    uno::Sequence< chart2::SubIncrement > aSubIncs( xIncrement->getSubIncrements() );
    uno::Sequence< uno::Any > aOrigin( m_xCoordinateSystem->getOrigin());

    bool bSetScale         = false;
    bool bSetIncrement     = false;
    bool bSetSubIncrement  = false;
    bool bSetOrigin   = false;
    bool bChangedOtherwise = false;

    uno::Any aValue;

    switch( nWhichId )
    {
        case SCHATTR_AXIS_AUTO_MAX:
            if( (reinterpret_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                aScale.Maximum.clear();
                bSetScale = true;
            }
            // else SCHATTR_AXIS_MAX must have some value
            break;

        case SCHATTR_AXIS_MAX:
            // only if auto if false
            if( ! (reinterpret_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_MAX )).GetValue() ))
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( aScale.Maximum != aValue )
                {
                    aScale.Maximum = aValue;
                    bSetScale = true;
                }
            }
            break;

        case SCHATTR_AXIS_AUTO_MIN:
            if( (reinterpret_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                aScale.Minimum.clear();
                bSetScale = true;
            }
            // else SCHATTR_AXIS_MIN must have some value
            break;

        case SCHATTR_AXIS_MIN:
            // only if auto if false
            if( ! (reinterpret_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_MIN )).GetValue() ))
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( aScale.Minimum != aValue )
                {
                    aScale.Minimum = aValue;
                    bSetScale = true;
                }
            }
            break;

        case SCHATTR_AXIS_LOGARITHM:
        {
            uno::Reference< lang::XServiceName > xServiceName( aScale.Scaling, uno::UNO_QUERY );
            bool bWasLogarithm =
                // if the following is true, we have logarithmic scaling,
                // otherwise not (per definition)
                ( xServiceName.is() &&
                  (xServiceName->getServiceName()).equals(
                      C2U( "com.sun.star.chart2.LogarithmicScaling" )));

            if( (reinterpret_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                // logarithm is true
                if( ! bWasLogarithm )
                {
                    aScale.Scaling = new LogarithmicScaling( 10.0 );
                    bSetScale = true;
                }
            }
            else
            {
                // logarithm is false => linear scaling
                if( bWasLogarithm )
                {
                    aScale.Scaling = new LinearScaling( 1.0, 0.0 );
                    bSetScale = true;
                }
            }
        }
        break;

        // Increment
        case SCHATTR_AXIS_AUTO_STEP_MAIN:
            if( (reinterpret_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                aInc.Distance.clear();
                bSetIncrement = true;
            }
            // else SCHATTR_AXIS_STEP_MAIN must have some value
            break;

        case SCHATTR_AXIS_STEP_MAIN:
            // only if auto if false
            if( ! (reinterpret_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_STEP_MAIN )).GetValue() ))
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( aInc.Distance != aValue )
                {
                    aInc.Distance = aValue;
                    bSetIncrement = true;
                }
            }
            break;

        // SubIncrement
        case SCHATTR_AXIS_AUTO_STEP_HELP:
            if( (reinterpret_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ) &&
                aSubIncs.getLength() > 0 &&
                aSubIncs[0].IntervalCount.hasValue() )
            {
                    aSubIncs[0].IntervalCount.clear();
                    bSetSubIncrement = true;
            }
            // else SCHATTR_AXIS_STEP_MAIN must have some value
            break;

        case SCHATTR_AXIS_STEP_HELP:
            // only if auto if false
            if( ! (reinterpret_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_STEP_HELP )).GetValue() ) &&
                aSubIncs.getLength() > 0 )
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( ! aSubIncs[0].IntervalCount.hasValue() ||
                    aSubIncs[0].IntervalCount != aValue )
                {
                    OSL_ASSERT( aValue.getValueTypeClass() == uno::TypeClass_LONG );
                    aSubIncs[0].IntervalCount = aValue;
                    bSetSubIncrement = true;
                }
            }
            break;

        case SCHATTR_AXIS_AUTO_ORIGIN:
        {
            if( (reinterpret_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ) &&
                ( aOrigin.getLength() > m_nDimension &&
                  aOrigin[ m_nDimension ].hasValue() ))
            {
                aOrigin[ m_nDimension ].clear();
                bSetOrigin = true;
            }
        }
        break;

        case SCHATTR_AXIS_ORIGIN:
        {
            if( ! (reinterpret_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_ORIGIN )).GetValue() ))
            {
                if( aOrigin.getLength() > m_nDimension )
                {
                    rItemSet.Get( nWhichId ).QueryValue( aValue );
                    if( aValue != aOrigin[ m_nDimension ] )
                    {
                        aOrigin[ m_nDimension ] = aValue;
                        bSetOrigin = true;
                    }
                }
            }
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            // convert int to double (divided by 100)
            double fVal = static_cast< double >(
                reinterpret_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue()) / 100.0;
            double fOldVal = 0.0;
            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( C2U( "TextRotation" )) >>= fOldVal );

            if( ! bPropExisted ||
                ( bPropExisted && fOldVal != fVal ))
            {
                GetPropertySet()->setPropertyValue( C2U( "TextRotation" ), uno::makeAny( fVal ));
                bChangedOtherwise = true;
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
//         case SCHATTR_AXIS_NUMFMT:
        {
            if( m_pNumberFormatterWrapper )
            {
                sal_Int32 nFmt = static_cast< sal_Int32 >(
                    reinterpret_cast< const SfxUInt32Item & >(
                        rItemSet.Get( nWhichId )).GetValue());

                aValue = uno::makeAny(
                    m_pNumberFormatterWrapper->getNumberFormatForKey( nFmt ));
                if( GetPropertySet()->getPropertyValue( C2U( "NumberFormat" )) != aValue )
                {
                    GetPropertySet()->setPropertyValue( C2U( "NumberFormat" ), aValue );
                    bChangedOtherwise = true;
                }
            }
            else
            {
                OSL_ENSURE( false, "No NumberFormatterWrapper !" );
            }
        }
        break;

//         case SID_ATTR_NUMBERFORMAT_SOURCE:
    }

    if( bSetScale &&
        xScale.is() )
        xScale->setScaleData( aScale );
    if( bSetIncrement &&
        xIncrement.is() )
        xIncrement->setIncrementData( aInc );
    if( bSetSubIncrement &&
        xIncrement.is() )
        xIncrement->setSubIncrements( aSubIncs );
    if( bSetOrigin )
        m_xCoordinateSystem->setOrigin( aOrigin );

    return (bSetScale || bSetIncrement || bSetSubIncrement ||
            bSetOrigin || bChangedOtherwise);
}

} //  namespace wrapper
} //  namespace chart
