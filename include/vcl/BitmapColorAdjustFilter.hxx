/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPCOLORADJUSTFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPCOLORADJUSTFILTER_HXX

#include <tools/color.hxx>

#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapColorAdjustFilter : public BitmapFilter
{
public:
    /** Change various global color characteristics

        @param nLuminancePercent
        Percent of luminance change, valid range [-100,100]. Values outside this range
        are clipped to the valid range.

        @param nContrastPercent
        Percent of contrast change, valid range [-100,100]. Values outside this range
        are clipped to the valid range.

        @param nChannelRPercent
        Percent of red channel change, valid range [-100,100]. Values outside this range
        are clipped to the valid range.

        @param nChannelGPercent
        Percent of green channel change, valid range [-100,100]. Values outside this range
        are clipped to the valid range.

        @param nChannelBPercent
        Percent of blue channel change, valid range [-100,100]. Values outside this range
        are clipped to the valid range.

        @param fGamma
        Exponent of the gamma function applied to the bitmap. The value 1.0 results in no
        change, the valid range is (0.0,10.0]. Values outside this range are regarded as 1.0.

        @param bInvert
        If true, invert the channel values with the logical 'not' operator

        @param msoBrightness
        Use the same formula for brightness as used by MSOffice.
     */
    BitmapColorAdjustFilter(short nLuminancePercent, short nContrastPercent = 0,
                            short nChannelRPercent = 0, short nChannelGPercent = 0,
                            short nChannelBPercent = 0, double fGamma = 1.0, bool bInvert = false,
                            bool bMsoBrightness = false)
        : mnLuminancePercent(nLuminancePercent)
        , mnContrastPercent(nContrastPercent)
        , mnChannelRPercent(nChannelRPercent)
        , mnChannelGPercent(nChannelGPercent)
        , mnChannelBPercent(nChannelBPercent)
        , mfGamma(fGamma)
        , mbInvert(bInvert)
        , mbMsoBrightness(bMsoBrightness)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) override;

private:
    short mnLuminancePercent;
    short mnContrastPercent;
    short mnChannelRPercent;
    short mnChannelGPercent;
    short mnChannelBPercent;
    double mfGamma;
    bool mbInvert;
    bool mbMsoBrightness;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
