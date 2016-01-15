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

#ifndef INCLUDED_VCL_INC_FONTATTRIBUTES_HXX
#define INCLUDED_VCL_INC_FONTATTRIBUTES_HXX

#include <unotools/fontdefs.hxx>
#include <rtl/textenc.h>
#include <vcl/vclenum.hxx>

class FontAttributes
{
public:
    explicit        FontAttributes();

    // device independent font functions
    const OUString& GetFamilyName() const                       { return maFamilyName; }
    FontFamily      GetFamilyType() const                       { return meFamily; }
    const OUString& GetStyleName() const                        { return maStyleName; }

    FontWeight      GetWeight() const                           { return meWeight; }
    FontItalic      GetSlantType() const                        { return meItalic; }
    FontPitch       GetPitch() const                            { return mePitch; }
    FontWidth       GetWidthType() const                        { return meWidthType; }
    rtl_TextEncoding GetCharSet() const                         { return meCharSet; }

    bool            IsSymbolFont() const                        { return mbSymbolFlag; }

    void            SetFamilyName(const OUString& sFamilyName)  { maFamilyName = sFamilyName; }
    void            SetStyleName( const OUString& sStyleName)   { maStyleName = sStyleName; }
    void            SetFamilyType(const FontFamily eFontFamily) { meFamily = eFontFamily; }

    void            SetPitch(const FontPitch ePitch )           { mePitch = ePitch; }
    void            SetItalic(const FontItalic eItalic )        { meItalic = eItalic; }
    void            SetWeight(const FontWeight eWeight )        { meWeight = eWeight; }
    void            SetWidthType(const FontWidth eWidthType)    { meWidthType = eWidthType; }

    void            SetSymbolFlag(const bool );

    bool            CompareDeviceIndependentFontAttributes(const FontAttributes& rOther) const;

    // Device dependent functions
    int             GetQuality() const                          { return mnQuality; }
    OUString        GetMapNames() const                         { return maMapNames; }

    bool            IsBuiltInFont() const                       { return mbDevice; }
    bool            CanEmbed() const                            { return mbEmbeddable; }
    bool            CanSubset() const                           { return mbSubsettable; }
    bool            CanRotate() const                           { return mbOrientation; }
    bool            HasMapNames() const                         { return (maMapNames.getLength() > 0); }

    void            SetQuality( int nQuality )                  { mnQuality = nQuality; }
    void            IncreaseQualityBy( int nQualityAmount )     { mnQuality += nQualityAmount; }
    void            DecreaseQualityBy( int nQualityAmount )     { mnQuality -= nQualityAmount; }
    void            SetMapNames( OUString const & aMapNames )   { maMapNames = aMapNames; }
    void            AddMapName( OUString const & aMapName )
                    {
                        if( maMapNames.getLength() > 0 )
                        {
                            maMapNames += ";";
                        }

                        if (aMapName.getLength() == 0)
                        {
                            SAL_WARN("vcl.fonts", "New map name is empty");
                            return;
                        }

                        maMapNames += aMapName;
                    }

    void            SetBuiltInFontFlag( bool bIsBuiltInFont )   { mbDevice = bIsBuiltInFont; }
    void            SetEmbeddableFlag ( bool bEmbeddable )      { mbEmbeddable = bEmbeddable; }
    void            SetSubsettableFlag( bool bSubsettable )     { mbSubsettable = bSubsettable; }
    void            SetOrientationFlag( bool bCanRotate )       { mbOrientation = bCanRotate; }
    void            SetCharSet( const rtl_TextEncoding );

private:
    // device independent variables
    OUString        maFamilyName;               // Font Family Name
    OUString        maStyleName;                // Font Style Name
    FontWeight      meWeight;                   // Weight Type
    FontFamily      meFamily;                   // Family Type
    FontPitch       mePitch;                    // Pitch Type
    FontWidth       meWidthType;                // Width Type
    FontItalic      meItalic;                   // Slant Type
    rtl_TextEncoding meCharSet;                 // RTL_TEXTENCODING_SYMBOL or RTL_TEXTENCODING_UNICODE
    bool            mbSymbolFlag;               // Is font a symbol?

    // device dependent variables
    OUString        maMapNames;                 // List of family name aliases separated with ';'
    int             mnQuality;                  // Quality (used when similar fonts compete)
    bool            mbOrientation;              // true: physical font can be rotated
    bool            mbDevice;                   // true: built in font
    bool            mbSubsettable;              // true: a subset of the font can be created
    bool            mbEmbeddable;               // true: the font can be embedded

};

inline void FontAttributes::SetSymbolFlag( const bool bSymbolFlag )
{
    mbSymbolFlag = bSymbolFlag;
    if ( bSymbolFlag )
        meCharSet = RTL_TEXTENCODING_SYMBOL;
}

inline void FontAttributes::SetCharSet( const rtl_TextEncoding aEncoding )
{
    meCharSet = aEncoding;
    mbSymbolFlag = ( meCharSet == RTL_TEXTENCODING_SYMBOL ? true : false );
}

#endif // INCLUDED_VCL_INC_FONTATTRIBUTES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
