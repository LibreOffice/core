/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BITMAPCOMBOSCALEFILTER_HXX
#define INCLUDED_VCL_BITMAPCOMBOSCALEFILTER_HXX

#include <vcl/BitmapFilter.hxx>

#include <comphelper/threadpool.hxx>

namespace vcl
{
class VCL_DLLPUBLIC BitmapComboScaleFilter : public BitmapFilter
{
private:
    double mrScaleX;
    double mrScaleY;

    std::shared_ptr<comphelper::ThreadTaskTag> mpThreadPoolTag;

    enum class ScaleType
    {
        NONE,
        HALF,
        HALF_HORIZONTAL,
        HALF_VERTICAL,
        QUARTER,
        QUARTER_HORIZONTAL,
        QUARTER_VERTICAL,
        OCTAL,
    };

    bool fastPrescale(Bitmap& rBitmap);
    bool scale(ScaleType type, Bitmap& rBitmap);

public:
    BitmapComboScaleFilter(const double& rScaleX, const double& rScaleY);
    ~BitmapComboScaleFilter() override;

    BitmapEx execute(BitmapEx const& rBitmap) const override;
};
}

#endif // INCLUDED_VCL_BITMAPSCALESUPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
