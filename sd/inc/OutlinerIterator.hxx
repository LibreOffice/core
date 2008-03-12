/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutlinerIterator.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:20:22 $
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

#ifndef SD_OUTLINER_ITERATOR_HXX
#define SD_OUTLINER_ITERATOR_HXX

#include <svx/svdobj.hxx>

#include "pres.hxx"
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#include <vector>

class SdDrawDocument;

namespace sd {

class DrawViewShell;
class Outliner;
class View;

namespace outliner {

class IteratorImplBase;
class IteratorPosition;

/** Use this enum to specify the initial location of the object pointed to by
    a newly created iterator.  The values are
    <ul><li><const>BEGIN</const> for the first object with reference to
    iteration direction.</li>
    <li>END for one past the last valid object or, if the iterator is a
    backward iterator, the object in front of the first valid one.</li>
    <li>CURRENT for the current object.  Because there is only a current
    page this usually is taken to be the first/last object on the current
    page.</li></ul>
*/
enum IteratorLocation {BEGIN,END,CURRENT};

/** Use this enum to specify the type of iterator when creating a new
    iterator:
    <ul><li>SELECTION for iteration over all objects that belong to the
    current mark list.</li>
    <li>SINGLE_VIEW for iteration over all objects in the current view.</li>
    <li>DOCUMENT for iteratioin over all object in all relevant
    views.</li></ul>
*/
enum IteratorType {SELECTION,SINGLE_VIEW,DOCUMENT};


/** This iterator can be used to iterate over all <type>SdrObject</type>
    objects of one of three set denoted by the <type>IteratorType</type>:
    <ul><li>All objects of the current mark list (selection)
    (type==SELECTION).</li>
    <li>All objects in the current view (type==SINGLE_VIEW).</li>
    <li>All objects in all views (type=DOCUMENT).</li></ul>

    <p>Note that the iterator does not change pages or views.  It is the
    task of the user of the iterator to take the information provided by the
    <type>IteratorPosition</type> as returned by the
    <member>operator*()</member> method and set view, visible page, and
    selection/edit mode markers to reflect this position.</p>

    <p>A simple forward iteration from the first to the last object would
    instantiate the iterator with
    <code>Iterator(pDocument,pViewShell,true,BEGIN)</code> for some document
    and view shell.  This iterator can then be compared against
    <code>Iterator(pDocument,pViewShell,true,END)</code>.  On equality the
    iteration should be stoped without evaluating the iterator: The position
    of an end iterator is not valid.</p>
*/
class Iterator
{
public:
    Iterator (void);

    /** The copy constructor creates a new iterator by copying the
        implementation object.
    */
    Iterator (const Iterator& rIterator);

    /** Create a new iterator with the implementation object being the
        provided one.
        @param pObject
            A copy of this object will become the implementation object.
    */
    explicit Iterator (IteratorImplBase* pObject);

    /** Create a new iterator with the implementation object being the copy
        of the provided one.
        @param rObject
            A copy of this object will become the implementation object.
    */
    explicit Iterator (const IteratorImplBase& rObject);

    ~Iterator (void);

