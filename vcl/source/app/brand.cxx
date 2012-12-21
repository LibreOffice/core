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

#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/pngread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/svgdata.hxx>

namespace {
    static bool loadPng(const char *pPath, const rtl::OUString &rName, BitmapEx &rBitmap)
    {
        rtl::OUString uri = rtl::OUString::createFromAscii( pPath ) + rName;
        rtl::Bootstrap::expandMacros( uri );
        INetURLObject aObj( uri );
        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
        if ( !aStrm.GetError() ) {
            vcl::PNGReader aReader( aStrm );
            rBitmap = aReader.Read();
            return !rBitmap.IsEmpty();
        }
        else
            return false;
    }
    static bool loadSvg(rtl::OUString aUri, BitmapEx &rBitmap)
    {
        rtl::Bootstrap::expandMacros( aUri );
        INetURLObject aObj( aUri );
        SvgData aSvgData(aObj.PathToFileName());
        rBitmap = aSvgData.getReplacement();
        return !rBitmap.IsEmpty();
    }
}

bool Application::LoadBrandBitmap (const char* pName, BitmapEx &rBitmap)
{
    // TODO - if we want more flexibility we could add a branding path
    // in an rc file perhaps fallback to "about.bmp"
    rtl::OUString aBaseName = ( rtl::OUString("/") +
                                rtl::OUString::createFromAscii( pName ) );
    rtl::OUString aPng( ".png" );

    rtl_Locale *pLoc = NULL;
    osl_getProcessLocale (&pLoc);
    LanguageTag aLanguageTag( *pLoc);

    rtl::OUString aName = aBaseName + aPng;
    rtl::OUString aLocaleName = ( aBaseName + rtl::OUString("-") +
                                  aLanguageTag.getBcp47() +
                                  aPng );

    return ( loadPng ("$BRAND_BASE_DIR/program/edition", aLocaleName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program", aLocaleName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program/edition", aName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program", aName, rBitmap) );
}

bool Application::LoadBrandSVG (const char *pName, BitmapEx &rBitmap)
{
    rtl::OUString aBaseName = ( rtl::OUString("/") +
                                rtl::OUString::createFromAscii( pName ) );
    rtl::OUString aSvg( ".svg" );

    rtl_Locale *pLoc = NULL;
    osl_getProcessLocale (&pLoc);
    LanguageTag aLanguageTag( *pLoc);

    rtl::OUString aName = aBaseName + aSvg;
    rtl::OUString aLocaleName = ( aBaseName + rtl::OUString("-") +
                                  aLanguageTag.getBcp47() +
                                  aSvg );
    rtl::OUString uriOpt = rtl::OUString::createFromAscii( "$BRAND_BASE_DIR/program/edition" ) + aLocaleName;
    rtl::OUString uri = rtl::OUString::createFromAscii( "$BRAND_BASE_DIR/program" ) + aBaseName+aSvg;
    return ( loadSvg( uriOpt, rBitmap ) || loadSvg( uri, rBitmap ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
