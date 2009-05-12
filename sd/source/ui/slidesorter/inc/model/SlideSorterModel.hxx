/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterModel.hxx,v $
 * $Revision: 1.7 $
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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_MODEL_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_MODEL_HXX

class SdDrawDocument;

#include "model/SlsPageEnumeration.hxx"
#include "model/SlsSharedPageDescriptor.hxx"

#include "pres.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <osl/mutex.hxx>

#include <memory>
#include <vector>
#include <functional>

namespace css = ::com::sun::star;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {
class PageObjectFactory;
} } }

namespace sd { namespace slidesorter { namespace model {

class DocumentPageContainer;

/** The model of the slide sorter gives access to the slides that are to be
    displayed in the slide sorter view.  Via the SetDocumentSlides() method
    this set of slides can be modified (but do not call it directly, use
    SlideSorterController::SetDocumentSlides() instead.)
*/
class SlideSorterModel
{
public:
    SlideSorterModel (SlideSorter& rSlideSorter);

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

    /** Set the edit mode to that currently used by the controller.
    */
    bool SetEditModeFromController (void);
    EditMode GetEditMode (void) const;
    PageKind GetPageType (void) const;

    /** Return the number of slides in the document regardless of whether
        they are visible or not or whether they are hidden or not.
        The number of slides depends on the set of slides available through
        the XIndexAccess given to SetDocumentSlides().
    */
    sal_Int32 GetPageCount (void) const;

    /** Return a page descriptor for the page with the specified index.
        Page descriptors are created on demand.  The page descriptor is
        found (or not found) in constant time.
        @param nPageIndex
            The index of the requested slide.  The valid values
            are 0 to GetPageCount()-1.
        @param bCreate
            When <TRUE/> and the requested page descriptor is missing then
            it is created.  When <FALSE/> then an empty reference is
            returned for missing descriptors.
        @return
            When the given index is not valid, i.e. lower then zero or
            larger than or equal to the number of pages then an empty
            reference is returned. Note that the page count may change
            between calls to GetPageCount() and GetPageDescriptor().
    */
    SharedPageDescriptor GetPageDescriptor (
        const sal_Int32 nPageIndex,
        const bool bCreate = true) const;

    /** Return a page descriptor for the given XDrawPage.  Page descriptors
        are created on demand.  The page descriptor is found (or not found)
        in (at most) linear time.  Note that all page descriptors in front of
        the one associated with the given XDrawPage are created when not yet
        present. When the XDrawPage is not found then all descriptors are
        created.
        @return
            Returns the index to the requested page descriptor or -1 when
            there is no such page descriptor.
    */
    sal_Int32 GetIndex (
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::XDrawPage>& rxSlide) const;

    /** Call this method after the document has changed its structure.  This
        will get the model in sync with the SdDrawDocument.  This method
        tries not to throw away to much information already gathered.  This
        is especially important for previews of complex pages that take some
        time to create.
    */
    void Resync (void);

    /** Delete all descriptors that currently are in the container.  The size
        of the container, however, is not altered.  Use the AdaptSize
        method for that.
    */
    void ClearDescriptorList (void);

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

    /** Set the XIndexAccess from which the called SlideSorterModel takes
        its pages.
        @param rxSlides
            The set of slides accessible through this XIndexAccess are not
            necessarily the same as the ones of the XModel of the
            XController (although it typically is a subset).
    */
    void SetDocumentSlides (const css::uno::Reference<css::container::XIndexAccess>& rxSlides);

    /** Return the set of pages that is currently displayed by the slide sorter.
    */
    css::uno::Reference<css::container::XIndexAccess> GetDocumentSlides (void) const;

    /** This method is called when the edit mode has changed.  It calls
        SetDocumentSlides() with the set of slides or master pages obtained
        from the model of the XController.
    */
    void UpdatePageList (void);

private:
    mutable ::osl::Mutex maMutex;
    SlideSorter& mrSlideSorter;
    ::com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> mxSlides;
    PageKind mePageKind;
    EditMode meEditMode;
    typedef ::std::vector<SharedPageDescriptor> DescriptorContainer;
    mutable DescriptorContainer maPageDescriptors;
    mutable ::std::auto_ptr<controller::PageObjectFactory> mpPageObjectFactory;

    /** Resize the descriptor container according to current values of
        page kind and edit mode.
    */
    void AdaptSize (void);

};

} } } // end of namespace ::sd::slidesorter::model

#endif
