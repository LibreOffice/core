/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontcvt.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_FONTCVT_HXX
#define _SV_FONTCVT_HXX

#include <vcl/dllapi.h>
#include <tools/string.hxx>

// ------------------
// - FontToSubsFont -
// ------------------

#define FONTTOSUBSFONT_IMPORT                   ((ULONG)0x00000001)
#define FONTTOSUBSFONT_EXPORT                   ((ULONG)0x00000002)
#define FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS     ((ULONG)0x00000004)

typedef void* FontToSubsFontConverter;
VCL_DLLPUBLIC FontToSubsFontConverter     CreateFontToSubsFontConverter( const String& rFontName, ULONG nFlags );
VCL_DLLPUBLIC void                        DestroyFontToSubsFontConverter( FontToSubsFontConverter hConverter );
VCL_DLLPUBLIC sal_Unicode                 ConvertFontToSubsFontChar( FontToSubsFontConverter hConverter, sal_Unicode c );
VCL_DLLPUBLIC String                      GetFontToSubsFontName( FontToSubsFontConverter hConverter );

// ----------------
// - SubsFontName -
// ----------------

#define SUBSFONT_ONLYONE    ((ULONG)0x00000001)
#define SUBSFONT_MS         ((ULONG)0x00000002)
#define SUBSFONT_PS         ((ULONG)0x00000004)
#define SUBSFONT_HTML       ((ULONG)0x00000008)
VCL_DLLPUBLIC String GetSubsFontName( const String& rName, ULONG nFlags );

// -----------------
// - FontTokenName -
// -----------------

VCL_DLLPUBLIC String GetFontToken( const String& rName, xub_StrLen nToken, xub_StrLen& rIndex );
inline String GetFontToken( const String& rName, xub_StrLen nToken )
{
    xub_StrLen nTempIndex = 0;
    return GetFontToken( rName, nToken, nTempIndex );
}

// ---------------------------
// - StarSymbolToMSMultiFont -
// ---------------------------

class VCL_DLLPUBLIC StarSymbolToMSMultiFont
{
public:
    //Returns the name of the best windows symbol font which this char can be
    //mapped to. Sets rChar to the correct position for that font. If no
    //match found, then no name is returned, and rChar is unchanged. If you
    //want to convert a string, you don't want to use this.
    virtual String ConvertChar(sal_Unicode &rChar) = 0;

    //Starts converting the string at position rIndex. It converts as much of
    //the string that can be converted to the same symbol font and returns the
    //name of that font. rIndex is modified to the last index that was
    //converted. Typically you call if continously until rIndex ==
    //rString.Len() and handle each section as seperate 8bit strings using
    //seperate fonts. Will return an empty string for a continous section
    //that has no possible mapping.
    virtual String ConvertString(String &rString, xub_StrLen &rIndex) = 0;
    virtual ~StarSymbolToMSMultiFont() {}
};

//with bPerfect set the converter will only try and convert symbols which have
//perfect mappings to the windows symbols fonts. With it not set, it will
//allow somewhat more dubious transformations that are nevertheless
//recognizably similiar. Even in this mode there will be characters that fail.
//The users of this might want to make a distinction between failed characters
//which were inside and those outside the unicode private area.
VCL_DLLPUBLIC StarSymbolToMSMultiFont *CreateStarSymbolToMSMultiFont(bool bPerfectOnly=false);
#endif // _SV_FONTCVT_HXX
