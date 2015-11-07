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

#include <editeng/charhiddenitem.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

TYPEINIT1_FACTORY(SvxCharHiddenItem, SfxBoolItem, new SvxCharHiddenItem(false, 0));

SfxPoolItem* SvxCharHiddenItem::CreateDefault() { return new  SvxCharHiddenItem(false, 0);}

SvxCharHiddenItem::SvxCharHiddenItem( const bool bHidden, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bHidden )
{
}

SfxPoolItem* SvxCharHiddenItem::Clone( SfxItemPool * ) const
{
    return new SvxCharHiddenItem( *this );
}

bool SvxCharHiddenItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper * /*pIntl*/
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_CHARHIDDEN_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_CHARHIDDEN_TRUE;
    rText = EE_RESSTR(nId);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
