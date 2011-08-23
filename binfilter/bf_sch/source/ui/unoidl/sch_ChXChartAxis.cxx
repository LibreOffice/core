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

#ifndef _CHXCHART_AXIS_HXX
#include "ChXChartAxis.hxx"
#endif
#include "schattr.hxx"

// header for class OGuard
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

// header for SvxDoubleItem
#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_DOUBLE 0
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT  SCHATTR_TEXT_ORIENT

#ifndef _SFXENUMITEM_HXX
#include <bf_svtools/eitem.hxx>
#endif

#endif
#include "charttyp.hxx"
// header for SvxUnoTextRangeBase
// for OWN_ATTR_...
#ifndef _SVX_UNOSHPRP_HXX
#include <bf_svx/unoshprp.hxx>
#endif
// for SID_ATTR_...
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef _SVX_UNOSHAPE_HXX 
#include <bf_svx/unoshape.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART_CHARTAXISARRANGEORDERTYPE_HPP_
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#endif

#include "chaxis.hxx"
#include "mapprov.hxx"
#include "pairs.hxx"

#ifndef _SCH_UNONAMES_HXX
#include "unonames.hxx"
#endif

namespace binfilter {

using namespace vos;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

    
ChXChartAxis::ChXChartAxis( ChartModel* _Model, long _WhichId ) :
        ChXChartObject( CHMAP_AXIS, _Model, _WhichId )
{
}

ChXChartAxis::~ChXChartAxis()
{}


::rtl::OUString SAL_CALL ChXChartAxis::getImplementationName()
throw( uno::RuntimeException )
{
    return SCH_ASCII_TO_OU( "ChXChartAxis" );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartAxis::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    uno::Sequence< ::rtl::OUString > aSeq( 4 );
    aSeq[ 0 ] = ::rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxis" );
    aSeq[ 1 ] = ::rtl::OUString::createFromAscii( "com.sun.star.drawing.LineProperties" );
    aSeq[ 2 ] = ::rtl::OUString::createFromAscii( "com.sun.star.style.CharacterProperties" );
    aSeq[ 3 ] = ::rtl::OUString::createFromAscii( "com.sun.star.xml.UserDefinedAttributeSupplier" );

    return aSeq;
}

// generate a unique Id
const uno::Sequence< sal_Int8 > & ChXChartAxis::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

void ChXChartAxis::InitNumberFormatter() throw( uno::RuntimeException )
{
    if( ! rNumberFormatter.is())
    {
        if( mpModel )
            rNumberFormatter = new SvNumberFormatsSupplierObj( mpModel->GetNumFormatter() );
        else
            rNumberFormatter = new SvNumberFormatsSupplierObj();
    }

    if( ! rNumberFormatter.is())
        throw uno::RuntimeException();
}

// XNumberFormatsSupplier
uno::Reference< beans::XPropertySet > SAL_CALL ChXChartAxis::getNumberFormatSettings() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    InitNumberFormatter();

    return rNumberFormatter.get()->getNumberFormatSettings();
}

uno::Reference< util::XNumberFormats > SAL_CALL ChXChartAxis::getNumberFormats() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    InitNumberFormatter();

    return rNumberFormatter.get()->getNumberFormats();
}

// XInterface
uno::Any SAL_CALL ChXChartAxis::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aAny = ChXChartObject::queryInterface( rType );

    if( ! aAny.hasValue() )
    {
        InitNumberFormatter();
        aAny = rNumberFormatter.get()->queryInterface( rType );
    }

    return aAny;
}

void SAL_CALL ChXChartAxis::acquire() throw()
{
    ChXChartObject::acquire();
}

