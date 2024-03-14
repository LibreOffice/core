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

#pragma once

#include <sal/config.h>

#include <vcl/dllapi.h>
#include <tools/degree.hxx>
#include <tools/long.hxx>
#include <tools/ref.hxx>
#include <fontattributes.hxx>

class FontMetricData;
typedef tools::SvRef<FontMetricData> FontMetricDataRef;

class OutputDevice;
namespace vcl::font
{
class FontSelectPattern;
}
class LogicalFontInstance;

class VCL_DLLPUBLIC FontMetricData final : public FontAttributes, public SvRefBase
{
public:
    SAL_DLLPRIVATE explicit FontMetricData( const vcl::font::FontSelectPattern& );

    // font instance attributes from the font request
    tools::Long            GetWidth() const                                                { return mnWidth; }
    Degree10        GetOrientation() const                                          { return mnOrientation; }

    void            SetWidth(tools::Long nWidth)                                           { mnWidth=nWidth; }
    void            SetOrientation(Degree10 nOrientation)                               { mnOrientation=nOrientation; }

    // font metrics measured for the font instance
    tools::Long            GetAscent() const                                               { return mnAscent; }
    tools::Long            GetDescent() const                                              { return mnDescent; }
    tools::Long            GetInternalLeading() const                                      { return mnIntLeading; }
    tools::Long            GetExternalLeading() const                                      { return mnExtLeading; }
    int             GetSlant() const                                                { return mnSlant; }
    double                 GetMinKashida() const                                           { return mnMinKashida; }
    tools::Long            GetHangingBaseline() const                                      { return mnHangingBaseline; }

    void            SetSlant(int nSlant)                                            { mnSlant=nSlant; }
    void            SetMinKashida(double nMinKashida )                                     { mnMinKashida=nMinKashida; }

    // font attributes queried from the font instance
    bool            IsFullstopCentered() const                                      { return mbFullstopCentered; }
    tools::Long            GetBulletOffset() const                                         { return mnBulletOffset; }

    void            SetFullstopCenteredFlag(bool bFullstopCentered)                 { mbFullstopCentered = bFullstopCentered; }

    // font metrics that are usually derived from the measurements
    tools::Long            GetUnderlineSize() const                                        { return mnUnderlineSize; }
    tools::Long            GetUnderlineOffset() const                                      { return mnUnderlineOffset; }
    tools::Long            GetBoldUnderlineSize() const                                    { return mnBUnderlineSize; }
    tools::Long            GetBoldUnderlineOffset() const                                  { return mnBUnderlineOffset; }
    tools::Long            GetDoubleUnderlineSize() const                                  { return mnDUnderlineSize; }
    tools::Long            GetDoubleUnderlineOffset1() const                               { return mnDUnderlineOffset1; }
    tools::Long            GetDoubleUnderlineOffset2() const                               { return mnDUnderlineOffset2; }
    tools::Long            GetWavelineUnderlineSize() const                                { return mnWUnderlineSize; }
    tools::Long            GetWavelineUnderlineOffset() const                              { return mnWUnderlineOffset; }
    tools::Long            GetAboveUnderlineSize() const                                   { return mnAboveUnderlineSize; }
    tools::Long            GetAboveUnderlineOffset() const                                 { return mnAboveUnderlineOffset; }
    tools::Long            GetAboveBoldUnderlineSize() const                               { return mnAboveBUnderlineSize; }
    tools::Long            GetAboveBoldUnderlineOffset() const                             { return mnAboveBUnderlineOffset; }
    tools::Long            GetAboveDoubleUnderlineSize() const                             { return mnAboveDUnderlineSize; }
    tools::Long            GetAboveDoubleUnderlineOffset1() const                          { return mnAboveDUnderlineOffset1; }
    tools::Long            GetAboveDoubleUnderlineOffset2() const                          { return mnAboveDUnderlineOffset2; }
    tools::Long            GetAboveWavelineUnderlineSize() const                           { return mnAboveWUnderlineSize; }
    tools::Long            GetAboveWavelineUnderlineOffset() const                         { return mnAboveWUnderlineOffset; }
    tools::Long            GetStrikeoutSize() const                                        { return mnStrikeoutSize; }
    tools::Long            GetStrikeoutOffset() const                                      { return mnStrikeoutOffset; }
    tools::Long            GetBoldStrikeoutSize() const                                    { return mnBStrikeoutSize; }
    tools::Long            GetBoldStrikeoutOffset() const                                  { return mnBStrikeoutOffset; }
    tools::Long            GetDoubleStrikeoutSize() const                                  { return mnDStrikeoutSize; }
    tools::Long            GetDoubleStrikeoutOffset1() const                               { return mnDStrikeoutOffset1; }
    tools::Long            GetDoubleStrikeoutOffset2() const                               { return mnDStrikeoutOffset2; }

