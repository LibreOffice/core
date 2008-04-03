/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsBitmapFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:17:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