    /** Assign the iterator from the given one.  The implementation object
        of this iterator will be a copy of the given iterator.
        @param rIterator
            The iterator which to assign from.
    */
    Iterator& operator= (const Iterator& rIterator);
    /** Return the current position of the iterator.
        @return
            Returns a reference to the current position.  Therefore this
            method is not thread safe.  The reason for this behaviour is, of
            course, to ommit the copying of the returned position.
    */
    const IteratorPosition& operator* () const;
    /** The prefix increment operator returns the iterator pointing to the
        next object.  When in doubt prefer this operator over the postfix
        increment operator.
        @return
            Returns a reference to this iterator pointing to the next object.
    */
    Iterator& operator++ ();
    /** The postfix increment operator returns the iterator still pointing
        to the current object.  Only the next call to
        <member>operator*()</member> will return the next object.  When in
        doubt rather use the prefix increment operator.
        @param dummy
            A dummy operator used by the compiler.
        @return
            Returns a copy of the iterator as it where before the operator
            was called.
    */
    Iterator operator++ (int);
    /** Test equality of two iterators.  Two iterators are taken to be equal
        when they point are of the same type (their implementation objects
        are instances of the same class) and point to the same object.
        @param rIterator
            The iterator to test equality with.
        @return
            Returns <TRUE/> when both iterators point to the same object.
    */
    bool operator== (const Iterator& rIterator);
    /** Test whether two iterators point to different objects.  This is just
        the negation of the result of the equality operator.
        @param rIterator
            The iterator to test inequality with.
        @return
            Returns <TRUE/> when both iterators point to the different objects.
    */
    bool operator!= (const Iterator& rIterator);
    /** Reverse the direction of iteration.  The position of the iterator is
        not changed.  Thus caling this method twice returns to the old state.
    */
    void Reverse (void);

private:
    /// The implementation object to which most of the methods are forwarded.
    IteratorImplBase* mpIterator;
};




/** This class wraps the <type>Outliner</type> class and represents it as
    a container of <type>SdrObject</type> objects.  Its main purpose is to
    provide iterators for certain sub-sets of those objects.  These sub-sets
    are a) the set of the currently selected objects, b) all objects in the
    current view, and c) all objects in all views.

    <p>The direction of the returned iterators depends on the underlying
    <type>Outliner</type> object and is usually set in the search
    dialog.</p>
*/
class OutlinerContainer
{
public:
    /** Create a new wraper object for the given outliner.
        @param pOutliner
            The outliner that is represented by the new object as
            <type>SdrObject</type> container.
    */
    OutlinerContainer (::sd::Outliner* pOutliner);

    /** Return an iterator that points to the first object of one of the
        sets described above.  This takes also into account the direction of
        iteration.
        @return
            The returned iterator points either to the first (forward
            search) or to the last object (backward search) of the set.
    */
    Iterator begin (void);

    /** Return an iterator that marks the end of the iteration.  This takes
        also into account the direction of iteration. The object pointed to
        is not valid.
        @return
            The returned iterator points either to that object past the last
            one (forward search) or to the one in front of the first
            (backward search).
    */
    Iterator end (void);

    /** Return an iterator that points to the current object of one of the
        sets described above.  This takes also into account the direction of
        iteration.
        @return
            The returned iterator points either to the first (forward
            search) or to the last object (backward search) of the set of
            selected objects or of the current page if the search set spans
            more than one page.
    */
    Iterator current (void);

private:
    /// The wrapped outliner that is represented as object container.
    ::sd::Outliner* mpOutliner;

    /** Create an iterator.  The object pointed to depends on the search
        direction retrieved from the outliner object
        <member>mpOutliner</member> and the given location.
        @param aLocation
            This specifies whether the returned iterator points to the
            first, (one past the) last, or current object.
        @return
            Returns an iterator as constructed by one of the
            <member>CreateSelectionIterator()</member>,
            <member>CreateViewIterator()</member>, or <member>CreateDocumentIterator()</member>.
     */
    Iterator CreateIterator (IteratorLocation aLocation);

    /** Create an iterator that iterates over all currently selected
        <type>SdrObjects</type> objects of the <member>mpOutliner</member>
        outliner.
        @param rObjectList
            List of currently selected objects.  This list is necessary
            so that the selection can be changed without affecting the
            iterator.
        @param pDocument
            The document to which the objects belong.
        @param pViewShell
            The view shell which displays the objects.
        @param bDirectionIsForward
            The direction of iteration.  It defaults to forward.
        @param aLocation
            This specifies at which object the iterator points initially.
    */
    Iterator CreateSelectionIterator (
        const ::std::vector<SdrObjectWeakRef>& rObjectList,
        SdDrawDocument* pDocument,
        DrawViewShell* pViewShell,
        bool bDirectionIsForward=true,
        IteratorLocation aLocation=BEGIN);

