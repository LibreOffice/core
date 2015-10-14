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

TYPEINIT1_AUTOFACTORY(SfxObjectShellItem,SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SfxObjectItem,SfxPoolItem)

bool SfxObjectShellItem::operator==( const SfxPoolItem &rItem ) const
{
     return dynamic_cast<const SfxObjectShellItem&>(rItem).pObjSh == pObjSh;
}

SfxPoolItem* SfxObjectShellItem::Clone( SfxItemPool *) const
{
    return new SfxObjectShellItem( Which(), pObjSh );
}

bool SfxObjectShellItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    if ( pObjSh )
    {
        // This item MUST provide a model. Please don't change this, there are UNO-based
        // implementations which need it!!
        rVal <<= pObjSh->GetModel();
    }
    else
    {
        rVal <<= ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >();
    }
    return true;
}

bool SfxObjectShellItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // This item MUST have a model. Please don't change this, there are UNO-based
    // implementations which need it!!
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel;

    if ( rVal >>= xModel )
    {
        if ( xModel.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(
                xModel, ::com::sun::star::uno::UNO_QUERY );
            if ( xTunnel.is() )
            {
                ::com::sun::star::uno::Sequence < sal_Int8 > aSeq = SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence();
                sal_Int64 nHandle = xTunnel->getSomething( aSeq );
                if ( nHandle )
                {
                    pObjSh = reinterpret_cast< SfxObjectShell* >(sal::static_int_cast<sal_IntPtr>( nHandle ));
                    return true;
                }
            }
        }

        pObjSh = 0;
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
     const SfxObjectItem *pOther = dynamic_cast<const SfxObjectItem*>( &rItem );
     return pOther->_pSh == _pSh;
}



SfxPoolItem* SfxObjectItem::Clone( SfxItemPool *) const
{
    return new SfxObjectItem( Which(), _pSh );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
