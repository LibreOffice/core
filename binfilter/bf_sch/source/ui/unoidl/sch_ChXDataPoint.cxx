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

#include "ChXDataPoint.hxx"
#include "mapprov.hxx"

// header for SvxServiceInfoHelper
// header for class OGuard
// header for class Application
#include <vcl/svapp.hxx>
// header for SvxUnoTextRangeBase
// for OWN_ATTR_...
#include <bf_svx/unoshprp.hxx>
// for SID_ATTR_...
#include <bf_svx/svxids.hrc>
#include <bf_svx/unoshape.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "schattr.hxx"

#ifndef _SVX_BRSHITEM_HXX 
#define ITEMID_BRUSH SCHATTR_SYMBOL_BRUSH
#include <bf_svx/brshitem.hxx>
#endif
// header for SVX_SYMBOLTYPE_BRUSHITEM
#include <bf_svx/tabline.hxx>
#include <bf_svx/unoapi.hxx>

#include <bf_svx/xflbstit.hxx>
#include <bf_svx/xflbmtit.hxx>

#include <com/sun/star/chart/ChartDataCaption.hpp>

// header for SvxChartDataDescrItem
#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR
#endif

#include "globfunc.hxx"			// for GlobalGenerate3DAttrDefaultItem
#include "unonames.hxx"
namespace binfilter {


extern SchUnoPropertyMapProvider aSchMapProvider;

using namespace ::com::sun::star;

ChXDataPoint::ChXDataPoint( sal_Int32 _Col, sal_Int32 _Row, ChartModel* _Model ) :
        maPropSet( aSchMapProvider.GetMap( _Model? CHMAP_DATAPOINT: CHMAP_NONE, _Model )),
        mpModel( _Model ),
        mnCol( _Col ),
        mnRow( _Row )
{}

ChXDataPoint::~ChXDataPoint()
{}


// generate a uniqueId
const uno::Sequence< sal_Int8 > & ChXDataPoint::getUnoTunnelId() throw()
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

// XTypeProvider
uno::Sequence< sal_Int8 > SAL_CALL ChXDataPoint::getImplementationId()
    throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ChXDataPoint::getPropertySetInfo() throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return maPropSet.getPropertySetInfo();
}

void ChXDataPoint::AddDataPointAttr( SfxItemSet& rOutAttributes )
{
    if( mpModel->IsPieChart())
        rOutAttributes.Put( mpModel->GetFullDataPointAttr( mnCol, 0 ));
    else
        rOutAttributes.Put( mpModel->GetDataPointAttr( mnCol, mnRow ));
}