    SAL_DLLPRIVATE void ImplInitTextLineSize( const OutputDevice* pDev );
    SAL_DLLPRIVATE void ImplInitAboveTextLineSize( const OutputDevice* pDev );
    SAL_DLLPRIVATE void ImplInitFlags( const OutputDevice* pDev );
    void            ImplCalcLineSpacing(LogicalFontInstance *pFontInstance);
    void            ImplInitBaselines(LogicalFontInstance *pFontInstance);

private:
    SAL_DLLPRIVATE bool ShouldNotUseUnderlineMetrics() const;
    SAL_DLLPRIVATE bool ImplInitTextLineSizeHarfBuzz(LogicalFontInstance *pFontInstance);
    SAL_DLLPRIVATE bool ShouldUseWinMetrics(int, int, int, int, int, int) const;

    // font instance attributes from the font request
    tools::Long            mnHeight;                   // Font size
    tools::Long            mnWidth;                    // Reference Width
    Degree10        mnOrientation;              // Rotation in 1/10 degrees

    // font metrics measured for the font instance
    tools::Long            mnAscent;                   // Ascent
    tools::Long            mnDescent;                  // Descent
    tools::Long            mnIntLeading;               // Internal Leading
    tools::Long            mnExtLeading;               // External Leading
    int             mnSlant;                    // Slant (Italic/Oblique)
    double                 mnMinKashida;               // Minimal width of kashida (Arabic)
    tools::Long            mnHangingBaseline;          // Offset of hanging baseline to Romn baseline

    // font attributes queried from the font instance
    bool            mbFullstopCentered;
    tools::Long            mnBulletOffset;             // Offset to position non-print character

    // font metrics that are usually derived from the measurements
    tools::Long            mnUnderlineSize;            // Lineheight of Underline
    tools::Long            mnUnderlineOffset;          // Offset from Underline to Baseline
    tools::Long            mnBUnderlineSize;           // Height of bold underline
    tools::Long            mnBUnderlineOffset;         // Offset from bold underline to baseline
    tools::Long            mnDUnderlineSize;           // Height of double underline
    tools::Long            mnDUnderlineOffset1;        // Offset from double underline to baseline
    tools::Long            mnDUnderlineOffset2;        // Offset from double underline to baseline
    tools::Long            mnWUnderlineSize;           // Height of WaveLine underline
    tools::Long            mnWUnderlineOffset;         // Offset from WaveLine underline to baseline, but centrered to WaveLine
    tools::Long            mnAboveUnderlineSize;       // Height of single underline (for Vertical Right)
    tools::Long            mnAboveUnderlineOffset;     // Offset from single underline to baseline (for Vertical Right)
    tools::Long            mnAboveBUnderlineSize;      // Height of bold underline (for Vertical Right)
    tools::Long            mnAboveBUnderlineOffset;    // Offset from bold underline to baseline (for Vertical Right)
    tools::Long            mnAboveDUnderlineSize;      // Height of double underline (for Vertical Right)
    tools::Long            mnAboveDUnderlineOffset1;   // Offset from double underline to baseline (for Vertical Right)
    tools::Long            mnAboveDUnderlineOffset2;   // Offset from double underline to baseline (for Vertical Right)
    tools::Long            mnAboveWUnderlineSize;      // Height of WaveLine-strike-out (for Vertical Right)
    tools::Long            mnAboveWUnderlineOffset;    // Offset from WaveLine-strike-out to baseline, but centrered to the WaveLine (for Vertical Right)
    tools::Long            mnStrikeoutSize;            // Height of single strike-out
    tools::Long            mnStrikeoutOffset;          // Offset from single strike-out to baseline
    tools::Long            mnBStrikeoutSize;           // Height of bold strike-out
    tools::Long            mnBStrikeoutOffset;         // Offset of bold strike-out to baseline
    tools::Long            mnDStrikeoutSize;           // Height of double strike-out
    tools::Long            mnDStrikeoutOffset1;        // Offset of double strike-out to baseline
    tools::Long            mnDStrikeoutOffset2;        // Offset of double strike-out to baseline

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
