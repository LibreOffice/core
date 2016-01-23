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

#include "fontattributes.hxx"

#include <boost/intrusive_ptr.hpp>

class ImplFontMetricData;
typedef boost::intrusive_ptr< ImplFontMetricData > ImplFontMetricDataPtr;

class OutputDevice;
class FontSelectPattern;

class ImplFontMetricData : public FontAttributes
{
public:
    explicit        ImplFontMetricData( const FontSelectPattern& );

    // font instance attributes from the font request
    long            GetWidth()                                                      { return mnWidth; }
    short           GetOrientation()                                                { return mnOrientation; }

    void            SetWidth(long nWidth)                                           { mnWidth=nWidth; }
    void            SetOrientation(short nOrientation)                              { mnOrientation=nOrientation; }

    // font metrics measured for the font instance
    long            GetAscent()                                                     { return mnAscent; }
    long            GetDescent()                                                    { return mnDescent; }
    long            GetInternalLeading()                                            { return mnIntLeading; }
    long            GetExternalLeading()                                            { return mnExtLeading; }
    int             GetSlant()                                                      { return mnSlant; }
    long            GetMinKashida()                                                 { return mnMinKashida; }

    void            SetAscent(long nAscent)                                         { mnAscent=nAscent; }
    void            SetDescent(long nDescent)                                       { mnDescent=nDescent; }
    void            SetInternalLeading(long nIntLeading)                            { mnIntLeading = nIntLeading; }
    void            SetExternalLeading(long nExtLeading)                            { mnExtLeading = nExtLeading; }
    void            SetSlant(int nSlant)                                            { mnSlant=nSlant; }
    void            SetMinKashida( long nMinKashida )                               { mnMinKashida=nMinKashida; }

    // font attributes queried from the font instance
    bool            IsScalable()                                                    { return mbScalableFont; }
    bool            IsTrueTypeFont()                                                { return mbTrueTypeFont; }
    bool            IsKernable()                                                    { return mbKernableFont; }
    bool            IsFullstopCentered()                                            { return mbFullstopCentered; }
    long            GetBulletOffset()                                               { return mnBulletOffset; }

    void            SetScalableFlag(bool bScalable)                                 { mbScalableFont = bScalable; }
    void            SetTrueTypeFlag(bool bTrueTypeFont)                             { mbTrueTypeFont = bTrueTypeFont; }
    void            SetKernableFlag(bool bKernable)                                 { mbKernableFont = bKernable; }
    void            SetFullstopCenteredFlag(bool bFullstopCentered)                 { mbFullstopCentered = bFullstopCentered; }
    void            SetBulletOffset(long nBulletOffset)                             { mnBulletOffset = nBulletOffset; }

    // font metrics that are usually derived from the measurements
    long            GetUnderlineSize()                                              { return mnUnderlineSize; }
    long            GetUnderlineOffset()                                            { return mnUnderlineOffset; }
    long            GetBoldUnderlineSize()                                          { return mnBUnderlineSize; }
    long            GetBoldUnderlineOffset()                                        { return mnBUnderlineOffset; }
    long            GetDoubleUnderlineSize()                                        { return mnDUnderlineSize; }
    long            GetDoubleUnderlineOffset1()                                     { return mnDUnderlineOffset1; }
    long            GetDoubleUnderlineOffset2()                                     { return mnDUnderlineOffset2; }
    long            GetWavelineUnderlineSize()                                      { return mnWUnderlineSize; }
    long            GetWavelineUnderlineOffset()                                    { return mnWUnderlineOffset; }
    long            GetAboveUnderlineSize()                                         { return mnAboveUnderlineSize; }
    long            GetAboveUnderlineOffset()                                       { return mnAboveUnderlineOffset; }
    long            GetAboveBoldUnderlineSize()                                     { return mnAboveBUnderlineSize; }
    long            GetAboveBoldUnderlineOffset()                                   { return mnAboveBUnderlineOffset; }
    long            GetAboveDoubleUnderlineSize()                                   { return mnAboveDUnderlineSize; }
    long            GetAboveDoubleUnderlineOffset1()                                { return mnAboveDUnderlineOffset1; }
    long            GetAboveDoubleUnderlineOffset2()                                { return mnAboveDUnderlineOffset2; }
    long            GetAboveWavelineUnderlineSize()                                 { return mnAboveWUnderlineSize; }
    long            GetAboveWavelineUnderlineOffset()                               { return mnAboveWUnderlineOffset; }
    long            GetStrikeoutSize()                                              { return mnStrikeoutSize; }
    long            GetStrikeoutOffset()                                            { return mnStrikeoutOffset; }
    long            GetBoldStrikeoutSize()                                          { return mnBStrikeoutSize; }
    long            GetBoldStrikeoutOffset()                                        { return mnBStrikeoutOffset; }
    long            GetDoubleStrikeoutSize()                                        { return mnDStrikeoutSize; }
    long            GetDoubleStrikeoutOffset1()                                     { return mnDStrikeoutOffset1; }
    long            GetDoubleStrikeoutOffset2()                                     { return mnDStrikeoutOffset2; }