void SAL_CALL ChXDataPoint::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bPropertyUnknown = sal_False;

    if( mpModel )
    {
        const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( aPropertyName );

        if( pMap && pMap->nWID )
        {
            if( pMap->nFlags & beans::PropertyAttribute::READONLY )
                throw beans::PropertyVetoException();

            USHORT nWID = pMap->nWID;
            SfxItemSet* pSet;

            if( nWID == SCHATTR_DATADESCR_DESCR || nWID == SCHATTR_DATADESCR_SHOW_SYM )
            {
                pSet = new SfxItemSet( mpModel->GetItemPool(),
                                       SCHATTR_DATADESCR_DESCR, SCHATTR_DATADESCR_DESCR,
                                       SCHATTR_DATADESCR_SHOW_SYM, SCHATTR_DATADESCR_SHOW_SYM, 0 );
            }
            else if( nWID == CHATTR_PIE_SEGMENT_OFFSET )	// member of ChartModel rather than property
            {
                sal_Int32 nVal;
                aValue >>= nVal;
                short nValToSet = (short)(abs( nVal ) % 101);		// cast long->short is ok (range 0..100)

                mpModel->SetPieSegOfs( mnCol, nValToSet );
                SvxChartStyle& rStyle = mpModel->ChartStyle();
                if( rStyle == CHSTYLE_2D_PIE_SEGOF1 ||
                    rStyle == CHSTYLE_2D_PIE_SEGOFALL )
                    rStyle = CHSTYLE_2D_PIE;

                mpModel->BuildChart( FALSE );
                return;				// RETURN
            }
            else if( nWID == OWN_ATTR_FILLBMP_MODE )
            {
                SfxItemSet aSet( mpModel->GetItemPool(),
                                 XATTR_FILLBMP_STRETCH, XATTR_FILLBMP_STRETCH,
                                 XATTR_FILLBMP_TILE, XATTR_FILLBMP_TILE, 0 );

                do
                {
                    drawing::BitmapMode eMode;
                    if(!(aValue >>= eMode) )
                    {
                        sal_Int32 nMode;
                        if(!(aValue >>= nMode))
                            break;

                        eMode = (drawing::BitmapMode)nMode;
                    }
                    aSet.Put( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                    aSet.Put( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                }
                while(0);
                mpModel->PutDataPointAttr( mnCol, mnRow, aSet );
                mpModel->BuildChart( FALSE );

                return;
            }
            else if( nWID == SCHATTR_SYMBOL_BRUSH )
                pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID,
                                       SCHATTR_STYLE_SYMBOL, SCHATTR_STYLE_SYMBOL, 0 );
            else
            {
                pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID );
            }

            pSet->Put( mpModel->GetFullDataPointAttr( mnCol, mnRow ));

            if( ! pSet->Count() )
            {
                // get default from ItemPool
                if( mpModel->GetItemPool().IsWhich( nWID ))
                {
                    if( ! ( OWN_ATTR_VALUE_START <= nWID && nWID <= OWN_ATTR_VALUE_END ))
                    {
                        pSet->Put( mpModel->GetItemPool().GetDefaultItem( nWID ));
                    }
                }
                else
                {
#ifdef DBG_UTIL
                    String aTmpString( aPropertyName );
                    ByteString aProp( aTmpString, RTL_TEXTENCODING_ASCII_US );
                    DBG_ERROR2( "Diagram: Property %s has an invalid ID (%d)", aProp.GetBuffer(), nWID );
#endif
                }
            }

            if( pSet->Count() )
            {
                sal_Int32 nVal;
                switch( nWID )
                {
                    case SCHATTR_STAT_KIND_ERROR:
                    case SCHATTR_STAT_INDICATE:
                    case SCHATTR_STAT_REGRESSTYPE:
                        // property is enum, but item is INT32
                        aValue >>= nVal;
                        pSet->Put( SfxInt32Item( nWID, nVal ) );
                        break;
                    // BM: theese two SCHATTRs are merged into one INT32, so extract them from aValue
                    case SCHATTR_DATADESCR_DESCR:
                    case SCHATTR_DATADESCR_SHOW_SYM:
                        {
                            // symbol
                            aValue >>= nVal;
                            pSet->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYM,
                                                    ((nVal & chart::ChartDataCaption::SYMBOL) != 0)));

                            // display text, percent or both or value
                            SvxChartDataDescr eDescr = CHDESCR_NONE;
                            if( nVal & chart::ChartDataCaption::TEXT )
                            {
                                if( nVal & chart::ChartDataCaption::PERCENT )
                                    eDescr = CHDESCR_TEXTANDPERCENT;
                                else if( (nVal & chart::ChartDataCaption::VALUE) )
                                    eDescr = CHDESCR_TEXTANDVALUE;
                                else
                                    eDescr = CHDESCR_TEXT;
                            }
                            else if( (nVal & chart::ChartDataCaption::VALUE) )
                            {
                                eDescr = CHDESCR_VALUE;
                            }
                            else
                            {
                                if( (nVal & chart::ChartDataCaption::PERCENT) )
                                    eDescr = CHDESCR_PERCENT;
                            }

                            // chart::ChartDataCaption::FORMAT (missing)

                            pSet->Put( SvxChartDataDescrItem( eDescr ) );
                        }
                        break;
                    case SCHATTR_SYMBOL_BRUSH:
                        {
                            ::rtl::OUString aURL;
                            aValue >>= aURL;
                            BfGraphicObject aGraphObj = CreateGraphicObjectFromURL( aURL );
                            SvxBrushItem aItem( SCHATTR_SYMBOL_BRUSH );
                            aItem.SetGraphic( aGraphObj.GetGraphic() );
                            pSet->Put( aItem );
                        }
                        break;

                    case XATTR_FILLBITMAP:
                    case XATTR_FILLGRADIENT:
                    case XATTR_FILLHATCH:
                    case XATTR_FILLFLOATTRANSPARENCE:
                    case XATTR_LINEEND:
                    case XATTR_LINESTART:
                    case XATTR_LINEDASH:
                        if( pMap->nMemberId == MID_NAME )
                        {
                            ::rtl::OUString aStr;
                            if( aValue >>= aStr )
                                SvxShape::SetFillAttribute( nWID, aStr, *pSet, mpModel );
                            break;
                        }
                        // note: this fall-through is intended

                    default:
                        // CL: convert special character properties
                        if( ! SvxUnoTextRangeBase::SetPropertyValueHelper( *pSet, pMap, aValue, *pSet ))
                            maPropSet.setPropertyValue( pMap, aValue, *pSet );
                        break;
                }
                mpModel->PutDataPointAttr( mnCol, mnRow, *pSet );

                mpModel->BuildChart( FALSE );
            }
            else
            {
                bPropertyUnknown = sal_True;
            }
            if( pSet )
                delete pSet;
        }
        else
        {
            bPropertyUnknown = sal_True;
        }
    }
    else
    {
        DBG_WARNING( "No Model" );
    }

    if( bPropertyUnknown )
    {
        ::rtl::OUString aMessage( RTL_CONSTASCII_USTRINGPARAM( "Data Point: Unknown Property " ));
        aMessage += aPropertyName;
        beans::UnknownPropertyException aExcpt( aMessage, (::cppu::OWeakObject*)this );
        throw aExcpt;
    }
}

