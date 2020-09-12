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


#include <sfx2/objsh.hxx>
#include <sfx2/objitem.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XModel.hpp>


SfxPoolItem* SfxObjectShellItem::CreateDefault() { return new SfxObjectShellItem; }

SfxPoolItem* SfxObjectItem::CreateDefault() { return new SfxObjectItem; }

bool SfxObjectShellItem::operator==( const SfxPoolItem &rItem ) const
{
     return SfxPoolItem::operator==(rItem) &&
        static_cast<const SfxObjectShellItem&>(rItem).pObjSh == pObjSh;
}

SfxObjectShellItem* SfxObjectShellItem::Clone( SfxItemPool *) const
{
    return new SfxObjectShellItem( *this );
}

bool SfxObjectShellItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    if ( pObjSh )
    {
        // This item MUST provide a model. Please don't change this, there are UNO-based
        // implementations which need it!!
        rVal <<= pObjSh->GetModel();
    }
    else
    {
        rVal <<= css::uno::Reference< css::frame::XModel >();
    }
    return true;
}

bool SfxObjectShellItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This item MUST have a model. Please don't change this, there are UNO-based
    // implementations which need it!!
    css::uno::Reference< css::frame::XModel > xModel;

    if ( rVal >>= xModel )
    {
        pObjSh = SfxObjectShell::GetShellFromComponent(xModel);
        return true;
    }

    return true;
}

SfxObjectItem::SfxObjectItem( sal_uInt16 nWhichId, SfxShell *pSh )
:   SfxPoolItem( nWhichId ),
    _pSh( pSh )
{}

bool SfxObjectItem::operator==( const SfxPoolItem &rItem ) const
{
     return SfxPoolItem::operator==(rItem) &&
        static_cast<const SfxObjectItem&>(rItem)._pSh == _pSh;
}

SfxObjectItem* SfxObjectItem::Clone( SfxItemPool *) const
{
    return new SfxObjectItem( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
