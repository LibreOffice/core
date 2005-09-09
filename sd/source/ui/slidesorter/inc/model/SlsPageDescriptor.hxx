/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageDescriptor.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:20:49 $
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

#ifndef SD_SLIDESORTER_PAGE_DESCRIPTOR_HXX
#define SD_SLIDESORTER_PAGE_DESCRIPTOR_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <tools/link.hxx>
#include <vcl/bitmap.hxx>
#include <sfx2/viewfrm.hxx>

class SdPage;

#include <memory>

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

class SlideSorterView;
class SlideRenderer;

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
{
public:
    PageDescriptor (
        SdPage& rPage,
        const controller::PageObjectFactory& rPageObjectFactory);
    ~PageDescriptor (void);

    /** Return the page that is represented by the descriptor.
    */
    SdPage* GetPage (void) const;

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
        page.  Use this method to synchornize a page descriptor with the
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

private:
    SdPage& mrPage;

    /// The factory that is used to create PageObject objects.
    const controller::PageObjectFactory* mpPageObjectFactory;

    /** The page object will be destroyed by the page into which it has
        been inserted.
    */
    view::PageObject* mpPageObject;

    bool mbIsSelected;
    bool mbVisible;
    bool mbFocused;

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