    /** Create an iterator that iterates over all <type>SdrObjects</type>
        objects of the <member>mpOutliner</member> outliner that belong to
        the current view.
        @param pDocument
            The document to which the objects belong.
        @param pViewShell
            The view shell which displays the objects.
        @param bDirectionIsForward
            The direction of iteration.  It defaults to forward.
        @param aLocation
            This specifies at which object the iterator points initially.
    */
    Iterator CreateViewIterator (
        SdDrawDocument* pDocument,
        DrawViewShell* pViewShell,
        bool bDirectionIsForward=true,
        IteratorLocation aLocation=BEGIN);

    /** Create an iterator that iterates over all <type>SdrObjects</type>
        objects of the <member>mpOutliner</member> outliner.
        @param pDocument
            The document to which the objects belong.
        @param pViewShell
            The view shell which displays the objects.
        @param bDirectionIsForward
            The direction of iteration.  It defaults to forward.
        @param aLocation
            This specifies at which object the iterator points initially.
    */
    Iterator CreateDocumentIterator (
        SdDrawDocument* pDocument,
        DrawViewShell* pViewShell,
        bool bDirectionIsForward=true,
        IteratorLocation aLocation=BEGIN);

    /** Return the index of a page that contains an object that a new
        iterator shall point to.  This page index depends primarily on the
        location, iteration direction, as well as on edit mode and page
        kind.
        @param pDocument
            The document to which the page belongs.
        @param pViewShell
            The view shell which displays the page.
        @param ePageKind
            Specifies the view the page belongs to.
        @param eEditMode
            Specifies whether the page is a master page.
        @param bDirectionIsForward
            The direction of iteration.
        @param aLocation
            This specifies at which object the iterator points initially.
    */
    sal_Int32 GetPageIndex (SdDrawDocument* pDocument,
        DrawViewShell* pViewShell,
        PageKind ePageKind,
        EditMode eEditMode,
        bool bDirectionIsForward,
        IteratorLocation aLocation);

    // Do not allow default constructor and copying of outliner containers.
    OutlinerContainer (const OutlinerContainer&) {};
    OutlinerContainer (void) {};
    OutlinerContainer& operator= (const OutlinerContainer&) {return *this;};
};




/** Data collection specifying a <type>SdrObject</type> and its position in
    a document and view.
*/
class IteratorPosition
{
public:
    /** Create a new object with all data members set to default values.
        These values should not be accessed.  The only use of the object as
        it is is as a marker in comparisons.
    */
    IteratorPosition (void);
    /** Create a new object with all data members set from the given
        position.
        @param aPosition
            The position object from which to take the values that are
            assigned to the data members of this object.
    */
    IteratorPosition (const IteratorPosition& aPosition);
    /** Create a new object and set its data members to the given values.
    */
    IteratorPosition (SdrObject* pObject, sal_Int32 nText, sal_Int32 nPageIndex, PageKind ePageKind, EditMode eEditMode);

    /// The destructor is a no-op at the moment.
    ~IteratorPosition (void);
    /** Assign the content of the given position to this one.
        @param aPosition
            This is the position object from which to take the values of all
            data members.
        @return
            Returns a reference to this object.
    */
    IteratorPosition& operator= (const IteratorPosition& aPosition);
    /** Compare two positions for equality.
        @return
            <TRUE/> is returned only when all data members have the same
            values in both position objects.
    */
    bool operator== (const IteratorPosition& aPosition) const;

    /// Pointer to the actual <type>SdrObject</type> object.
    SdrObjectWeakRef mxObject;

    /// Number of the actual SdrText from the current <type>SdrObject</type>
    sal_Int32 mnText;

    /// The index of a page where the object is located on.
    sal_Int32 mnPageIndex;
    /// Page kind of the view.
    PageKind mePageKind;
    /// Edit mode of the view.
    EditMode meEditMode;
};


} } // end of namespace ::sd::outliner


#endif // _SD_OUTLINER_ITERATOR_HXX

