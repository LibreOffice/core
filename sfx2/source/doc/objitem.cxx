/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif

#include <sfx2/objsh.hxx>
//#include "objshimp.hxx"
#include <sfx2/objitem.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>

//====================================================================

TYPEINIT1_AUTOFACTORY(SfxObjectShellItem,SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SfxObjectItem,SfxPoolItem)

//=========================================================================

int SfxObjectShellItem::operator==( const SfxPoolItem &rItem ) const
{
     return PTR_CAST(SfxObjectShellItem, &rItem)->pObjSh == pObjSh;
}

//--------------------------------------------------------------------

String SfxObjectShellItem::GetValueText() const
{
    return String();
}

//--------------------------------------------------------------------

SfxPoolItem* SfxObjectShellItem::Clone( SfxItemPool *) const
{
    return new SfxObjectShellItem( Which(), pObjSh );
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShellItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
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
    return sal_True;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShellItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
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
                ::com::sun::star::uno::Sequence < sal_Int8 > aSeq( (sal_Int8*) SvGlobalName( SFX_GLOBAL_CLASSID ).GetBytes(), 16 );
                sal_Int64 nHandle = xTunnel->getSomething( aSeq );
                if ( nHandle )
                {
                    pObjSh = reinterpret_cast< SfxObjectShell* >(sal::static_int_cast<sal_IntPtr>( nHandle ));
                    return sal_True;
                }
            }
        }

        pObjSh = 0;
        return sal_True;
    }

    return sal_False;
}

//=========================================================================

SfxObjectItem::SfxObjectItem( sal_uInt16 nWhichId, SfxShell *pSh )
:   SfxPoolItem( nWhichId ),
    _pSh( pSh )
{}

//--------------------------------------------------------------------

int SfxObjectItem::operator==( const SfxPoolItem &rItem ) const
{
     SfxObjectItem *pOther = PTR_CAST(SfxObjectItem, &rItem);
     return pOther->_pSh == _pSh;
}

//--------------------------------------------------------------------

SfxPoolItem* SfxObjectItem::Clone( SfxItemPool *) const
{
    return new SfxObjectItem( Which(), _pSh );
}