    void            SetUnderlineSize( long nUnderlineSize )                         { mnUnderlineSize = nUnderlineSize; }
    void            SetUnderlineOffset( long nUnderlineOffset )                     { mnUnderlineOffset = nUnderlineOffset; }
    void            SetBoldUnderlineSize( long nBUnderlineSize )                    { mnBUnderlineSize = nBUnderlineSize; }
    void            SetBoldUnderlineOffset( long nBUnderlineOffset )                { mnBUnderlineOffset = nBUnderlineOffset; }
    void            SetDoubleUnderlineSize( long nDUnderlineSize )                  { mnDUnderlineSize = nDUnderlineSize; }
    void            SetDoubleUnderlineOffset1( long nDUnderlineOffset1 )            { mnDUnderlineOffset1 = nDUnderlineOffset1; }
    void            SetDoubleUnderlineOffset2( long nDUnderlineOffset2 )            { mnDUnderlineOffset2 = nDUnderlineOffset2; }
    void            SetWavelineUnderlineSize( long nWUnderlineSize )                { mnWUnderlineSize = nWUnderlineSize; }
    void            SetWavelineUnderlineOffset( long nWUnderlineOffset )            { mnWUnderlineOffset = nWUnderlineOffset; }
    void            SetAboveUnderlineSize( long nAboveUnderlineSize )               { mnAboveUnderlineSize = nAboveUnderlineSize; }
    void            SetAboveUnderlineOffset( long nAboveUnderlineOffset )           { mnAboveUnderlineOffset = nAboveUnderlineOffset; }
    void            SetAboveBoldUnderlineSize( long nAboveBUnderlineSize )          { mnAboveBUnderlineSize = nAboveBUnderlineSize; }
    void            SetAboveBoldUnderlineOffset( long nAboveBUnderlineOffset )      { mnAboveBUnderlineOffset = nAboveBUnderlineOffset; }
    void            SetAboveDoubleUnderlineSize( long nAboveDUnderlineSize )        { mnAboveDUnderlineSize = nAboveDUnderlineSize; }
    void            SetAboveDoubleUnderlineOffset1( long nAboveDUnderlineOffset1 )  { mnAboveDUnderlineOffset1 = nAboveDUnderlineOffset1; }
    void            SetAboveDoubleUnderlineOffset2( long nAboveDUnderlineOffset2 )  { mnAboveDUnderlineOffset2 = nAboveDUnderlineOffset2; }
    void            SetAboveWavelineUnderlineSize( long nAboveWUnderlineSize )      { mnAboveWUnderlineSize = nAboveWUnderlineSize; }
    void            SetAboveWavelineUnderlineOffset( long nAboveWUnderlineOffset )  { mnAboveWUnderlineOffset = nAboveWUnderlineOffset; }
    void            SetStrikeoutSize( long nStrikeoutSize )                         { mnStrikeoutSize = nStrikeoutSize; }
    void            SetStrikeoutOffset( long nStrikeoutOffset )                     { mnStrikeoutOffset = nStrikeoutOffset; }
    void            SetBoldStrikeoutSize( long nBStrikeoutSize )                    { mnBStrikeoutSize = nBStrikeoutSize; }
    void            SetBoldStrikeoutOffset( long nBStrikeoutOffset )                { mnBStrikeoutOffset = nBStrikeoutOffset; }
    void            SetDoubleStrikeoutSize( long nDStrikeoutSize )                  { mnDStrikeoutSize = nDStrikeoutSize; }
    void            SetDoubleStrikeoutOffset1( long nDStrikeoutOffset1 )            { mnDStrikeoutOffset1 = nDStrikeoutOffset1; }
    void            SetDoubleStrikeoutOffset2( long nDStrikeoutOffset2 )            { mnDStrikeoutOffset2 = nDStrikeoutOffset2; }

    void            ImplInitTextLineSize( const OutputDevice* pDev );
    void            ImplInitAboveTextLineSize();

private:
    friend void intrusive_ptr_add_ref(ImplFontMetricData* pImplFontMetricData);
    friend void intrusive_ptr_release(ImplFontMetricData* pImplFontMetricData);

    long            mnRefCount;

    // font instance attributes from the font request
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
    bool            mbScalableFont;
    bool            mbTrueTypeFont;
    bool            mbKernableFont;
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

inline void intrusive_ptr_add_ref(ImplFontMetricData* pImplFontMetricData)
{
    ++pImplFontMetricData->mnRefCount;
}

inline void intrusive_ptr_release(ImplFontMetricData* pImplFontMetricData)
{
    if (--pImplFontMetricData->mnRefCount == 0)
        delete pImplFontMetricData;
}

#endif // INCLUDED_VCL_INC_IMPFONTMETRICDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
