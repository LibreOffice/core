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
#include <i18nlangtag/languagetag.hxx>
#include <vcl/vclenum.hxx>
#include <fontinstance.hxx>

#include <boost/intrusive_ptr.hpp>

/* The following class is extraordinarily similar to FontAttributes. */

class ImplFont
{
public:
    explicit            ImplFont();
    explicit            ImplFont( const ImplFont& );

    // device independent font functions
    const OUString&     GetFamilyName() const                           { return maFamilyName; }
    FontFamily          GetFamily()                                     { if(meFamily==FAMILY_DONTKNOW)  AskConfig(); return meFamily; }
    FontFamily          GetFamilyNoAsk() const                          { return meFamily; }
    FontFamily          GetFamilyType()                                 { return GetFamily(); }
    FontFamily          GetFamilyTypeNoAsk() const                      { return GetFamilyNoAsk(); }
    const OUString&     GetStyleName() const                            { return maStyleName; }

    FontWeight          GetWeight()                                     { if(meWeight==WEIGHT_DONTKNOW)  AskConfig(); return meWeight; }
    FontWeight          GetWeightNoAsk() const                          { return meWeight; }
    FontItalic          GetItalic()                                     { return GetItalic(); }
    FontItalic          GetItalic() const                               { return GetItalicNoAsk(); }
    FontItalic          GetItalic()                                     { if(meItalic==ITALIC_DONTKNOW)  AskConfig(); return meItalic; }
    FontItalic          GetItalicNoAsk() const                          { return meItalic; }
    FontPitch           GetPitch()                                      { if(mePitch==PITCH_DONTKNOW)    AskConfig(); return mePitch; }
    FontPitch           GetPitchNoAsk() const                           { return mePitch; }
    FontWidth           GetWidthType()                                  { if(meWidthType==WIDTH_DONTKNOW) AskConfig(); return meWidthType; }
    FontWidth           GetWidthTypeNoAsk() const                       { return meWidthType; }

    bool                IsSymbolFont() const                            { return mbSymbol; }

    void                SetFamilyName( const OUString& sFamilyName )    { maFamilyName = sFamilyName; }
    void                SetStyleName( const OUString& sStyleName )      { maStyleName = sStyleName; }
    void                SetFamilyType( const FontFamily eFontFamily )   { meFamily = eFontFamily; }

    void                SetPitch( const FontPitch ePitch )              { mePitch = ePitch; }
    void                SetItalic( const FontItalic eItalic )           { meItalic = eItalic; }
    void                SetWeight( const FontWeight eWeight )           { meWeight = eWeight; }
    void                SetWidthType( const FontWidth eWidthType )      { meWidthType = eWidthType; }

    void                SetSymbolFlag( const bool bSymbolFlag )         { mbSymbol = bSymbolFlag; }

    // device dependent functions
    /* Missing function: int GetQuality() const; */
    /* Missing function: OUString GetMapNames() const; */

    /* Missing function: bool IsBuiltInFont() const; */
    /* Missing function: bool CanEmbed() const; */
    /* Missing function: bool CanSubSet() const; */
    /* Missing function: bool CanRotate() const; */
    /* Missing function: bool HasMapNames() const; */

    /* Missing function: void SetQuality( int nQuality ); */
    /* Missing function: void IncreaseQualityBy( int nQualityAmount ); */
    /* Missing function: void DecreaseQualityBy( int nQualityAmount ); */
    /* Missing function: void SetNames( OUString const& ); */
    /* Missing function: Vod AddMapName( OUString const& ); */

    /* Missing function: SetBuiltInFontFlag( bool ); */
    /* Missing function: SetEmbeddableFlag( bool ); */
    /* Missing function: SetSettableFlag( bool ); */
    /* missing function: SetOrientationFlag( bool ); */

    bool                operator==( const ImplFont& ) const;

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
    bool                mbSymbol;
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

#endif // INCLUDED_VCL_INC_IMPFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
