/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsBitmapFactory.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_SLIDESORTER_PREVIEW_BITMAP_FACTORY_HXX
#define SD_SLIDESORTER_PREVIEW_BITMAP_FACTORY_HXX

#include "PreviewRenderer.hxx"
#include <boost/shared_ptr.hpp>

class BitmapEx;
class SdPage;
class Size;

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace cache {

/** This factory class creates preview bitmaps for page objects.  It is
    merely an adapter for the PreviewRenderer.
*/
class BitmapFactory
{
public:
    BitmapFactory (void);
    ~BitmapFactory (void);

    ::boost::shared_ptr<BitmapEx> CreateBitmap (
        const SdPage& rPage,
        const Size& rPixelSize);

private:
    PreviewRenderer maRenderer;
};



} } } // end of namespace ::sd::slidesorter::cache

#endif
