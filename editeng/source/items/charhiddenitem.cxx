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

TYPEINIT1_FACTORY(SvxCharHiddenItem, SfxBoolItem, new SvxCharHiddenItem(sal_False, 0));

SvxCharHiddenItem::SvxCharHiddenItem( const sal_Bool bHidden, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bHidden )
{
}

SfxPoolItem* SvxCharHiddenItem::Clone( SfxItemPool * ) const
{
    return new SvxCharHiddenItem( *this );
}

SfxItemPresentation SvxCharHiddenItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_CHARHIDDEN_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_CHARHIDDEN_TRUE;
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
