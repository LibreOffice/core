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

#ifndef INCLUDED_VCL_INC_IMPFONTMETRICDATA_HXX
#define INCLUDED_VCL_INC_IMPFONTMETRICDATA_HXX

#include <vcl/dllapi.h>
#include <tools/ref.hxx>
#include "fontattributes.hxx"

#include <vector>

class ImplFontMetricData;
typedef tools::SvRef<ImplFontMetricData> ImplFontMetricDataRef;

class OutputDevice;
class FontSelectPattern;

class ImplFontMetricData : public FontAttributes, public SvRefBase
{
public:
    explicit        ImplFontMetricData( const FontSelectPattern& );

    // font instance attributes from the font request
    sal_Int32       GetWidth()                                                      { return mnWidth; }
    short           GetOrientation()                                                { return mnOrientation; }

    void            SetWidth(sal_Int32 nWidth)                                           { mnWidth=nWidth; }
    void            SetOrientation(short nOrientation)                              { mnOrientation=nOrientation; }

    // font metrics measured for the font instance
    sal_Int32       GetAscent()                                                     { return mnAscent; }
    sal_Int32       GetDescent()                                                    { return mnDescent; }
    sal_Int32       GetInternalLeading()                                            { return mnIntLeading; }
    sal_Int32       GetExternalLeading()                                            { return mnExtLeading; }
    int             GetSlant()                                                      { return mnSlant; }
    sal_Int32       GetMinKashida()                                                 { return mnMinKashida; }

    void            SetSlant(int nSlant)                                            { mnSlant=nSlant; }
    void            SetMinKashida( sal_Int32 nMinKashida )                               { mnMinKashida=nMinKashida; }

    // font attributes queried from the font instance
    bool            IsFullstopCentered()                                            { return mbFullstopCentered; }
    sal_Int32       GetBulletOffset()                                               { return mnBulletOffset; }

    void            SetFullstopCenteredFlag(bool bFullstopCentered)                 { mbFullstopCentered = bFullstopCentered; }

    // font metrics that are usually derived from the measurements
    sal_Int32       GetUnderlineSize()                                              { return mnUnderlineSize; }
    sal_Int32       GetUnderlineOffset()                                            { return mnUnderlineOffset; }
    sal_Int32       GetBoldUnderlineSize()                                          { return mnBUnderlineSize; }
    sal_Int32       GetBoldUnderlineOffset()                                        { return mnBUnderlineOffset; }
    sal_Int32       GetDoubleUnderlineSize()                                        { return mnDUnderlineSize; }
    sal_Int32       GetDoubleUnderlineOffset1()                                     { return mnDUnderlineOffset1; }
    sal_Int32       GetDoubleUnderlineOffset2()                                     { return mnDUnderlineOffset2; }
    sal_Int32       GetWavelineUnderlineSize()                                      { return mnWUnderlineSize; }
    sal_Int32       GetWavelineUnderlineOffset()                                    { return mnWUnderlineOffset; }
    sal_Int32       GetAboveUnderlineSize()                                         { return mnAboveUnderlineSize; }
    sal_Int32       GetAboveUnderlineOffset()                                       { return mnAboveUnderlineOffset; }
    sal_Int32       GetAboveBoldUnderlineSize()                                     { return mnAboveBUnderlineSize; }
    sal_Int32       GetAboveBoldUnderlineOffset()                                   { return mnAboveBUnderlineOffset; }
    sal_Int32       GetAboveDoubleUnderlineSize()                                   { return mnAboveDUnderlineSize; }
    sal_Int32       GetAboveDoubleUnderlineOffset1()                                { return mnAboveDUnderlineOffset1; }
    sal_Int32       GetAboveDoubleUnderlineOffset2()                                { return mnAboveDUnderlineOffset2; }
    sal_Int32       GetAboveWavelineUnderlineSize()                                 { return mnAboveWUnderlineSize; }
    sal_Int32       GetAboveWavelineUnderlineOffset()                               { return mnAboveWUnderlineOffset; }
    sal_Int32       GetStrikeoutSize()                                              { return mnStrikeoutSize; }
    sal_Int32       GetStrikeoutOffset()                                            { return mnStrikeoutOffset; }
    sal_Int32       GetBoldStrikeoutSize()                                          { return mnBStrikeoutSize; }
    sal_Int32       GetBoldStrikeoutOffset()                                        { return mnBStrikeoutOffset; }
    sal_Int32       GetDoubleStrikeoutSize()                                        { return mnDStrikeoutSize; }
    sal_Int32       GetDoubleStrikeoutOffset1()                                     { return mnDStrikeoutOffset1; }
    sal_Int32       GetDoubleStrikeoutOffset2()                                     { return mnDStrikeoutOffset2; }

