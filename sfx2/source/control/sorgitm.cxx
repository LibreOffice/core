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

#include <sfx2/sfxsids.hrc>
#include "sorgitm.hxx"
// STATIC DATA -----------------------------------------------------------

SfxScriptOrganizerItem::SfxScriptOrganizerItem() :

    SfxStringItem()

{
}

//------------------------------------------------------------------------

SfxScriptOrganizerItem::SfxScriptOrganizerItem( const String& rLanguage ) :

     SfxStringItem( SID_SCRIPTORGANIZER, rLanguage ),

     aLanguage( rLanguage )

{
}

//------------------------------------------------------------------------

SfxScriptOrganizerItem::SfxScriptOrganizerItem( const SfxScriptOrganizerItem& rItem ) :

    SfxStringItem( rItem ),

    aLanguage( rItem.aLanguage )

{
}

//------------------------------------------------------------------------

SfxScriptOrganizerItem::~SfxScriptOrganizerItem()
{
}

//------------------------------------------------------------------------

SfxPoolItem* SfxScriptOrganizerItem::Clone( SfxItemPool * ) const
{
    return new SfxScriptOrganizerItem( *this );
}

//------------------------------------------------------------------------

int SfxScriptOrganizerItem::operator==( const SfxPoolItem& rItem) const
{
    const SfxScriptOrganizerItem* pSfxScriptOrganizerItem = dynamic_cast< const SfxScriptOrganizerItem* >(&rItem);

     return pSfxScriptOrganizerItem &&
         SfxStringItem::operator==(rItem) &&
         aLanguage == pSfxScriptOrganizerItem->aLanguage;
}


sal_Bool SfxScriptOrganizerItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    String aValue;
    sal_Bool bIsString = sal_False;
    sal_Bool bValue = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        case MID_SCRIPT_ORGANIZER_LANGUAGE:
            bIsString = sal_True;
            aValue = aLanguage;
            break;
        default:
            DBG_ERROR("Wrong MemberId!");
               return sal_False;
     }

    if ( bIsString )
        rVal <<= ::rtl::OUString( aValue );
    else
        rVal <<= bValue;
    return sal_True;
}

sal_Bool SfxScriptOrganizerItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    ::rtl::OUString aValue;
    sal_Bool bRet = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        case MID_SCRIPT_ORGANIZER_LANGUAGE:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aLanguage = aValue;
            break;
        default:
            DBG_ERROR("Wrong MemberId!");
            return sal_False;
    }

    return bRet;
}

