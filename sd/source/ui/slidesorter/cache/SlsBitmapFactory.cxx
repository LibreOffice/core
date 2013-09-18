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


#include "SlsBitmapFactory.hxx"

#include "PreviewRenderer.hxx"
#include "view/SlideSorterView.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include <drawdoc.hxx>
#include "DrawDocShell.hxx"
#include <svx/svdtypes.hxx>
#include <svx/svdpage.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/pngwrite.hxx>

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
class PageObjectViewObjectContact;
} } }

namespace sd { namespace slidesorter { namespace cache {

BitmapFactory::BitmapFactory (void)
    : maRenderer(NULL, false)
{
}




BitmapFactory::~BitmapFactory (void)
{
}




Bitmap BitmapFactory::CreateBitmap (
    const SdPage& rPage,
    const Size& rPixelSize,
    const bool bDoSuperSampling)
{
    Size aSize (rPixelSize);
    if (bDoSuperSampling)
    {
        // Supersampling factor
        int aSuperSamplingFactor = 2;
        aSize.Width() *= aSuperSamplingFactor;
        aSize.Height() *= aSuperSamplingFactor;
    }

    Bitmap aPreview (maRenderer.RenderPage (
        &rPage,
        aSize,
        OUString(),
        true,
        false).GetBitmapEx().GetBitmap());
    if (bDoSuperSampling)
    {
        aPreview.Scale(rPixelSize, BMP_SCALE_BESTQUALITY);
    }

    return aPreview;
}


} } } // end of namespace ::sd::slidesorter::cache



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