uno::Any SAL_CALL ChXDataPoint::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    
    uno::Any aAny;

    if( mpModel )
    {
        const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( PropertyName );

        if( pMap && pMap->nWID )
        {
            SfxItemSet* pSet;
            USHORT nWID = pMap->nWID;

            if( nWID == SCHATTR_STYLE_SHAPE &&
                ! mpModel->IsReal3D())
                return aAny;

            if( nWID == SCHATTR_DATADESCR_DESCR || nWID == SCHATTR_DATADESCR_SHOW_SYM )
            {
                pSet = new SfxItemSet( mpModel->GetItemPool(),
                    SCHATTR_DATADESCR_DESCR, SCHATTR_DATADESCR_DESCR,
                    SCHATTR_DATADESCR_SHOW_SYM, SCHATTR_DATADESCR_SHOW_SYM, 0 );
            }
            else if( nWID == CHATTR_PIE_SEGMENT_OFFSET )
            {
                sal_Int32 nVal = mpModel->PieSegOfs( mnCol );
                aAny <<= nVal;
                return aAny;								// RETURN
            }
            else if( nWID == OWN_ATTR_FILLBMP_MODE )
            {
                SfxItemSet aSet( mpModel->GetItemPool(),
                                 XATTR_FILLBMP_STRETCH, XATTR_FILLBMP_STRETCH,
                                 XATTR_FILLBMP_TILE, XATTR_FILLBMP_TILE, 0 );

                aSet.Put( mpModel->GetFullDataPointAttr( mnCol, mnRow ));

                XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)&aSet.Get( XATTR_FILLBMP_STRETCH );
                XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)&aSet.Get( XATTR_FILLBMP_TILE );

                if( pTileItem && pTileItem->GetValue() )
                {
                    aAny <<= drawing::BitmapMode_REPEAT;
                }
                else if( pStretchItem && pStretchItem->GetValue() )
                {
                    aAny <<= drawing::BitmapMode_STRETCH;
                }
                else
                {
                    aAny <<= drawing::BitmapMode_NO_REPEAT;
                }
                return aAny;
            }
            else
            {
                pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID );
            }
            pSet->Put( mpModel->GetFullDataPointAttr( mnCol, mnRow ));

            if( ! pSet->Count() )
            {
                // get default from pool
                if( mpModel->GetItemPool().IsWhich( nWID ))
                {
                    if( OWN_ATTR_VALUE_START > nWID || nWID > OWN_ATTR_VALUE_END )
                    {
                        pSet->Put( mpModel->GetItemPool().GetDefaultItem( nWID ) );
                    }
                }
                else
                {
#ifdef DBG_UTIL
                    String aTmpString( PropertyName );
                    ByteString aProp( aTmpString, RTL_TEXTENCODING_ASCII_US );
                    DBG_ERROR2( "Diagram: Property %s has an invalid ID (%d)", aProp.GetBuffer(), nWID );
#endif
                }
            }

            if( pSet->Count() )
            {
                // get value from ItemSet

                // theese two SCHATTRs have to be merged into one INT32 in aAny
                if( nWID == SCHATTR_DATADESCR_DESCR || nWID == SCHATTR_DATADESCR_SHOW_SYM )
                {
                    BOOL bShowSymbol = ((const SfxBoolItem&)
                        (pSet->Get(SCHATTR_DATADESCR_SHOW_SYM))).GetValue();
                    SvxChartDataDescr eDescr = ((const SvxChartDataDescrItem&)
                        (pSet->Get(SCHATTR_DATADESCR_DESCR))).GetValue();

                    sal_Int32 nVal = 0;
                    switch( eDescr )
                    {
                        case CHDESCR_NONE:
                            nVal = chart::ChartDataCaption::NONE;
                            break;
                        case CHDESCR_VALUE:
                            nVal = chart::ChartDataCaption::VALUE;
                            break;
                        case CHDESCR_PERCENT:
                            nVal = chart::ChartDataCaption::PERCENT;
                            break;
                        case CHDESCR_TEXT:
                            nVal = chart::ChartDataCaption::TEXT;
                            break;
                        case CHDESCR_TEXTANDPERCENT:
                            nVal = chart::ChartDataCaption::PERCENT | chart::ChartDataCaption::TEXT;
                            break;
                        case CHDESCR_TEXTANDVALUE:
                            nVal = chart::ChartDataCaption::VALUE | chart::ChartDataCaption::TEXT;
                    // chart::ChartDataCaption::FORMAT (missing)
                    }
                    if( bShowSymbol ) nVal |= chart::ChartDataCaption::SYMBOL;

                    aAny <<= nVal;
                }
                else if( nWID == SCHATTR_SYMBOL_BRUSH )
                {
                    ::rtl::OUString aURL;
                    const BfGraphicObject* pGraphObj =
                        ((const SvxBrushItem &)(pSet->Get( nWID ))).GetGraphicObject();
                    if( pGraphObj )
                    {
                        aURL = ::rtl::OUString::createFromAscii( UNO_NAME_GRAPHOBJ_URLPREFIX );
                        aURL += ::rtl::OUString::createFromAscii( pGraphObj->GetUniqueID().GetBuffer());
                    }
                    aAny <<= aURL;
                }
                else
                {
                    aAny = maPropSet.getPropertyValue( pMap, *pSet );
                    if( *pMap->pType != aAny.getValueType() )
                    {
                        // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                        if( ( *pMap->pType == ::getCppuType((const sal_Int16*)0)) && aAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
                        {
                            sal_Int32 nValue;
                            aAny >>= nValue;
                            aAny <<= static_cast< sal_Int16 >( nValue );
                        }
                        else
                        {
                            DBG_ERROR( "getPropertyValue(): wrong Type!" );
                        }
                    }
                }
            }
            else
            {
                throw beans::UnknownPropertyException();
            }
            if( pSet )
                delete pSet;
        }
        else
        {
            throw beans::UnknownPropertyException();
        }
    }

    return aAny;
}

