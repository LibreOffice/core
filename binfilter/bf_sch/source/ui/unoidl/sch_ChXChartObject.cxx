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

#include "ChXChartObject.hxx"
#include "schattr.hxx"

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

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE           0
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT  SCHATTR_TEXT_ORIENT
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR

#include <bf_svtools/eitem.hxx>

#endif
#include <bf_svx/xflbstit.hxx>
#include <bf_svx/xflbmtit.hxx>

#include "app.hrc"			// for SID_TEXTBREAK

#include "mapprov.hxx"
#include "globfunc.hxx"			// for GlobalGenerate3DAttrDefaultItem

#include <com/sun/star/chart/ChartLegendPosition.hpp>

// header for any2enum
#include <comphelper/extract.hxx>

#include <memory>
#include "unonames.hxx"
namespace binfilter {


extern SchUnoPropertyMapProvider aSchMapProvider;

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

ChXChartObject::ChXChartObject( long _MapId, ChartModel* _Model, long _WhichId, long _Index ) :
    maPropSet( aSchMapProvider.GetMap( (short)_MapId, _Model )),
    mpModel( _Model ),
    mnWhichId( _WhichId? _WhichId: CHOBJID_ANY ),
    mnIndex( _Index ),
    maListenerList (maMutex)
{}

ChXChartObject::~ChXChartObject()
{}

long ChXChartObject::GetId() const
{
    return mnWhichId;
}

// XShape

// the following 'Hack' methods are copied from bf_svx/unodraw/unoshape.cxx
// they are needed as long as the drawing layer requires this

sal_Bool needLogicRectHack( SdrObject* pObj )
{
    if( pObj->GetObjInventor() == SdrInventor)
    {
        switch(pObj->GetObjIdentifier())
        {
        case OBJ_GRUP:
        case OBJ_LINE:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_SPLNLINE:
        case OBJ_SPLNFILL:
        case OBJ_EDGE:
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
        case OBJ_MEASURE:
            return sal_True;
        }
    }
    return sal_False;
}

Rectangle getLogicRectHack( SdrObject* pObj )
{
    if(needLogicRectHack(pObj))
    {
        return pObj->GetSnapRect();
    }
    else
    {
        return pObj->GetLogicRect();
    }
}

void setLogicRectHack( SdrObject* pObj, const Rectangle& rRect )
{
    if(needLogicRectHack(pObj))
    {
        pObj->SetSnapRect( rRect );
    }
    else
    {
        pObj->SetLogicRect( rRect );
    }
}

SdrObject* ChXChartObject::GetCurrentSdrObject() const
{
    SolarMutexGuard aGuard;

    SdrObject* pResult = NULL;
    if( mpModel )
        pResult = mpModel->GetObjectWithId( mnWhichId );

    // use first subobject for axes
    if( pResult &&
        pResult->ISA( SdrObjGroup ) &&
        ( mnWhichId == CHOBJID_DIAGRAM_X_AXIS ||
          mnWhichId == CHOBJID_DIAGRAM_Y_AXIS ||
          mnWhichId == CHOBJID_DIAGRAM_Z_AXIS ||
          mnWhichId == CHOBJID_DIAGRAM_A_AXIS ||
          mnWhichId == CHOBJID_DIAGRAM_B_AXIS ))
    {
        pResult = pResult->GetSubList()->GetObj( 0 );
    }

    return pResult;
}

// XShape interface methods
awt::Point SAL_CALL ChXChartObject::getPosition() throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    SdrObject* pObj = GetCurrentSdrObject();
    if( pObj )
    {
        Rectangle aRect( getLogicRectHack( pObj ));
        Point aPt( aRect.Left(), aRect.Top() );

        // Position is relative to anchor - calculate absoulte position
        aPt -= pObj->GetAnchorPos();

        return awt::Point( aPt.X(), aPt.Y() );
    }
    else
        DBG_ERROR( "Couldn't get position due to invalid SdrObject" );

    return awt::Point();
}

void SAL_CALL ChXChartObject::setPosition( const awt::Point& aPosition ) throw( uno::RuntimeException )
{
    switch( mnWhichId )
    {
        // the following objects can not be positioned
        case CHOBJID_DIAGRAM_X_AXIS:
        case CHOBJID_DIAGRAM_Y_AXIS:
        case CHOBJID_DIAGRAM_Z_AXIS:
        case CHOBJID_DIAGRAM_AREA:
        case CHOBJID_DIAGRAM_WALL:
            DBG_ERROR( "Cannot set position of this object" );
            return;
    }

    SolarMutexGuard aGuard;

    SdrObject* pObj = GetCurrentSdrObject();
    if( pObj )
    {
        Rectangle aRect( getLogicRectHack( pObj ) );
        Point aLocalPos( aPosition.X, aPosition.Y );

        // Position is absolute - position relative to anchor is required
        aLocalPos -= pObj->GetAnchorPos();

        long nDX = aLocalPos.X() - aRect.Left();
        long nDY = aLocalPos.Y() - aRect.Top();

        if( nDX != 0 || nDY != 0 )
        {
            pObj->Move( Size( nDX, nDY ) );
            if( mpModel )
            {
                mpModel->SetChanged();

                mpModel->SetUseRelativePositions( TRUE );
                mpModel->SetHasBeenMoved( mnWhichId );
            }
        }
    }
    else
        DBG_ERROR( "Couldn't set position due to invalid SdrObject" );
}

