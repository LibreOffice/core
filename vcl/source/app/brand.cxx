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

#include <sal/config.h>

#include <string_view>

#include <config_folders.h>

#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>

namespace {
    bool loadPng( std::u16string_view rPath, BitmapEx &rBitmap)
    {
        INetURLObject aObj( rPath );
        SvFileStream aStrm( aObj.PathToFileName(), StreamMode::STD_READ );
        if ( !aStrm.GetError() ) {
            vcl::PngImageReader aReader( aStrm );
            rBitmap = aReader.read();
            return !rBitmap.IsEmpty();
        }
        else
            return false;
    }
    bool tryLoadPng( std::u16string_view rBaseDir, std::u16string_view rName, BitmapEx& rBitmap )
    {
        return loadPng( rtl::Concat2View(OUString::Concat(rBaseDir) + "/" LIBO_ETC_FOLDER + rName), rBitmap);
    }
}

bool Application::LoadBrandBitmap (std::u16string_view pName, BitmapEx &rBitmap)
{
    // TODO - if we want more flexibility we could add a branding path
    // in an rc file perhaps fallback to "about.bmp"
    OUString aBaseDir( u"$BRAND_BASE_DIR"_ustr);
    rtl::Bootstrap::expandMacros( aBaseDir );
    OUString aBaseName(OUStringChar('/') + pName);
    OUString aPng( u".png"_ustr );

    rtl_Locale *pLoc = nullptr;
    osl_getProcessLocale (&pLoc);
    LanguageTag aLanguageTag( *pLoc);

    ::std::vector< OUString > aFallbacks( aLanguageTag.getFallbackStrings( true));
    for (const OUString & aFallback : aFallbacks)
    {
        if (tryLoadPng( aBaseDir, Concat2View(aBaseName + "-" + aFallback + aPng), rBitmap))
            return true;
    }

    return tryLoadPng( aBaseDir, Concat2View(aBaseName + aPng), rBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
