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

#include <vcl/outdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/alpha.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>

#include "image.h"
#include <memory>

ImplImageList::ImplImageList()
{
}

ImplImageList::ImplImageList( const ImplImageList &aSrc )
    : maPrefix(aSrc.maPrefix)
    , maImageSize(aSrc.maImageSize)
{
    maImages.reserve( aSrc.maImages.size() );
    for ( std::vector<ImageAryData *>::const_iterator aIt = aSrc.maImages.begin(), aEnd = aSrc.maImages.end(); aIt != aEnd; ++aIt )
    {
        ImageAryData* pAryData = new ImageAryData( **aIt );
        maImages.push_back( pAryData );
        if( !pAryData->maName.isEmpty() )
            maNameHash [ pAryData->maName ] = pAryData;
    }
}

ImplImageList::~ImplImageList()
{
    for ( std::vector<ImageAryData *>::iterator aIt = maImages.begin(), aEnd = maImages.end(); aIt != aEnd; ++aIt )
        delete *aIt;
}

void ImplImageList::AddImage( const OUString &aName,
                              sal_uInt16 nId, const BitmapEx &aBitmapEx )
{
    ImageAryData *pImg = new ImageAryData( aName, nId, aBitmapEx );
    maImages.push_back( pImg );
    if( !aName.isEmpty() )
        maNameHash [ aName ] = pImg;
}

void ImplImageList::RemoveImage( sal_uInt16 nPos )
{
    ImageAryData *pImg = maImages[ nPos ];
    if( !pImg->maName.isEmpty() )
        maNameHash.erase( pImg->maName );
    maImages.erase( maImages.begin() + nPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
