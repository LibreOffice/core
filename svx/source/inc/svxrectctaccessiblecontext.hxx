/*************************************************************************
 *
 *  $RCSfile: svxrectctaccessiblecontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2002-03-12 09:44:30 $
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


#ifndef _SVXRECTACCESSIBLECONTEXT_HXX
#define _SVXRECTACCESSIBLECONTEXT_HXX

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_IllegalAccessibleComponentStateException_HPP_
#include <drafts/com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/compbase6.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif


#include <cppuhelper/implbase6.hxx>
#include <unotools/servicehelper.hxx>

#ifndef _SVX_RECTENUM_HXX //autogen
#include <rectenum.hxx>
#endif

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } };

class Rectangle;
class SvxRectCtl;
class SvxRectCtlChildAccessibleContext;
class Window;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef ::cppu::WeakAggComponentImplHelper7<
                ::drafts::com::sun::star::accessibility::XAccessible,
                ::drafts::com::sun::star::accessibility::XAccessibleComponent,
                ::drafts::com::sun::star::accessibility::XAccessibleContext,
                ::drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                ::drafts::com::sun::star::accessibility::XAccessibleSelection,
                ::com::sun::star::lang::XServiceInfo,
                ::com::sun::star::lang::XServiceName >
                SvxRectCtlAccessibleContext_Base;

class SvxRectCtlAccessibleContext : public SvxRectCtlAccessibleContext_Base, ::comphelper::OBaseMutex
{
public:
    //=====  internal  ========================================================
    SvxRectCtlAccessibleContext(
        const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        SvxRectCtl&             rRepresentation,
        const ::rtl::OUString*  pName = NULL,
        const ::rtl::OUString*  pDescription = NULL );
protected:
    virtual ~SvxRectCtlAccessibleContext();
public:
    //=====  XAccessible  =====================================================

    /// Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext (void) throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

    /** Tests whether the specified point lies within this object's bounds.

        <p>The test point's coordinates are defined relative to the
        coordinate system of the object.</p>

        @param point
            Coordinates of the point to test.

        @return
            Returns <TRUE/> if the point lies within or on the object's bounding
            box and <FALSE/> otherwise.
    */
    virtual sal_Bool SAL_CALL contains(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the Accessible child that is rendered under the given point.

        @param aPoint
            Coordinates of the test point for which to find the Accessible
            child.

        @return
            If there is one child which is rendered so that its bounding box
            contains the test point then a reference to that object is
            returned.  If there is more than one child which satisfies that
            condition then a reference to that one is returned that is
            painted on top of the others.  If no there is no child which is
            rendered at the test point an empty reference is returned.
    */
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAt(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the bounding box of this object.

        <p>The returned bounding box has the form of a rectangle.  Its
        coordinates are relative to the object's parent coordinate system.
        Note that the two methods <member>getLocation</member> and
        <member>getSize</member> return the same information.  With method
        <member>getLocationOnScreen</member> you can get the bound box
        position in screen coordinates.</p>

        @return
            The coordinates of the returned rectangle are relative to this
            object's parent or relative to the screen on which this object
            is rendered if it has no parent.  If the object is not on any
            screen the returnred rectangle is empty and located at position
            (0,0).
    */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  )
        throw (::com::sun::star::uno::RuntimeException);


    /** Returns the location of the upper left corner of the object's
        bounding box relative to the parent.</p>.

        <p>The coordinates of the bounding box are given relative to the
        parent's coordinate system.</p>

        @return
            The coordinates of the returned position are relative to this
            object's parent or relative to the screen on which this object
            is rendered if it has no parent.  If the object is not on any
            screen the returnred position is (0,0).
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the location of the upper left corner of the object's
        bounding box in screen coordinates.

        <p>This method returns the same point as does the method
        <member>getLocation</member>.  The difference is that the
        coordinates are absolute screen coordinates of the screen to which
        the object is rendered instead of being relative to the object's
        parent.</p>

        @return
            The coordinates of the returned position are relative to the
            screen on which this object is rendered.  If the object is not
            on any screen the returnred position is (0,0).
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the size of this object's bounding box.

        @return
            The returned size is the size of this object or empty if it is
            not rendered on any screen.
    */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Determines if the object is showing.

        <p>An object is showing if it and all its parents are visible.  This
        does not neccessarily mean that the object is visible on the screen.
        There may be other objects obscuring it.  The showing property can
        be modified by calling <member>setVisible</member> on the objects
        and on its parents.</p>

        @return
            Returns <TRUE/> if the object is showing and <FALSE/> otherwise.

        @see isVisible, setVisible
    */
    virtual sal_Bool SAL_CALL isShowing(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Determines if the object is visible.

        <p>If an object and all of its parents are visible then the object
        is also called showing.  If an object is showing then it has also
        set the <const>AccessibleStateType::VISIBLE</const> state set in its
        <type>AccessibleStateSet</type>.</p>

        @return
            Returns <TRUE/> if the object is visible and <FALSE/> otherwise.

        @see isShowing
    */
    virtual sal_Bool SAL_CALL isVisible(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns whether the object can accept the focus or not.

        <p>If it can it has also set the
        <const>AccessibleStateType::FOCUSABLE</const> state set in its
        <type>AccessibleStateSet</type>.

        @return
            An object returns <TRUE/> if it can accept the focus and returns
            <FALSE/> otherwise.
    */
    virtual sal_Bool SAL_CALL isFocusTraversable(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Adds a new focus listener.

        <p>The focus listener will receive future focus events from this
        component.  If the specified listener is already a member of the set
        of registered focus listeners this method call is ignored.</p>

        @param xListener
            The listener object that is called for future focus events.
    */
    virtual void SAL_CALL addFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    /** Removes a focus listener.

        <p>The specified focus listener will no longer receive focus events
        from this component.  If this listener has not been added by a prior
        call to <member>addFocusListener</member> then this method call is
        ignored.</p>

        @param xListener
            Listener to remove from the list of focus listener.
    */
    virtual void SAL_CALL removeFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    /** Grabs the focus to this object.

        <p>If this object can not accept the focus,
        i.e. <member>isFocusTraversable</member> returns <FALSE/> for this
        object then nothing happens.  Otherwise the object will attempt to
        take the focus.  Nothing happens if that fails, otherwise the object
        has the focus.  This method is called <code>requestFocus</code> in
        the Java Accessibility API 1.4.</p>
    */
    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the key bindings associated with this object.

        <p>This method stems from the Java interface
        <code>AccessibleExtendedComponent</code>.</p>

        @return
            The returned value represents the current key bindings
            associated with this object.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  )
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual long SAL_CALL
        getAccessibleChildCount (void) throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::com::sun::star::uno::RuntimeException,
            ::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException);

    /// only needed to be not incompatible (remove as soon as possible)
    virtual void SAL_CALL
        addPropertyChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException) {}

    /// only needed to be not incompatible (remove as soon as possible)
    virtual void SAL_CALL removePropertyChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException) {}

    //=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addEventListener (
            const ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //  Remove an existing event listener.
    virtual void SAL_CALL
        removeEventListener (
            const ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XServiceName  ====================================================

    /** Returns the name of the service that can be used to create another
        object like this one.
    */
    virtual ::rtl::OUString SAL_CALL
        getServiceName (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XAccessibleSelection =============================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        clearAccessibleSelection()
        throw (::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        selectAllAccessible()
        throw (::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        deselectSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );


protected:
    //=====  internals ========================================================

    void checkChildIndex( long nIndexOfChild ) throw (::com::sun::star::lang::IndexOutOfBoundsException );

    void checkChildIndexOnSelection( long nIndexOfChild ) throw (::com::sun::star::lang::IndexOutOfBoundsException );

    /** Selects a new child by index.

        <p>If the child was not selected before, the state of the child will
        be updated. If the index is invalid, the index will internaly set to NOCHILDSELECTED</p>

        @param nIndexOfChild
            Index of the new child which should be selected.
    */
    void selectChild( long nIndexOfChild );

public:
    /** Selects a new child by point.

        <p>If the child was not selected before, the state of the child will
        be updated. If the point is not invalid, the index will internaly set to NOCHILDSELECTED</p>

        @param eButton
            Button which belongs to the child which should be selected.
    */
    void selectChild( RECT_POINT ePoint );

    /// Sets the name
    void setName( const ::rtl::OUString& rName );

    /// Sets the description
    void setDescription( const ::rtl::OUString& rDescr );
private:
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUniqueId( void );
protected:

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Calls all Listener to tell they the change.
    void CommitChange(const drafts::com::sun::star::accessibility::AccessibleEventObject& rEvent);

    virtual void SAL_CALL disposing();

private:
    /// Reference to the parent object.
    ::com::sun::star::uno::Reference<
         ::drafts::com::sun::star::accessibility::XAccessible> mxParent;

    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    ::rtl::OUString msDescription;

    /** Name of this object.
    */
    ::rtl::OUString msName;

    /// pointer to internal representation

    SvxRectCtl* mpRepr;

    /// array with fix length for all possible childs

    SvxRectCtlChildAccessibleContext* mpChilds[ 9 ];

    /// List of property change listeners.
    cppu::OInterfaceContainerHelper* mpEventListeners;

    /// actual selected child
    long mnSelectedChild;

    /// mode of control (true -> 8 points, false -> 9 points)
    sal_Bool mbAngleMode;
};


typedef ::cppu::WeakAggComponentImplHelper6<
                ::drafts::com::sun::star::accessibility::XAccessible,
                ::drafts::com::sun::star::accessibility::XAccessibleComponent,
                ::drafts::com::sun::star::accessibility::XAccessibleContext,
                ::drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                ::com::sun::star::lang::XServiceInfo,
                ::com::sun::star::lang::XServiceName >
                SvxRectCtlChildAccessibleContext_Base;


class SvxRectCtlChildAccessibleContext : public SvxRectCtlChildAccessibleContext_Base
{
public:
    SvxRectCtlChildAccessibleContext(
        const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        const Window& rParentWindow,
        const ::rtl::OUString& rName, const ::rtl::OUString& rDescription,
        const Rectangle& rBoundingBox,
        long nIndexInParent );
protected:
    virtual ~SvxRectCtlChildAccessibleContext();
public:
    //=====  XAccessible  =====================================================

    /// Returns the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext (void) throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

    /** Tests whether the specified point lies within this object's bounds.

        <p>The test point's coordinates are defined relative to the
        coordinate system of the object.</p>

        @param point
            Coordinates of the point to test.

        @return
            Returns <TRUE/> if the point lies within or on the object's bounding
            box and <FALSE/> otherwise.
    */
    virtual sal_Bool SAL_CALL contains(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the Accessible child that is rendered under the given point.

        @param aPoint
            Coordinates of the test point for which to find the Accessible
            child.

        @return
            If there is one child which is rendered so that its bounding box
            contains the test point then a reference to that object is
            returned.  If there is more than one child which satisfies that
            condition then a reference to that one is returned that is
            painted on top of the others.  If no there is no child which is
            rendered at the test point an empty reference is returned.
    */
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAt(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the bounding box of this object.

        <p>The returned bounding box has the form of a rectangle.  Its
        coordinates are relative to the object's parent coordinate system.
        Note that the two methods <member>getLocation</member> and
        <member>getSize</member> return the same information.  With method
        <member>getLocationOnScreen</member> you can get the bound box
        position in screen coordinates.</p>

        @return
            The coordinates of the returned rectangle are relative to this
            object's parent or relative to the screen on which this object
            is rendered if it has no parent.  If the object is not on any
            screen the returnred rectangle is empty and located at position
            (0,0).
    */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  )
        throw (::com::sun::star::uno::RuntimeException);


    /** Returns the location of the upper left corner of the object's
        bounding box relative to the parent.</p>.

        <p>The coordinates of the bounding box are given relative to the
        parent's coordinate system.</p>

        @return
            The coordinates of the returned position are relative to this
            object's parent or relative to the screen on which this object
            is rendered if it has no parent.  If the object is not on any
            screen the returnred position is (0,0).
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the location of the upper left corner of the object's
        bounding box in screen coordinates.

        <p>This method returns the same point as does the method
        <member>getLocation</member>.  The difference is that the
        coordinates are absolute screen coordinates of the screen to which
        the object is rendered instead of being relative to the object's
        parent.</p>

        @return
            The coordinates of the returned position are relative to the
            screen on which this object is rendered.  If the object is not
            on any screen the returnred position is (0,0).
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the size of this object's bounding box.

        @return
            The returned size is the size of this object or empty if it is
            not rendered on any screen.
    */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Determines if the object is showing.

        <p>An object is showing if it and all its parents are visible.  This
        does not neccessarily mean that the object is visible on the screen.
        There may be other objects obscuring it.  The showing property can
        be modified by calling <member>setVisible</member> on the objects
        and on its parents.</p>

        @return
            Returns <TRUE/> if the object is showing and <FALSE/> otherwise.

        @see isVisible, setVisible
    */
    virtual sal_Bool SAL_CALL isShowing(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Determines if the object is visible.

        <p>If an object and all of its parents are visible then the object
        is also called showing.  If an object is showing then it has also
        set the <const>AccessibleStateType::VISIBLE</const> state set in its
        <type>AccessibleStateSet</type>.</p>

        @return
            Returns <TRUE/> if the object is visible and <FALSE/> otherwise.

        @see isShowing
    */
    virtual sal_Bool SAL_CALL isVisible(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns whether the object can accept the focus or not.

        <p>If it can it has also set the
        <const>AccessibleStateType::FOCUSABLE</const> state set in its
        <type>AccessibleStateSet</type>.

        @return
            An object returns <TRUE/> if it can accept the focus and returns
            <FALSE/> otherwise.
    */
    virtual sal_Bool SAL_CALL isFocusTraversable(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Adds a new focus listener.

        <p>The focus listener will receive future focus events from this
        component.  If the specified listener is already a member of the set
        of registered focus listeners this method call is ignored.</p>

        @param xListener
            The listener object that is called for future focus events.
    */
    virtual void SAL_CALL addFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    /** Removes a focus listener.

        <p>The specified focus listener will no longer receive focus events
        from this component.  If this listener has not been added by a prior
        call to <member>addFocusListener</member> then this method call is
        ignored.</p>

        @param xListener
            Listener to remove from the list of focus listener.
    */
    virtual void SAL_CALL removeFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    /** Grabs the focus to this object.

        <p>If this object can not accept the focus,
        i.e. <member>isFocusTraversable</member> returns <FALSE/> for this
        object then nothing happens.  Otherwise the object will attempt to
        take the focus.  Nothing happens if that fails, otherwise the object
        has the focus.  This method is called <code>requestFocus</code> in
        the Java Accessibility API 1.4.</p>
    */
    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the key bindings associated with this object.

        <p>This method stems from the Java interface
        <code>AccessibleExtendedComponent</code>.</p>

        @return
            The returned value represents the current key bindings
            associated with this object.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  )
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual long SAL_CALL
        getAccessibleChildCount (void) throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::com::sun::star::uno::RuntimeException,
            ::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException);

    /// only needed to be not incompatible (remove as soon as possible)
    virtual void SAL_CALL
        addPropertyChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException) {}

    /// only needed to be not incompatible (remove as soon as possible)
    virtual void SAL_CALL removePropertyChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException) {}

    //=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addEventListener (
            const ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //  Remove an existing event listener.
    virtual void SAL_CALL
        removeEventListener (
            const ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XTypeProvider  ===================================================

    /** Returns the implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XServiceName  ====================================================

    /** Returns the name of the service that can be used to create another
        object like this one.
    */
    virtual ::rtl::OUString SAL_CALL
        getServiceName (void)
        throw (::com::sun::star::uno::RuntimeException);


    /// Sets the checked status
    void setStateChecked( sal_Bool bChecked );
protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Calls all Listener to tell they the change.
    void CommitChange(const drafts::com::sun::star::accessibility::AccessibleEventObject& rEvent);


    /// Mutex guarding this object.
    ::osl::Mutex        maMutex;

private:

    /// Bounding box
    Rectangle*          mpBoundingBox;

    /// window of parent
    const Window&       mrParentWindow;

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                        mxParent;

    /** Description of this object.  This is not a constant because it can
        be set from the outside.  Furthermore, it changes according the the
        draw page's display mode.
    */
    ::rtl::OUString     msDescription;

    /** Name of this object.  It changes according the the draw page's
        display mode.
    */
    ::rtl::OUString     msName;

    /// List of property change listeners.
    cppu::OInterfaceContainerHelper*
                        mpEventListeners;

    /// index of child in parent
    long                mnIndexInParent;

    /// Indicates, if object is checked
    sal_Bool            mbIsChecked;
};


#endif
