/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_TEMPORARYFONTS_HXX
#define VCL_TEMPORARYFONTS_HXX

#include <vcl/dllapi.h>

#include <rtl/ustring.hxx>

/**
 Management of temporary fonts (e.g. embedded in documents).

 This class handles adding of temporary fonts.
 @since LibreOffice 3.7
*/
class VCL_DLLPUBLIC TemporaryFonts
{
public:
    /**
      Returns an URL for a file where to store contents of a temporary font, or an empty string
      if this font is already known. The file will be cleaned up automatically as appropriate.
      Use activateTemporaryFont() to actually enable usage of the font.

      @param fontName name of the font (e.g. 'Times New Roman')
      @param fontStyle font style, "" for regular, "bi" for bold italic, etc.
    */
    static OUString fileUrlForFont( const OUString& fontName, const char* fontStyle );

    /**
      Adds the given font to the list of known fonts. The font is used only until application
      exit.

      @param fontName name of the font (e.g. 'Times New Roman')
      @param fileUrl URL of the font file
    */
    static void activateFont( const OUString& fontName, const OUString& fileUrl );

    /**
      Removes all temporary fonts.
      @internal
    */
    static void clear();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
