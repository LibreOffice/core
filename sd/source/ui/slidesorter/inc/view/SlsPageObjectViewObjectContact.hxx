/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectViewObjectContact.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:40:01 $
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_VIEW_OBJECT_CONTACT_HXX
#define SD_SLIDESORTER_PAGE_OBJECT_VIEW_OBJECT_CONTACT_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>
#include <sfx2/viewfrm.hxx>

#include <memory>
#include <boost/shared_ptr.hpp>

class SdrPage;

namespace sdr { namespace contact {
class DisplayInfo;
} }

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }

namespace sd { namespace slidesorter { namespace controller {
class Properties;
} } }

namespace sd { namespace slidesorter { namespace view {

class PageNotificationObjectContact;
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
        const ::boost::shared_ptr<cache::PageCache>& rpCache,
        const ::boost::shared_ptr<controller::Properties>& rpProperties);
    virtual ~PageObjectViewObjectContact (void);

    /** This method is primarily for releasing the current preview cache (by
        providing a NULL pointer.)
    */
    void SetCache (const ::boost::shared_ptr<cache::PageCache>& rpCache);

    virtual void PaintObject (::sdr::contact::DisplayInfo& rDisplayInfo);

    /** The object is about to be deleted.  Tell the cache that it has not
        to rendere a preview anymore and absolutely must not call back.
    */
    virtual void PrepareDelete (void);

    /** Return the page that is painted by this object.
    */
    const SdrPage* GetPage (void) const;

    /** This fallback method is called when no preview cache is available.
        It creates a preview for the page.
    */
    BitmapEx CreatePreview (OutputDevice& rDevice) const;

    /** This paint method calls the more specialized paint methods
        that paint single aspects of the content.
    */
    virtual void PaintContent (OutputDevice& rDevice);

    /** Return the page descriptor of the slide sorter model that is
        associated with the same page object as this contact object is.
    */
    model::SharedPageDescriptor GetPageDescriptor (void) const;

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

    /** Paint all parts of the frame arround a preview.  These are the
        border, the selection frame, the focus rectangle, and the mouse over
        effect.
        @param rDevice
            The output device to paint on.
        @param bShowMouseOverEffect
            This flag specifies whether to paint the mouse over effect or not.
    */
    void PaintFrame (
        OutputDevice& rDevice,
        bool bShowMouseOverEffect = false) const;

    /** Paint a mouse over effect.
        @param bVisible
            When bVisible is <FALSE/> then paint the area of the mouse over
            effect in the background color, i.e. erase it.
    */
    void PaintMouseOverEffect (OutputDevice& rDevice, bool bVisible) const;

    enum BoundingBoxType {
        // This is the outer bounding box that includes the preview, page
        // number, title.
        PageObjectBoundingBox,
        // Bounding box of the actual preview.
        PreviewBoundingBox,
        // Bounding box of the mouse indicator indicator frame.
        MouseOverIndicatorBoundingBox,
        // Bounding box of the focus indicator frame.
        FocusIndicatorBoundingBox,
        // Bounding box of the selection indicator frame.
        SelectionIndicatorBoundingBox,
        // Bounding box of the page number.
        PageNumberBoundingBox,
        // Bounding box of the pane name.
        NameBoundingBox,
        FadeEffectIndicatorBoundingBox
    };
    enum CoordinateSystem { ModelCoordinateSystem, PixelCoordinateSystem };
    /** Return the bounding box of the page object or one of its graphical
        parts.
        @param rDevice
            This device is used to translate between model and window
            coordinates.
        @param eType
            The part of the page object for which to return the bounding
            box.
        @param eCoodinateSystem
            The bounding box can be returned in model and in pixel
            (window) coordinates.
    */
    Rectangle GetBoundingBox (
        OutputDevice& rDevice,
        BoundingBoxType eType,
        CoordinateSystem eCoordinateSystem) const;

    /** This convenience method paints a dotted or dashed rectangle.  The
        length of dots or dashes is indepent of zoom factor or map mode.
    */
    enum DashType { Dotted, Dashed };
    static void PaintDottedRectangle (
        OutputDevice& rDevice,
        const Rectangle& rRectangle,
        const DashType eDashType = Dotted);


    enum ColorSpec { CS_SELECTION, CS_BACKGROUND, CS_WINDOW, CS_TEXT };
    /** Return a color for one of the screen elements in ColorSpec.  For
        Background the background color is updated when
        mbIsBackgroundColorUpdatePending is <TRUE/>.
        @param rDevice
            Base colors are taken from the device.
        @param eSpec
            The type of color to return.
        @param nOpacity
            This parameter controls the blending between the background and
            the actual color.
    */
    Color GetColor (
        const OutputDevice& rDevice,
        const ColorSpec eSpec,
        const double nOpacity = 1.0) const;

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
    /// Offset and thickness of the mouse over effect rectangle.
    static const sal_Int32 mnMouseOverEffectOffset;
    static const sal_Int32 mnMouseOverEffectThickness;

    model::SharedPageDescriptor mpPageDescriptor;

    /** This flag is set to <FALSE/> when PrepareDelete() is called to
        indicate that further calls made to it must not call outside.
    */
    bool mbIsValid;

    bool mbInPrepareDelete;

    /** Set this flag to <TRUE/> to update the background color on the next
        call to GetBackgroundColor().
    */
    mutable bool mbIsBackgroundColorUpdatePending;

    ::boost::shared_ptr<cache::PageCache> mpCache;

    ::std::auto_ptr<PageNotificationObjectContact> mpNotifier;

    ::boost::shared_ptr<controller::Properties> mpProperties;

    /** Do not use this member directly.  Use GetColor(Background) instead.
        That method determines the background color when
        mbIsBackgroundColorUpdatePending is <TRUE/>.
    */
    mutable Color maBackgroundColor;

    BitmapEx GetPreview (
        OutputDevice& rDevice,
        const Rectangle& rNewSizePixel);

    virtual void ActionChanged (void);

    /** Return the bounding box of where the page number is painted (when it
        is painted).
    */
    Rectangle GetPageNumberArea (OutputDevice& rDevice) const;

    void PaintBackground (OutputDevice& rDevice) const;

    /** Paint the preview bitmap.
    */
    void PaintPreview (OutputDevice& rDevice);

    /** Paint a border arround the page preview.
    */
    void PaintBorder (OutputDevice& rDevice) const;

    /** Paint the focus indicator for the specified page.
    */
    void PaintFocusIndicator (
        OutputDevice& rDevice,
        bool bEraseBackground) const;

    /** Paint the selection indicator when the page is currently selected.
        Otherwise the call is ignored.
    */
    void PaintSelectionIndicator (OutputDevice& rDevice) const;

    /** Paint the fade effect indicator which indicates whether a fade
        effect is currently associated with a page.
        @param rDescriptor
            The descriptor of the page for which to paint the fade effect
            indicator.
    */
    void PaintFadeEffectIndicator (OutputDevice& rDevice) const;

    /** Paint the name of the page to the bottom right of the page object.
    */
    void PaintPageName (OutputDevice& rDevice) const;

    /** Paint the number of the page to the upper left of the page object.
    */
    void PaintPageNumber (OutputDevice& rDevice) const;

    Color GetBackgroundColor (const OutputDevice& rDevice) const;
};

} } } // end of namespace ::sd::slidesorter::view

#endif
