/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsBitmapFactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:17:07 $
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

#include "SlsBitmapFactory.hxx"

#include "PreviewRenderer.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
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
    : maRenderer(NULL,false)
{
}




BitmapFactory::~BitmapFactory (void)
{
}




::boost::shared_ptr<BitmapEx> BitmapFactory::CreateBitmap (
    const SdPage& rPage,
    const Size& rPixelSize)
{
    Image aPreview (maRenderer.RenderPage (
        &rPage,
        rPixelSize,
        String()));

    return ::boost::shared_ptr<BitmapEx>(new BitmapEx(aPreview.GetBitmapEx()));
}


} } } // end of namespace ::sd::slidesorter::cache