awt::Size SAL_CALL ChXChartObject::getSize() throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    SdrObject* pObj = GetCurrentSdrObject();
    if( pObj )
    {
        Rectangle aRect( getLogicRectHack( pObj ));
        Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
        return awt::Size( aObjSize.getWidth(), aObjSize.getHeight() );
    }
    else
        DBG_ERROR( "Couldn't get size due to invalid SdrObject" );

    return awt::Size();
}

void SAL_CALL ChXChartObject::setSize( const awt::Size& aSize )
    throw( beans::PropertyVetoException, uno::RuntimeException )
{
    // set size is not supported by any chart object except
    // the diagram which is covered by ChXDiagram

    DBG_ERROR( "Size of chart objects cannot be changed" );
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ChXChartObject::getPropertySetInfo() throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return maPropSet.getPropertySetInfo();
}

void SAL_CALL ChXChartObject::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( mpModel && mnWhichId != CHOBJID_ANY )
    {
        if( aPropertyName.compareToAscii( UNONAME_TEXT_STRING ) == 0 )
        {
            ::rtl::OUString aStr;
            aValue >>= aStr;
            mpModel->SetTitle( mnWhichId, aStr );
            mpModel->BuildChart( FALSE );
            return;
        }

        const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( aPropertyName );

        if( pMap && pMap->nWID )
        {
            if( pMap->nFlags & beans::PropertyAttribute::READONLY )
                throw beans::PropertyVetoException();

            USHORT nWID = pMap->nWID;

            // create itemset capable of holding necessary items
            SfxItemSet* pSet = NULL;
            switch( nWID )
            {
                case OWN_ATTR_FILLBMP_MODE:
                    pSet = new SfxItemSet( mpModel->GetItemPool(),
                                           XATTR_FILLBMP_STRETCH, XATTR_FILLBMP_STRETCH,
                                           XATTR_FILLBMP_TILE, XATTR_FILLBMP_TILE, 0 );
                    break;

                default:
                    pSet = new SfxItemSet( mpModel->GetItemPool(), nWID, nWID );
                    break;
            }

            // special handling
            switch( nWID )
            {
                case SCHATTR_LEGEND_POS:
                    if( mnWhichId == CHOBJID_LEGEND )
                    {
                        chart::ChartLegendPosition ePos;
                        cppu::any2enum< chart::ChartLegendPosition >( ePos, aValue );
                        //ToDo: NONE shouldn't be used to disable the legend. There should be a way
                        // to set the position to 'unanchored' plus a flag for
                        // 'vertically|horizontally' extending
                        mpModel->SetShowLegend( ePos != chart::ChartLegendPosition_NONE );
                        mpModel->SetLegendHasBeenMoved( FALSE );
                        // note: SetShowLegend sets pos to CHLEGEND_RIGHT
                        pSet->Put( SvxChartLegendPosItem( 
                            SAL_STATIC_CAST( SvxChartLegendPos, SAL_STATIC_CAST( int, ePos ))));
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

                case OWN_ATTR_FILLBMP_MODE:
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
                        pSet->Put( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                        pSet->Put( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                    }
                    while(0);
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
                    // some item is required for cloning
                    mpModel->GetAttr( mnWhichId, *pSet, mnIndex );

                    if( ! pSet->Count() &&
                        mpModel->GetItemPool().IsWhich( nWID ) &&
                        ! (( OWN_ATTR_VALUE_START <= nWID ) && ( nWID <= OWN_ATTR_VALUE_END )))
                    {
                        pSet->Put( mpModel->GetItemPool().GetDefaultItem( nWID ));
                    }

                    // CL: convert special character properties
                    if( ! SvxUnoTextRangeBase::SetPropertyValueHelper( *pSet, pMap, aValue, *pSet ))
                    {
                        // standard mapping of any to item
                        maPropSet.setPropertyValue( pMap, aValue, *pSet );
                    }
                    break;
            }

            if( pSet->Count() )
                mpModel->ChangeAttr( *pSet, mnWhichId, mnIndex );

            delete pSet;
        }
    }
    else
    {
        throw beans::UnknownPropertyException();
    }
}

uno::Any SAL_CALL ChXChartObject::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    uno::Any aResultaAny;
    sal_Bool bPropertyUnknown = sal_False;

    if( mpModel && mnWhichId != CHOBJID_ANY )
    {
        // titles
        if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( UNONAME_TEXT_STRING )))
        {
            aResultaAny <<= ::rtl::OUString( mpModel->GetTitle( mnWhichId ));
            return aResultaAny;
        }

        // legend alignment
        if( mnWhichId == CHOBJID_LEGEND &&
            PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( UNONAME_CHARTLEGENDALIGN )))
        {
            SfxItemSet rSet = mpModel->GetLegendAttr();
            const SfxPoolItem *pPoolItem = NULL;
            SvxChartLegendPos ePos =
                rSet.GetItemState( SCHATTR_LEGEND_POS,TRUE, &pPoolItem) == SFX_ITEM_SET
                ? ((const SvxChartLegendPosItem*)pPoolItem)->GetValue()
                : CHLEGEND_RIGHT;
            chart::ChartLegendPosition ePosition = (chart::ChartLegendPosition)ePos;
            aResultaAny <<= ePosition;
            return aResultaAny;
        }

        // properties using map
        const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( PropertyName );
        if( pMap && pMap->nWID )
        {
            USHORT nWID = pMap->nWID;

            if( nWID == OWN_ATTR_FILLBMP_MODE )
            {
                SfxItemSet aSet( mpModel->GetItemPool(),
                                 XATTR_FILLBMP_STRETCH, XATTR_FILLBMP_STRETCH,
                                 XATTR_FILLBMP_TILE, XATTR_FILLBMP_TILE, 0 );

                // get 'full attributes', ie some members are copied to the set
                mpModel->GetAttr( mnWhichId, aSet, mnIndex );

                XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)&aSet.Get( XATTR_FILLBMP_STRETCH );
                XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)&aSet.Get( XATTR_FILLBMP_TILE );

                if( pTileItem && pTileItem->GetValue() )
                {
                    aResultaAny <<= drawing::BitmapMode_REPEAT;
                }
                else if( pStretchItem && pStretchItem->GetValue() )
                {
                    aResultaAny <<= drawing::BitmapMode_STRETCH;
                }
                else
                {
                    aResultaAny <<= drawing::BitmapMode_NO_REPEAT;
                }
                return aResultaAny;
            }

            std::auto_ptr<SfxItemSet> aSet;
            switch (nWID)
            {
                case SCHATTR_TEXT_DEGREES:
                    //	Needing the text orientation for adapting 
                    //	CHTXTORIENT_AUTOMATIC to corresponging degrees.
                    aSet = std::auto_ptr<SfxItemSet> (new SfxItemSet (mpModel->GetItemPool(),
                        nWID, nWID,
                        SCHATTR_TEXT_ORIENT,SCHATTR_TEXT_ORIENT,
                        0));
                    break;
                    
                default:
                    //	Just interested in the single item.
                    aSet = std::auto_ptr<SfxItemSet> (new SfxItemSet (mpModel->GetItemPool(),
                        nWID, nWID));
            }

            // get 'full attributes', ie some members are copied to the set
            mpModel->GetAttr( mnWhichId, *aSet, mnIndex );

            if( SvxUnoTextRangeBase::GetPropertyValueHelper( *aSet, pMap, aResultaAny ))
                return aResultaAny;

            // item is not set => use default
            if( !aSet->Count() )
            {
                // get default value from pool
                if( mpModel->GetItemPool().IsWhich( nWID ) )
                {
                    if( ! ( OWN_ATTR_VALUE_START <= nWID && nWID <= OWN_ATTR_VALUE_END ))	// 'private' properties from SvxShape
                    {
                        aSet->Put( mpModel->GetItemPool().GetDefaultItem( nWID ) );
                    }
                }
                else if( nWID == SID_TEXTBREAK )
                {
                    aSet->Put( SfxBoolItem( SID_TEXTBREAK,
                                           ( mnWhichId == CHOBJID_DIAGRAM_X_AXIS ||
                                             mnWhichId == CHOBJID_DIAGRAM_A_AXIS )));
                }
                else if( nWID == SID_ATTR_NUMBERFORMAT_SOURCE )
                {
                    aSet->Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, TRUE ));
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

            // item or default has been set
            if( aSet->Count() )
            {
                //	Special case: Axis title rotation angle.
                if (nWID == SCHATTR_TEXT_DEGREES)
                {
                    //	Automatic text orientation is changed into corresponding
                    //	rotation angle.
                    SvxChartTextOrient eOrientation = 
                        ((const SvxChartTextOrientItem&)aSet->Get(
                            SCHATTR_TEXT_ORIENT)).GetValue();
                    if (eOrientation == CHTXTORIENT_AUTOMATIC)
                    {
                        switch (mnWhichId)
                        {
                            case CHOBJID_DIAGRAM_TITLE_X_AXIS:
                                if (mpModel->IsXVertikal())
                                    eOrientation = CHTXTORIENT_BOTTOMTOP;
                                else
                                    eOrientation = CHTXTORIENT_AUTOMATIC;
                                break;
                    
                            case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
                                if (mpModel->IsXVertikal())
                                    eOrientation = CHTXTORIENT_AUTOMATIC;
                                else
                                    eOrientation = CHTXTORIENT_BOTTOMTOP;
                                break;

                            case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
                                if (mpModel->IsXVertikal())
                                    eOrientation = CHTXTORIENT_TOPBOTTOM;
                                else
                                    eOrientation = CHTXTORIENT_AUTOMATIC;
                                break;
    
                            default:
                                eOrientation = CHTXTORIENT_AUTOMATIC;
                                break;
                        }
                        long nAngle = GetTextRotation (*aSet, eOrientation);
                        aSet->Put (SfxInt32Item (SCHATTR_TEXT_DEGREES, nAngle));
                    }
                }

                if( nWID == SCHATTR_TEXT_ORIENT )
                {
                    // map enum to bool: only stacked or not stacked is interesting
                    SvxChartTextOrient eOrientation = 
                        ((const SvxChartTextOrientItem&)aSet->Get(
                            SCHATTR_TEXT_ORIENT)).GetValue();
                    aResultaAny <<=  static_cast< sal_Bool >( eOrientation == CHTXTORIENT_STACKED );
                }
                else
                {
                    // get value from ItemSet
                    aResultaAny = maPropSet.getPropertyValue( pMap, *aSet );
                    if( *pMap->pType != aResultaAny.getValueType() )
                    {
                        // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                        if( ( *pMap->pType == ::getCppuType((const sal_Int16*)0)) && aResultaAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
                        {
                            sal_Int32 nValue;
                            aResultaAny >>= nValue;
                            aResultaAny <<= static_cast< sal_Int16 >( nValue );
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
                bPropertyUnknown = sal_True;
            }
        }
        else
        {
            bPropertyUnknown = sal_True;
        }

        if( bPropertyUnknown )
        {
            ::rtl::OUString aMessage( RTL_CONSTASCII_USTRINGPARAM( "Chart Object: Unknown Property " ));
            aMessage += PropertyName;
            beans::UnknownPropertyException aExcpt( aMessage, (::cppu::OWeakObject*)this );
            throw aExcpt;
        }
    }

    return aResultaAny;
}

