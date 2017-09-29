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
#include <osl/diagnose.h>
#include <svl/itempool.hxx>
#include <svl/itemprop.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svx/svxids.hrc>

using namespace ::com::sun::star;

namespace chart { namespace wrapper {

ItemConverter::ItemConverter(
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool ) :
        m_xPropertySet( rPropertySet ),
        m_xPropertySetInfo( nullptr ),
        m_rItemPool( rItemPool )
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

SfxItemSet ItemConverter::CreateEmptyItemSet() const
{
    return SfxItemSet( GetItemPool(), GetWhichPairs() );
}

void ItemConverter::_disposing( const lang::EventObject& )
{
}

void ItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    const sal_uInt16 * pRanges = rOutItemSet.GetRanges();
    tPropertyNameWithMemberId aProperty;
    SfxItemPool & rPool = GetItemPool();

    assert(pRanges != nullptr);
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
                            pItem->SetWhich(nWhich);
                            rOutItemSet.Put( *pItem );
                            delete pItem;
                        }
                    }
                    catch( const beans::UnknownPropertyException &ex )
                    {
                        delete pItem;
                        SAL_WARN( "chart2", ex << " - unknown Property: " << aProperty.first);
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
{
    OSL_FAIL( "ItemConverter: Unhandled special item found!" );
}

bool ItemConverter::ApplySpecialItem(
    sal_uInt16 /*nWhichId*/, const SfxItemSet & /*rItemSet*/ )
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
        if( rItemSet.GetItemState( pItem->Which(), false ) == SfxItemState::SET )
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
                    SAL_WARN( "chart2", ex << " - unknown Property: " << aProperty.first);
                }
                catch( const uno::Exception &ex )
                {
                    SAL_WARN( "chart2", ex );
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
    const SfxPoolItem *pPoolItem = nullptr;

    while (nWhich)
    {
        if ((rSourceSet.GetItemState(nWhich, true, &pPoolItem) == SfxItemState::SET) &&
            (rDestSet.GetItemState(nWhich, true, &pPoolItem) == SfxItemState::SET))
        {
            if (rSourceSet.Get(nWhich) != rDestSet.Get(nWhich))
            {
                if( nWhich != SID_CHAR_DLG_PREVIEW_STRING )
                {
                    rDestSet.InvalidateItem(nWhich);
                }
            }
        }
        else if( rSourceSet.GetItemState(nWhich, true, &pPoolItem) == SfxItemState::DONTCARE )
            rDestSet.InvalidateItem(nWhich);

        nWhich = aIter.NextWhich ();
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