void SAL_CALL ChXDataPoint::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                       const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXDataPoint::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                          const uno::Reference< beans::XPropertyChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXDataPoint::addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                       const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXDataPoint::removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                          const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

// XPropertyState
beans::PropertyState SAL_CALL ChXDataPoint::getPropertyState( const ::rtl::OUString& PropertyName )
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( PropertyName );

    if( mpModel &&
        pMap && pMap->nWID )
    {
        if( pMap->nWID == CHATTR_PIE_SEGMENT_OFFSET )
        {
            sal_Int32 nOffset;
            uno::Any aAny = getPropertyValue( PropertyName );
            aAny >>= nOffset;
            if( nOffset )
                return beans::PropertyState_DIRECT_VALUE;
            else
                return beans::PropertyState_DEFAULT_VALUE;
        }
        else if( pMap->nWID == SCHATTR_DATADESCR_DESCR )
        {
            SfxItemSet aSet( mpModel->GetItemPool(),
                             SCHATTR_DATADESCR_DESCR, SCHATTR_DATADESCR_DESCR,
                             SCHATTR_DATADESCR_SHOW_SYM, SCHATTR_DATADESCR_SHOW_SYM, 0 );
            AddDataPointAttr( aSet );

            SfxItemState eState1 = aSet.GetItemState( SCHATTR_DATADESCR_DESCR );
            SfxItemState eState2 = aSet.GetItemState( SCHATTR_DATADESCR_SHOW_SYM );
            if( eState1 == SFX_ITEM_DEFAULT &&
                eState2 == SFX_ITEM_DEFAULT )
                return beans::PropertyState_DEFAULT_VALUE;
            else if( eState1 < SFX_ITEM_DEFAULT &&
                     eState2 < SFX_ITEM_DEFAULT )
                return beans::PropertyState_AMBIGUOUS_VALUE;
            else
                return beans::PropertyState_DIRECT_VALUE;
        }
        else if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            SfxItemSet aSet( mpModel->GetItemPool(),
                             XATTR_FILLBMP_STRETCH, XATTR_FILLBMP_STRETCH,
                             XATTR_FILLBMP_TILE, XATTR_FILLBMP_TILE, 0 );
            AddDataPointAttr( aSet );

            if( aSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET ||
                aSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
                return beans::PropertyState_DIRECT_VALUE;
            else
                return beans::PropertyState_DEFAULT_VALUE;
        }
        else
        {
            SfxItemSet aSet( mpModel->GetItemPool(), pMap->nWID, pMap->nWID );
            AddDataPointAttr( aSet );

            SfxItemState eState = aSet.GetItemState( pMap->nWID );
            if( eState == SFX_ITEM_DEFAULT )
                return beans::PropertyState_DEFAULT_VALUE;
            else if( eState < SFX_ITEM_DEFAULT )
                return beans::PropertyState_AMBIGUOUS_VALUE;
            else
                return beans::PropertyState_DIRECT_VALUE;
        }
    }

    return beans::PropertyState_DIRECT_VALUE;
}

