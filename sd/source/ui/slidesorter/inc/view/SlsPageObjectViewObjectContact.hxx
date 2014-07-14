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



#ifndef SD_SLIDESORTER_PAGE_OBJECT_VIEW_OBJECT_CONTACT_HXX
#define SD_SLIDESORTER_PAGE_OBJECT_VIEW_OBJECT_CONTACT_HXX

#include <svx/sdr/contact/viewobjectcontactofpageobj.hxx>
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

class SlideSorterView;

/** This object-view-contact of page objects maintains a preview bitmap for
    the page to speed up redraws of the same.  It does so by colaborating
    with a cache of bitmaps (see ../cache).
*/

// needs to be derived from ViewObjectContactOfPageObj, else the calls to parent implementations
// would use ViewObjectContact and thus not enable e.g. the correct primitive creation
// for view-independent printer output
// changed: ViewObjectContact -> ViewObjectContactOfPageObj

class PageObjectViewObjectContact : public ::sdr::contact::ViewObjectContactOfPageObj
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

    /** Return the page that is painted by this object.
    */
    const SdrPage* GetPage (void) const;

    /** This fallback method is called when no preview cache is available.
        It creates a preview for the page.
    */
    BitmapEx CreatePreview (const sdr::contact::DisplayInfo& rDisplayInfo);

    /** Return the page descriptor of the slide sorter model that is
        associated with the same page object as this contact object is.
    */
    model::SharedPageDescriptor GetPageDescriptor (void) const;

    /** Return the border widths in the screen coordinate system of the
        border around the page object.  The border contains frames for
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

    /** Paint a mouse over effect.
        @param bVisible
            When bVisible is <false/> then paint the area of the mouse over
            effect in the background color, i.e. erase it.
    */
    drawinglayer::primitive2d::Primitive2DSequence createMouseOverEffectPrimitive2DSequence();

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
    basegfx::B2DRange GetBoundingBox (
        OutputDevice& rDevice,
        BoundingBoxType eType,
        CoordinateSystem eCoordinateSystem) const;

    // create the graphical visualisation data
    virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const sdr::contact::DisplayInfo& rDisplayInfo) const;

    // access to the current page content primitive vector which may be used for visualisation
    const drawinglayer::primitive2d::Primitive2DSequence& getCurrentPageContents() const { return mxCurrentPageContents; }

    virtual void ActionChanged (void);

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

    /** This flag is set to <true/> when the destructor is called to
        indicate that further calls made to it must not call outside.
    */
    bool mbInDestructor;

    /// The primitive sequence of the page contents, completely scaled
    /// and prepared for painiting
    drawinglayer::primitive2d::Primitive2DSequence      mxCurrentPageContents;

    ::boost::shared_ptr<cache::PageCache> mpCache;

    ::boost::shared_ptr<controller::Properties> mpProperties;

    BitmapEx GetPreview (
        const sdr::contact::DisplayInfo& rDisplayInfo,
        const basegfx::B2DRange& rNewSizePixel);

    /** Return the bounding box of where the page number is painted (when it
        is painted).
    */
    Rectangle GetPageNumberArea (OutputDevice& rDevice) const;
};

} } } // end of namespace ::sd::slidesorter::view

#endif
