/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "arrdecl.hxx"
#include <sfx2/hintpost.hxx>
#include <sfx2/app.hxx>
#include "sfxtypes.hxx"

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
    aLink = rLink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
