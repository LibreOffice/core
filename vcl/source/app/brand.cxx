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

#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/locale.hxx>
#include <osl/process.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/pngread.hxx>
#include <vcl/svgread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/rendergraphicrasterizer.hxx>

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

    static vcl::RenderGraphicRasterizer loadSvg(const char *pPath, const rtl::OUString &rName)
    {
        rtl::OUString uri = rtl::OUString::createFromAscii( pPath ) + rName;
        rtl::Bootstrap::expandMacros( uri );
        INetURLObject aObj( uri );
        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );

        vcl::RenderGraphic aRenderGraphic;
        vcl::RenderGraphic aRasterizer ( aRenderGraphic );

        if ( !aStrm.GetError() ) {
            vcl::SVGReader aSVGReader( aStrm );
            aRenderGraphic = aSVGReader.GetRenderGraphic();
            vcl::RenderGraphic aNewRasterizer ( aRenderGraphic );
            aRasterizer = aNewRasterizer;
        }
        return aRasterizer;
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
    rtl::OLocale aLoc( pLoc );

    rtl::OUString aName = aBaseName + aPng;
    rtl::OUString aLocaleName = ( aBaseName + rtl::OUString("-") +
                                  aLoc.getLanguage() +
                                  rtl::OUString("_") +
                                  aLoc.getCountry() + aPng );

    return ( loadPng ("$BRAND_BASE_DIR/program/edition", aLocaleName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program", aLocaleName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program/edition", aName, rBitmap) ||
             loadPng ("$BRAND_BASE_DIR/program", aName, rBitmap) );
}

vcl::RenderGraphicRasterizer Application::LoadBrandSVG (const char* pName)
{
    rtl::OUString aBaseName = ( rtl::OUString("/") +
                                rtl::OUString::createFromAscii( pName ) );
    rtl::OUString aSvg( ".svg" );

    rtl_Locale *pLoc = NULL;
    osl_getProcessLocale (&pLoc);
    rtl::OLocale aLoc( pLoc );

    rtl::OUString aName = aBaseName + aSvg;
    rtl::OUString aLocaleName = ( aBaseName + rtl::OUString("-") +
                                  aLoc.getLanguage() +
                                  rtl::OUString("_") +
                                  aLoc.getCountry() + aSvg );

    vcl::RenderGraphicRasterizer aRasterizer = loadSvg ("$BRAND_BASE_DIR/program/edition", aLocaleName);
    if (!aRasterizer.GetRenderGraphic().IsEmpty())
        return aRasterizer;
    aRasterizer = loadSvg ("$BRAND_BASE_DIR/program", aLocaleName);
    if (!aRasterizer.GetRenderGraphic().IsEmpty())
        return aRasterizer;
    aRasterizer = loadSvg ("$BRAND_BASE_DIR/program/edition", aName);
    if (!aRasterizer.GetRenderGraphic().IsEmpty())
        return aRasterizer;

    aRasterizer = loadSvg ("$BRAND_BASE_DIR/program", aName);
    return aRasterizer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