/*
uno::Sequence< beans::PropertyState > SAL_CALL ChXDataPoint::getPropertyStates(
        const uno::Sequence< ::rtl::OUString >& aPropertyName )
        throw( beans::UnknownPropertyException,
               uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    const sal_Int32 nCount = aPropertyName.getLength();
    const ::rtl::OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aRet( nCount );
    beans::PropertyState* pState = aRet.getArray();

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
        pState[ nIdx ] = getPropertyState( pNames[ nIdx ] );

    return aRet;
}
*/

uno::Sequence< beans::PropertyState > SAL_CALL
    ChXDataPoint::getPropertyStates	(
        const uno::Sequence< ::rtl::OUString > & aPropertyName)
        throw (beans::UnknownPropertyException,
            uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Int32 nCount = aPropertyName.getLength();
    const ::rtl::OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence<beans::PropertyState> aResult (nCount);
    beans::PropertyState * pStateArray = aResult.getArray();

    // for pie charts data point attributes are stored in data rows
    // which are returned by GetDataPointAttr() if no explicit
    // arguments exist
    if (mpModel->IsDataPointAttrSet (mnCol, mnRow) ||
        (mpModel->IsPieChart()))
    {
        uno::Any						aAny;
        SfxAllItemSet					aSet( mpModel->GetItemPool() );
        SfxItemState					eState1,eState2;
        const SfxItemPropertyMap	*	pMap;

        AddDataPointAttr( aSet );

        for (sal_Int32 nIndex=0; nIndex<nCount; nIndex++)
        {
            pMap = maPropSet.getPropertyMapEntry (pNames[nIndex]);

            if (mpModel && pMap && pMap->nWID)
            {
                switch (pMap->nWID)
                {
                    case	CHATTR_PIE_SEGMENT_OFFSET:
                        if( mpModel->IsPieChart() )
                        {
                            pStateArray[nIndex] = (mpModel->PieSegOfs (mnCol) != 0)
                                ? beans::PropertyState_DIRECT_VALUE
                                : beans::PropertyState_DEFAULT_VALUE;
                        }
                        else
                        {
                            pStateArray[nIndex] = beans::PropertyState_DEFAULT_VALUE;
                        }
                        break;
        
                    case	SCHATTR_DATADESCR_DESCR:
                        eState1 = aSet.GetItemState (SCHATTR_DATADESCR_DESCR);
                        eState2 = aSet.GetItemState (SCHATTR_DATADESCR_SHOW_SYM);
                        if (eState1 == SFX_ITEM_DEFAULT && eState2 == SFX_ITEM_DEFAULT)
                            pStateArray[nIndex] = beans::PropertyState_DEFAULT_VALUE;
                        else if (eState1 < SFX_ITEM_DEFAULT && eState2 < SFX_ITEM_DEFAULT)
                            pStateArray[nIndex] = beans::PropertyState_AMBIGUOUS_VALUE;
                        else
                            pStateArray[nIndex] = beans::PropertyState_DIRECT_VALUE;
                        break;

                    case    OWN_ATTR_FILLBMP_MODE:
                        eState1 = aSet.GetItemState( XATTR_FILLBMP_STRETCH, false );
                        eState2 = aSet.GetItemState( XATTR_FILLBMP_TILE, false );
                        if (eState1 == SFX_ITEM_SET || eState2 == SFX_ITEM_SET)
                            pStateArray[nIndex] = beans::PropertyState_DIRECT_VALUE;
                        else
                            pStateArray[nIndex] = beans::PropertyState_DEFAULT_VALUE;
                        break;

                    default:
                        switch (aSet.GetItemState (pMap->nWID))
                        {
                            case	SFX_ITEM_DEFAULT:	//	0x0020
                                pStateArray[nIndex] = beans::PropertyState_DEFAULT_VALUE;
                                break;
                        
                            case	SFX_ITEM_UNKNOWN:	//	0x0000
                            case	SFX_ITEM_DISABLED:	//	0x0001
                            case	SFX_ITEM_READONLY:	//	0x0002
                            case	SFX_ITEM_DONTCARE:	//	0x0010
                                pStateArray[nIndex] = beans::PropertyState_AMBIGUOUS_VALUE;
                                break;
                        
                            default:
                                //	SFX_ITEM_SET	0x0030
                                pStateArray[nIndex] = beans::PropertyState_DIRECT_VALUE;
                        }
                }
            }
            else
                pStateArray[nIndex] = beans::PropertyState_DEFAULT_VALUE;
//			pStateArray[nIndex] = beans::PropertyState_DIRECT_VALUE;
        }
    }
    else
        for (sal_Int32 nIndex=0; nIndex<nCount; nIndex++)
            pStateArray[nIndex] = beans::PropertyState_DEFAULT_VALUE;
            
    return aResult;
}




