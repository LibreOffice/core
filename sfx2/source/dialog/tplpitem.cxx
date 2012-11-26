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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#include "sfx2/tplpitem.hxx"
#include <com/sun/star/frame/status/Template.hpp>


// STATIC DATA -----------------------------------------------------------

IMPL_POOLITEM_FACTORY(SfxTemplateItem)

SfxTemplateItem::SfxTemplateItem() :
    SfxFlagItem()
{
}

SfxTemplateItem::SfxTemplateItem
(
    sal_uInt16 nWhichId,      // Slot-ID
    const String& rStyle, // Name des aktuellen Styles
    sal_uInt16 nValue         // Flags f"ur das Filtern bei automatischer Anzeige
) : SfxFlagItem( nWhichId, nValue ),
    aStyle( rStyle )
{
}

//-------------------------------------------------------------------------

// copy ctor
SfxTemplateItem::SfxTemplateItem( const SfxTemplateItem& rCopy ) :

    SfxFlagItem( rCopy ),

    aStyle( rCopy.aStyle )
{
}

//-------------------------------------------------------------------------

// op ==

int SfxTemplateItem::operator==( const SfxPoolItem& rCmp ) const
{
    return ( SfxFlagItem::operator==( rCmp ) &&
             aStyle == ( (const SfxTemplateItem&)rCmp ).aStyle );
}

//-------------------------------------------------------------------------

SfxPoolItem* SfxTemplateItem::Clone( SfxItemPool *) const
{
    return new SfxTemplateItem(*this);
}

//-------------------------------------------------------------------------
sal_Bool SfxTemplateItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    ::com::sun::star::frame::status::Template aTemplate;

    aTemplate.Value = GetValue();
    aTemplate.StyleName = aStyle;
    rVal <<= aTemplate;

    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxTemplateItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    ::com::sun::star::frame::status::Template aTemplate;

    if ( rVal >>= aTemplate )
    {
        SetValue( sal::static_int_cast< sal_uInt16 >( aTemplate.Value ) );
        aStyle = aTemplate.StyleName;
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

sal_uInt8 SfxTemplateItem::GetFlagCount() const
{
    return sizeof(sal_uInt16) * 8;
}


