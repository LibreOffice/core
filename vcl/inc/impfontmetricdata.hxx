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
#include "sft.hxx"

#include <vector>

class ImplFontMetricData;
typedef tools::SvRef<ImplFontMetricData> ImplFontMetricDataRef;

class OutputDevice;
class FontSelectPattern;

class VCL_DLLPUBLIC ImplFontMetricData : public FontAttributes, public SvRefBase
{
public:
    explicit        ImplFontMetricData( const FontSelectPattern& );

    // font instance attributes from the font request
    long            GetWidth() const                                                { return mnWidth; }
    short           GetOrientation() const                                          { return mnOrientation; }

    void            SetWidth(long nWidth)                                           { mnWidth=nWidth; }
    void            SetOrientation(short nOrientation)                              { mnOrientation=nOrientation; }

    // font metrics measured for the font instance
    long            GetAscent() const                                               { return mnAscent; }
    long            GetDescent() const                                              { return mnDescent; }
    long            GetInternalLeading() const                                      { return mnIntLeading; }
    long            GetExternalLeading() const                                      { return mnExtLeading; }
    int             GetSlant() const                                                { return mnSlant; }
    long            GetMinKashida() const                                           { return mnMinKashida; }

    void            SetSlant(int nSlant)                                            { mnSlant=nSlant; }
    void            SetMinKashida( long nMinKashida )                               { mnMinKashida=nMinKashida; }

    // font attributes queried from the font instance
    bool            IsFullstopCentered() const                                      { return mbFullstopCentered; }
    long            GetBulletOffset() const                                         { return mnBulletOffset; }

    void            SetFullstopCenteredFlag(bool bFullstopCentered)                 { mbFullstopCentered = bFullstopCentered; }

    // font metrics that are usually derived from the measurements
    long            GetUnderlineSize() const                                        { return mnUnderlineSize; }
    long            GetUnderlineOffset() const                                      { return mnUnderlineOffset; }
    long            GetBoldUnderlineSize() const                                    { return mnBUnderlineSize; }
    long            GetBoldUnderlineOffset() const                                  { return mnBUnderlineOffset; }
    long            GetDoubleUnderlineSize() const                                  { return mnDUnderlineSize; }
    long            GetDoubleUnderlineOffset1() const                               { return mnDUnderlineOffset1; }
    long            GetDoubleUnderlineOffset2() const                               { return mnDUnderlineOffset2; }
    long            GetWavelineUnderlineSize() const                                { return mnWUnderlineSize; }
    long            GetWavelineUnderlineOffset() const                              { return mnWUnderlineOffset; }
    long            GetAboveUnderlineSize() const                                   { return mnAboveUnderlineSize; }
    long            GetAboveUnderlineOffset() const                                 { return mnAboveUnderlineOffset; }
    long            GetAboveBoldUnderlineSize() const                               { return mnAboveBUnderlineSize; }
    long            GetAboveBoldUnderlineOffset() const                             { return mnAboveBUnderlineOffset; }
    long            GetAboveDoubleUnderlineSize() const                             { return mnAboveDUnderlineSize; }
    long            GetAboveDoubleUnderlineOffset1() const                          { return mnAboveDUnderlineOffset1; }
    long            GetAboveDoubleUnderlineOffset2() const                          { return mnAboveDUnderlineOffset2; }
    long            GetAboveWavelineUnderlineSize() const                           { return mnAboveWUnderlineSize; }
    long            GetAboveWavelineUnderlineOffset() const                         { return mnAboveWUnderlineOffset; }
    long            GetStrikeoutSize() const                                        { return mnStrikeoutSize; }
    long            GetStrikeoutOffset() const                                      { return mnStrikeoutOffset; }
    long            GetBoldStrikeoutSize() const                                    { return mnBStrikeoutSize; }
    long            GetBoldStrikeoutOffset() const                                  { return mnBStrikeoutOffset; }
    long            GetDoubleStrikeoutSize() const                                  { return mnDStrikeoutSize; }
    long            GetDoubleStrikeoutOffset1() const                               { return mnDStrikeoutOffset1; }
    long            GetDoubleStrikeoutOffset2() const                               { return mnDStrikeoutOffset2; }

    void            ImplInitTextLineSize( const OutputDevice* pDev );
    void            ImplInitAboveTextLineSize();
    void            ImplInitFlags( const OutputDevice* pDev );
    void            ImplCalcLineSpacing(const std::vector<uint8_t>& rHhea,
                                        const std::vector<uint8_t>& rOS_2,
                                        int nUPEM);

private:
    bool            ShouldUseWinMetrics(const vcl::TTGlobalFontInfo& rInfo);

    // font instance attributes from the font request
    long const      mnHeight;                   // Font size
    long            mnWidth;                    // Reference Width
    short           mnOrientation;              // Rotation in 1/10 degrees

    // font metrics measured for the font instance
    long            mnAscent;                   // Ascent
    long            mnDescent;                  // Descent
    long            mnIntLeading;               // Internal Leading
    long            mnExtLeading;               // External Leading
    int             mnSlant;                    // Slant (Italic/Oblique)
    long            mnMinKashida;               // Minimal width of kashida (Arabic)

    // font attributes queried from the font instance
    bool            mbFullstopCentered;
    long            mnBulletOffset;             // Offset to position non-print character

    // font metrics that are usually derived from the measurements
    long            mnUnderlineSize;            // Lineheight of Underline
    long            mnUnderlineOffset;          // Offset from Underline to Baseline
    long            mnBUnderlineSize;           // Height of bold underline
    long            mnBUnderlineOffset;         // Offset from bold underline to baseline
    long            mnDUnderlineSize;           // Height of double underline
    long            mnDUnderlineOffset1;        // Offset from double underline to baseline
    long            mnDUnderlineOffset2;        // Offset from double underline to baseline
    long            mnWUnderlineSize;           // Height of WaveLine underline
    long            mnWUnderlineOffset;         // Offset from WaveLine underline to baseline, but centrered to WaveLine
    long            mnAboveUnderlineSize;       // Height of single underline (for Vertical Right)
    long            mnAboveUnderlineOffset;     // Offset from single underline to baseline (for Vertical Right)
    long            mnAboveBUnderlineSize;      // Height of bold underline (for Vertical Right)
    long            mnAboveBUnderlineOffset;    // Offset from bold underline to baseline (for Vertical Right)
    long            mnAboveDUnderlineSize;      // Height of double underline (for Vertical Right)
    long            mnAboveDUnderlineOffset1;   // Offset from double underline to baseline (for Vertical Right)
    long            mnAboveDUnderlineOffset2;   // Offset from double underline to baseline (for Vertical Right)
    long            mnAboveWUnderlineSize;      // Height of WaveLine-strike-out (for Vertical Right)
    long            mnAboveWUnderlineOffset;    // Offset from WaveLine-strike-out to baseline, but centrered to the WaveLine (for Vertical Right)
    long            mnStrikeoutSize;            // Height of single strike-out
    long            mnStrikeoutOffset;          // Offset from single strike-out to baseline
    long            mnBStrikeoutSize;           // Height of bold strike-out
    long            mnBStrikeoutOffset;         // Offset of bold strike-out to baseline
    long            mnDStrikeoutSize;           // Height of double strike-out
    long            mnDStrikeoutOffset1;        // Offset of double strike-out to baseline
    long            mnDStrikeoutOffset2;        // Offset of double strike-out to baseline

};

#endif // INCLUDED_VCL_INC_IMPFONTMETRICDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