void SAL_CALL ChXChartAxis::release() throw()
{
    ChXChartObject::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL ChXChartAxis::getTypes() throw( uno::RuntimeException )
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( ChXChartObject::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        maTypeSequence.realloc( nBaseTypes + 1 );		// Note: Keep the size updated !!
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< util::XNumberFormatsSupplier >*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL ChXChartAxis::getImplementationId() throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XShapeDescriptor
::rtl::OUString SAL_CALL ChXChartAxis::getShapeType()
    throw( uno::RuntimeException )
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxis" );
}

// XUnoTunnel
sal_Int64 SAL_CALL ChXChartAxis::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( uno::RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                                 aIdentifier.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// XPropertySet
uno::Any SAL_CALL ChXChartAxis::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex());

    if( mpModel )
    {
        uno::Any aAny;
        if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( UNONAME_TEXT_ORDER )))
        {
            SfxItemSet aSet( mpModel->GetItemPool(), SCHATTR_TEXT_ORDER, SCHATTR_TEXT_ORDER );
            mpModel->GetAttr( mnWhichId, aSet );

            chart::ChartAxisArrangeOrderType eArrOrder;
            SvxChartTextOrder eTextOrder =
                ((const SvxChartTextOrderItem&)(aSet.Get( SCHATTR_TEXT_ORDER ))).GetValue();

            switch( eTextOrder )
            {
                case CHTXTORDER_SIDEBYSIDE:
                    eArrOrder = chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE; break;
                case CHTXTORDER_DOWNUP:
                    eArrOrder = chart::ChartAxisArrangeOrderType_STAGGER_EVEN; break;
                case CHTXTORDER_UPDOWN:
                    eArrOrder = chart::ChartAxisArrangeOrderType_STAGGER_ODD; break;
                case CHTXTORDER_AUTO:
                default:
                    eArrOrder = chart::ChartAxisArrangeOrderType_AUTO; break;
            }
            aAny <<= eArrOrder;
            return aAny;
        }
        else if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( UNONAME_NUMBERFORMAT )))
        {
            // if we have a percent chart use the correct number format item
            ChartType aType( mpModel );
            if( aType.IsPercent())
            {
                SfxItemSet aSet( mpModel->GetItemPool(), SCHATTR_AXIS_NUMFMTPERCENT, SCHATTR_AXIS_NUMFMTPERCENT );
                mpModel->GetAttr( mnWhichId, aSet );

                aAny <<= (sal_Int32)(((const SfxUInt32Item&)(aSet.Get( SCHATTR_AXIS_NUMFMTPERCENT ))).GetValue());
                return aAny;
            }
        }
    }

    return ChXChartObject::getPropertyValue( PropertyName );
}

