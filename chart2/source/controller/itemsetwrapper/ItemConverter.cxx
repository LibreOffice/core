/*************************************************************************
 *
 *  $RCSfile: ItemConverter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-07 17:18:32 $
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
#include "ItemConverter.hxx"

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
// header for class SfxWhichIter
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

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
    if( m_xPropertySet.is())
    {
        m_xPropertySetInfo = m_xPropertySet->getPropertySetInfo();

        uno::Reference< lang::XComponent > xComp( m_xPropertySet, uno::UNO_QUERY );
        if( xComp.is())
        {
            // method of base class ::utl::OEventListenerAdapter
            startComponentListening( xComp );
        }
    }
}

ItemConverter::~ItemConverter()
{
    stopAllComponentListening();
}

SfxItemPool & ItemConverter::GetItemPool() const
{
    return m_rItemPool;
}

SfxItemSet ItemConverter::CreateEmptyItemSet() const
{
    return SfxItemSet( GetItemPool(), GetWhichPairs() );
}

bool ItemConverter::IsValid() const
{
    return m_bIsValid;
}

uno::Reference< beans::XPropertySet > ItemConverter::GetPropertySet() const
{
    return m_xPropertySet;
}

void ItemConverter::_disposing( const lang::EventObject& _rSource )
{
    if( _rSource.Source == m_xPropertySet )
    {
        m_bIsValid = false;
    }
}

void ItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    const USHORT * pRanges = rOutItemSet.GetRanges();
    ::rtl::OUString aPropName;
    SfxItemPool & rPool = GetItemPool();

    OSL_ASSERT( pRanges != NULL );
    OSL_ASSERT( m_xPropertySetInfo.is());
    OSL_ASSERT( m_xPropertySet.is());

    while( (*pRanges) != 0)
    {
        USHORT nBeg = (*pRanges);
        ++pRanges;
        USHORT nEnd = (*pRanges);
        ++pRanges;

        OSL_ASSERT( nBeg <= nEnd );
        for( USHORT nWhich = nBeg; nWhich <= nEnd; ++nWhich )
        {
            if( GetItemPropertyName( nWhich, aPropName ))
            {
                // put the Property into the itemset
                SfxPoolItem * pItem = rPool.GetDefaultItem( nWhich ).Clone();

                if( pItem )
                {
                    try
                    {
                        if( ! pItem->PutValue( m_xPropertySet->getPropertyValue( aPropName ),
                                               0 // nMemberId
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
                    catch( beans::UnknownPropertyException ex )
                    {
                        OSL_ENSURE( false,
                                    ::rtl::OUStringToOString(
                                        ex.Message +
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                             " - unknown Property: " )) + aPropName,
                                        RTL_TEXTENCODING_ASCII_US ).getStr());
                        delete pItem;
                    }
                    catch( uno::Exception ex )
                    {
                        OSL_ENSURE( false, ::rtl::OUStringToOString(
                                        ex.Message, RTL_TEXTENCODING_ASCII_US).getStr());
                    }
                }
            }
            else
            {
                FillSpecialItem( nWhich, rOutItemSet );
            }
        }
    }
}

bool ItemConverter::GetItemPropertyName(
    USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    return false;
}

void ItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
{
    OSL_ENSURE( false, "ItemConverter: Unhandled special item found!" );
}

bool ItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet ) const
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
    ::rtl::OUString aPropName;
    uno::Any aValue;

    while( pItem )
    {
        if( rItemSet.GetItemState( pItem->Which(), FALSE ) == SFX_ITEM_SET )
        {
            if( GetItemPropertyName( pItem->Which(), aPropName ))
            {
                pItem->QueryValue( aValue, 0 /* nMemberId */ );

                try
                {
                    if( aValue != m_xPropertySet->getPropertyValue( aPropName ))
                    {
                        m_xPropertySet->setPropertyValue( aPropName, aValue );
                        bItemsChanged = true;
                    }
                }
                catch( beans::UnknownPropertyException ex )
                {
                    OSL_ENSURE( false,
                                ::rtl::OUStringToOString(
                                    ex.Message +
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                         " - unknown Property: " )) + aPropName,
                                    RTL_TEXTENCODING_ASCII_US).getStr());
                }
                catch( uno::Exception ex )
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

//static
void ItemConverter::InvalidateUnequalItems( SfxItemSet  &rDestSet, const SfxItemSet &rSourceSet )
{
    SfxWhichIter      aIter (rSourceSet);
    USHORT            nWhich     = aIter.FirstWhich ();
    const SfxPoolItem *pPoolItem = NULL;

    while (nWhich)
    {
        if ((rSourceSet.GetItemState(nWhich, TRUE, &pPoolItem) == SFX_ITEM_SET) &&
            (rDestSet.GetItemState(nWhich, TRUE, &pPoolItem) == SFX_ITEM_SET))
            if (rSourceSet.Get(nWhich) != rDestSet.Get(nWhich))
                rDestSet.InvalidateItem(nWhich);

        nWhich = aIter.NextWhich ();
    }
}

} //  namespace comphelper
