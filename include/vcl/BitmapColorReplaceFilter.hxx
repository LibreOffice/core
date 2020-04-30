/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPCOLORREPLACEFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPCOLORREPLACEFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapColorReplaceFilter final : public BitmapFilter
{
public:
    /** Replaces a color with another by changing pixels, without shortcuts like modifying palette
        (that is how it's different from using Bitmap::Replace).

        @param cReplaceWhat
        Color that will be replaced.

        @param cReplaceTo
        New color that will replace cReplaceWhat.

     */
    BitmapColorReplaceFilter(const Color& cReplaceWhat, const Color& cReplaceTo)
        : m_aReplaceWhat(cReplaceWhat)
        , m_aReplaceTo(cReplaceTo)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    Color m_aReplaceWhat;
    Color m_aReplaceTo;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