void SAL_CALL ChXChartAxis::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex());

    if( mpModel && mnWhichId != CHOBJID_ANY )
    {
        const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( aPropertyName );

        if( pMap && pMap->nWID )
        {
            if( pMap->nFlags & beans::PropertyAttribute::READONLY )
                throw beans::PropertyVetoException();

            USHORT nWID = pMap->nWID;

            // create item set that contains nWID and maybe more
            USHORT nAutoWid = 0;
            SfxItemSet* pSet = NULL;
            switch( nWID )
            {
                // min/max/stepmain/stephelp/origin need to disable the auto-settings
                case SCHATTR_AXIS_MIN:
                    nAutoWid = SCHATTR_AXIS_AUTO_MIN;					
                    break;
                case SCHATTR_AXIS_MAX:
                    nAutoWid = SCHATTR_AXIS_AUTO_MAX;
                    break;
                case SCHATTR_AXIS_STEP_MAIN:
                    nAutoWid = SCHATTR_AXIS_AUTO_STEP_MAIN;
                    break;
                case SCHATTR_AXIS_STEP_HELP:
                    nAutoWid = SCHATTR_AXIS_AUTO_STEP_HELP;
                    break;
                case SCHATTR_AXIS_ORIGIN:
                    nAutoWid = SCHATTR_AXIS_AUTO_ORIGIN;
                    break;

                case SCHATTR_AXIS_LOGARITHM:
                    pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID,
                                           SCHATTR_AXIS_AUTO_STEP_MAIN, SCHATTR_AXIS_AUTO_STEP_MAIN,
                                           SCHATTR_AXIS_MIN, SCHATTR_AXIS_MIN,
                                           SCHATTR_AXIS_MAX, SCHATTR_AXIS_MAX, 0 );
                    break;
                case SCHATTR_AXIS_AUTO_STEP_MAIN:
                    pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID,
                                           SCHATTR_AXIS_STEP_MAIN, SCHATTR_AXIS_STEP_MAIN, 0 );
                    break;
                case SCHATTR_AXIS_AUTO_STEP_HELP:
                    pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID,
                                           SCHATTR_AXIS_STEP_HELP, SCHATTR_AXIS_STEP_HELP, 0 );
                    break;

                case XATTR_LINEEND:
                case XATTR_LINESTART:
                case XATTR_LINEDASH:
                    // there is a special handling done in the base class for this
                    ChXChartObject::setPropertyValue( aPropertyName, aValue );
                    return;                           // RETURN !

                case SCHATTR_AXIS_NUMFMT:
                    {
                        ChartType aType( mpModel );
                        if( aType.IsPercent())
                        {
                            nWID = SCHATTR_AXIS_NUMFMTPERCENT;
                        }
                    }
                    // intentionally not breaking ! (nWID just changed)
                default:
                    pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID );
            }
            if( nAutoWid )
                pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID,
                                              nAutoWid, nAutoWid, 0 );
            DBG_ASSERT( pSet, "ItemSet was not created!" );

            mpModel->GetAttr( mnWhichId, *pSet );

            if( pSet->GetItemState( nWID ) != SFX_ITEM_SET )
            {
                // get default from ItemPool
                if( mpModel->GetItemPool().IsWhich( nWID ))
                {
                    pSet->Put( mpModel->GetItemPool().GetDefaultItem( nWID ));
                }
                else if( nWID != SID_ATTR_NUMBERFORMAT_SOURCE )
                {
#ifdef DBG_UTIL
                    String aTmpString( aPropertyName );
                    ByteString aProp( aTmpString, RTL_TEXTENCODING_ASCII_US );
                    DBG_ERROR2( "ChartAxis: Property %s has an invalid ID (%d)", aProp.GetBuffer(), nWID );
#endif
                }
            }

            // special treatment for some properties
            switch( nWID )
            {
                case  SCHATTR_TEXT_ORDER:
                    {
                        SvxChartTextOrder eTextOrder;
                        chart::ChartAxisArrangeOrderType eArrOrder;
                        if( ! ( aValue >>= eArrOrder ))
                        {
                            // basic might give us an int instead of the enum type
                            sal_Int32 nIntVal;
                            if( aValue >>= nIntVal )
                            {
                                eArrOrder = SAL_STATIC_CAST( chart::ChartAxisArrangeOrderType, nIntVal );
                            }
                        }
                        switch( eArrOrder )
                        {
                            case chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
                                eTextOrder = CHTXTORDER_SIDEBYSIDE; break;
                            case chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
                                eTextOrder = CHTXTORDER_DOWNUP; break;
                            case chart::ChartAxisArrangeOrderType_STAGGER_ODD:
                                eTextOrder = CHTXTORDER_UPDOWN; break;
                            case chart::ChartAxisArrangeOrderType_AUTO:
                            default:
                                eTextOrder = CHTXTORDER_AUTO; break;
                        }
                        pSet->Put( SvxChartTextOrderItem( eTextOrder ));
                    }
                    break;
                case SCHATTR_TEXT_ORIENT:
                    {
                        sal_Bool bVal;
                        if( aValue >>= bVal )
                        {
                            pSet->Put( SvxChartTextOrientItem(
                                bVal
                                ? CHTXTORIENT_STACKED
                                : CHTXTORIENT_AUTOMATIC ));
                        }
                    }
                    break;
                case SCHATTR_AXIS_AUTO_STEP_MAIN:
                case SCHATTR_AXIS_AUTO_STEP_HELP:
                    {
                        sal_Bool bVal;
                        aValue >>= bVal;
                        if( ! bVal )
                        {
                            USHORT nStepWID = ( (nWID == SCHATTR_AXIS_AUTO_STEP_MAIN) ?
                                                SCHATTR_AXIS_STEP_MAIN :
                                                SCHATTR_AXIS_STEP_HELP );

                            BOOL bLog = FALSE;
                            ChartAxis* pAxis = mpModel->GetAxis( mnWhichId );
                            if( pAxis )
                                bLog = pAxis->IsLogarithm();
                            const SfxPoolItem* pPoolItem = NULL;
                            if( pSet->GetItemState( nStepWID, TRUE, &pPoolItem ) == SFX_ITEM_SET )
                            {
                                double fOld = ((const SvxDoubleItem*)pPoolItem)->GetValue();
                                if( fOld <= (bLog? 1.0: 0.0) )
                                    pSet->Put( SvxDoubleItem( bLog? 10.0 : 1.0, nStepWID ) );
                            }
                        }
                        maPropSet.setPropertyValue( pMap, aValue, *pSet );
                    }
                    break;
                case SCHATTR_AXIS_LOGARITHM:
                    {
                        sal_Bool bVal;
                        aValue >>= bVal;
                        if( bVal )
                        {
                            // for logarithmic scale always use automatic steps
                            pSet->Put( SfxBoolItem( SCHATTR_AXIS_AUTO_STEP_MAIN, TRUE ) );

                            const SfxPoolItem* pPoolItem = NULL;
                            double fMin = 0.0;
                            if( pSet->GetItemState( SCHATTR_AXIS_MIN, TRUE, &pPoolItem ) == SFX_ITEM_SET )
                            {
                                fMin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
                                if( fMin <= 0.0 )
                                    pSet->Put( SvxDoubleItem( 1.0, SCHATTR_AXIS_MIN ) );
                            }
                            if( pSet->GetItemState( SCHATTR_AXIS_MAX, TRUE, &pPoolItem ) == SFX_ITEM_SET )
                            {
                                if( ((const SvxDoubleItem*)pPoolItem)->GetValue() <= fMin )
                                    pSet->Put( SvxDoubleItem( fMin * 10.0, SCHATTR_AXIS_MAX ));
                            }
                        }
                        maPropSet.setPropertyValue( pMap, aValue, *pSet );
                    }
                    break;

                case SCHATTR_AXIS_STEP_MAIN:
                case SCHATTR_AXIS_STEP_HELP:
                    {
                        BOOL bLog = FALSE;
                        ChartAxis* pAxis = mpModel->GetAxis( mnWhichId );
                        if( pAxis )
                            bLog = pAxis->IsLogarithm();
                        double fVal;
                        aValue >>= fVal;
                        if( fVal <= bLog? 1.0: 0.0 )
                            throw lang::IllegalArgumentException();

                        pSet->Put( SfxBoolItem( nAutoWid, FALSE ));
                        maPropSet.setPropertyValue( pMap, aValue, *pSet );
                    }
                    break;

                case SCHATTR_AXIS_MIN:
                case SCHATTR_AXIS_MAX:
                    {
                        ChartAxis* pAxis = mpModel->GetAxis( mnWhichId );
                        double fVal;
                        aValue >>= fVal;
                        if( pAxis &&
                            pAxis->IsLogarithm() && fVal <= 0.0 )
                            throw lang::IllegalArgumentException();

                        pSet->Put( SfxBoolItem( nAutoWid, FALSE ));
                        maPropSet.setPropertyValue( pMap, aValue, *pSet );
                    }
                    break;

                case SCHATTR_AXIS_ORIGIN:
                    pSet->Put( SfxBoolItem( nAutoWid, FALSE ));
                    maPropSet.setPropertyValue( pMap, aValue, *pSet );
                    break;

                case SCHATTR_AXIS_NUMFMTPERCENT:
                    // was mapped from SCHATTR_AXIS_NUMFMT, so don't use standard mechanism
                    {
                        sal_Int32 nFmt;
                        aValue >>= nFmt;
                        pSet->Put( SfxUInt32Item( nWID, nFmt ));
                    }
                    break;

                case SID_ATTR_NUMBERFORMAT_SOURCE:
                    // this is not a which id (>4999)
                    {
                        sal_Bool bUseSource = sal_False;
                        aValue >>= bUseSource;
                        pSet->Put( SfxBoolItem( nWID, bUseSource ));
                    }
                    break;

                default:
                    // CL: convert special character properties
                    if( !SvxUnoTextRangeBase::SetPropertyValueHelper( *pSet, pMap, aValue, *pSet ))
                        maPropSet.setPropertyValue( pMap, aValue, *pSet );
                    break;
            }

            if( pSet->Count() )
                mpModel->ChangeAttr( *pSet, mnWhichId );
            else
                throw beans::UnknownPropertyException();

            delete pSet;
        }
        else
        {
            throw beans::UnknownPropertyException();
        }
    }
}



