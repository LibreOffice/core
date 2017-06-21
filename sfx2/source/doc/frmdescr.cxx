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

#include <tools/stream.hxx>
#include <vcl/splitwin.hxx>
#include <svl/itemset.hxx>

#include <sfx2/frmdescr.hxx>
#include <sfx2/app.hxx>

struct SfxFrameDescriptor_Impl
{
    Wallpaper*  pWallpaper;
    SfxItemSet* pArgs;
    bool        bEditable;

    SfxFrameDescriptor_Impl() : pWallpaper( nullptr ), pArgs( nullptr ), bEditable( true ) {}
    ~SfxFrameDescriptor_Impl()
    {
        delete pWallpaper;
        delete pArgs;
    }
};

SfxFrameDescriptor::SfxFrameDescriptor() :
    aMargin( -1, -1 ),
    eScroll( ScrollingMode::Auto ),
    bHasBorder( true ),
    bHasBorderSet( false ),
    bResizeVertical( true ),
    pImpl( new SfxFrameDescriptor_Impl )
{
}

SfxFrameDescriptor::~SfxFrameDescriptor()
{
}

SfxItemSet* SfxFrameDescriptor::GetArgs()
{
    if( !pImpl->pArgs )
        pImpl->pArgs = new SfxAllItemSet( SfxGetpApp()->GetPool() );
    return pImpl->pArgs;
}

void SfxFrameDescriptor::SetURL( const OUString& rURL )
{
    aURL = INetURLObject(rURL);
    SetActualURL(aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ));
}

void SfxFrameDescriptor::SetActualURL( const OUString& rURL )
{
    aActualURL = INetURLObject(rURL);
    if ( pImpl->pArgs )
        pImpl->pArgs->ClearItem();
}

void SfxFrameDescriptor::SetEditable( bool bSet )
{
    pImpl->bEditable = bSet;
}

void SfxFrameDescriptor::SetWallpaper( const Wallpaper& rWallpaper )
{
    DELETEZ( pImpl->pWallpaper );

    if ( rWallpaper.GetStyle() != WallpaperStyle::NONE )
        pImpl->pWallpaper = new Wallpaper( rWallpaper );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
