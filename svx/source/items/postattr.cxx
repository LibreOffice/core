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

#include <svx/postattr.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

SfxPoolItem* SvxPostItAuthorItem::CreateDefault() { return new SvxPostItAuthorItem(TypedWhichId<SvxPostItAuthorItem>(0)); }
SfxPoolItem* SvxPostItDateItem::CreateDefault() { return new SvxPostItDateItem(TypedWhichId<SvxPostItDateItem>(0)); }
SfxPoolItem* SvxPostItTextItem::CreateDefault() { return new SvxPostItTextItem(TypedWhichId<SvxPostItTextItem>(0)); }
SfxPoolItem* SvxPostItIdItem::CreateDefault() { return new SvxPostItIdItem(TypedWhichId<SvxPostItIdItem>(0)); }

SvxPostItAuthorItem::SvxPostItAuthorItem( TypedWhichId<SvxPostItAuthorItem> _nWhich )
{
    SetWhich( _nWhich );
}


SvxPostItAuthorItem::SvxPostItAuthorItem( const OUString& rAuthor,
                                          TypedWhichId<SvxPostItAuthorItem> _nWhich ) :
    SfxStringItem( _nWhich, rAuthor )
{
}


bool SvxPostItAuthorItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = GetValue();
            return true;
        case SfxItemPresentation::Complete:
            rText = SvxResId(RID_SVXITEMS_AUTHOR_COMPLETE) + GetValue();
            return true;
        default: ;//prevent warning
    }
    return false;
}

SvxPostItAuthorItem* SvxPostItAuthorItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItAuthorItem( *this );
}

SvxPostItDateItem::SvxPostItDateItem( TypedWhichId<SvxPostItDateItem> _nWhich )
    : SfxStringItem(_nWhich)
{
}


SvxPostItDateItem::SvxPostItDateItem( const OUString& rDate, TypedWhichId<SvxPostItDateItem> _nWhich ) :
    SfxStringItem( _nWhich, rDate )
{
}


bool SvxPostItDateItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = GetValue();
            return true;
        case SfxItemPresentation::Complete:
            rText = SvxResId(RID_SVXITEMS_DATE_COMPLETE) + GetValue();
            return true;
        default: ;//prevent warning
    }
    return false;
}


SvxPostItDateItem* SvxPostItDateItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItDateItem( *this );
}

SvxPostItTextItem::SvxPostItTextItem( TypedWhichId<SvxPostItTextItem> _nWhich )
{
    SetWhich( _nWhich );
}

SvxPostItTextItem::SvxPostItTextItem( const OUString& rText, TypedWhichId<SvxPostItTextItem> _nWhich ) :

    SfxStringItem( _nWhich, rText )
{
}

bool SvxPostItTextItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = GetValue();
            return true;
        case SfxItemPresentation::Complete:
            rText = SvxResId(RID_SVXITEMS_TEXT_COMPLETE) + GetValue();
            return true;
        default: ;//prevent warning
    }
    return false;
}

SvxPostItTextItem* SvxPostItTextItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItTextItem( *this );
}

SvxPostItIdItem::SvxPostItIdItem( TypedWhichId<SvxPostItIdItem> _nWhich )
{
    SetWhich( _nWhich );
}

SvxPostItIdItem* SvxPostItIdItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItIdItem( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
