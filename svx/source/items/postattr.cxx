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

#include "svx/postattr.hxx"
#include <editeng/itemtype.hxx>
#include <svx/svxitems.hrc>
#include <svx/dialmgr.hxx>




SfxPoolItem* SvxPostItAuthorItem::CreateDefault() { return new SvxPostItAuthorItem(0); }
SfxPoolItem* SvxPostItDateItem::CreateDefault() { return new SvxPostItDateItem(0); }
SfxPoolItem* SvxPostItTextItem::CreateDefault() { return new SvxPostItTextItem(0); }

SvxPostItAuthorItem::SvxPostItAuthorItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}



SvxPostItAuthorItem::SvxPostItAuthorItem( const OUString& rAuthor,
                                          sal_uInt16 _nWhich ) :
    SfxStringItem( _nWhich, rAuthor )
{
}



bool SvxPostItAuthorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return true;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_AUTHOR_COMPLETE) + GetValue();
            return true;
        default: ;//prevent warning
    }
    return false;
}



SfxPoolItem* SvxPostItAuthorItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItAuthorItem( *this );
}

SvxPostItDateItem::SvxPostItDateItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}



SvxPostItDateItem::SvxPostItDateItem( const OUString& rDate, sal_uInt16 _nWhich ) :

    SfxStringItem( _nWhich, rDate )
{
}



bool SvxPostItDateItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return true;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_DATE_COMPLETE) + GetValue();
            return true;
        default: ;//prevent warning
    }
    return false;
}



SfxPoolItem* SvxPostItDateItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItDateItem( *this );
}

SvxPostItTextItem::SvxPostItTextItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}



SvxPostItTextItem::SvxPostItTextItem( const OUString& rText, sal_uInt16 _nWhich ) :

    SfxStringItem( _nWhich, rText )
{
}



bool SvxPostItTextItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return true;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_TEXT_COMPLETE) + GetValue();
            return true;
        default: ;//prevent warning
    }
    return false;
}



SfxPoolItem* SvxPostItTextItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItTextItem( *this );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
