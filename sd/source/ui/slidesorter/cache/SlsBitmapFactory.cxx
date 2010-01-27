/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsBitmapFactory.cxx,v $
 *
 * $Revision: 1.5 $
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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "precompiled_sd.hxx"

#include "SlsBitmapFactory.hxx"

#include "PreviewRenderer.hxx"
#include "view/SlideSorterView.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include <svx/svdtypes.hxx>
#include <svx/svdpage.hxx>
#include <vcl/bitmapex.hxx>

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




::boost::shared_ptr<BitmapEx> BitmapFactory::CreateBitmap (
    const SdPage& rPage,
    const Size& rPixelSize,
    const bool bDoSuperSampling)
{
    Size aSize (rPixelSize);
    if (bDoSuperSampling)
    {
        aSize.Width() *= 2;
        aSize.Height() *= 2;
    }

    const Image aPreview (maRenderer.RenderPage (&rPage, aSize, String()));

    ::boost::shared_ptr<BitmapEx> pPreview (new BitmapEx(aPreview.GetBitmapEx()));
    if (bDoSuperSampling)
        pPreview->Scale(rPixelSize, BMP_SCALE_INTERPOLATE);

    return pPreview;
}


} } } // end of namespace ::sd::slidesorter::cache



