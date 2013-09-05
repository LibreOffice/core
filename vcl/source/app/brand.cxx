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

#include <config_folders.h>

#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/pngread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/svgdata.hxx>

namespace {
    static bool loadPng( const OUString & rPath, BitmapEx &rBitmap)
    {
        INetURLObject aObj( rPath );
        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
        if ( !aStrm.GetError() ) {
            vcl::PNGReader aReader( aStrm );
            rBitmap = aReader.Read();
            return !rBitmap.IsEmpty();
        }
        else
            return false;
    }
    static bool tryLoadPng( const OUString& rBaseDir, const OUString& rName, BitmapEx& rBitmap )
    {
        return loadPng( rBaseDir + "/" LIBO_ETC_FOLDER + rName, rBitmap);
    }
}

bool Application::LoadBrandBitmap (const char* pName, BitmapEx &rBitmap)
{
    // TODO - if we want more flexibility we could add a branding path
    // in an rc file perhaps fallback to "about.bmp"
    OUString aBaseDir( "$BRAND_BASE_DIR");
    rtl::Bootstrap::expandMacros( aBaseDir );
    OUString aBaseName( "/" + OUString::createFromAscii( pName ) );
    OUString aPng( ".png" );

    rtl_Locale *pLoc = NULL;
    osl_getProcessLocale (&pLoc);
    LanguageTag aLanguageTag( *pLoc);

    ::std::vector< OUString > aFallbacks( aLanguageTag.getFallbackStrings( true));
    for (size_t i=0; i < aFallbacks.size(); ++i)
    {
        if (tryLoadPng( aBaseDir, aBaseName + "-" + aFallbacks[i] + aPng, rBitmap))
            return true;
    }

    if (tryLoadPng( aBaseDir, aBaseName + aPng, rBitmap))
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
