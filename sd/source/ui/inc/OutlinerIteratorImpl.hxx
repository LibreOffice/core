/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SD_OUTLINER_ITERATOR_IMPL_HXX
#define SD_OUTLINER_ITERATOR_IMPL_HXX

#include <svx/svdobj.hxx>
#include "OutlinerIterator.hxx"
#include <boost/weak_ptr.hpp>

class SdDrawDocument;
class SdPage;
class SdrObjListIter;

namespace sd {

class ViewShell;

namespace outliner {

class IteratorImplBase;

/** Base class for the polymorphic implementation class of the
    <type>Iterator</type> class.  The iterators based on this class are
    basically uni directional iterators.  Their direction can, however, be
    reversed at any point of their life time.
*/
class IteratorImplBase
{
public:
    /** The constructor stores the given arguments to be used by the derived
        classes.
        @param pDocument
            The document provides the information to be iterated on.
        @param pViewShellWeak
            Some information has to be taken from the view shell.
        @param bDirectionIsForward
            This flag defines the iteration direction.  When <TRUE/> then
            the direction is forwards otherwise it is backwards.
    */
    IteratorImplBase (SdDrawDocument* pDocument,
        const ::boost::weak_ptr<ViewShell>& rpViewShellWeak,
        bool bDirectionIsForward);
    IteratorImplBase (SdDrawDocument* pDocument,
        const ::boost::weak_ptr<ViewShell>& rpViewShellWeak,
        bool bDirectionIsForward, PageKind ePageKind, EditMode eEditMode);
    virtual ~IteratorImplBase (void);

    /** Advance to the next text of the current object or to the next object.
        This takes the iteration direction into
        account.  The new object pointed to can be retrieved (among other
        information) by calling the <member>GetPosition</member> method.
    */
    virtual void GotoNextText (void) = 0;
    /** Return an object that describes the current object.
        @return
            The returned object describes the current object pointed to by
            the iterator.  See the description of
            <type>IteratorPosition</type> for details on the available
            information.
    */
    virtual const IteratorPosition& GetPosition (void);
    /** Create an exact copy of this object.  No argument should be
        specified when called from the outside.  It then creates an object
        first and passes that to the inherited <member>Clone()</member>
        methods to fill in class specific information.
        @return
            Returns a copy of this object.  When this method is called with
            an argument then this value will be returned.
    */
    virtual IteratorImplBase* Clone (IteratorImplBase* pObject=NULL) const;
    /** Test the equality of the this object and the given iterator.  Two
        iterators are taken to be equal when they point to the same object.
        Iteration direction is not taken into account.
        @param rIterator
            The iterator to compare to.
        @return
            When both iterators ar equal <TRUE/> is returned, <FALSE/> otherwise.
    */
    virtual bool operator== (const IteratorImplBase& rIterator) const;
    /** This method is used by the equality operator.  Additionaly to the
        iterator it takes a type information which is taken into account on
        comparison.  It is part of a "multimethod" pattern.
        @param rIterator
            The iterator to compare to.
        @param aType
            The type of the iterator.
        @return
            Returns <TRUE/> when both iterators point to the same object.
    */
    virtual bool IsEqual (const IteratorImplBase& rIterator, IteratorType aType) const;
    /** Reverse the direction of iteration.  The current object stays the same.
    */
    virtual void Reverse (void);

protected:
    /// The current position as returned by <member>GetPosition()</member>.
    IteratorPosition maPosition;
    /// The document on whose data the iterator operates.
    SdDrawDocument* mpDocument;
    /// Necessary secondary source of information.
    ::boost::weak_ptr<ViewShell> mpViewShellWeak;
    /// Specifies the search direction.
    bool mbDirectionIsForward;
};




/** Iterator all objects that belong to the current mark list
    a.k.a. selection.  It is assumed that all marked objects belong to the
    same page.  It is further assumed that the mark list does not change
    while an iterator is alive.  It is therefore the responsibility of an
    iterator's owner to handle the case of a changed mark list.

    <p>For documentation of the methods please refere to the base class
    <type>IteratorImplBase</type>.</p>
*/
class SelectionIteratorImpl
    : public IteratorImplBase
{
public:
    SelectionIteratorImpl (
        const ::std::vector< SdrObjectWeakRef >& rObjectList,
        sal_Int32 nObjectIndex,
        SdDrawDocument* pDocument,
        const ::boost::weak_ptr<ViewShell>& rpViewShellWeak,
        bool bDirectionIsForward);
    SelectionIteratorImpl (const SelectionIteratorImpl& rObject);
    virtual ~SelectionIteratorImpl (void);

    virtual void GotoNextText (void);
    virtual const IteratorPosition& GetPosition (void);
    virtual IteratorImplBase* Clone (IteratorImplBase* pObject) const;
    virtual bool operator== (const IteratorImplBase& rIterator) const;

private:
    const ::std::vector<SdrObjectWeakRef>& mrObjectList;
    sal_Int32 mnObjectIndex;

    /** Compare the given iterator with this object.  This method handles
        only the case that the given iterator is an instance of this class.
        @param rIterator
            The iterator to compare to.
        @param aType
            The type of the iterator.
        @return
            Returns <TRUE/> when both iterators point to the same object.
    */
    virtual bool IsEqual (const IteratorImplBase& rIterator, IteratorType aType) const;

    IteratorImplBase& operator= (const IteratorImplBase& rIterator);
};


/** Iterator for iteration over all objects in a single view.  On reaching
    the last object on the last page (or the first object on the first page)
    the view is *not* switched.  Further calls to the
    <member>GotoNextObject()</member> method will be ignored.

    <p>For documentation of the methods please refere to the base class
    <type>IteratorImplBase</type>.</p>
*/
class ViewIteratorImpl : public IteratorImplBase
{
public:
    ViewIteratorImpl (
        sal_Int32 nPageIndex,
        SdDrawDocument* pDocument,
        const ::boost::weak_ptr<ViewShell>& rpViewShellWeak,
        bool bDirectionIsForward);
    ViewIteratorImpl (
        sal_Int32 nPageIndex,
        SdDrawDocument* pDocument,
        const ::boost::weak_ptr<ViewShell>& rpViewShellWeak,
        bool bDirectionIsForward,
        PageKind ePageKind,
        EditMode eEditMode);
    virtual ~ViewIteratorImpl (void);

