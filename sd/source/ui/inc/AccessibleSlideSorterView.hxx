/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleSlideSorterView.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:52:11 $
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

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_SLIDE_SORTER_VIEW_HXX
#define SD_ACCESSIBILITY_ACCESSIBLE_SLIDE_SORTER_VIEW_HXX

#include "MutexOwner.hxx"

#ifndef _CPPUHELPER_COMPBASE6_HXX_
#include <cppuhelper/compbase6.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif


#include <memory>

class Window;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace accessibility {


class AccessibleSlideSorterObject;

typedef ::cppu::WeakComponentImplHelper6<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
    ::com::sun::star::accessibility::XAccessibleContext,
    ::com::sun::star::accessibility::XAccessibleComponent,
    ::com::sun::star::accessibility::XAccessibleSelection,
    ::com::sun::star::lang::XServiceInfo
    > AccessibleSlideSorterViewBase;

/** This class makes the SlideSorterViewShell accessible.  It uses objects
    of the AccessibleSlideSorterObject class to the make the page objects
    accessible.
*/
class AccessibleSlideSorterView
    : public ::sd::MutexOwner,
      public AccessibleSlideSorterViewBase
{
public:
    AccessibleSlideSorterView(
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> & rxParent,
        ::Window* pParentWindow);

    virtual ~AccessibleSlideSorterView (void);

    /** This method acts like a dispose call.  It sends a disposing to all
        of its listeners.  It may be called twice.
    */
    void Destroyed (void);

    void FireAccessibleEvent (
        short nEventId,
        const ::com::sun::star::uno::Any& rOldValue,
        const ::com::sun::star::uno::Any& rNewValue);

    virtual void SAL_CALL disposing (void);

    /** Return the implementation object of the specified child.
        @param nIndex
            Index of the child for which to return the implementation object.
    */
    AccessibleSlideSorterObject* GetAccessibleChildImplementation (sal_Int32 nIndex);

    //===== XAccessible =======================================================

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext (void)
        throw (::com::sun::star::uno::RuntimeException);


    //===== XAccessibleEventBroadcaster =======================================
    virtual void SAL_CALL
        addEventListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeEventListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener )
        throw (::com::sun::star::uno::RuntimeException);

    using cppu::WeakComponentImplHelperBase::addEventListener;
    using cppu::WeakComponentImplHelperBase::removeEventListener;

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void) throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or throw exception.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
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
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::accessibility::IllegalAccessibleComponentStateException);

    //=====  XAccessibleComponent  ================================================

    /** The default implementation uses the result of
        <member>getBounds</member> to determine whether the given point lies
        inside this object.
    */
    virtual sal_Bool SAL_CALL containsPoint (
        const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty reference.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty rectangle.
    */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the location.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty position, i.e. the
    * result of the default constructor of <type>com::sun::star::awt::Point</type>.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the size.
    */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation does nothing.
    */
    virtual void SAL_CALL grabFocus (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns black as the default foreground color.
    */
    virtual sal_Int32 SAL_CALL getForeground (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns white as the default background color.
    */
    virtual sal_Int32 SAL_CALL getBackground (void)
        throw (::com::sun::star::uno::RuntimeException);


    //===== XAccessibleSelection ==============================================

    virtual void SAL_CALL
        selectAccessibleChild (sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        clearAccessibleSelection(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        selectAllAccessibleChildren(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException);


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

    /** Returns a list of all supported services.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);


private:
    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    ::sd::slidesorter::SlideSorter& mrSlideSorter;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> mxParent;

    sal_uInt32 mnClientId;

    ::Window* mpContentWindow;

    /** Check whether or not the object has been disposed (or is in the
        state of beeing disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.
    */
    void ThrowIfDisposed (void)
        throw (::com::sun::star::lang::DisposedException);

    /** Check whether or not the object has been disposed (or is in the
        state of beeing disposed).

        @return sal_True, if the object is disposed or in the course
        of being disposed. Otherwise, sal_False is returned.
    */
    sal_Bool IsDisposed (void);
};

} // end of namespace ::accessibility

#endif
