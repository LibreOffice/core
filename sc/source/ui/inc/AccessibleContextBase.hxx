/*************************************************************************
 *
 *  $RCSfile: AccessibleContextBase.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:13:46 $
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


#ifndef _SC_ACCESSIBLECONTEXTBASE_HXX
#define _SC_ACCESSIBLECONTEXTBASE_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_IllegalAccessibleComponentStateException_HPP_
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
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


#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#include <cppuhelper/compbase5.hxx>
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#include <unotools/servicehelper.hxx>
#include <comphelper/broadcasthelper.hxx>

class Rectangle;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef cppu::WeakAggComponentImplHelper5<
                ::com::sun::star::accessibility::XAccessible,
                ::com::sun::star::accessibility::XAccessibleComponent,
                ::com::sun::star::accessibility::XAccessibleContext,
                ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                ::com::sun::star::lang::XServiceInfo
                > ScAccessibleContextBaseWeakImpl;

typedef cppu::ImplHelper1<
                ::com::sun::star::accessibility::XAccessibleEventListener
                > ScAccessibleContextBaseImplEvent;

class ScAccessibleContextBase
    :   public comphelper::OBaseMutex,
        public ScAccessibleContextBaseWeakImpl,
        public ScAccessibleContextBaseImplEvent,
        public SfxListener
{
public:
    //=====  internal  ========================================================
    ScAccessibleContextBase(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        const sal_Int16 aRole);

    virtual void Init();
    virtual void SAL_CALL disposing();
protected:
    virtual ~ScAccessibleContextBase(void);
public:
    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  XInterface  =====================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ();

    virtual void SAL_CALL release() throw ();

    ///=====  XAccessible  =====================================================

    /// Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext(void) throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL containsPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isShowing(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVisible(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount(void) throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale(void)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::accessibility::IllegalAccessibleComponentStateException);

    ///=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //  Remove an existing event listener.
    virtual void SAL_CALL
        removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleEventListener  ========================================

    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        notifyEvent(
        const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService(const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext and Accessible service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XTypeProvider  ===================================================

     /// returns the possible types
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException);

public:
    /// Calls all Listener to tell they the change.
    void
        CommitChange(const com::sun::star::accessibility::AccessibleEventObject& rEvent) const;

    /// change the name and call the listener to tell they the change
    void
        ChangeName();

protected:
    /// Calls all Listener to tell they that the object is now DEFUNC
    void CommitDefunc() const;

    /// Calls all FocusListener to tell they that the focus is gained.
    void CommitFocusGained() const;

    /// Calls all FocusListener to tell they that the focus is lost.
    void CommitFocusLost() const;

    sal_Bool IsDefunc() const { return rBHelper.bDisposed; }

    virtual void IsObjectValid() const
        throw (::com::sun::star::lang::DisposedException);

    /// Use this method to set initial Name without notification
    void SetName(const rtl::OUString& rName) { msName = rName; }
    /// Use this method to set initial Description without notification
    void SetDescription(const rtl::OUString& rDesc) { msDescription = rDesc; }

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference<
         ::com::sun::star::accessibility::XAccessible> mxParent;

private:
    /** Description of this object.  This is not a constant because it can
        be set from the outside.  Furthermore, it changes according the the
        draw page's display mode.
    */
    ::rtl::OUString msDescription;

    /** Name of this object.  It changes according the the draw page's
        display mode.
    */
    ::rtl::OUString msName;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

    /** This is the role of this object.
    */
    sal_Int16 maRole;
};


#endif
