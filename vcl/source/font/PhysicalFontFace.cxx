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

#include <sal/config.h>

#include <sal/types.h>
#include <tools/fontenum.hxx>
#include <unotools/fontdefs.hxx>

#include <fontattributes.hxx>

#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontFace.hxx>

#include <string_view>

namespace vcl::font
{

PhysicalFontFace::PhysicalFontFace( const FontAttributes& rDFA )
    : FontAttributes( rDFA )
{
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( !IsSymbolFont() )
        if ( IsStarSymbol( GetFamilyName() ) )
            SetSymbolFlag( true );
}

sal_Int32 PhysicalFontFace::CompareIgnoreSize( const PhysicalFontFace& rOther ) const
{
    // compare their width, weight, italic, style name and family name
    if( GetWidthType() < rOther.GetWidthType() )
        return -1;
    else if( GetWidthType() > rOther.GetWidthType() )
        return 1;

    if( GetWeight() < rOther.GetWeight() )
        return -1;
    else if( GetWeight() > rOther.GetWeight() )
        return 1;

    if( GetItalic() < rOther.GetItalic() )
        return -1;
    else if( GetItalic() > rOther.GetItalic() )
        return 1;

    sal_Int32 nRet = GetFamilyName().compareTo( rOther.GetFamilyName() );

    if (nRet == 0)
    {
        nRet = GetStyleName().compareTo( rOther.GetStyleName() );
    }

    return nRet;
}

static int FamilyNameMatchValue(FontSelectPattern const& rFSP, std::u16string_view sFontFamily)
{
    const OUString& rFontName = rFSP.maTargetName;

    if (rFontName.equalsIgnoreAsciiCase(sFontFamily))
        return 240000;

    return 0;
}

static int StyleNameMatchValue(FontMatchStatus const& rStatus, OUString const& rStyle)
{
    if (rStatus.mpTargetStyleName && rStyle.equalsIgnoreAsciiCase(*rStatus.mpTargetStyleName))
        return 120000;

    return 0;
}

static int PitchMatchValue(FontSelectPattern const& rFSP, FontPitch ePitch)
{
    if ((rFSP.GetPitch() != PITCH_DONTKNOW) && (rFSP.GetPitch() == ePitch))
        return 20000;

    return 0;
}

static int PreferNormalFontWidthMatchValue(FontWidth eWidthType)
{
    // TODO: change when the upper layers can tell their width preference
    if (eWidthType == WIDTH_NORMAL)
        return 400;
    else if ((eWidthType == WIDTH_SEMI_EXPANDED) || (eWidthType == WIDTH_SEMI_CONDENSED))
        return 300;

    return 0;
}

static int WeightMatchValue(FontSelectPattern const& rFSP, FontWeight eWeight)
{
    int nMatch = 0;

    if (rFSP.GetWeight() != WEIGHT_DONTKNOW)
    {
        // if not bold or requiring emboldening prefer light fonts to bold fonts
        FontWeight ePatternWeight = rFSP.mbEmbolden ? WEIGHT_NORMAL : rFSP.GetWeight();

        int nReqWeight = static_cast<int>(ePatternWeight);
        if (ePatternWeight > WEIGHT_MEDIUM)
            nReqWeight += 100;

        int nGivenWeight = static_cast<int>(eWeight);
        if (eWeight > WEIGHT_MEDIUM)
            nGivenWeight += 100;

        int nWeightDiff = nReqWeight - nGivenWeight;

        if (nWeightDiff == 0)
            nMatch += 1000;
        else if (nWeightDiff == +1 || nWeightDiff == -1)
            nMatch += 700;
        else if (nWeightDiff < +50 && nWeightDiff > -50)
            nMatch += 200;
    }
    else
    {
        // prefer NORMAL font weight
        // TODO: change when the upper layers can tell their weight preference
        if (eWeight == WEIGHT_NORMAL)
            nMatch += 450;
        else if (eWeight == WEIGHT_MEDIUM)
            nMatch += 350;
        else if ((eWeight == WEIGHT_SEMILIGHT) || (eWeight == WEIGHT_SEMIBOLD))
            nMatch += 200;
        else if (eWeight == WEIGHT_LIGHT)
            nMatch += 150;
    }

    return nMatch;
}

static int ItalicMatchValue(FontSelectPattern const& rFSP, FontItalic eItalic)
{
    // if requiring custom matrix to fake italic, prefer upright font
    FontItalic ePatternItalic
        = rFSP.maItalicMatrix != ItalicMatrix() ? ITALIC_NONE : rFSP.GetItalic();

    if (ePatternItalic == ITALIC_NONE)
    {
        if (eItalic == ITALIC_NONE)
            return 900;
    }
    else
    {
        if (ePatternItalic == eItalic)
            return 900;
        else if (eItalic != ITALIC_NONE)
            return 600;
    }

    return 0;
}

bool PhysicalFontFace::IsBetterMatch( const FontSelectPattern& rFSP, FontMatchStatus& rStatus ) const
{
    int nMatch = FamilyNameMatchValue(rFSP, GetFamilyName());
    nMatch += StyleNameMatchValue(rStatus, GetStyleName());
    nMatch += PitchMatchValue(rFSP, GetPitch());
    nMatch += PreferNormalFontWidthMatchValue(GetWidthType());
    nMatch += WeightMatchValue(rFSP, GetWeight());
    nMatch += ItalicMatchValue(rFSP, GetItalic());

    if (rFSP.mnOrientation != 0_deg10)
        nMatch += 80;
    else if (rFSP.mnWidth != 0)
        nMatch += 25;
    else
        nMatch += 5;

    if( rStatus.mnFaceMatch > nMatch )
    {
        return false;
    }
    else if( rStatus.mnFaceMatch < nMatch )
    {
        rStatus.mnFaceMatch      = nMatch;
        return true;
    }

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
