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

#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGSDTOR

#include <svl/svstdarr.hxx>
#include <svx/clipfmtitem.hxx>
#include <com/sun/star/frame/status/ClipboardFormats.hpp>

struct SvxClipboardFmtItem_Impl
{
    SvStringsDtor aFmtNms;
    SvULongs aFmtIds;
    static String sEmptyStr;

    SvxClipboardFmtItem_Impl() : aFmtNms( 8, 8 ), aFmtIds( 8, 8 ) {}
    SvxClipboardFmtItem_Impl( const SvxClipboardFmtItem_Impl& );
};

String SvxClipboardFmtItem_Impl::sEmptyStr;

SvxClipboardFmtItem_Impl::SvxClipboardFmtItem_Impl(
                            const SvxClipboardFmtItem_Impl& rCpy )
{
    aFmtIds.Insert( &rCpy.aFmtIds, 0 );
    for( sal_uInt16 n = 0, nEnd = rCpy.aFmtNms.Count(); n < nEnd; ++n )
    {
        String* pStr = rCpy.aFmtNms[ n ];
        if( pStr )
            pStr = new String( *pStr );
        aFmtNms.Insert( pStr, n );
    }
}

IMPL_POOLITEM_FACTORY(SvxClipboardFmtItem)
SvxClipboardFmtItem::SvxClipboardFmtItem( sal_uInt16 nId )
    : SfxPoolItem( nId ), pImpl( new SvxClipboardFmtItem_Impl )
{
}

SvxClipboardFmtItem::SvxClipboardFmtItem( const SvxClipboardFmtItem& rCpy )
    : SfxPoolItem( rCpy.Which() ),
    pImpl( new SvxClipboardFmtItem_Impl( *rCpy.pImpl ) )
{
}

SvxClipboardFmtItem::~SvxClipboardFmtItem()
{
    delete pImpl;
}

sal_Bool SvxClipboardFmtItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_uInt16 nCount = Count();

    ::com::sun::star::frame::status::ClipboardFormats aClipFormats;

    aClipFormats.Identifiers.realloc( nCount );
    aClipFormats.Names.realloc( nCount );
    for ( sal_uInt16 n=0; n < nCount; n++ )
    {
        aClipFormats.Identifiers[n] = (sal_Int64)GetClipbrdFormatId( n );
        aClipFormats.Names[n] = GetClipbrdFormatName( n );
    }

    rVal <<= aClipFormats;
    return sal_True;
}

sal_Bool SvxClipboardFmtItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    ::com::sun::star::frame::status::ClipboardFormats aClipFormats;
    if ( rVal >>= aClipFormats )
    {
        sal_uInt16 nCount = sal_uInt16( aClipFormats.Identifiers.getLength() );

        pImpl->aFmtIds.Remove( 0, pImpl->aFmtIds.Count() );
        pImpl->aFmtNms.Remove( 0, pImpl->aFmtNms.Count() );
        for ( sal_uInt16 n=0; n < nCount; n++ )
            AddClipbrdFormat( sal_uIntPtr( aClipFormats.Identifiers[n] ), aClipFormats.Names[n], n );

        return sal_True;
    }

    return sal_False;
}

int SvxClipboardFmtItem::operator==( const SfxPoolItem& rComp ) const
{
    int nRet = 0;
    const SvxClipboardFmtItem& rCmp = (SvxClipboardFmtItem&)rComp;
    if( rCmp.pImpl->aFmtNms.Count() == pImpl->aFmtNms.Count() )
    {
        nRet = 1;
        const String* pStr1, *pStr2;
        for( sal_uInt16 n = 0, nEnd = rCmp.pImpl->aFmtNms.Count(); n < nEnd; ++n )
        {
            if( pImpl->aFmtIds[ n ] != rCmp.pImpl->aFmtIds[ n ] ||
                ( (0 == ( pStr1 = pImpl->aFmtNms[ n ] )) ^
                  (0 == ( pStr2 = rCmp.pImpl->aFmtNms[ n ] ) )) ||
                ( pStr1 && *pStr1 != *pStr2 ))
            {
                nRet = 0;
                break;
            }
        }
    }
    return nRet;
}

SfxPoolItem* SvxClipboardFmtItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxClipboardFmtItem( *this );
}

void SvxClipboardFmtItem::AddClipbrdFormat( sal_uIntPtr nId, sal_uInt16 nPos )
{
    if( nPos > pImpl->aFmtNms.Count() )
        nPos = pImpl->aFmtNms.Count();
    String* pStr = 0;
    pImpl->aFmtNms.Insert( pStr, nPos );
    pImpl->aFmtIds.Insert( nId, nPos );
}

void SvxClipboardFmtItem::AddClipbrdFormat( sal_uIntPtr nId, const String& rName,
                            sal_uInt16 nPos )
{
    if( nPos > pImpl->aFmtNms.Count() )
        nPos = pImpl->aFmtNms.Count();
    String* pStr = new String( rName );
    pImpl->aFmtNms.Insert( pStr, nPos );
    pImpl->aFmtIds.Insert( nId, nPos );
}

sal_uInt16 SvxClipboardFmtItem::Count() const
{
    return pImpl->aFmtIds.Count();
}

sal_uIntPtr SvxClipboardFmtItem::GetClipbrdFormatId( sal_uInt16 nPos ) const
{
    return pImpl->aFmtIds[ nPos ];
}

const String& SvxClipboardFmtItem::GetClipbrdFormatName( sal_uInt16 nPos ) const
{
    const String* pS = pImpl->aFmtNms[ nPos ];
    return pS ? *pS : SvxClipboardFmtItem_Impl::sEmptyStr;
}