    void            ImplInitTextLineSize( const OutputDevice* pDev );
    void            ImplInitAboveTextLineSize();
    void            ImplCalcLineSpacing(const std::vector<uint8_t>& rHhea,
                                        const std::vector<uint8_t>& rOS_2,
                                        int nUPEM);

private:
    // font instance attributes from the font request
    sal_Int32       mnHeight;                   // Font size
    sal_Int32       mnWidth;                    // Reference Width
    short           mnOrientation;              // Rotation in 1/10 degrees

    // font metrics measured for the font instance
    sal_Int32       mnAscent;                   // Ascent
    sal_Int32       mnDescent;                  // Descent
    sal_Int32       mnIntLeading;               // Internal Leading
    sal_Int32       mnExtLeading;               // External Leading
    int             mnSlant;                    // Slant (Italic/Oblique)
    sal_Int32       mnMinKashida;               // Minimal width of kashida (Arabic)

    // font attributes queried from the font instance
    bool            mbFullstopCentered;
    sal_Int32       mnBulletOffset;             // Offset to position non-print character

    // font metrics that are usually derived from the measurements
    sal_Int32       mnUnderlineSize;            // Lineheight of Underline
    sal_Int32       mnUnderlineOffset;          // Offset from Underline to Baseline
    sal_Int32       mnBUnderlineSize;           // Height of bold underline
    sal_Int32       mnBUnderlineOffset;         // Offset from bold underline to baseline
    sal_Int32       mnDUnderlineSize;           // Height of double underline
    sal_Int32       mnDUnderlineOffset1;        // Offset from double underline to baseline
    sal_Int32       mnDUnderlineOffset2;        // Offset from double underline to baseline
    sal_Int32       mnWUnderlineSize;           // Height of WaveLine underline
    sal_Int32       mnWUnderlineOffset;         // Offset from WaveLine underline to baseline, but centrered to WaveLine
    sal_Int32       mnAboveUnderlineSize;       // Height of single underline (for Vertical Right)
    sal_Int32       mnAboveUnderlineOffset;     // Offset from single underline to baseline (for Vertical Right)
    sal_Int32       mnAboveBUnderlineSize;      // Height of bold underline (for Vertical Right)
    sal_Int32       mnAboveBUnderlineOffset;    // Offset from bold underline to baseline (for Vertical Right)
    sal_Int32       mnAboveDUnderlineSize;      // Height of double underline (for Vertical Right)
    sal_Int32       mnAboveDUnderlineOffset1;   // Offset from double underline to baseline (for Vertical Right)
    sal_Int32       mnAboveDUnderlineOffset2;   // Offset from double underline to baseline (for Vertical Right)
    sal_Int32       mnAboveWUnderlineSize;      // Height of WaveLine-strike-out (for Vertical Right)
    sal_Int32       mnAboveWUnderlineOffset;    // Offset from WaveLine-strike-out to baseline, but centrered to the WaveLine (for Vertical Right)
    sal_Int32       mnStrikeoutSize;            // Height of single strike-out
    sal_Int32       mnStrikeoutOffset;          // Offset from single strike-out to baseline
    sal_Int32       mnBStrikeoutSize;           // Height of bold strike-out
    sal_Int32       mnBStrikeoutOffset;         // Offset of bold strike-out to baseline
    sal_Int32       mnDStrikeoutSize;           // Height of double strike-out
    sal_Int32       mnDStrikeoutOffset1;        // Offset of double strike-out to baseline
    sal_Int32       mnDStrikeoutOffset2;        // Offset of double strike-out to baseline

};

#endif // INCLUDED_VCL_INC_IMPFONTMETRICDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
