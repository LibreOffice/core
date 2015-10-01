/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_MODEL_SLIDESORTERMODEL_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_MODEL_SLIDESORTERMODEL_HXX

class SdDrawDocument;

#include "model/SlsPageEnumeration.hxx"
#include "model/SlsSharedPageDescriptor.hxx"

#include "pres.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <osl/mutex.hxx>
#include <vcl/region.hxx>

#include <vector>
#include <functional>

class SdrPage;
class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace model {

inline sal_Int32 FromCoreIndex (const sal_uInt16 nCoreIndex) { return (nCoreIndex-1)/2; }

/** The model of the slide sorter gives access to the slides that are to be
    displayed in the slide sorter view.  Via the SetDocumentSlides() method
    this set of slides can be modified (but do not call it directly, use
    SlideSorterController::SetDocumentSlides() instead.)
*/
class SlideSorterModel
{
public:
    SlideSorterModel (SlideSorter& rSlideSorter);

    virtual ~SlideSorterModel();
    void Dispose();

    /** This method is present to let the view create a ShowView for
        displaying slides.
    */
    SdDrawDocument* GetDocument();

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

    EditMode GetEditMode() const { return meEditMode;}
    PageKind GetPageType() const { return mePageKind;}

    /** Return the number of slides in the document regardless of whether
        they are visible or not or whether they are hidden or not.
        The number of slides depends on the set of slides available through
        the XIndexAccess given to SetDocumentSlides().
    */
    sal_Int32 GetPageCount() const;

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

    /** Return a page descriptor for the given SdrPage.  Page descriptors
        are created on demand.  The page descriptor is found (or not found)
        in (at most) linear time.  Note that all page descriptors in front of
        the one associated with the given XDrawPage are created when not yet
        present. When the SdrPage is not found then all descriptors are
        created.
        @return
            Returns the index to the requested page descriptor or -1 when
            there is no such page descriptor.
    */
    sal_Int32 GetIndex (const SdrPage* pPage) const;

    /** Return an index for accessing an SdrModel that corresponds to the
        given SlideSorterModel index.  In many cases we just have to apply
        the n*2+1 magic.  Only when a special model is set, like a custom
        slide show, then the returned value is different.
    */
    sal_uInt16 GetCoreIndex (const sal_Int32 nIndex) const;

    /** Call this method after the document has changed its structure.  This
        will get the model in sync with the SdDrawDocument.  This method
        tries not to throw away to much information already gathered.  This
        is especially important for previews of complex pages that take some
        time to create.
    */
    void Resync();

    /** Delete all descriptors that currently are in the container.  The size
        of the container, however, is not altered.  Use the AdaptSize
        method for that.
    */
    void ClearDescriptorList();

    /** Set the selection of the document to exactly that of the called model.
    */
    void SynchronizeDocumentSelection();

    /** Set the selection of the called model to exactly that of the document.
    */
    void SynchronizeModelSelection();

    /** Return the mutex so that the caller can lock it and then safely
        access the model.
    */
    ::osl::Mutex& GetMutex() { return maMutex;}

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
    css::uno::Reference<css::container::XIndexAccess> GetDocumentSlides() const;

    /** This method is called when the edit mode has changed.  It calls
        SetDocumentSlides() with the set of slides or master pages obtained
        from the model of the XController.
    */
    void UpdatePageList();

    bool IsReadOnly() const;

    /** The current selection is saved by copying the ST_Selected state into
        ST_WasSelected for slides.
    */
    void SaveCurrentSelection();

    /** The current selection is restored from the ST_WasSelected state from
        the slides.
        @returns
            The returned region has to be repainted to reflect the updated
            selection states.
    */
    vcl::Region RestoreSelection();

    /** Typically called from controller::Listener this method handles the
        insertion and deletion of single pages.
        @return
            Returns <TRUE/> when the given page is relevant for the current
            page kind and edit mode.
    */
    bool NotifyPageEvent (const SdrPage* pPage);

private:
    mutable ::osl::Mutex maMutex;
    SlideSorter& mrSlideSorter;
    ::com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> mxSlides;
    PageKind mePageKind;
    EditMode meEditMode;
    typedef ::std::vector<SharedPageDescriptor> DescriptorContainer;
    mutable DescriptorContainer maPageDescriptors;

    /** Resize the descriptor container according to current values of
        page kind and edit mode.
    */
    void AdaptSize();

    SdPage* GetPage (const sal_Int32 nCoreIndex) const;
    void InsertSlide (SdPage* pPage);
    void DeleteSlide (const SdPage* pPage);
    void UpdateIndices (const sal_Int32 nFirstIndex);
};

} } } // end of namespace ::sd::slidesorter::model

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