void SAL_CALL ChXDataPoint::setPropertyToDefault( const ::rtl::OUString& PropertyName )
        throw( beans::UnknownPropertyException,
               uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( PropertyName );

    if( mpModel && pMap && pMap->nWID )
    {
        if( pMap->nWID == CHATTR_PIE_SEGMENT_OFFSET )
        {
                mpModel->SetPieSegOfs( mnCol, 0 );
        }
        else if( pMap->nWID == SCHATTR_DATADESCR_DESCR )
        {
            SfxItemSet aSet( mpModel->GetItemPool(),
                             SCHATTR_DATADESCR_DESCR, SCHATTR_DATADESCR_DESCR,
                             SCHATTR_DATADESCR_SHOW_SYM, SCHATTR_DATADESCR_SHOW_SYM, 0 );

            aSet.ClearItem( SCHATTR_DATADESCR_DESCR );
            aSet.ClearItem( SCHATTR_DATADESCR_SHOW_SYM );
            mpModel->PutDataPointAttr( mnCol, mnRow, aSet );
        }
        else
        {
            SfxItemSet aSet( mpModel->GetItemPool(), pMap->nWID, pMap->nWID );

            aSet.ClearItem( pMap->nWID );
            mpModel->PutDataPointAttr( mnCol, mnRow, aSet );
        }
    }
}

