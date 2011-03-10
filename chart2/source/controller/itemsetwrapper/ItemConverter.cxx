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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
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
                    catch( beans::UnknownPropertyException &ex )
                    {
                        delete pItem;
                        OSL_ENSURE( false,
                                    ::rtl::OUStringToOString(
                                        ex.Message +
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                             " - unknown Property: " )) + aProperty.first,
                                        RTL_TEXTENCODING_ASCII_US ).getStr());
                    }
                    catch( uno::Exception &ex )
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
                catch( uno::Exception &ex )
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
    OSL_ENSURE( false, "ItemConverter: Unhandled special item found!" );
}

bool ItemConverter::ApplySpecialItem(
    sal_uInt16 /*nWhichId*/, const SfxItemSet & /*rItemSet*/ )
    throw( uno::Exception )
{
    OSL_ENSURE( false, "ItemConverter: Unhandled special item found!" );
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
                catch( beans::UnknownPropertyException &ex )
                {
                    OSL_ENSURE( false,
                                ::rtl::OUStringToOString(
                                    ex.Message +
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                         " - unknown Property: " )) + aProperty.first,
                                    RTL_TEXTENCODING_ASCII_US).getStr());
                }
                catch( uno::Exception &ex )
                {
                    OSL_ENSURE( false, ::rtl::OUStringToOString(
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

// --------------------------------------------------------------------------------
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
