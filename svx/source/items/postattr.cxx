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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#define _SVX_POSTATTR_CXX
#include "svx/postattr.hxx"
#include <editeng/itemtype.hxx>
#include <svx/svxitems.hrc>
#include <svx/dialmgr.hxx>

// class SvxPostItAuthorItem ---------------------------------------------

SvxPostItAuthorItem::SvxPostItAuthorItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}

// -----------------------------------------------------------------------

SvxPostItAuthorItem::SvxPostItAuthorItem( const XubString& rAuthor,
                                          sal_uInt16 _nWhich ) :
    SfxStringItem( _nWhich, rAuthor )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItAuthorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return SFX_ITEM_PRESENTATION_NAMELESS;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_AUTHOR_COMPLETE);
            rText += GetValue();
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItAuthorItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItAuthorItem( *this );
}

// class SvxPostItDateItem -----------------------------------------------

SvxPostItDateItem::SvxPostItDateItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}

// -----------------------------------------------------------------------

SvxPostItDateItem::SvxPostItDateItem( const XubString& rDate, sal_uInt16 _nWhich ) :

    SfxStringItem( _nWhich, rDate )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItDateItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return SFX_ITEM_PRESENTATION_NAMELESS;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_DATE_COMPLETE);
            rText += GetValue();
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItDateItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItDateItem( *this );
}

// class SvxPostItTextItem -----------------------------------------------

SvxPostItTextItem::SvxPostItTextItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}

// -----------------------------------------------------------------------

SvxPostItTextItem::SvxPostItTextItem( const XubString& rText, sal_uInt16 _nWhich ) :

    SfxStringItem( _nWhich, rText )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItTextItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return SFX_ITEM_PRESENTATION_NAMELESS;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_TEXT_COMPLETE);
            rText += GetValue();
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItTextItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItTextItem( *this );
}


