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

#include "ItemConverter.hxx"
#include "macros.hxx"
#include <com/sun/star/lang/XComponent.hpp>
#include <svl/itemprop.hxx>
#include <svl/itemiter.hxx>
// header for class SfxWhichIter
#include <svl/whiter.hxx>
#include <svx/svxids.hrc>

using namespace ::com::sun::star;

namespace comphelper
{

ItemConverter::ItemConverter(
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool ) :
        m_xPropertySet( rPropertySet ),
        m_xPropertySetInfo( NULL ),
        m_rItemPool( rItemPool ),
        m_bIsValid( true )
{
    resetPropertySet( m_xPropertySet );
}

ItemConverter::~ItemConverter()
{
    stopAllComponentListening();
}

void ItemConverter::resetPropertySet(
    const uno::Reference< beans::XPropertySet > & xPropSet )
{
    if( xPropSet.is())
    {
        stopAllComponentListening();
        m_xPropertySet = xPropSet;
        m_xPropertySetInfo = m_xPropertySet->getPropertySetInfo();

        uno::Reference< lang::XComponent > xComp( m_xPropertySet, uno::UNO_QUERY );
        if( xComp.is())
        {
            // method of base class ::utl::OEventListenerAdapter
            startComponentListening( xComp );
        }
    }
}

SfxItemPool & ItemConverter::GetItemPool() const
{
    return m_rItemPool;
}

SfxItemSet ItemConverter::CreateEmptyItemSet() const
{
    return SfxItemSet( GetItemPool(), GetWhichPairs() );
}

uno::Reference< beans::XPropertySet > ItemConverter::GetPropertySet() const
{
    return m_xPropertySet;
}

void ItemConverter::_disposing( const lang::EventObject& rSource )
{
    if( rSource.Source == m_xPropertySet )
    {
        m_bIsValid = false;
    }
}

void ItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    const sal_uInt16 * pRanges = rOutItemSet.GetRanges();
    tPropertyNameWithMemberId aProperty;
    SfxItemPool & rPool = GetItemPool();

    OSL_ASSERT( pRanges != NULL );
    OSL_ASSERT( m_xPropertySetInfo.is());
    OSL_ASSERT( m_xPropertySet.is());

    while( (*pRanges) != 0)
    {
        sal_uInt16 nBeg = (*pRanges);
        ++pRanges;
        sal_uInt16 nEnd = (*pRanges);
        ++pRanges;

        OSL_ASSERT( nBeg <= nEnd );
        for( sal_uInt16 nWhich = nBeg; nWhich <= nEnd; ++nWhich )
        {
            if( GetItemProperty( nWhich, aProperty ))
            {
                // put the Property into the itemset
                SfxPoolItem * pItem = rPool.GetDefaultItem( nWhich ).Clone();

                if( pItem )
                {
                    try
                    {
                        if( ! pItem->PutValue( m_xPropertySet->getPropertyValue( aProperty.first ),
                                               aProperty.second // nMemberId
                                ))
                        {
                            delete pItem;
                        }
                        else
                        {
                            rOutItemSet.Put( *pItem, nWhich );
                            delete pItem;
                        }
                    }
                    catch( const beans::UnknownPropertyException &ex )
                    {
                        delete pItem;
                        (void)ex;
                        OSL_FAIL(
                                    OUStringToOString(
                                        ex.Message +
                                        " - unknown Property: " + aProperty.first,
                                        RTL_TEXTENCODING_ASCII_US ).getStr());
                    }
                    catch( const uno::Exception &ex )
                    {
                        ASSERT_EXCEPTION( ex );
                    }
                }
            }
            else
            {
                try
                {
                    FillSpecialItem( nWhich, rOutItemSet );
                }
                catch( const uno::Exception &ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
        }
    }
}

void ItemConverter::FillSpecialItem(
    sal_uInt16 /*nWhichId*/, SfxItemSet & /*rOutItemSet*/ ) const
    throw( uno::Exception )
{
    OSL_FAIL( "ItemConverter: Unhandled special item found!" );
}

bool ItemConverter::ApplySpecialItem(
    sal_uInt16 /*nWhichId*/, const SfxItemSet & /*rItemSet*/ )
    throw( uno::Exception )
{
    OSL_FAIL( "ItemConverter: Unhandled special item found!" );
    return false;
}

bool ItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    OSL_ASSERT( m_xPropertySet.is());

    bool bItemsChanged = false;
    SfxItemIter aIter( rItemSet );
    const SfxPoolItem * pItem = aIter.FirstItem();
    tPropertyNameWithMemberId aProperty;
    uno::Any aValue;

    while( pItem )
    {
        if( rItemSet.GetItemState( pItem->Which(), sal_False ) == SFX_ITEM_SET )
        {
            if( GetItemProperty( pItem->Which(), aProperty ))
            {
                pItem->QueryValue( aValue, aProperty.second /* nMemberId */ );

                try
                {
                    if( aValue != m_xPropertySet->getPropertyValue( aProperty.first ))
                    {
                        m_xPropertySet->setPropertyValue( aProperty.first, aValue );
                        bItemsChanged = true;
                    }
                }
                catch( const beans::UnknownPropertyException &ex )
                {
                    (void)ex;
                    OSL_FAIL(
                                OUStringToOString(
                                    ex.Message +
                                    " - unknown Property: " + aProperty.first,
                                    RTL_TEXTENCODING_ASCII_US).getStr());
                }
                catch( const uno::Exception &ex )
                {
                    (void)ex;
                    OSL_FAIL( OUStringToOString(
                                    ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
                }
            }
            else
            {
                bItemsChanged = ApplySpecialItem( pItem->Which(), rItemSet ) || bItemsChanged;
            }
        }
        pItem = aIter.NextItem();
    }

    return bItemsChanged;
}

void ItemConverter::InvalidateUnequalItems( SfxItemSet  &rDestSet, const SfxItemSet &rSourceSet )
{
    SfxWhichIter      aIter (rSourceSet);
    sal_uInt16            nWhich     = aIter.FirstWhich ();
    const SfxPoolItem *pPoolItem = NULL;

    while (nWhich)
    {
        if ((rSourceSet.GetItemState(nWhich, sal_True, &pPoolItem) == SFX_ITEM_SET) &&
            (rDestSet.GetItemState(nWhich, sal_True, &pPoolItem) == SFX_ITEM_SET))
        {
            if (rSourceSet.Get(nWhich) != rDestSet.Get(nWhich))
            {
                if( SID_CHAR_DLG_PREVIEW_STRING != nWhich )
                {
                    rDestSet.InvalidateItem(nWhich);
                }
            }
        }
        else if( rSourceSet.GetItemState(nWhich, sal_True, &pPoolItem) == SFX_ITEM_DONTCARE )
            rDestSet.InvalidateItem(nWhich);

        nWhich = aIter.NextWhich ();
    }
}

} //  namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
