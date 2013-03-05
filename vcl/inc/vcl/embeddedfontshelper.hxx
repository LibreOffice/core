/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_EMBEDDEDFONTSHELPER_HXX
#define VCL_EMBEDDEDFONTSHELPER_HXX

#include <vcl/dllapi.h>

#include <rtl/ustring.hxx>
#include <tools/fontenum.hxx>

/**
 Helper functions for handling embedded fonts in documents.

*/
class VCL_DLLPUBLIC EmbeddedFontsHelper
{
public:
    /**
      Returns URL for a font file for the given font, or empty if it does not exist.
    */
    static OUString fontFileUrl( const OUString& familyName, FontFamily family, FontItalic italic,
        FontWeight weight, FontPitch pitch, rtl_TextEncoding encoding );
    /**
      Returns an URL for a file where to store contents of a given temporary font.
      The file may or not may not exist yet, and will be cleaned up automatically as appropriate.
      Use activateTemporaryFont() to actually enable usage of the font.

      @param fontName name of the font (e.g. 'Times New Roman')
      @param extra additional text to use for name (e.g. to distinguish regular from bold, italic,...), "?" for unique
    */
    static OUString fileUrlForTemporaryFont( const OUString& fontName, const char* extra );

    /**
      Adds the given font to the list of known fonts. The font is used only until application
      exit.

      @param fontName name of the font (e.g. 'Times New Roman')
      @param fileUrl URL of the font file
    */
    static void activateFont( const OUString& fontName, const OUString& fileUrl );

    /**
      Removes all temporary fonts in the path used by fileUrlForTemporaryFont().
      @internal
    */
    static void clearTemporaryFontFiles();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
