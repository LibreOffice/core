/*************************************************************************
 *
 *  $RCSfile: SlsPageObjectViewObjectContact.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:57:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_PAGE_OBJECT_VIEW_OBJECT_CONTACT_HXX
#define SD_SLIDESORTER_PAGE_OBJECT_VIEW_OBJECT_CONTACT_HXX

#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>
#include <so3/iface.hxx>

class SdrPage;

namespace sdr { namespace contact {
class DisplayInfo;
} }

namespace sd { namespace slidesorter { namespace model {
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }

namespace sd { namespace slidesorter { namespace view {

class SlideSorterView;

/** This object-view-contact of page objects maintains a preview bitmap for
    the page to speed up redraws of the same.  It does so by colaborating
    with a cache of bitmaps (see ../cache).

    This class started as a subclass of ::sdr::contact::VOCBitmapBuffer but
    the class specific code had to be copied and adapted.  Therefore, it is
    now derived directly from ::sdr::contact::ViewObjectContact.
*/
class PageObjectViewObjectContact
    : public ::sdr::contact::ViewObjectContact
{
public:
    /** Create a new view-object-contact object for the given contact
        objects of both model and view.
        @param pCache
            The caller should provide a pointer to a preview cache, if
            available.  If given then the cache is used to control when to
            create a preview and to store it.  If NULL is given then the
            preview is created every time when requested.
    */
    PageObjectViewObjectContact (
        ::sdr::contact::ObjectContact& rObjectContact,
        ::sdr::contact::ViewContact& rViewContact,
        cache::PageCache* pCache);
    virtual void PaintObject (::sdr::contact::DisplayInfo& rDisplayInfo);

    /** The object is about to be deleted.  Tell the cache that it has not
        to rendere a preview anymore and absolutely must not call back.
    */
    virtual void PrepareDelete (void);

    /** Return the page that is painted by this object.
    */
    const SdrPage* GetPage (void) const;

    /** Create a preview for the page (as returned by GetPage()) according
        to the given display info.
    */
    BitmapEx CreatePreview (::sdr::contact::DisplayInfo& rDisplayInfo);

    /** This paint method simply calls the more specialized paint methods
        that paint single aspects of the content.  If you want to add or
        remove an aspect then overload this method.
    */
    virtual void PaintContent (
        ::sdr::contact::DisplayInfo& rDisplayInfo);

    Rectangle GetModelBoundingBox (void) const;

    /** Return the page descriptor of the slide sorter model that is
        associated with the same page object as this contact object is.
    */
    model::PageDescriptor& GetPageDescriptor (void) const;

    /** Return the device independent part of the page border.  This border
        is returned in pixel coordinates.  Note that the device dependent
        parts--for the page number and the page title--have to be added.
        This is done by CalculatePageModelBorder().
    */
    static SvBorder GetPagePixelBorder (void);

    /** Return the border widths in the screen coordinate system of the
        border arround the page object.  The border contains frames for
        selection, focus, the page name and number, and the indicator for
        the page transition.
        @param pDevice
            The output device is used to convert pixel coordinates into
            model coordinates.  When NULL is given then the device dependent
            part is not re-calculated but taken from an earlier calculation
            or from the default values.
        @param nPageCount
            The total number of pages is used to determine the width of the
            box that contains the page number.
    */
    static SvBorder CalculatePageModelBorder (
        OutputDevice* pDevice,
        int nPageCount);

    /** Calculate the size of the page number area so that all page numbers
        including the given number fit in.  Because this is device dependent
        we need the device as parameter.  The result is returned and stored
        in maPageNumberAreaPixelSize so that it can be used later without
        access to the device or page count.
    */
    static Size CalculatePageNumberAreaModelSize (
        OutputDevice* pDevice,
        int nPageCount);

    /** Return the bounding box of where the fade effect indicator is
        painted (when it is painted).
    */
    Rectangle GetFadeEffectIndicatorArea (OutputDevice* pDevice) const;

private:
    /// Gap between border of page object and inside of selection rectangle.
    static const sal_Int32 mnSelectionIndicatorOffset;
    /// Thickness of the selection rectangle.
    static const sal_Int32 mnSelectionIndicatorThickness;
    /// Gap between border of page object and inside of focus rectangle.
    static const sal_Int32 mnFocusIndicatorOffset;
    /// Size of width and height of the fade effect indicator in pixels.
    static const sal_Int32 mnFadeEffectIndicatorSize;
    static const sal_Int32 mnFadeEffectIndicatorOffset;
    /// Gap between border of page object and number rectangle.
    static const sal_Int32 mnPageNumberOffset;

    /** This flag is set to <FALSE/> when PrepareDelete() is called to
        indicate that further calls made to it must not call outside.
    */
    bool mbIsValid;

    cache::PageCache* mpCache;

    BitmapEx GetPreview (
        ::sdr::contact::DisplayInfo& rDisplayInfo,
        const Rectangle& rNewSizePixel);
    /** Return the rectangle of the whole page object, the preview toghether
        with frames, indicators, and title, in pixel coordinates.
    */
    Rectangle GetPixelBox (::sdr::contact::DisplayInfo& rDisplayInfo);
    /** Return the rectangle of the preview in pixel coordinates.
    */
    Rectangle GetPreviewPixelBox (
        ::sdr::contact::DisplayInfo& rDisplayInfo);

    virtual void ActionChanged (void);

    /** Return the bounding box of where the page number is painted (when it
        is painted).
    */
    Rectangle GetPageNumberArea (OutputDevice* pDevice);

    /** Paint the preview bitmap.
    */
    void PaintPreview (::sdr::contact::DisplayInfo& rDisplayInfo);

    /** Paint a frame arround the page preview.
    */
    void PaintFrame (::sdr::contact::DisplayInfo& rDisplayInfo) const;

    /** Paint the focus indicator for the specified page.
        @param bVisible
            Indicates whether the focus indicator is visible or not, i.e. a
            previously visible focus indicator is erased.
    */
    void PaintFocusIndicator (
        ::sdr::contact::DisplayInfo& rDisplayInfo) const;

    /** Paint the fade effect indicator which indicates whether a fade
        effect is currently associated with a page.
        @param rDescriptor
            The descriptor of the page for which to paint the fade effect
            indicator.
        @param bHighlight
            This flag specifies whether to paint the indicator highlighted
            (i.e. to indicate a mouse over effect) or the normal way.
    */
    void PaintFadeEffectIndicator (
        ::sdr::contact::DisplayInfo& rDisplayInfo,
        bool bHighlight = false) const;

    /** Paint the selection indicator when the page is currently selected.
        Otherwise the call is ignored.
    */
    void PaintSelectionIndicator (
        ::sdr::contact::DisplayInfo& rDisplayInfo) const;

    /** Paint the name of the page to the bottom right of the page object.
    */
    void PaintPageName (
        ::sdr::contact::DisplayInfo& rDisplayInfo) const;

    /** Paint the number of the page to the upper left of the page object.
    */
    void PaintPageNumber (
        ::sdr::contact::DisplayInfo& rDisplayInfo);
};

} } } // end of namespace ::sd::slidesorter::view

#endif
