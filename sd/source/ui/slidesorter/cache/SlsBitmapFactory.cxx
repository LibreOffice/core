/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "precompiled_sd.hxx"

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

const static sal_Int32 gnSuperSampleFactor (2);
const static bool gbAllowSuperSampling (false);


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
    if (bDoSuperSampling && gbAllowSuperSampling)
    {
        aSize.Width() *= gnSuperSampleFactor;
        aSize.Height() *= gnSuperSampleFactor;
    }

    Bitmap aPreview (maRenderer.RenderPage (
        &rPage,
        aSize,
        String(),
        true,
        false).GetBitmapEx().GetBitmap());
    if (bDoSuperSampling && gbAllowSuperSampling)
    {
        aPreview.Scale(rPixelSize, BMP_SCALE_INTERPOLATE);
    }

    return aPreview;
}


} } } // end of namespace ::sd::slidesorter::cache



