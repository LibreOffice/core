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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECONTEXTBASE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECONTEXTBASE_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <svl/lstner.hxx>
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/servicehelper.hxx>
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
    virtual void SAL_CALL disposing() override;
protected:
    virtual ~ScAccessibleContextBase();
public:

    bool SAL_CALL isShowing(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual bool SAL_CALL isVisible()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XInterface  =====================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    ///=====  XAccessible  =====================================================

    /// Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL containsPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               std::exception) override;

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::accessibility::IllegalAccessibleComponentStateException, std::exception) override;

    ///=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    //  Remove an existing event listener.
    virtual void SAL_CALL
        removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleEventListener  ========================================

    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        notifyEvent(
        const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService(const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext and Accessible service.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

     /// returns the possible types
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox() const
        throw (::com::sun::star::uno::RuntimeException, std::exception);

public:
    /// Calls all Listener to tell they the change.
    void
        CommitChange(const com::sun::star::accessibility::AccessibleEventObject& rEvent) const;

protected:
    /// Calls all FocusListener to tell they that the focus is gained.
    void CommitFocusGained() const;

    /// Calls all FocusListener to tell they that the focus is lost.
    void CommitFocusLost() const;

    bool IsDefunc() const { return rBHelper.bDisposed; }

    void IsObjectValid() const
        throw (::com::sun::star::lang::DisposedException);

    /// Use this method to set initial Name without notification
    void SetName(const OUString& rName) { msName = rName; }
    /// Use this method to set initial Description without notification
    void SetDescription(const OUString& rDesc) { msDescription = rDesc; }

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference<
         ::com::sun::star::accessibility::XAccessible> mxParent;

private:
    /** Description of this object.  This is not a constant because it can
        be set from the outside.  Furthermore, it changes according to the
        draw page's display mode.
    */
    OUString msDescription;

    /** Name of this object.  It changes according the draw page's
        display mode.
    */
    OUString msName;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

    /** This is the role of this object.
    */
    sal_Int16 maRole;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
