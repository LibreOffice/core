/*************************************************************************
 *
 *  $RCSfile: tfrmitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:19:17 $
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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>

#include "tfrmitem.hxx"

DBG_NAME( SfxTargetFrameItem );
TYPEINIT1( SfxTargetFrameItem, SfxPoolItem );

// -----------------------------------------------------------------------

SfxTargetFrameItem::SfxTargetFrameItem( USHORT nWhich ) :
    SfxPoolItem( nWhich )
{
    DBG_CTOR( SfxTargetFrameItem, 0 );
}

// -----------------------------------------------------------------------

SfxTargetFrameItem::SfxTargetFrameItem( const SfxTargetFrameItem& rItem ) :
    SfxPoolItem( rItem )
{
    DBG_CTOR( SfxTargetFrameItem, 0 );
    for( USHORT nCur = 0; nCur <= (USHORT)SfxOpenModeLast; nCur++ )
        _aFrames[nCur] = rItem._aFrames[nCur];
}

// -----------------------------------------------------------------------

SfxTargetFrameItem::SfxTargetFrameItem( USHORT nWhich,
    const String& rOpenSelectFrame, const String& rOpenOpenFrame,
    const String& rOpenAddTaskFrame ) : SfxPoolItem( nWhich )
{
    DBG_CTOR( SfxTargetFrameItem, 0 );
    _aFrames[ (USHORT)SfxOpenSelect ]  = rOpenSelectFrame;
    _aFrames[ (USHORT)SfxOpenOpen ]    = rOpenOpenFrame;
    _aFrames[ (USHORT)SfxOpenAddTask ] = rOpenAddTaskFrame;
}

// -----------------------------------------------------------------------

SfxTargetFrameItem::~SfxTargetFrameItem()
{
    DBG_DTOR(SfxTargetFrameItem, 0);
}

// -----------------------------------------------------------------------

String SfxTargetFrameItem::GetTargetFrame( SfxOpenMode eMode ) const
{
    DBG_CHKTHIS( SfxTargetFrameItem, 0 );
    if( eMode <= SfxOpenModeLast )
        return _aFrames[ (USHORT)eMode ];
    String aResult;
    return aResult;
}

// -----------------------------------------------------------------------

int SfxTargetFrameItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS( SfxTargetFrameItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    for( USHORT nCur = 0; nCur <= (USHORT)SfxOpenModeLast; nCur++ )
    {
        if( _aFrames[nCur] != ((const SfxTargetFrameItem&)rItem)._aFrames[nCur] )
            return 0;
    }
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxTargetFrameItem::Create( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxTargetFrameItem, 0);
    SfxTargetFrameItem* pItem = new SfxTargetFrameItem( Which() );
    USHORT nCount = 0;
    rStream >> nCount;
    for(USHORT nCur=0; nCur<= (USHORT)SfxOpenModeLast && nCount; nCur++,nCount--)
    {
        readByteString(rStream, pItem->_aFrames[ nCur ]);
    }
    // die uebriggebliebenen ueberspringen
    String aTemp;
    while( nCount )
    {
        readByteString(rStream, aTemp);
        nCount--;
    }
    return pItem;
}

// -----------------------------------------------------------------------

SvStream& SfxTargetFrameItem::Store( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS( SfxTargetFrameItem, 0 );
    USHORT nCount = (USHORT)(SfxOpenModeLast+1);
    rStream << nCount;
    for( USHORT nCur = 0; nCur <= (USHORT)SfxOpenModeLast; nCur++ )
    {
        writeByteString(rStream, _aFrames[ nCur ]);
    }
    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxTargetFrameItem::Clone( SfxItemPool* ) const
{
    DBG_CHKTHIS( SfxTargetFrameItem, 0 );
    return new SfxTargetFrameItem( *this );
}

// -----------------------------------------------------------------------
// virtual
BOOL SfxTargetFrameItem::QueryValue( com::sun::star::uno::Any& rVal,
                                      BYTE nMemberId ) const
{
    String aVal;
    for ( int i = 0; i <= SfxOpenModeLast; i++ )
    {
        aVal += _aFrames[ i ];
        aVal += ';' ;
    }

    rVal <<= rtl::OUString( aVal );
    return TRUE;
}

// -----------------------------------------------------------------------
// virtual
BOOL SfxTargetFrameItem::PutValue( const com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId )
{
    rtl::OUString aValue;
    if ( rVal >>= aValue )
    {
        const String aVal( aValue );

        for ( int i = 0; i <= SfxOpenModeLast; i++ )
            _aFrames[ i ] = aVal.GetToken( i );

        return TRUE;
    }

    DBG_ERROR( "SfxTargetFrameItem::PutValue - Wrong type!" );
    return FALSE;
}

