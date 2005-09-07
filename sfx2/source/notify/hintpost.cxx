/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hintpost.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:20:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef GCC
#pragma hdrstop
#endif

#include "arrdecl.hxx"
#include "hintpost.hxx"
#include "app.hxx"
#include "sfxtypes.hxx"

//====================================================================

void SfxHintPoster::RegisterEvent()
{
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

SfxHintPoster::SfxHintPoster()
{
    RegisterEvent();
}

//--------------------------------------------------------------------

SfxHintPoster::SfxHintPoster( const GenLink& rLink ):
    aLink(rLink)
{
}


//--------------------------------------------------------------------

SfxHintPoster::~SfxHintPoster()
{
}

//--------------------------------------------------------------------

void SfxHintPoster::Post( SfxHint* pHintToPost )
{
    GetpApp()->PostUserEvent( ( LINK(this, SfxHintPoster, DoEvent_Impl) ), pHintToPost );
    AddRef();
}

//--------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxHintPoster, DoEvent_Impl, SfxHint *, pPostedHint )
{
    DBG_MEMTEST();
    Event( pPostedHint );
    ReleaseRef();
    return 0;
}
IMPL_LINK_INLINE_END( SfxHintPoster, DoEvent_Impl, SfxHint *, pPostedHint )

//--------------------------------------------------------------------

void SfxHintPoster::Event( SfxHint* pPostedHint )
{
    aLink.Call( pPostedHint );
}

//--------------------------------------------------------------------

void SfxHintPoster::SetEventHdl( const GenLink& rLink )
{
    DBG_MEMTEST();
    aLink = rLink;
}


#define LOG( x )
#if 0
#define LOG( x )                                                \
{                                                               \
    SvFileStream aStrm( "f:\\temp\\log", STREAM_READWRITE );    \
    aStrm.Seek( STREAM_SEEK_TO_END );                           \
    aStrm << x.GetStr() << '\n';                                \
}
#endif
