/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/temporaryfonts.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

void TemporaryFonts::clear()
{
    OUString path = "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( path );
    path += "/user/temp/fonts/";
    osl::Directory dir( path );
    dir.reset();
    for(;;)
    {
        osl::DirectoryItem item;
        if( dir.getNextItem( item ) != osl::Directory::E_None )
            break;
        osl::FileStatus status( osl_FileStatus_Mask_FileURL );
        if( item.getFileStatus( status ) == osl::File::E_None )
            osl::File::remove( status.getFileURL());
    }
}

OUString TemporaryFonts::fileUrlForFont( const OUString& fontName, const char* fontStyle )
{
    OUString path = "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( path );
    path += "/user/temp/fonts/";
    osl::Directory::createPath( path );
    OUString filename = fontName;
    filename += OStringToOUString( fontStyle, RTL_TEXTENCODING_ASCII_US );
    filename += ".ttf"; // TODO is it always ttf?
    return path + filename;
}

void TemporaryFonts::activateFont( const OUString& fontName, const OUString& fileUrl )
{
    OutputDevice *pDevice = Application::GetDefaultDevice();
    pDevice->AddTempDevFont( fileUrl, fontName );
    pDevice->ImplUpdateAllFontData( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
