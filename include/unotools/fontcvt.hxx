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


enum class FontToSubsFontFlags
{
    IMPORT                   = 0x01,
    EXPORT                   = 0x02
};

typedef void* FontToSubsFontConverter;
UNOTOOLS_DLLPUBLIC FontToSubsFontConverter     CreateFontToSubsFontConverter( const OUString& rFontName, FontToSubsFontFlags nFlags );
UNOTOOLS_DLLPUBLIC sal_Unicode                 ConvertFontToSubsFontChar( FontToSubsFontConverter hConverter, sal_Unicode c );
UNOTOOLS_DLLPUBLIC OUString                    GetFontToSubsFontName( FontToSubsFontConverter hConverter );


class UNOTOOLS_DLLPUBLIC StarSymbolToMSMultiFont
{
public:
    // Returns the name of the best windows symbol font which this char can be
    // mapped to. Sets rChar to the correct position for that font. If no
    // match found, then no name is returned, and rChar is unchanged. If you
    // want to convert a string, you don't want to use this.
    virtual OUString ConvertChar(sal_Unicode &rChar) = 0;

    virtual ~StarSymbolToMSMultiFont() {}
};


// The converter will allow somewhat dubious mappings to the windows symbols fonts,
// that are nevertheless recognizably similar. Even in this mode there will be characters that fail.
// The users of this might want to make a distinction between failed characters
// which were inside and those outside the unicode private area.
UNOTOOLS_DLLPUBLIC StarSymbolToMSMultiFont *CreateStarSymbolToMSMultiFont();

#endif // INCLUDED_UNOTOOLS_FONTCVT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
