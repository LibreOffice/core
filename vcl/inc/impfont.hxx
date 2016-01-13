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

#ifndef INCLUDED_VCL_INC_IMPFONT_HXX
#define INCLUDED_VCL_INC_IMPFONT_HXX

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx>
#include <vcl/fntstyle.hxx>
#include <fontinstance.hxx>

#include <boost/intrusive_ptr.hpp>

class ImplFontCharMap;
typedef boost::intrusive_ptr< ImplFontCharMap > ImplFontCharMapPtr;

// - ImplFont -

class ImplFont
{
public:
                        ImplFont();
                        ImplFont( const ImplFont& );

    bool                operator==( const ImplFont& ) const;

    FontPitch           GetPitch()      { if(mePitch==PITCH_DONTKNOW)    AskConfig(); return mePitch; }
    FontFamily          GetFamily()     { if(meFamily==FAMILY_DONTKNOW)  AskConfig(); return meFamily; }
    FontItalic          GetItalic()     { if(meItalic==ITALIC_DONTKNOW)  AskConfig(); return meItalic; }
    FontWeight          GetWeight()     { if(meWeight==WEIGHT_DONTKNOW)  AskConfig(); return meWeight; }
    FontWidth           GetWidthType()  { if(meWidthType==WIDTH_DONTKNOW)AskConfig(); return meWidthType; }

private:
    friend class vcl::Font;
    void                AskConfig();

    sal_uInt32          mnRefCount;
    OUString            maFamilyName;
    OUString            maStyleName;
    Size                maSize;
    Color               maColor;        // compatibility, now on output device
    Color               maFillColor;    // compatibility, now on output device
    rtl_TextEncoding    meCharSet;
    LanguageTag         maLanguageTag;
    LanguageTag         maCJKLanguageTag;
    FontFamily          meFamily;
    FontPitch           mePitch;
    TextAlign           meAlign;
    FontWeight          meWeight;
    FontWidth           meWidthType;
    FontItalic          meItalic;
    FontUnderline       meUnderline;
    FontUnderline       meOverline;
    FontStrikeout       meStrikeout;
    FontRelief          meRelief;
    FontEmphasisMark    meEmphasisMark;
    short               mnOrientation;
    FontKerning         mnKerning;
    bool                mbWordLine:1,
                        mbOutline:1,
                        mbConfigLookup:1,   // there was a config lookup
                        mbShadow:1,
                        mbVertical:1,
                        mbTransparent:1;    // compatibility, now on output device

    friend SvStream&    ReadImplFont( SvStream& rIStm, ImplFont& );
    friend SvStream&    WriteImplFont( SvStream& rOStm, const ImplFont& );
};

// - ImplFontMetric -

class ImplFontMetric
{
    friend class ::OutputDevice;

private:
    long                mnAscent;      // Ascent
    long                mnDescent;     // Descent
    long                mnIntLeading;  // Internal Leading
    long                mnExtLeading;  // External Leading
    long                mnLineHeight;  // Ascent+Descent+EmphasisMark
    long                mnSlant;       // Slant
    long                mnBulletOffset;// Offset for non-printing character
    sal_uInt16          mnMiscFlags;   // Misc Flags
    sal_uInt32          mnRefCount;    // Reference Counter

    bool                mbScalableFont;
    bool                mbFullstopCentered;

    // TODO: As these are progressively moved from bit fields into boolean variables, comment them out.
    // Eventually this enum will not be needed and we can remove it.
    enum {
        DEVICE_FLAG=1,
        /* SCALABLE_FLAG=2, */
        LATIN_FLAG=4,
        CJK_FLAG=8,
        CTL_FLAG=16
        /* FULLSTOP_CENTERED_FLAG=32 */
    };

public:

    bool                operator==( const ImplFontMetric& ) const;

                        ImplFontMetric();
    void                AddReference();
    void                DeReference();

    long                GetAscent() const                           { return mnAscent; }
    long                GetDescent() const                          { return mnDescent; }
    long                GetIntLeading() const                       { return mnIntLeading; }
    long                GetExtLeading() const                       { return mnExtLeading; }
    long                GetLineHeight() const                       { return mnLineHeight; }
    long                GetSlant() const                            { return mnSlant; }
    long                GetBulletOffset() const                     { return mnBulletOffset; }

    bool                IsScalable() const                          { return mbScalableFont; }
    bool                IsFullstopCentered() const                  { return mbFullstopCentered; }

    void                SetScalableFlag(bool bScalable)             { mbScalableFont = bScalable; }
    void                SetFullstopCenteredFlag(bool bCentered)     { mbFullstopCentered = bCentered; }
};


#endif // INCLUDED_VCL_INC_IMPFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