    virtual void GotoNextText (void);
    virtual IteratorImplBase* Clone (IteratorImplBase* pObject) const;
    virtual void Reverse (void);

protected:
    /// Number of pages in the view that is specified by <member>maPosition</member>.
    sal_Int32 mnPageCount;

    /** Initialize this iterator with respect to the given location.  After
        this call the object looks like newly constructed.
    */
    void Init (IteratorLocation aLocation);

    /** Set up page pointer and object list iterator for the specified
        page.
        @param nPageIndex
            Index of the new page.  It may lie outside the valid range for
            page indices.
    */
    void SetPage (sal_Int32 nPageIndex);

private:
    /// Indicates whether a page changed occured on switching to current page.
    bool mbPageChangeOccured;
    /// Pointer to the page associated with the current page index. May be NULL.
    SdPage* mpPage;
    /// Iterator of all objects on the current page.
    SdrObjListIter* mpObjectIterator;

    // Don't use this operator.
    ViewIteratorImpl& operator= (const ViewIteratorImpl&){return *this;};
};




/** Iterator for iteration over all objects in all views.  It automatically
    switches views when reaching the end/beginning of a view.

    <p>For documentation of the methods please refere to the base class
    <type>IteratorImplBase</type>.</p>
*/
class DocumentIteratorImpl : public ViewIteratorImpl
{
public:
    DocumentIteratorImpl (
        sal_Int32 nPageIndex,
        PageKind ePageKind,
        EditMode eEditMode,
        SdDrawDocument* pDocument,
        const ::boost::weak_ptr<ViewShell>& rpViewShellWeak,
        bool bDirectionIsForward);
    virtual ~DocumentIteratorImpl (void);

    virtual void GotoNextText (void);
    virtual IteratorImplBase* Clone (IteratorImplBase* pObject) const;

private:
    sal_Int32 mnPageCount;

    // Don't use this operator.
    DocumentIteratorImpl& operator= (const DocumentIteratorImpl& ){return *this;};
};


} } // end of namespace ::sd::outliner

#endif
