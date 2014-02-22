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


#include "arrdecl.hxx"
#include <sfx2/hintpost.hxx>
#include <sfx2/app.hxx>
#include "sfxtypes.hxx"



SfxHintPoster::SfxHintPoster( const GenLink& rLink ):
    aLink(rLink)
{
}




SfxHintPoster::~SfxHintPoster()
{
}



void SfxHintPoster::Post( SfxHint* pHintToPost )
{
    GetpApp()->PostUserEvent( ( LINK(this, SfxHintPoster, DoEvent_Impl) ), pHintToPost );
    AddRef();
}



IMPL_LINK_INLINE_START( SfxHintPoster, DoEvent_Impl, SfxHint *, pPostedHint )
{
    Event( pPostedHint );
    ReleaseRef();
    return 0;
}
IMPL_LINK_INLINE_END( SfxHintPoster, DoEvent_Impl, SfxHint *, pPostedHint )



void SfxHintPoster::Event( SfxHint* pPostedHint )
{
    aLink.Call( pPostedHint );
}



void SfxHintPoster::SetEventHdl( const GenLink& rLink )
{
    aLink = rLink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
