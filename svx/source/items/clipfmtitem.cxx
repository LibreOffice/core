/*************************************************************************
 *
 *  $RCSfile: clipfmtitem.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-29 18:11:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGSDTOR

#include <svtools/svstdarr.hxx>
#include <clipfmtitem.hxx>



struct SvxClipboardFmtItem_Impl
{
    SvStringsDtor aFmtNms;
    SvULongs aFmtIds;
    static String sEmptyStr;

    SvxClipboardFmtItem_Impl() : aFmtNms( 8, 8 ), aFmtIds( 8, 8 ) {}
    SvxClipboardFmtItem_Impl( const SvxClipboardFmtItem_Impl& );
};

String SvxClipboardFmtItem_Impl::sEmptyStr;

TYPEINIT1( SvxClipboardFmtItem, SfxPoolItem );

SvxClipboardFmtItem_Impl::SvxClipboardFmtItem_Impl(
                            const SvxClipboardFmtItem_Impl& rCpy )
{
    aFmtIds.Insert( &rCpy.aFmtIds, 0 );
    for( USHORT n = 0, nEnd = rCpy.aFmtNms.Count(); n < nEnd; ++n )
    {
        String* pStr = rCpy.aFmtNms[ n ];
        if( pStr )
            pStr = new String( *pStr );
        aFmtNms.Insert( pStr, n );
    }
}

SvxClipboardFmtItem::SvxClipboardFmtItem( USHORT nId )
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

int SvxClipboardFmtItem::operator==( const SfxPoolItem& rComp ) const
{
    int nRet = 0;
    const SvxClipboardFmtItem& rCmp = (SvxClipboardFmtItem&)rComp;
    if( rCmp.pImpl->aFmtNms.Count() == pImpl->aFmtNms.Count() )
    {
        nRet = 1;
        const String* pStr1, *pStr2;
        for( USHORT n = 0, nEnd = rCmp.pImpl->aFmtNms.Count(); n < nEnd; ++n )
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

SfxPoolItem* SvxClipboardFmtItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxClipboardFmtItem( *this );
}

void SvxClipboardFmtItem::AddClipbrdFormat( ULONG nId, USHORT nPos )
{
    if( nPos > pImpl->aFmtNms.Count() )
        nPos = pImpl->aFmtNms.Count();
    String* pStr = 0;
    pImpl->aFmtNms.Insert( pStr, nPos );
    pImpl->aFmtIds.Insert( nId, nPos );
}

void SvxClipboardFmtItem::AddClipbrdFormat( ULONG nId, const String& rName,
                            USHORT nPos )
{
    if( nPos > pImpl->aFmtNms.Count() )
        nPos = pImpl->aFmtNms.Count();
    String* pStr = new String( rName );
    pImpl->aFmtNms.Insert( pStr, nPos );
    pImpl->aFmtIds.Insert( nId, nPos );
}

USHORT SvxClipboardFmtItem::Count() const
{
    return pImpl->aFmtIds.Count();
}

ULONG SvxClipboardFmtItem::GetClipbrdFormatId( USHORT nPos ) const
{
    return pImpl->aFmtIds[ nPos ];
}

const String& SvxClipboardFmtItem::GetClipbrdFormatName( USHORT nPos ) const
{
    const String* pS = pImpl->aFmtNms[ nPos ];
    return pS ? *pS : SvxClipboardFmtItem_Impl::sEmptyStr;
}