/*
    In contrast to calling the method setPropertyValue (singular) for every given property name
    the implementation of this method exploits the following properties:
    1:	Both the given sequence of property names and the member property map are sorted according
        to the property names.
    2:	The item set which gathers all items changed by setting the properties is created only once.
    3:	The solar mutex is acquired only once.
*/
void SAL_CALL ChXChartAxis::setPropertyValues	(
        const Sequence<OUString >	& aPropertyNames,
        const Sequence<Any >		& aValues ) 
    throw (	beans::PropertyVetoException,
            lang::IllegalArgumentException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex());
    
    if (mpModel == NULL || mnWhichId == CHOBJID_ANY)
        return;
        
    //	Get pointers to first elements of lists.
    const SfxItemPropertyMap *	pProperty = maPropSet.getPropertyMap ();
    const OUString * pPropertyName = aPropertyNames.getConstArray ();
    const Any * pValue = aValues.getConstArray ();
    
    USHORT nWID, nAutoWID;
    
    //	Create the item which collects all items modified by setting a property.
    SfxItemSet aAttributes (mpModel->GetItemPool(), nAxisWhichPairs);
    mpModel->GetAttr (mnWhichId, aAttributes);

    //	Iterate over all the given property names.
    sal_Int32	nCounter = aPropertyNames.getLength();
    while (nCounter-- > 0)
    {
        AdvanceToName (pProperty, pPropertyName);
        
        nWID = pProperty->nWID;

        if (pProperty == NULL || pProperty->nWID == 0)
        {
            OUString sMessage = OUString (RTL_CONSTASCII_USTRINGPARAM ( "ChXChartAxis::setPropertyValues: unknown property "));
            sMessage += *pPropertyName; 
            throw UnknownPropertyException (sMessage, (::cppu::OWeakObject*)this);
        }

        //	If the item that internally represents the property is not set, then get it's
        //	default value.
        if (aAttributes.GetItemState(nWID) != SFX_ITEM_SET)
        {
            //	Get default from the model's item pool.
            if (mpModel->GetItemPool().IsWhich (nWID))
                aAttributes.Put (mpModel->GetItemPool().GetDefaultItem(nWID));
#ifdef DBG_UTIL
            else if (nWID != SID_ATTR_NUMBERFORMAT_SOURCE)
            {
                String aTmpString (*pPropertyName);
                ByteString aProp( aTmpString, RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR2( "ChartAxis: Property %s has an invalid ID (%d)", aProp.GetBuffer(), nWID );
            }
#endif
        }
            
        //	Set the property's new value and handle some special cases.
        switch (nWID)
        {
            case  SCHATTR_TEXT_ORDER:
                {
                    SvxChartTextOrder eTextOrder;
                    chart::ChartAxisArrangeOrderType eArrOrder;
                    if ( ! ( *pValue >>= eArrOrder ))
                    {
                        // basic might give us an int instead of the enum type
                        sal_Int32 nIntVal;
                        if( *pValue >>= nIntVal )
                            eArrOrder = SAL_STATIC_CAST (chart::ChartAxisArrangeOrderType, nIntVal);
                    }
                    switch( eArrOrder )
                        {
                        case chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
                            eTextOrder = CHTXTORDER_SIDEBYSIDE;
                            break;
                        case chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
                            eTextOrder = CHTXTORDER_DOWNUP; 
                            break;
                        case chart::ChartAxisArrangeOrderType_STAGGER_ODD:
                            eTextOrder = CHTXTORDER_UPDOWN; 
                            break;
                        case chart::ChartAxisArrangeOrderType_AUTO:
                        default:
                            eTextOrder = CHTXTORDER_AUTO; 
                            break;
                    }
                    aAttributes.Put (SvxChartTextOrderItem( eTextOrder));
                }
                break;
            
            case SCHATTR_TEXT_ORIENT:
                {
                    sal_Bool bVal;
                    if( *pValue >>= bVal )
                    {
                        aAttributes.Put( SvxChartTextOrientItem(
                            bVal
                            ? CHTXTORIENT_STACKED
                            : CHTXTORIENT_AUTOMATIC ));
                    }
                }
                break;

            case SCHATTR_AXIS_AUTO_STEP_MAIN:
            case SCHATTR_AXIS_AUTO_STEP_HELP:
                {
                    sal_Bool bVal;
                    *pValue >>= bVal;
                    if( ! bVal )
                    {
                        USHORT nStepWID = ( (nWID == SCHATTR_AXIS_AUTO_STEP_MAIN) ?
                            SCHATTR_AXIS_STEP_MAIN :
                            SCHATTR_AXIS_STEP_HELP );

                        BOOL bLog = FALSE;
                        ChartAxis* pAxis = mpModel->GetAxis( mnWhichId );
                        if( pAxis )
                            bLog = pAxis->IsLogarithm();
                        const SfxPoolItem* pPoolItem = NULL;
                        if( aAttributes.GetItemState( nStepWID, TRUE, &pPoolItem ) == SFX_ITEM_SET )
                        {
                            double fOld = ((const SvxDoubleItem*)pPoolItem)->GetValue();
                            if( fOld <= (bLog? 1.0: 0.0) )
                                aAttributes.Put( SvxDoubleItem( bLog? 10.0 : 1.0, nStepWID ) );
                        }
                    }
                    maPropSet.setPropertyValue( pProperty, *pValue, aAttributes );
                }
                break;
                
            case SCHATTR_AXIS_LOGARITHM:
                {
                    sal_Bool bVal;
                    *pValue >>= bVal;
                    if( bVal )
                    {
                        // for logarithmic scale always use automatic steps
                        aAttributes.Put( SfxBoolItem( SCHATTR_AXIS_AUTO_STEP_MAIN, TRUE ) );

                        const SfxPoolItem* pPoolItem = NULL;
                        double fMin = 0.0;
                        if( aAttributes.GetItemState( SCHATTR_AXIS_MIN, TRUE, &pPoolItem ) == SFX_ITEM_SET )
                        {
                            fMin = ((const SvxDoubleItem*)pPoolItem)->GetValue();
                            if( fMin <= 0.0 )
                                aAttributes.Put( SvxDoubleItem( 1.0, SCHATTR_AXIS_MIN ) );
                        }
                        if( aAttributes.GetItemState( SCHATTR_AXIS_MAX, TRUE, &pPoolItem ) == SFX_ITEM_SET )
                        {
                            if( ((const SvxDoubleItem*)pPoolItem)->GetValue() <= fMin )
                                aAttributes.Put( SvxDoubleItem( fMin * 10.0, SCHATTR_AXIS_MAX ));
                        }
                    }
                    maPropSet.setPropertyValue( pProperty, *pValue, aAttributes );
                }
                break;

            case SCHATTR_AXIS_STEP_MAIN:
            case SCHATTR_AXIS_STEP_HELP:
                {
                    BOOL bLog = FALSE;
                    ChartAxis* pAxis = mpModel->GetAxis( mnWhichId );
                    if( pAxis )
                        bLog = pAxis->IsLogarithm();
                    double fVal;
                    *pValue >>= fVal;
                    if( fVal <= bLog? 1.0: 0.0 )
                        throw lang::IllegalArgumentException();

                    if (nWID == SCHATTR_AXIS_STEP_MAIN)
                        nAutoWID = SCHATTR_AXIS_AUTO_STEP_MAIN;
                    else
                        nAutoWID = SCHATTR_AXIS_AUTO_STEP_HELP;
                    aAttributes.Put( SfxBoolItem( nAutoWID, FALSE ));
                    maPropSet.setPropertyValue( pProperty, *pValue, aAttributes );
                }
                break;

            case SCHATTR_AXIS_MIN:
            case SCHATTR_AXIS_MAX:
                {
                    ChartAxis* pAxis = mpModel->GetAxis( mnWhichId );
                    double fVal;
                    *pValue >>= fVal;
                    if (	pAxis 
                        &&	pAxis->IsLogarithm()
                        &&	fVal <= 0.0)
                        throw lang::IllegalArgumentException();
                    if (nWID == SCHATTR_AXIS_MIN)
                        nAutoWID = SCHATTR_AXIS_AUTO_MIN;
                    else
                        nAutoWID = SCHATTR_AXIS_AUTO_MAX;

                    aAttributes.Put( SfxBoolItem( nAutoWID, FALSE ));
                    maPropSet.setPropertyValue (pProperty, *pValue, aAttributes);
                }
                break;

            case SCHATTR_AXIS_ORIGIN:
                aAttributes.Put (SfxBoolItem (SCHATTR_AXIS_AUTO_ORIGIN, FALSE));
                maPropSet.setPropertyValue (pProperty, *pValue, aAttributes);
                break;

            case SCHATTR_AXIS_NUMFMTPERCENT:
                // was mapped from SCHATTR_AXIS_NUMFMT, so don't use standard mechanism
                {
                    sal_Int32 nFmt;
                    *pValue >>= nFmt;
                    aAttributes.Put( SfxUInt32Item( nWID, nFmt ));
                }
                break;

            case SID_ATTR_NUMBERFORMAT_SOURCE:
                // this is not a which id (>4999)
                {
                    sal_Bool bUseSource = sal_False;
                    *pValue >>= bUseSource;
                    aAttributes.Put (SfxBoolItem( nWID, bUseSource));
                }
                break;

            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_LINEDASH:
                if (pProperty->nMemberId == MID_NAME )
                {
                    ::rtl::OUString aString;
                    if (*pValue >>= aString)
                        SvxShape::SetFillAttribute (nWID, aString, aAttributes, mpModel);
                    break;
                }
                // note: this fall-through is intended

            default:
                // CL: convert special character properties
                if( !SvxUnoTextRangeBase::SetPropertyValueHelper (
                        aAttributes, pProperty, *pValue, aAttributes))
                    maPropSet.setPropertyValue (pProperty, *pValue, aAttributes);
                break;
        }
        
        //	Advance to the next property, property name and value.
        pPropertyName++;
        pValue++;
    }
    
    //	Finally set the modified item set to the axis.
    mpModel->ChangeAttr (aAttributes, mnWhichId);
}




