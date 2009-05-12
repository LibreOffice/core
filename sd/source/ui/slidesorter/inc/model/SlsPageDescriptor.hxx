/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsPageDescriptor.hxx,v $
 * $Revision: 1.8 $
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

#ifndef SD_SLIDESORTER_PAGE_DESCRIPTOR_HXX
#define SD_SLIDESORTER_PAGE_DESCRIPTOR_HXX

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/bitmap.hxx>
#include <sfx2/viewfrm.hxx>

#include <memory>
#include <boost/enable_shared_from_this.hpp>

class SdPage;

namespace sdr { namespace contact {
class ObjectContact;
} }

namespace sd { namespace slidesorter { namespace view {
class PageObject;
class PageObjectViewObjectContact;
} } }

namespace sd { namespace slidesorter { namespace controller {
class PageObjectFactory;
} } }

namespace sd { namespace slidesorter { namespace model {

class SlideRenderer;

namespace css = ::com::sun::star;

/** Each PageDescriptor object represents the preview of one draw page,
    slide, or master page of a Draw or Impress document as they are displayed
    in the slide sorter.  This class gives access to some associated
    information like prerendered preview or position on the screen.

    <p>Bounding boxes of page objects come in four varieties:
    Model and screen/pixel coordinates and the bounding boxes of the actual
    page objects and the larger bounding boxes that include page names and
    fade symbol.</p>
*/
class PageDescriptor
    : public ::boost::enable_shared_from_this<PageDescriptor>
{
public:
    /** Create a PageDescriptor for the given SdPage object.
        @param rxPage
            The page that is represented by the new PageDescriptor object.
        @param pPage
            The page pointer can in some situations not be detected from
            rxPage, e.g. after undo of page deletion.  Therefore supply it
            seperately.
        @param nIndex
            This index is displayed in the view as page number.  It is not
            necessaryily the page index (not even when you add or subtract 1
            or use (x-1)/2 magic).
    */
    PageDescriptor (
        const css::uno::Reference<css::drawing::XDrawPage>& rxPage,
        SdPage* pPage,
        const sal_Int32 nIndex,
        const controller::PageObjectFactory& rPageObjectFactory);

    ~PageDescriptor (void);

    /** Return the page that is represented by the descriptor as SdPage pointer .
    */
    SdPage* GetPage (void) const;

    /** Return the page that is represented by the descriptor as XDrawPage reference.
    */
    css::uno::Reference<css::drawing::XDrawPage> GetXDrawPage (void) const;

    /** Returns the index of the page as it is displayed in the view as page
        number.  The value may differ from the index returned by the
        XDrawPage when there are hidden slides and the XIndexAccess used to
        access the model filters them out.
    */
    sal_Int32 GetPageIndex (void) const;

    /** Return the page shape that is used for visualizing the page.
    */
    view::PageObject* GetPageObject (void);
    void ReleasePageObject (void);

    /** Return <TRUE/> when the page object is fully or parially visible. */
    bool IsVisible (void) const;

    /** Set the visible state that is returned by the IsVisible() method.
        This method is typically called by the view who renders the object
        onto the screen.
    */
    void SetVisible (bool bVisible);

    /** Make sure that the page is selected and return whether the
        selection state changed.
    */
    bool Select (void);
    /** Make sure that the page is not selected and return whether the
        selection state changed.
    */
    bool Deselect (void);

    /** Return whether the page is selected (and thus bypasses the internal
        mbIsSelected flag.
    */
    bool IsSelected (void) const;

    /** Set the internal mbIsSelected flag to the selection state of the
        page.  Use this method to synchronize a page descriptor with the
        page it describes and determine whether a redraw to update the
        selection indicator is necessary.
        @return
            When the two selection states were different <TRUE/> is
            returned.  When they were the same this method returns
            <FALSE/>.
    */
    bool UpdateSelection (void);

    bool IsFocused (void) const;
    void SetFocus (void);
    void RemoveFocus (void);

    view::PageObjectViewObjectContact* GetViewObjectContact (void) const;

    void SetViewObjectContact (
        view::PageObjectViewObjectContact* pViewObjectContact);

    /** Return the currently used page object factory.
    */
    const controller::PageObjectFactory& GetPageObjectFactory (void) const;

    /** Replace the current page object factory by the given one.
    */
    void SetPageObjectFactory (const controller::PageObjectFactory& rFactory);

    void SetModelBorder (const SvBorder& rBorder);
    SvBorder GetModelBorder (void) const;

    /** The size of the area in which the page number is displayed is
        calculated by the SlideSorterView and then stored in the page
        descriptors so that the contact objects can access them.  The
        contact objects can not calculate them on demand because the total
        number of slides is needed to do that and this number is not known
        to the contact objects.
    */
    void SetPageNumberAreaModelSize (const Size& rSize);
    Size GetPageNumberAreaModelSize (void) const;

    /** Returns <TRUE/> when the slide is the current slide.
    */
    bool IsCurrentPage (void) const;

    /** Set or revoke the state of this slide being the current slide.
    */
    void SetIsCurrentPage (const bool bIsCurrent);

private:
    SdPage* mpPage;
    css::uno::Reference<css::drawing::XDrawPage> mxPage;
    /** This index is displayed as page number in the view.  It may or may
        not be actual page index.
    */
    const sal_Int32 mnIndex;

    /// The factory that is used to create PageObject objects.
    const controller::PageObjectFactory* mpPageObjectFactory;

    /** The page object will be destroyed by the page into which it has
        been inserted.
    */
    view::PageObject* mpPageObject;

    bool mbIsSelected;
    bool mbIsVisible;
    bool mbIsFocused;
    bool mbIsCurrent;

    view::PageObjectViewObjectContact* mpViewObjectContact;

    /// The borders in model coordinates arround the page object.
    SvBorder maModelBorder;

    /// The size of the page number area in model coordinates.
    Size maPageNumberAreaModelSize;

    // Do not use the copy constructor operator.  It is not implemented.
    PageDescriptor (const PageDescriptor& rDescriptor);

    // Do not use the assignment operator.  It is not implemented
    // (mrPage can not be assigned).
    PageDescriptor& operator= (const PageDescriptor& rDescriptor);
};

} } } // end of namespace ::sd::slidesorter::model

#endif
