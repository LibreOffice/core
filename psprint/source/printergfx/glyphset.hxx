/*************************************************************************
 *
 *  $RCSfile: glyphset.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 14:24:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _PSPRINT_GLYPHSET_HXX_
#define _PSPRINT_GLYPHSET_HXX_

#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

namespace psp {

class PrinterGfx;
class PrintFontManager;

class GlyphSet
{
private:

    sal_Int32           mnFontID;
    sal_Bool            mbVertical;
    rtl::OString        maBaseName;
    fonttype::type      meBaseType;
    rtl_TextEncoding    mnBaseEncoding;

    typedef std::hash_map< sal_Unicode, sal_uInt8 > char_map_t;
    typedef std::list< char_map_t > char_list_t;
    typedef std::hash_map< sal_uInt32, sal_uInt8 > glyph_map_t;
    typedef std::list< glyph_map_t > glyph_list_t;

    char_list_t     maCharList;
    glyph_list_t    maGlyphList;

    rtl::OString    GetGlyphSetName (sal_Int32 nGlyphSetID);
    rtl::OString    GetCharSetName (sal_Int32 nGlyphSetID);
    sal_Int32       GetGlyphSetEncoding (sal_Int32 nGlyphSetID);
    rtl::OString    GetGlyphSetEncodingName (sal_Int32 nGlyphSetID);

    rtl::OString    GetReencodedFontName (sal_Int32 nGlyphSetID);
    void            PSDefineReencodedFont (osl::File* pOutFile,
                                           sal_Int32 nGlyphSetID);

    sal_Bool        GetCharID (sal_Unicode nChar,
                                sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        LookupCharID (sal_Unicode nChar,
                                   sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        AddCharID (sal_Unicode nChar,
                                sal_uChar* nOutGlyphID,
                                sal_Int32* nOutGlyphSetID);
    sal_Bool        GetGlyphID (sal_uInt32 nGlyph, sal_Unicode nUnicode,
                                sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        LookupGlyphID (sal_uInt32 nGlyph,
                                   sal_uChar* nOutGlyphID, sal_Int32* nOutGlyphSetID);
    sal_Bool        AddGlyphID (sal_uInt32 nGlyph, sal_Unicode nUnicode,
                                sal_uChar* nOutGlyphID,
                                sal_Int32* nOutGlyphSetID);
    void            AddNotdef (char_map_t &rCharMap);
    void            AddNotdef (glyph_map_t &rGlyphMap);
    sal_uChar       GetAnsiMapping (sal_Unicode nUnicodeChar);
    sal_uChar       GetSymbolMapping (sal_Unicode nUnicodeChar);

    void            ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                                  const sal_Unicode* pStr, sal_Int16 nLen);
    void            ImplDrawText (PrinterGfx &rGfx, const Point& rPoint,
                                  const sal_Unicode* pStr, sal_Int16 nLen,
                                  const sal_Int32* pDeltaArray);

public:

    GlyphSet ();
    GlyphSet (sal_Int32 nFontID, sal_Bool bVertical);
    ~GlyphSet ();

    sal_Int32       GetFontID ();
    fonttype::type  GetFontType ();
    static rtl::OString
    GetReencodedFontName (rtl_TextEncoding nEnc,
                          const rtl::OString &rFontName);
    static rtl::OString
    GetGlyphSetEncodingName (rtl_TextEncoding nEnc,
                             const rtl::OString &rFontName);
    sal_Bool        IsVertical ();

    sal_Bool        SetFont (sal_Int32 nFontID, sal_Bool bVertical);

    void            DrawText (PrinterGfx &rGfx, const Point& rPoint,
                              const sal_Unicode* pStr, sal_Int16 nLen,
                              const sal_Int32* pDeltaArray = NULL);
    void            DrawGlyphs (PrinterGfx& rGfx,
                                const Point& rPoint,
                                const sal_uInt32* pGlyphIds,
                                const sal_Unicode* pUnicodes,
                                sal_Int16 nLen,
                                const sal_Int32* pDeltaArray );
    sal_Bool        PSUploadEncoding(osl::File* pOutFile, PrinterGfx &rGfx);
    sal_Bool        PSUploadFont (osl::File& rOutFile, PrinterGfx &rGfx, bool bAsType42, std::list< rtl::OString >& rSuppliedFonts );
};


} /* namespace psp */

#endif