uno::Any SAL_CALL ChXDataPoint::getPropertyDefault( const ::rtl::OUString& aPropertyName )
        throw( beans::UnknownPropertyException,
               lang::WrappedTargetException,
               uno::RuntimeException )
{
    uno::Any aAny;
    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( aPropertyName );

    if( mpModel && ! mpModel->GetItemPool().IsWhich( pMap->nWID ))
        throw beans::UnknownPropertyException();

    if( mpModel && pMap && pMap->nWID )
    {
        if( pMap->nWID == CHATTR_PIE_SEGMENT_OFFSET )
        {
            aAny <<= (sal_Int32)0;
        }
        else if( pMap->nWID == SCHATTR_DATADESCR_DESCR )
        {
            SfxItemSet aSet( mpModel->GetItemPool(),
                             SCHATTR_DATADESCR_DESCR, SCHATTR_DATADESCR_DESCR,
                             SCHATTR_DATADESCR_SHOW_SYM, SCHATTR_DATADESCR_SHOW_SYM, 0 );

            aSet.Put( mpModel->GetItemPool().GetDefaultItem( SCHATTR_DATADESCR_DESCR ));
            aSet.Put( mpModel->GetItemPool().GetDefaultItem( SCHATTR_DATADESCR_SHOW_SYM ));
            aAny = maPropSet.getPropertyValue( pMap, aSet );
        }
        else
        {
            SfxItemSet aSet( mpModel->GetItemPool(), pMap->nWID, pMap->nWID );
            aSet.Put( mpModel->GetItemPool().GetDefaultItem( pMap->nWID ));
            aAny = maPropSet.getPropertyValue( pMap, aSet );
        }

        if( *pMap->pType != aAny.getValueType() )
        {
            // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
            if( ( *pMap->pType == ::getCppuType((const sal_Int16*)0)) && aAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
            {
                sal_Int32 nValue;
                aAny >>= nValue;
                aAny <<= static_cast< sal_Int16 >( nValue );
            }
            else
            {
                DBG_ERROR( "getPropertyDefault(): wrong Type!" );
            }
        }
    }

    return aAny;
}


// XShapeDescriptor ( ::XShape ::XDiagram )
::rtl::OUString SAL_CALL ChXDataPoint::getShapeType() throw( uno::RuntimeException )
{
    return SCH_ASCII_TO_OU( "com.sun.star.chart.ChartDataPoint" );
}


// XServiceInfo
::rtl::OUString SAL_CALL ChXDataPoint::getImplementationName() throw( uno::RuntimeException )
{
    return SCH_ASCII_TO_OU( "ChXDataPoint" );
}

sal_Bool SAL_CALL ChXDataPoint::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );	
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXDataPoint::getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq;
    SvxServiceInfoHelper::addToSequence(
        aSeq, 5,
        "com.sun.star.chart.ChartDataPointProperties",
        "com.sun.star.drawing.LineProperties",
        "com.sun.star.drawing.FillProperties",
        "com.sun.star.style.CharacterProperties",
        "com.sun.star.xml.UserDefinedAttributeSupplier" );

    if( mpModel )
    {
        long nType = mpModel->GetBaseType();
        if( mpModel->Is3DChart() && (nType == CHTYPE_BAR || nType == CHTYPE_COLUMN) )
            SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.chart.Chart3DBarProperties" );
        else if( ! mpModel->Is3DChart() && nType == CHTYPE_CIRCLE )
            SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.chart.ChartPieSegmentProperties" );
    }

    return aSeq;
}

// XUnoTunnel
sal_Int64 SAL_CALL ChXDataPoint::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( uno::RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                                 aIdentifier.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
