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

#include <sfx2/tplpitem.hxx>
#include <com/sun/star/frame/status/Template.hpp>

SfxPoolItem* SfxTemplateItem::CreateDefault() { return new SfxTemplateItem; }


SfxTemplateItem::SfxTemplateItem() :
    SfxFlagItem()
{
}

SfxTemplateItem::SfxTemplateItem
(
    sal_uInt16 nWhichId,      // Slot-ID
    const OUString& rStyle,    // Name of the current Styles
    const OUString& rStyleIdentifier     // Prog Name of current Style
) : SfxFlagItem( nWhichId, static_cast<sal_uInt16>(SfxStyleSearchBits::All) ),
    aStyle( rStyle ),
    aStyleIdentifier( rStyleIdentifier )
{
}

// op ==

bool SfxTemplateItem::operator==( const SfxPoolItem& rCmp ) const
{
    return ( SfxFlagItem::operator==( rCmp ) &&
             aStyle == static_cast<const SfxTemplateItem&>(rCmp).aStyle &&
             aStyleIdentifier == static_cast<const SfxTemplateItem&>(rCmp).aStyleIdentifier );
}

SfxTemplateItem* SfxTemplateItem::Clone( SfxItemPool *) const
{
    return new SfxTemplateItem(*this);
}

bool SfxTemplateItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    css::frame::status::Template aTemplate;

    aTemplate.Value = static_cast<sal_uInt16>(GetValue());
    aTemplate.StyleName = aStyle;
    aTemplate.StyleNameIdentifier = aStyleIdentifier;
    rVal <<= aTemplate;

    return true;
}


bool SfxTemplateItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    css::frame::status::Template aTemplate;

    if ( rVal >>= aTemplate )
    {
        SetValue( static_cast<SfxStyleSearchBits>(aTemplate.Value) );
        aStyle = aTemplate.StyleName;
        aStyleIdentifier = aTemplate.StyleNameIdentifier;
        return true;
    }

    return false;
}


sal_uInt8 SfxTemplateItem::GetFlagCount() const
{
    return sizeof(sal_uInt16) * 8;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
