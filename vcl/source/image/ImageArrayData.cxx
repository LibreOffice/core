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
#include <vcl/svapp.hxx>
#include <vcl/ImageTree.hxx>

#include <image.h>
#include <memory>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

#include <vcl/BitmapProcessor.hxx>

ImageAryData::ImageAryData( const ImageAryData& rData ) :
    maName( rData.maName ),
    mnId( rData.mnId ),
    maBitmapEx( rData.maBitmapEx )
{
}

ImageAryData::ImageAryData( const OUString &aName,
                            sal_uInt16 nId, const BitmapEx &aBitmap )
        : maName( aName ), mnId( nId ), maBitmapEx( aBitmap )
{
}

ImageAryData::~ImageAryData()
{
}

ImageAryData& ImageAryData::operator=( const ImageAryData& rData )
{
    maName = rData.maName;
    mnId = rData.mnId;
    maBitmapEx = rData.maBitmapEx;

    return *this;
}

void ImageAryData::Load(const OUString &rPrefix)
{
    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

    OUString aFileName = rPrefix;
    aFileName += maName;

    bool bSuccess = ImageTree::get().loadImage(aFileName, aIconTheme, maBitmapEx, true);

    if (bSuccess)
    {}
#if OSL_DEBUG_LEVEL > 0
    else
    {
        OStringBuffer aMessage;
        aMessage.append( "ImageAryData::Load: failed to load image '" );
        aMessage.append( OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8 ).getStr() );
        aMessage.append( "'" );
        aMessage.append( " from icon theme '" );
        aMessage.append( OUStringToOString( aIconTheme, RTL_TEXTENCODING_UTF8 ).getStr() );
        aMessage.append( "'" );
        OSL_FAIL( aMessage.makeStringAndClear().getStr() );
    }
#endif
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
