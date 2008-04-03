/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleSlideSorterObject.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:51:40 $
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

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_SLIDE_SORTER_OBJECT_HXX
#define SD_ACCESSIBILITY_ACCESSIBLE_SLIDE_SORTER_OBJECT_HXX

#include "MutexOwner.hxx"
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
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
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace accessibility {


typedef ::cppu::WeakComponentImplHelper5<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
    ::com::sun::star::accessibility::XAccessibleContext,
    ::com::sun::star::accessibility::XAccessibleComponent,
    ::com::sun::star::lang::XServiceInfo > AccessibleSlideSorterObjectBase;


/** This class makes page objects of the slide sorter accessible.
*/
class AccessibleSlideSorterObject
    : public ::sd::MutexOwner,
      public AccessibleSlideSorterObjectBase
{
public:
    /** Create a new object that represents a page object in the slide
        sorter.
        @param rxParent
            The accessible parent.
        @param rSlideSorter
            The slide sorter whose model manages the page.
        @param nPageNumber
            The number of the page in the range [0,nPageCount).
    */
    AccessibleSlideSorterObject(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        sal_uInt16 nPageNumber);
    ~AccessibleSlideSorterObject (void);

    /** Return the page that is made accessible by the called object.
    */
    SdPage* GetPage (void) const;

    /** The page number as given to the constructor.
    */
    sal_uInt16 GetPageNumber (void) const;

    void FireAccessibleEvent (
        short nEventId,
        const ::com::sun::star::uno::Any& rOldValue,
        const ::com::sun::star::uno::Any& rNewValue);

    virtual void SAL_CALL disposing (void);



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

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void) throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL
        getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::accessibility::IllegalAccessibleComponentStateException);


    //=====  XAccessibleComponent  ================================================

    virtual sal_Bool SAL_CALL containsPoint (
        const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocation (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getSize (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL grabFocus (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getForeground (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground (void)
        throw (::com::sun::star::uno::RuntimeException);



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
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> mxParent;
    sal_uInt16 mnPageNumber;
    ::sd::slidesorter::SlideSorter& mrSlideSorter;
    sal_uInt32 mnClientId;

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
