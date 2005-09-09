/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterModel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:20:19 $
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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_MODEL_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_MODEL_HXX

class SdDrawDocument;

#include "SlsPageEnumeration.hxx"

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#include <osl/mutex.hxx>

#include <memory>
#include <vector>
#include <functional>

namespace sd { namespace slidesorter { namespace controller {
class PageObjectFactory;
} } }

namespace sd { namespace slidesorter { namespace model {

class DocumentPageContainer;
class PageDescriptor;
class PageEnumeration;

class SlideSorterModel
{
public:
    typedef PageEnumeration Enumeration;

    SlideSorterModel (SdDrawDocument& rDocument,
        PageKind ePageKind = PK_STANDARD,
        EditMode eEditMode= EM_PAGE);

    virtual ~SlideSorterModel (void);

    /** This method is present to let the view create a ShowView for
        displaying slides.
    */
    SdDrawDocument* GetDocument (void);

    /** Set a new edit mode and return whether the edit mode really
        has been changed.  When the edit mode is changed then the
        previous page descriptor list is replaced by a new one which
        has to be repainted.
        @return
            A return value of <TRUE/> indicates that the edit mode has
            changed and thus the page descriptor list has been set up
            to reflect that change.  A repaint is necessary.
    */
    bool SetEditMode (EditMode eEditMode);
    EditMode GetEditMode (void) const;
    PageKind GetPageType (void) const;

    int GetPageCount (void) const;

    /** Return a page descriptor for the page with the specified index.
        Page descriptors are created on demand.  The page descriptor is
        found (or not found) in constant time.
        @return
            When the given index is not valid, i.e. lower then zero or
            larger than or equal to the number of pages as returned by
            GetPageCount() then NULL is returned. Note that the page count
            may change between subsequent calls to GetPageCount() and
            GetPageDescriptor().
    */
    PageDescriptor* GetPageDescriptor (int nPageIndex) const;

    /** Return the page descriptor for the page with the specified index.
        In contrast to GetPageDescriptor() this method does not create a
        page descriptor when not already present.  This method exists mainly
        for debugging so use it only when you know what you are doing.
    */
    PageDescriptor* GetRawPageDescriptor (int nPageIndex) const;

    /** Return the page descriptor whose page has the given object as UNO
        wrapper.  This method takes linear time (in the number of pages).
        @return
            Returns NULL when no page descriptor is found with the given UNO
            wrapper.
    */
    PageDescriptor* FindPageDescriptor (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& rxPage) const;

    Enumeration GetAllPagesEnumeration (void) const;
    Enumeration GetSelectedPagesEnumeration (void) const;
    Enumeration GetVisiblePagesEnumeration (void) const;

    /** Call this method after the document has changed its structure.  This
        will get the model in sync with the SdDrawDocument.  This method
        tries not to throw away to much information already gathered.  This
        is especially important for previews of complex pages that take some
        time to create.
    */
    void Resync (void);

    /** Set the selection of the document to exactly that of the called model.
    */
    void SynchronizeDocumentSelection (void);

    /** Set the selection of the called model to exactly that of the document.
    */
    void SynchronizeModelSelection (void);

    /** Replace the factory for the creation of the page objects and
        contacts with the given object.  The old factory is destroyed.
    */
    void SetPageObjectFactory(
        ::std::auto_ptr<controller::PageObjectFactory> pPageObjectFactory);

    /** Return the page object factory.  It none has been set so far or it
        has been reset, then a new one is created.
    */
    const controller::PageObjectFactory& GetPageObjectFactory (void) const;

    /** Return the mutex so that the caller can lock it and then safely
        access the model.
    */
    ::osl::Mutex& GetMutex (void);

private:
    mutable ::osl::Mutex maMutex;
    typedef ::std::vector<PageDescriptor*> DescriptorContainer;
    SdDrawDocument& mrDocument;
    PageKind mePageKind;
    EditMode meEditMode;
    mutable DescriptorContainer maPageDescriptors;
    mutable ::std::auto_ptr<controller::PageObjectFactory> mpPageObjectFactory;

    /** Resize the descriptor container according to current values of
        page kind and edit mode.
    */
    void AdaptSize (void);

    /** Delete all descriptors that currently are in the container.  The size
        of the container, however, is not altered.  Use the AdaptSize
        method for that.
    */
    void ClearDescriptorList (void);
};

} } } // end of namespace ::sd::slidesorter::model

#endif
