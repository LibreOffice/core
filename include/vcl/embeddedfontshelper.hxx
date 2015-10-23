/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_EMBEDDEDFONTSHELPER_HXX
#define INCLUDED_VCL_EMBEDDEDFONTSHELPER_HXX

#include <vcl/dllapi.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <tools/fontenum.hxx>
#include <vector>

/**
 Helper functions for handling embedded fonts in documents.

*/
class VCL_DLLPUBLIC EmbeddedFontsHelper
{
public:
    /// Specification of what kind of operation is allowed when embedding a font
    enum FontRights
    {
        ViewingAllowed, ///< Font may be embedded for viewing the document (but not editing)
        EditingAllowed ///< Font may be embedded for editing document (implies viewing)
    };

    /**
      Returns URL for a font file for the given font, or empty if it does not exist.
    */
    static OUString fontFileUrl( const OUString& familyName, FontFamily family, FontItalic italic,
        FontWeight weight, FontPitch pitch, rtl_TextEncoding encoding, FontRights rights );

    /**
      Reads a font from the input stream, saves it to a temporary font file and activates the font.
      @param stream stream of font data
      @param fontName name of the font (e.g. 'Times New Roman')
      @param extra additional text to use for name (e.g. to distinguish regular from bold, italic,...), "?" for unique
      @param key key to xor the data with, from the start until the key's length (not repeated)
      @param eot whether the data is compressed in Embedded OpenType format
    */
    static bool addEmbeddedFont( css::uno::Reference< css::io::XInputStream > stream,
        const OUString& fontName, const char* extra,
        std::vector< unsigned char > key = std::vector< unsigned char >(), bool eot = false);

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
      Returns if the restrictions specified in the font (if present) allow embedding
      the font for a particular purpose.
      @param data font data
      @param size size of the font data
      @param rights type of operation to be allowed for the font
    */
    static bool sufficientTTFRights( const void* data, long size, FontRights rights );

    /**
      Removes all temporary fonts in the path used by fileUrlForTemporaryFont().
      @internal
    */
    static void clearTemporaryFontFiles();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
