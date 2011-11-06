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

#include "arrdecl.hxx"
#include <sfx2/hintpost.hxx>
#include <sfx2/app.hxx>
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
