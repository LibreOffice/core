/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
        String(),
        true,
        false).GetBitmapEx().GetBitmap());
    if (bDoSuperSampling)
    {
        aPreview.Scale(rPixelSize, BMP_SCALE_BEST);
    }

    return aPreview;
}


} } } // end of namespace ::sd::slidesorter::cache



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
