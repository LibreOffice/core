/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprimitive2d.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_PAINTER_HEADER
#define SD_SLIDESORTER_PAGE_OBJECT_PAINTER_HEADER

#include "SlideSorter.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include <boost/scoped_ptr.hpp>

namespace sd { namespace slidesorter { namespace cache { class PageCache; } } }

namespace sd { namespace slidesorter { namespace view {

class Layouter;
class PageObjectLayouter;


class PageObjectPainter
{
public:
    PageObjectPainter (const SlideSorter& rSlideSorter);
    ~PageObjectPainter (void);

    void PaintPageObject (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor);

    void NotifyResize (void);

private:
    const Layouter& mrLayouter;
    ::boost::shared_ptr<PageObjectLayouter> mpPageObjectLayouter;
    ::boost::shared_ptr<cache::PageCache> mpCache;
    ::boost::shared_ptr<controller::Properties> mpProperties;
    ::boost::shared_ptr<view::Theme> mpTheme;
    ::boost::shared_ptr<Font> mpPageNumberFont;
    BitmapEx maStartPresentationIcon;
    BitmapEx maShowSlideIcon;
    BitmapEx maNewSlideIcon;
    class FramePainter;
    ::boost::scoped_ptr<FramePainter> mpShadowPainter;
    Bitmap maNormalBackground;
    Bitmap maSelectionBackground;
    Bitmap maMouseOverBackground;

    void PaintBackground (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintPreview (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintPageNumber (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintTransitionEffect (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintButtons (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PrepareBackgrounds (OutputDevice& rDevice);
    Bitmap CreateBackgroundBitmap(
        const OutputDevice& rReferenceDevice,
        const Theme::GradientColorType eType) const;
};

} } } // end of namespace sd::slidesorter::view

#endif