void SAL_CALL ChXChartObject::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                     const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartObject::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                        const uno::Reference< beans::XPropertyChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartObject::addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                     const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartObject::removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                        const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}



//=====  XMultiPropertySet  ===================================================

/*
    In contrast to calling the method setPropertyValue (singular) for every given property name
    the implementation of this method exploits the following properties:
    1:	Both the given sequence of property names and the member property map are sorted according
        to the property names.
    2:	The item set which gathers all items changed by setting the properties is created only once.
    3:	The solar mutex is acquired only once.
*/
void SAL_CALL ChXChartObject::setPropertyValues	(
        const uno::Sequence< ::rtl::OUString >& aPropertyNames,
        const uno::Sequence< uno::Any >& aValues ) 
    throw (	beans::PropertyVetoException,
            lang::IllegalArgumentException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    
    //	Get pointers to first elements of lists.
    const SfxItemPropertyMap *	pProperty = maPropSet.getPropertyMap ();
    const OUString * pPropertyName = aPropertyNames.getConstArray ();
    const Any * pValue = aValues.getConstArray ();
    
    //	Check preconditions.
    if (mpModel == NULL)
        throw UnknownPropertyException (OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ChXChartObject::setPropertyValues: no model")), (::cppu::OWeakObject*)this);
    if (mnWhichId == CHOBJID_ANY)
        throw UnknownPropertyException (OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ChXChartObject::setPropertyValues: invalid object")), (::cppu::OWeakObject*)this);
    if (pProperty == NULL || pProperty->pName == NULL)
        throw UnknownPropertyException (OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ChXChartObject::setPropertyValues: empty property list")), (::cppu::OWeakObject*)this);
    
    //	Some variables used in the following loop.
    USHORT nWID;
    
    //	Item set used to collect all modifications from the individual properties to set.
    SfxItemSet	aModifications (mpModel->GetItemPool());
    mpModel->GetAttr (mnWhichId, aModifications, mnIndex);

    //	Iterate over all the given property names.
    sal_Int32	nCounter = aPropertyNames.getLength();
    while (nCounter-- > 0)
    {
        AdvanceToName (pProperty, pPropertyName);

        //	Handle the properties in the property map.  The case statements handle
        //	the special cases while the default statement handles all remaining cases.
        switch (nWID = pProperty->nWID)
        {
            case CHATTR_TITLE_MAIN:
                //	Handle special cases of title objects.
                {
                   OUString aString;
                *pValue >>= aString;
                   mpModel->SetTitle (mnWhichId, aString);
                mpModel->BuildChart (FALSE);
                }
                break;

            case SCHATTR_LEGEND_POS:
                //	This one is only valid if this object is a legend.
                if (mnWhichId == CHOBJID_LEGEND)
                {
                    chart::ChartLegendPosition ePos;
                    cppu::any2enum< chart::ChartLegendPosition >(ePos, *pValue);
                    //	ToDo: NONE shouldn't be used to disable the legend. There should
                    //	be a way to set the position to 'unanchored' plus a flag for
                    // 'vertically|horizontally' extending
                    mpModel->SetShowLegend (ePos != chart::ChartLegendPosition_NONE);
                    mpModel->SetLegendHasBeenMoved (FALSE);
                    // note: SetShowLegend sets pos to CHLEGEND_RIGHT
                    aModifications.Put (SvxChartLegendPosItem ( 
                        SAL_STATIC_CAST (SvxChartLegendPos, 
                            SAL_STATIC_CAST (int, ePos))));
                }
                break;

            case SCHATTR_TEXT_ORIENT:
                {
                    sal_Bool bVal;
                    if( *pValue >>= bVal )
                    {
                        aModifications.Put( SvxChartTextOrientItem(
                            bVal
                            ? CHTXTORIENT_STACKED
                            : CHTXTORIENT_AUTOMATIC ));
                    }
                }
                break;

            case OWN_ATTR_FILLBMP_MODE:
                {
                drawing::BitmapMode eMode;
                if ( ! (*pValue >>= eMode))
                {
                    sal_Int32 nMode;
                    if ( ! (*pValue >>= nMode))
                        break;
                    eMode = (drawing::BitmapMode)nMode;
                }
                aModifications.Put (XFillBmpStretchItem (eMode==drawing::BitmapMode_STRETCH));
                aModifications.Put (XFillBmpTileItem (eMode==drawing::BitmapMode_REPEAT));
                }
                break;

            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_FILLFLOATTRANSPARENCE:
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_LINEDASH:
                if (pProperty->nMemberId == MID_NAME )
                {
                    ::rtl::OUString aString;
                    if (*pValue >>= aString)
                        SvxShape::SetFillAttribute (nWID, aString, aModifications, mpModel);
                    break;
                }
                // note: this fall-through is intended

            default:
/*				if (	aModifications.Count() == 0
                    &&	mpModel->GetItemPool().IsWhich (nWID)
                    &&	! ((nWID >= OWN_ATTR_VALUE_START) && (nWID <= OWN_ATTR_VALUE_END)))
                {
                    aModifications.Put (mpModel->GetItemPool().GetDefaultItem (nWID));
                }
*/
                //	First look if we are looking at a special character property.
                if ( ! SvxUnoTextRangeBase::SetPropertyValueHelper (aModifications, pProperty,
                        *pValue, aModifications))
                {
                    //	Not a special character property.
                    //	Convert the given property value to an item and set it.
                    maPropSet.setPropertyValue (pProperty, *pValue, aModifications);
                }
                break;
        }
        
        //	Advance to the next property, property name and value.
        pPropertyName++;
        pValue++;
    }
    //	Propagate the modified item set to the chart model.
    if (aModifications.Count() > 0)
        mpModel->ChangeAttr (aModifications, mnWhichId, mnIndex);
}



/*
    Again three facts are exploited:
    1.	Getting the solar mutex once outside the main loop.
    2.	Using the sorting of both the given list of property names and of the list of properties.
    3.	Creating and filling once of the model's item set.
    
    This method uses two methods to make it usable in a generic way by derived classes.
    1.	CreateItemSet creates and fills an item set with all known items which is used to set the
        requested properties.
    2.	GetPropertyValue is used to retrieve the individual values.
    Both methods can be overloaded to implement class specific behaviour.
    
*/
uno::Sequence< uno::Any > SAL_CALL ChXChartObject::getPropertyValues	(
        const uno::Sequence< ::rtl::OUString >& aPropertyNames ) 
    throw ( uno::RuntimeException)
{
#if 0
    uno::Sequence<uno::Any>	aResult (aPropertyNames.getLength());
    
    for (sal_Int32 i=0; i<aPropertyNames.getLength(); i++)
    {
        aResult[i] = getPropertyValue (aPropertyNames[i]);
    }
    
    return aResult;
#endif
    SolarMutexGuard aGuard;
    
    //	This sequence is filled with the requested values for the given property names.
    Sequence<Any> aResult (aPropertyNames.getLength());
    
    //	Get pointers to first elements of lists.
    const SfxItemPropertyMap *	pProperty = maPropSet.getPropertyMap ();
    const OUString * pPropertyName = aPropertyNames.getConstArray ();
    Any * pValue = aResult.getArray ();
    
    //	Check preconditions.
    if (mpModel == NULL || mnWhichId == CHOBJID_ANY)
        return aResult;

    //	Get the models attributes.
    SfxItemSet * pAttributes = CreateItemSet();
    
    //	Iterate over all given property names.		
    sal_Int32 nCounter = aPropertyNames.getLength();
    while (nCounter-- > 0)
    {
        AdvanceToName (pProperty, pPropertyName);

        GetPropertyValue (*pProperty, *pValue, *pAttributes);

        //	Advance to the next property, property name and value.
        pPropertyName++;
        pValue++;
    }

    delete pAttributes;
    return aResult;
}




void SAL_CALL ChXChartObject::addPropertiesChangeListener	(
        const uno::Sequence< ::rtl::OUString >& aPropertyNames,
        const uno::Reference< beans::XPropertiesChangeListener >& xListener ) 
    throw (	uno::RuntimeException)
{
    //	Not implemented.
}




void SAL_CALL ChXChartObject::removePropertiesChangeListener	(
        const uno::Reference< beans::XPropertiesChangeListener >& xListener ) 
    throw (	uno::RuntimeException)
{
    //	Not implemented.
}




void SAL_CALL ChXChartObject::firePropertiesChangeEvent	(
        const uno::Sequence< ::rtl::OUString >& aPropertyNames,
        const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw (	uno::RuntimeException)
{
    //	Not implemented.
}



// XPropertyState
beans::PropertyState SAL_CALL ChXChartObject::getPropertyState( const ::rtl::OUString& PropertyName )
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( PropertyName );

    if( mpModel &&
        pMap && pMap->nWID )
    {
        USHORT nWID = pMap->nWID;

        if( mnWhichId == CHOBJID_LEGEND &&
            nWID == CHATTR_LEGEND_POS )
        {
            const SfxItemSet& rSet = mpModel->GetLegendAttr();
            SfxItemState aState = rSet.GetItemState( SCHATTR_LEGEND_POS, sal_False );

            if( aState == SFX_ITEM_DEFAULT )
                return beans::PropertyState_DEFAULT_VALUE;
            else
                return beans::PropertyState_DIRECT_VALUE;
        }
        else if( nWID == CHATTR_TITLE_MAIN ||	// property 'String' for ALL titles
                 nWID == SID_TEXTBREAK  )       // x, y and z axis have different defaults
        {
            return beans::PropertyState_DIRECT_VALUE;
        }
        else if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            SfxItemSet aSet( mpModel->GetItemPool(),
                             XATTR_FILLBMP_STRETCH, XATTR_FILLBMP_STRETCH,
                             XATTR_FILLBMP_TILE, XATTR_FILLBMP_TILE, 0 );
            mpModel->GetAttr( mnWhichId, aSet, mnIndex );

            if( aSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET ||
                aSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
                return beans::PropertyState_DIRECT_VALUE;
            else
                return beans::PropertyState_DEFAULT_VALUE;
        }
        else
        {
            SfxItemSet aSet( mpModel->GetItemPool(), nWID, nWID );
            mpModel->GetAttr( mnWhichId, aSet, mnIndex );

            switch( aSet.GetItemState( nWID, sal_False ) )
            {
                case SFX_ITEM_DONTCARE:
                case SFX_ITEM_DISABLED:
                    return beans::PropertyState_AMBIGUOUS_VALUE;
                case SFX_ITEM_READONLY:
                case SFX_ITEM_SET:
                    return beans::PropertyState_DIRECT_VALUE;
                case SFX_ITEM_DEFAULT:
                    return beans::PropertyState_DEFAULT_VALUE;
                case SFX_ITEM_UNKNOWN:
                default:
                    throw beans::UnknownPropertyException();
            }
        }
    }

    return beans::PropertyState_DIRECT_VALUE;
}

uno::Sequence< beans::PropertyState > SAL_CALL ChXChartObject::getPropertyStates(
        const uno::Sequence< ::rtl::OUString > & aPropertyNames)
        throw( beans::UnknownPropertyException,
               uno::RuntimeException )
{
#if 0
    SolarMutexGuard aGuard;

    const sal_Int32 nCount = aPropertyNames.getLength();
    const ::rtl::OUString * pName = aPropertyNames.getConstArray();
    Sequence<PropertyState > aStates (nCount);
    PropertyState * pState = aStates.getArray();
    
    if (mpModel == NULL)
        return aStates;

    for (sal_Int32 nIdx = 0; nIdx < nCount; nIdx++)
        pState[nIdx] = getPropertyState (pName[nIdx]);

    return aStates;
#else
    SolarMutexGuard aGuard;

    //	Get pointers to first elements of lists.
    const SfxItemPropertyMap * pProperty = maPropSet.getPropertyMap ();
    const sal_Int32 nCount = aPropertyNames.getLength();
    const OUString * pPropertyName = aPropertyNames.getConstArray();
    Sequence<PropertyState > aStates (nCount);
    PropertyState * pState = aStates.getArray();
    
    //	Check precondition.
    if (mpModel == NULL)
        return aStates;

    //	Get the models attributes.
    SfxItemSet aAttributes (mpModel->GetItemPool(),
                            mpModel->GetAttr (mnWhichId, mnIndex).GetRanges());
    mpModel->GetAttr (mnWhichId, aAttributes, mnIndex);
    
    //	Some variables used in the following loop.
    USHORT nWID;
    
    //	Iterate over all given property names.		
    sal_Int32 nCounter = aPropertyNames.getLength();
    while (nCounter-- > 0)
    {
        AdvanceToName (pProperty, pPropertyName);

        switch (nWID = pProperty->nWID)
        {
            case CHATTR_LEGEND_POS:
                if (mnWhichId == CHOBJID_LEGEND)
                {
                    const SfxItemSet& rSet = mpModel->GetLegendAttr();
                    SfxItemState aState = rSet.GetItemState( SCHATTR_LEGEND_POS, sal_False );

                    if( aState == SFX_ITEM_DEFAULT )
                        *pState = PropertyState_DEFAULT_VALUE;
                    else
                        *pState = PropertyState_DIRECT_VALUE;
                }
                break;
                
            case	CHATTR_TITLE_MAIN:	  // property 'String' for ALL titles
            case	SID_TEXTBREAK:		  // x, y and z axis have different defaults
                *pState = PropertyState_DIRECT_VALUE;
                break;

            case	OWN_ATTR_FILLBMP_MODE:
                if (	aAttributes.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET 
                    ||	aAttributes.GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
                    *pState = PropertyState_DIRECT_VALUE;
                else
                    *pState = PropertyState_DEFAULT_VALUE;
                break;

            case    SID_ATTR_NUMBERFORMAT_SOURCE:
                if( aAttributes.GetItemState( SID_ATTR_NUMBERFORMAT_SOURCE ) != SFX_ITEM_SET )
                    *pState = PropertyState_DEFAULT_VALUE;
                else
                    *pState = PropertyState_DIRECT_VALUE;
                break;

            default:
                switch (aAttributes.GetItemState( nWID, sal_False ))
                {
                    case SFX_ITEM_DONTCARE:
                    case SFX_ITEM_DISABLED:
                        *pState = PropertyState_AMBIGUOUS_VALUE;
                        break;
                        
                    case SFX_ITEM_READONLY:
                    case SFX_ITEM_SET:
                        *pState = PropertyState_DIRECT_VALUE;
                        break;
                        
                    case SFX_ITEM_DEFAULT:
                        *pState = PropertyState_DEFAULT_VALUE;
                        break;
                        
                    case SFX_ITEM_UNKNOWN:
                    default:
                        {
                        OUString aMessage (RTL_CONSTASCII_USTRINGPARAM ( "Chart Object: Unknown Property " ));
                        aMessage += *pPropertyName;
                        throw UnknownPropertyException (aMessage, (::cppu::OWeakObject*)this );
                        }
                }
        }
        pPropertyName ++;
        pState ++;
    }

    return aStates;
#endif
}

void SAL_CALL ChXChartObject::setPropertyToDefault( const ::rtl::OUString& PropertyName )
        throw( beans::UnknownPropertyException,
               uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( PropertyName );

    if( mpModel )
    {
        // set an empty item to change attribute to default
        SfxItemSet aSet( mpModel->GetItemPool(), pMap->nWID, pMap->nWID );
        mpModel->ChangeAttr( aSet, mnWhichId, mnIndex );
    }
}

uno::Any SAL_CALL ChXChartObject::getPropertyDefault( const ::rtl::OUString& aPropertyName )
        throw( beans::UnknownPropertyException,
               lang::WrappedTargetException,
               uno::RuntimeException )
{
    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry( aPropertyName );
    SfxItemSet aSet( mpModel->GetItemPool(), pMap->nWID, pMap->nWID );

    if( mpModel->GetItemPool().IsWhich( pMap->nWID ))
    {
        aSet.Put( mpModel->GetItemPool().GetDefaultItem( pMap->nWID ));
    }
    else if( pMap->nWID == SID_TEXTBREAK )
    {
        // default for x axis is true, false for y/z
        BOOL bTextBreak =
            ( mnWhichId == CHOBJID_DIAGRAM_X_AXIS ||
              mnWhichId == CHOBJID_DIAGRAM_A_AXIS );
        aSet.Put( SfxBoolItem( SID_TEXTBREAK, bTextBreak ));
    }
    else
        throw beans::UnknownPropertyException();

    uno::Any aAny = maPropSet.getPropertyValue( pMap, aSet );

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

    return aAny;
}




//	XMultiPropertyStates

//	ChXChartObject::getPropertyStates is also part of XPropertyState and is defined above.



void SAL_CALL ChXChartObject::setAllPropertiesToDefault	(void)
    throw (	uno::RuntimeException)
{
    DBG_ERROR ("ChXChartObject::setAllPropertiesToDefault");
}




void SAL_CALL ChXChartObject::setPropertiesToDefault	(
            const uno::Sequence< ::rtl::OUString >& aPropertyNames )
    throw (	beans::UnknownPropertyException,
            uno::RuntimeException)
{
    for (sal_Int32 i=0; i<aPropertyNames.getLength(); i++)
    {
        setPropertyToDefault (aPropertyNames[i]);
    }
}




uno::Sequence< uno::Any > SAL_CALL ChXChartObject::getPropertyDefaults	(
            const uno::Sequence< ::rtl::OUString >& aPropertyNames ) 
    throw (	beans::UnknownPropertyException, 
            lang::WrappedTargetException, 
            uno::RuntimeException)
{
    uno::Sequence<uno::Any>	aResult (aPropertyNames.getLength());
    
    for (sal_Int32 i=0; i<aPropertyNames.getLength(); i++)
    {
        aResult[i] = getPropertyDefault (aPropertyNames[i]);
    }
    
    return aResult;
}



// XComponent
void SAL_CALL ChXChartObject::dispose() throw( uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvent (*this);
    maListenerList.disposeAndClear (aEvent);
}

void SAL_CALL ChXChartObject::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw( uno::RuntimeException )
{
    maListenerList.addInterface (xListener);
}

void SAL_CALL ChXChartObject::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw( uno::RuntimeException )
{
    maListenerList.removeInterface (xListener);
}

// XServiceInfo
sal_Bool SAL_CALL ChXChartObject::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}


// generate a uniqueId
const uno::Sequence< sal_Int8 > & ChXChartObject::getUnoTunnelId() throw()
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

// XUnoTunnel
sal_Int64 SAL_CALL ChXChartObject::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( uno::RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                                 aIdentifier.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}



//=====  protected methods  =======================================================================

void	ChXChartObject::AdvanceToName	(const SfxItemPropertyMap *& pProperty, 
                                        const OUString * pPropertyName)
{
    sal_Int32	nComparisonResult;

    //	Advance over the property list to the property with the current name.
    while ((nComparisonResult=pPropertyName->compareToAscii (pProperty->pName)) > 0)
    {
        pProperty++;
        if (pProperty->pName == NULL)
        {
            //	We reached the end of the property list.  Therefore all remaining given
            //	property names can not be found.
            OUString sMessage = OUString (RTL_CONSTASCII_USTRINGPARAM ( "ChXChartObject::setPropertyValues: unknown property "));
            sMessage += *pPropertyName; 
            throw UnknownPropertyException (sMessage, (::cppu::OWeakObject*)this);
        }
    }
    if (nComparisonResult < 0)
    {
        //	All remaining given property names can not match (because they lie 
        //	alphabetically before) the remaining properties => Error
        OUString sMessage = OUString (RTL_CONSTASCII_USTRINGPARAM ( "ChXChartObject::setPropertyValues: unknown property "));
        sMessage += *pPropertyName;
        throw UnknownPropertyException (sMessage, (::cppu::OWeakObject*)this);
    }
    
    //	If we get here then we have found the propert with the name pProperty->pName.
}



SfxItemSet	*	ChXChartObject::CreateItemSet	(void)
{
    SfxItemSet * pAttributes = new SfxItemSet (mpModel->GetItemPool());
    mpModel->GetAttr (mnWhichId, *pAttributes, mnIndex);
    return pAttributes;
}




void	ChXChartObject::GetPropertyValue	(const SfxItemPropertyMap & rProperty,
                                            Any & rValue, 
                                            SfxItemSet & rAttributes)
{
    USHORT nWID = rProperty.nWID;
    
    switch (nWID)
    {
        case	CHATTR_TITLE_MAIN:
            rValue <<= OUString( mpModel->GetTitle( mnWhichId ));
            break;
            
        case SCHATTR_LEGEND_POS:
            if (mnWhichId == CHOBJID_LEGEND)
               {
                SfxItemSet rSet = mpModel->GetLegendAttr();
                const SfxPoolItem *pPoolItem = NULL;
                SvxChartLegendPos ePos =
                    rSet.GetItemState( SCHATTR_LEGEND_POS,TRUE, &pPoolItem) == SFX_ITEM_SET
                    ? ((const SvxChartLegendPosItem*)pPoolItem)->GetValue()
                    : CHLEGEND_RIGHT;
                chart::ChartLegendPosition ePosition = (chart::ChartLegendPosition)ePos;
                rValue <<= ePosition;
            }
            break;

        case OWN_ATTR_FILLBMP_MODE:
            {
                   XFillBmpStretchItem * pStretchItem = (XFillBmpStretchItem*)&rAttributes.Get (
                    XATTR_FILLBMP_STRETCH );
                   XFillBmpTileItem * pTileItem = (XFillBmpTileItem*)&rAttributes.Get (
                    XATTR_FILLBMP_TILE );
                
                if( pTileItem && pTileItem->GetValue() )
                    rValue <<= drawing::BitmapMode_REPEAT;
                else if( pStretchItem && pStretchItem->GetValue() )
                    rValue <<= drawing::BitmapMode_STRETCH;
                else
                    rValue <<= drawing::BitmapMode_NO_REPEAT;
            }
            break;

        case SCHATTR_TEXT_ORIENT:
            rValue <<= static_cast< sal_Bool >(
                CHTXTORIENT_STACKED ==
                static_cast< const SvxChartTextOrientItem & >( rAttributes.Get( SCHATTR_TEXT_ORIENT )).GetValue() );
            break;

        default	:
            //	Handle some special properties.
            if (SvxUnoTextRangeBase::GetPropertyValueHelper (rAttributes, &rProperty, rValue))
                break;

            // item is not set => use default
            switch (rAttributes.GetItemState (nWID, FALSE))
            {
                case SFX_ITEM_UNKNOWN:
                    {
                        OUString sMessage (RTL_CONSTASCII_USTRINGPARAM ( "ChXChartObject::getPropertyValues: unknown property "));
                        sMessage += OUString::createFromAscii(rProperty.pName);
                           throw UnknownPropertyException (sMessage, (::cppu::OWeakObject*)this);
                    }
                        
                case SFX_ITEM_DEFAULT:
                    //	Get default value from the item pool.
                    if (	mpModel->GetItemPool().IsWhich( nWID )
                        &&	(nWID < OWN_ATTR_VALUE_START || nWID > OWN_ATTR_VALUE_END))
                    {
                        //	`Private' properties from SvxShape.
                        rAttributes.Put( mpModel->GetItemPool().GetDefaultItem( nWID ) );
                    }
                    else if( nWID == SID_TEXTBREAK )
                    {
                        rAttributes.Put( SfxBoolItem( SID_TEXTBREAK,
                            (	mnWhichId == CHOBJID_DIAGRAM_X_AXIS
                            ||	mnWhichId == CHOBJID_DIAGRAM_A_AXIS )));
                    }
                    else if( nWID == SID_ATTR_NUMBERFORMAT_SOURCE )
                    {
                        rAttributes.Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, TRUE ));
                    }
                    else
                    {
#ifdef DBG_UTIL
                        DBG_ERROR2( "Diagram: Property %s has an invalid ID (%d)", 
                            rProperty.pName, nWID );
#endif
                    }
                        //	Fall through.
                        
                default	:
                    rValue = maPropSet.getPropertyValue( &rProperty, rAttributes );
                    if( *rProperty.pType != rValue.getValueType() )
                    {
                        // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                        if( ( *rProperty.pType == ::getCppuType((const sal_Int16*)0)) && rValue.getValueType() == ::getCppuType((const sal_Int32*)0) )
                        {
                            sal_Int32 nValue;
                            rValue >>= nValue;
                            rValue <<= static_cast< sal_Int16 >( nValue );
                        }
                        else
                        {
                            DBG_ERROR( "GetPropertyValue(): wrong Type!" );
                        }
                    }
            }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
