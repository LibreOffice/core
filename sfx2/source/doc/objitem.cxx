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

#ifndef GCC
#endif

#include <sfx2/objsh.hxx>
//#include "objshimp.hxx"
#include <sfx2/objitem.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>

//====================================================================
IMPL_POOLITEM_FACTORY(SfxObjectShellItem)

int SfxObjectShellItem::operator==( const SfxPoolItem &rItem ) const
{
     return dynamic_cast< const SfxObjectShellItem* >( &rItem)->pObjSh == pObjSh;
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

IMPL_POOLITEM_FACTORY(SfxObjectItem)
SfxObjectItem::SfxObjectItem( sal_uInt16 nWhichId, SfxShell *pSh )
:   SfxPoolItem( nWhichId ),
    _pSh( pSh )
{}

//--------------------------------------------------------------------

int SfxObjectItem::operator==( const SfxPoolItem &rItem ) const
{
     const SfxObjectItem *pOther = dynamic_cast< const SfxObjectItem* >( &rItem);
     return pOther->_pSh == _pSh;
}

//--------------------------------------------------------------------

SfxPoolItem* SfxObjectItem::Clone( SfxItemPool *) const
{
    return new SfxObjectItem( Which(), _pSh );
}
