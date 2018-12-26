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

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <vcl/vclenum.hxx>
#include <tools/fontenum.hxx>


/* The following class is extraordinarily similar to ImplFont. */

class VCL_DLLPUBLIC FontAttributes
{
public:
    explicit            FontAttributes();

    // device independent font functions
    const OUString&     GetFamilyName() const                       { return maFamilyName; }
    FontFamily          GetFamilyType() const                       { return meFamily; }
    const OUString&     GetStyleName() const                        { return maStyleName; }

    FontWeight          GetWeight() const                           { return meWeight; }
    FontItalic          GetItalic() const                           { return meItalic; }
    FontPitch           GetPitch() const                            { return mePitch; }
    FontWidth           GetWidthType() const                        { return meWidthType; }
    rtl_TextEncoding    GetCharSet() const                          { return meCharSet; }

    bool                IsSymbolFont() const                        { return mbSymbolFlag; }

    void                SetFamilyName(const OUString& sFamilyName)  { maFamilyName = sFamilyName; }
    void                SetStyleName( const OUString& sStyleName)   { maStyleName = sStyleName; }
    void                SetFamilyType(const FontFamily eFontFamily) { meFamily = eFontFamily; }

    void                SetPitch(const FontPitch ePitch )           { mePitch = ePitch; }
    void                SetItalic(const FontItalic eItalic )        { meItalic = eItalic; }
    void                SetWeight(const FontWeight eWeight )        { meWeight = eWeight; }
    void                SetWidthType(const FontWidth eWidthType)    { meWidthType = eWidthType; }

    void                SetSymbolFlag(const bool );

    bool                CompareDeviceIndependentFontAttributes(const FontAttributes& rOther) const;

    // Device dependent functions
    int                 GetQuality() const                          { return mnQuality; }
    const OUString&     GetMapNames() const                         { return maMapNames; }


    void                SetQuality( int nQuality )                  { mnQuality = nQuality; }
    void                IncreaseQualityBy( int nQualityAmount )     { mnQuality += nQualityAmount; }
    void                AddMapName( OUString const& );

private:
    // device independent variables
    OUString            maFamilyName;               // Font Family Name
    OUString            maStyleName;                // Font Style Name
    FontWeight          meWeight;                   // Weight Type
    FontFamily          meFamily;                   // Family Type
    FontPitch           mePitch;                    // Pitch Type
    FontWidth           meWidthType;                // Width Type
    FontItalic          meItalic;                   // Slant Type
    rtl_TextEncoding    meCharSet;                  // RTL_TEXTENCODING_SYMBOL or RTL_TEXTENCODING_UNICODE
    bool                mbSymbolFlag;               // Is font a symbol?

    // device dependent variables
    OUString            maMapNames;                 // List of family name aliases separated with ';'
    int                 mnQuality;                  // Quality (used when similar fonts compete)

};

inline void FontAttributes::SetSymbolFlag( const bool bSymbolFlag )
{
    mbSymbolFlag = bSymbolFlag;
    if ( bSymbolFlag )
    {
        meCharSet = RTL_TEXTENCODING_SYMBOL;
    }
    else
    {
        // if the symbol flag is unset, but it was a symbol font before then
        // until the character set encoding is set via SetCharSet then we
        // can't know what the characterset is!
        if ( meCharSet == RTL_TEXTENCODING_SYMBOL )
        {
            meCharSet = RTL_TEXTENCODING_DONTKNOW;
        }
    }
}

inline void FontAttributes::AddMapName( OUString const & aMapName )
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

#endif // INCLUDED_VCL_INC_FONTATTRIBUTES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