/*
Sequence<PropertyState > SAL_CALL ChXChartAxis::getPropertyStates(
        const Sequence<OUString > & aPropertyNames)
        throw( UnknownPropertyException,
               RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    const sal_Int32 nCount = aPropertyNames.getLength();
    const ::rtl::OUString * pName = aPropertyNames.getConstArray();
    Sequence<PropertyState > aStates (nCount);
    PropertyState * pState = aStates.getArray();
    
    if (mpModel == NULL)
        return aStates;

    for (sal_Int32 nIdx = 0; nIdx < nCount; nIdx++)
        pState[nIdx] = getPropertyState (pName[nIdx]);

    return aStates;
}
*/


//=====  protected methods  =======================================================================

SfxItemSet	*	ChXChartAxis::CreateItemSet	(void)
{
    SfxItemSet * pAttributes = new SfxItemSet (mpModel->GetItemPool(), nAxisWhichPairs);
    mpModel->GetAttr (mnWhichId, *pAttributes, mnIndex);
    return pAttributes;
}



void	ChXChartAxis::GetPropertyValue	(const SfxItemPropertyMap & rProperty,
                                        ::com::sun::star::uno::Any & rValue, 
                                        SfxItemSet & rAttributes)
{
    switch (rProperty.nWID)
    {
        case SCHATTR_TEXT_ORDER:
        {
            chart::ChartAxisArrangeOrderType eArrOrder;

            switch (((const SvxChartTextOrderItem&)(rAttributes.Get( SCHATTR_TEXT_ORDER ))).GetValue())
            {
                case CHTXTORDER_SIDEBYSIDE:
                    eArrOrder = chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE; 
                    break;
                case CHTXTORDER_DOWNUP:
                    eArrOrder = chart::ChartAxisArrangeOrderType_STAGGER_EVEN; 
                    break;
                case CHTXTORDER_UPDOWN:
                    eArrOrder = chart::ChartAxisArrangeOrderType_STAGGER_ODD; 
                    break;
                case CHTXTORDER_AUTO:
                default:
                    eArrOrder = chart::ChartAxisArrangeOrderType_AUTO; 
                    break;
            }
            rValue <<= eArrOrder;
            break;
        }
        case SCHATTR_AXIS_NUMFMT:
            {
                // if we have a percent chart use the correct number format item
                ChartType aType( mpModel );
                if( aType.IsPercent())
                {
                    rValue <<= (sal_Int32)(((const SfxUInt32Item&)(rAttributes.Get( SCHATTR_AXIS_NUMFMTPERCENT ))).GetValue());
                }
                else
                {
                    rValue <<= (sal_Int32)(((const SfxUInt32Item&)(rAttributes.Get( SCHATTR_AXIS_NUMFMT ))).GetValue());
                }
            }
            break;


        default:
            ChXChartObject::GetPropertyValue (rProperty, rValue, rAttributes);
    }
}


}
