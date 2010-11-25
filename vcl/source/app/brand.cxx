/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *              Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Michael Meeks <michael.meeks@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/locale.hxx>
#include <osl/process.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/pngread.hxx>
#include <vcl/svapp.hxx>

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
}

bool Application::LoadBrandBitmap (const char* pName, BitmapEx &rBitmap)
{
    // TODO - if we want more flexibility we could add a branding path
    // in an rc file perhaps fallback to "about.bmp"
    rtl::OUString aBaseName = ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
                                rtl::OUString::createFromAscii( pName ) );
    rtl::OUString aPng( RTL_CONSTASCII_USTRINGPARAM(".png") );

    rtl_Locale *pLoc = NULL;
    osl_getProcessLocale (&pLoc);
    rtl::OLocale aLoc( pLoc );

    rtl::OUString aName = aBaseName + aPng;
    rtl::OUString aLocaleName = ( aBaseName + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("-")) +
                                  aLoc.getLanguage() +
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("_")) +
                                  aLoc.getCountry() + aPng );

    return ( loadPng ("$BRAND_BASE_DIR/program/edition", aLocaleName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program", aLocaleName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program/edition", aName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program", aName, rBitmap) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
