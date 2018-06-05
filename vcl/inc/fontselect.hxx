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

#ifndef INCLUDED_VCL_INC_FONTSELECT_HXX
#define INCLUDED_VCL_INC_FONTSELECT_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ref.hxx>

#include "fontattributes.hxx"

#include <unordered_map>

namespace vcl { class Font; }

class LogicalFontInstance;
class PhysicalFontFace;
class Size;

class FontSelectPatternAttributes : public FontAttributes
{
public:
                    FontSelectPatternAttributes( const vcl::Font&, const OUString& rSearchName,
                                                 const Size&, float fExactHeight );
#ifdef _WIN32
                    FontSelectPatternAttributes( const PhysicalFontFace&, const Size&,
                                                 float fExactHeight, int nOrientation, bool bVertical );
#endif

    size_t          hashCode() const;
    bool operator==(const FontSelectPatternAttributes& rOther) const;
    bool operator!=(const FontSelectPatternAttributes& rOther) const
    {
        return !(*this == rOther);
    }

    static const char FEAT_PREFIX;
    static const char FEAT_SEPARATOR;

public:
    OUString        maTargetName;               // name of the font name token that is chosen
    OUString        maSearchName;               // name of the font that matches best
    int             mnWidth;                    // width of font in pixel units
    int             mnHeight;                   // height of font in pixel units
    float           mfExactHeight;              // requested height (in pixels with subpixel details)
    int             mnOrientation;              // text orientation in 3600 system
    LanguageType    meLanguage;                 // text language
    bool            mbVertical;                 // vertical mode of requested font
    bool            mbNonAntialiased;           // true if antialiasing is disabled

    bool            mbEmbolden;                 // Force emboldening
    ItalicMatrix    maItalicMatrix;             // Force matrix for slant
};


class FontSelectPattern : public FontSelectPatternAttributes
{
public:
                    FontSelectPattern( const vcl::Font&, const OUString& rSearchName,
                                       const Size&, float fExactHeight );
#ifdef _WIN32
// ifdeffed to prevent it going into unusedcode.easy
                    FontSelectPattern( const PhysicalFontFace&, const Size&,
                                       float fExactHeight, int nOrientation, bool bVertical );
#endif

public: // TODO: change to private
    rtl::Reference<LogicalFontInstance> mpFontInstance;                // pointer to the resulting FontCache entry

    void            copyAttributes(const FontSelectPatternAttributes &rAttributes);
};

#endif // INCLUDED_VCL_INC_FONTSELECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
