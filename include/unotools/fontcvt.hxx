/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_UNOTOOLS_FONTCVT_HXX
#define INCLUDED_UNOTOOLS_FONTCVT_HXX

#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>
#include <tools/solar.h>

// - FontToSubsFont -

#define FONTTOSUBSFONT_IMPORT                   ((sal_uLong)0x00000001)
#define FONTTOSUBSFONT_EXPORT                   ((sal_uLong)0x00000002)
#define FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS     ((sal_uLong)0x00000004)

typedef void* FontToSubsFontConverter;
UNOTOOLS_DLLPUBLIC FontToSubsFontConverter     CreateFontToSubsFontConverter( const OUString& rFontName, sal_uLong nFlags );
UNOTOOLS_DLLPUBLIC void                        DestroyFontToSubsFontConverter( FontToSubsFontConverter hConverter );
UNOTOOLS_DLLPUBLIC sal_Unicode                 ConvertFontToSubsFontChar( FontToSubsFontConverter hConverter, sal_Unicode c );
UNOTOOLS_DLLPUBLIC OUString                    GetFontToSubsFontName( FontToSubsFontConverter hConverter );

// - StarSymbolToMSMultiFont -

class UNOTOOLS_DLLPUBLIC StarSymbolToMSMultiFont
{
public:
    //Returns the name of the best windows symbol font which this char can be
    //mapped to. Sets rChar to the correct position for that font. If no
    //match found, then no name is returned, and rChar is unchanged. If you
    //want to convert a string, you don't want to use this.
    virtual OUString ConvertChar(sal_Unicode &rChar) = 0;

    //Starts converting the string at position rIndex. It converts as much of
    //the string that can be converted to the same symbol font and returns the
    //name of that font. rIndex is modified to the last index that was
    //converted. Typically you call if continuously until rIndex ==
    //rString.Len() and handle each section as separate 8bit strings using
    //separate fonts. Will return an empty string for a continuous section
    //that has no possible mapping.
    virtual OUString ConvertString(OUString &rString, sal_Int32 &rIndex) = 0;
    virtual ~StarSymbolToMSMultiFont() {}
};

//with bPerfect set the converter will only try and convert symbols which have
//perfect mappings to the windows symbols fonts. With it not set, it will
//allow somewhat more dubious transformations that are nevertheless
//recognizably similar. Even in this mode there will be characters that fail.
//The users of this might want to make a distinction between failed characters
//which were inside and those outside the unicode private area.
UNOTOOLS_DLLPUBLIC StarSymbolToMSMultiFont *CreateStarSymbolToMSMultiFont(bool bPerfectOnly=false);
#endif // INCLUDED_UNOTOOLS_FONTCVT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
