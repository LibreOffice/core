/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _UNOTOOLS_FONTCVT_HXX
#define _UNOTOOLS_FONTCVT_HXX

#include <unotools/unotoolsdllapi.h>
#include <tools/string.hxx>

// ------------------
// - FontToSubsFont -
// ------------------

#define FONTTOSUBSFONT_IMPORT                   ((sal_uLong)0x00000001)
#define FONTTOSUBSFONT_EXPORT                   ((sal_uLong)0x00000002)
#define FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS     ((sal_uLong)0x00000004)

typedef void* FontToSubsFontConverter;
UNOTOOLS_DLLPUBLIC FontToSubsFontConverter     CreateFontToSubsFontConverter( const String& rFontName, sal_uLong nFlags );
UNOTOOLS_DLLPUBLIC void                        DestroyFontToSubsFontConverter( FontToSubsFontConverter hConverter );
UNOTOOLS_DLLPUBLIC sal_Unicode                 ConvertFontToSubsFontChar( FontToSubsFontConverter hConverter, sal_Unicode c );
UNOTOOLS_DLLPUBLIC String                      GetFontToSubsFontName( FontToSubsFontConverter hConverter );

// ---------------------------
// - StarSymbolToMSMultiFont -
// ---------------------------

class UNOTOOLS_DLLPUBLIC StarSymbolToMSMultiFont
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
UNOTOOLS_DLLPUBLIC StarSymbolToMSMultiFont *CreateStarSymbolToMSMultiFont(bool bPerfectOnly=false);
#endif // _UNOTOOLS_FONTCVT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
