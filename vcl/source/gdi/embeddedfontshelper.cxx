/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/embeddedfontshelper.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#if defined(UNX) && !defined(MACOSX)
#include <vcl/fontmanager.hxx>
#endif

using namespace std;

void EmbeddedFontsHelper::clearTemporaryFontFiles()
{
    OUString path = "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( path );
    path += "/user/temp/embeddedfonts/";
    osl::Directory dir( path );
    if( dir.reset() == osl::Directory::E_None )
    {
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
}

OUString EmbeddedFontsHelper::fileUrlForTemporaryFont( const OUString& fontName, const char* fontStyle )
{
    OUString path = "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( path );
    path += "/user/temp/embeddedfonts/";
    osl::Directory::createPath( path );
    OUString filename = fontName;
    filename += OStringToOUString( fontStyle, RTL_TEXTENCODING_ASCII_US );
    filename += ".ttf"; // TODO is it always ttf?
    return path + filename;
}

void EmbeddedFontsHelper::activateFont( const OUString& fontName, const OUString& fileUrl )
{
    OutputDevice *pDevice = Application::GetDefaultDevice();
    pDevice->AddTempDevFont( fileUrl, fontName );
    pDevice->ImplUpdateAllFontData( true );
}

OUString EmbeddedFontsHelper::fontFileUrl( const OUString& familyName, FontFamily family, FontItalic italic,
    FontWeight weight, FontPitch pitch, rtl_TextEncoding )
{
    OUString url;
#if defined(UNX) && !defined(MACOSX)
    psp::PrintFontManager& mgr = psp::PrintFontManager::get();
    list< psp::fontID > fontIds;
    mgr.getFontList( fontIds );
    for( list< psp::fontID >::const_iterator it = fontIds.begin();
         it != fontIds.end();
         ++it )
    {
        psp::fontID id = *it;
        psp::FastPrintFontInfo info;
        if( !mgr.getFontFastInfo( id, info ))
            continue;
        if( info.m_aFamilyName == familyName )
        {
            // Ignore comparing text encodings, at least for now. They cannot be trivially compared
            // (e.g. UCS2 and UTF8 are technically the same characters, just have different encoding,
            // and just having a unicode font doesn't say what glyphs it actually contains).
            // It is possible that it still may be needed to do at least some checks here
            // for some encodings (can one font have more font files for more encodings?).
            if(( family == FAMILY_DONTKNOW || info.m_eFamilyStyle == family )
                && ( italic == ITALIC_DONTKNOW || info.m_eItalic == italic )
                && ( weight == WEIGHT_DONTKNOW || info.m_eWeight == weight )
                && ( pitch == PITCH_DONTKNOW || info.m_ePitch == pitch ))
            { // Exact match, return it immediately.
                OUString ret;
                osl::File::getFileURLFromSystemPath(
                    OStringToOUString( mgr.getFontFileSysPath( id ), RTL_TEXTENCODING_UTF8 ), ret );
                return ret;
            }
            if(( info.m_eFamilyStyle == FAMILY_DONTKNOW || family == FAMILY_DONTKNOW || info.m_eFamilyStyle == family )
                && ( info.m_eItalic == ITALIC_DONTKNOW || italic == ITALIC_DONTKNOW || info.m_eItalic == italic )
                && ( info.m_eWeight == WEIGHT_DONTKNOW || weight == WEIGHT_DONTKNOW || info.m_eWeight == weight )
                && ( info.m_ePitch == PITCH_DONTKNOW || pitch == PITCH_DONTKNOW || info.m_ePitch == pitch ))
            { // Some fonts specify 'DONTKNOW' for some things, still a good match, if we don't find a better one.
                osl::File::getFileURLFromSystemPath(
                    OStringToOUString( mgr.getFontFileSysPath( id ), RTL_TEXTENCODING_UTF8 ), url );
            }
        }
    }
#else
    (void) familyName;
    (void) family;
    (void) italic;
    (void) weight;
    (void) pitch;
#endif
    return url;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
