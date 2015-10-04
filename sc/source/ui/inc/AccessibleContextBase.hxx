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
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/servicehelper.hxx>

class Rectangle;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef cppu::WeakAggComponentImplHelper<
                css::accessibility::XAccessible,
                css::accessibility::XAccessibleComponent,
                css::accessibility::XAccessibleContext,
                css::accessibility::XAccessibleEventBroadcaster,
                css::lang::XServiceInfo
                > ScAccessibleContextBaseWeakImpl;

typedef cppu::ImplHelper<
                css::accessibility::XAccessibleEventListener
                > ScAccessibleContextBaseImplEvent;

class ScAccessibleContextBase
    :   public cppu::BaseMutex,
        public ScAccessibleContextBaseWeakImpl,
        public ScAccessibleContextBaseImplEvent,
        public SfxListener
{
public:
    //=====  internal  ========================================================
    ScAccessibleContextBase(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        const sal_Int16 aRole);

    virtual void Init();
    virtual void SAL_CALL disposing() override;
protected:
    virtual ~ScAccessibleContextBase() override;
public:

    bool SAL_CALL isShowing(  )
        throw (css::uno::RuntimeException);

    virtual bool SAL_CALL isVisible()
        throw (css::uno::RuntimeException, std::exception);

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    ///=====  XAccessible  =====================================================

    /// Return the XAccessibleContext.
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL containsPoint(
        const css::awt::Point& rPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const css::awt::Point& rPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Rectangle SAL_CALL getBounds(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocation(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocationOnScreen(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Size SAL_CALL getSize(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
               css::lang::IndexOutOfBoundsException,
               std::exception) override;

    /// Return a reference to the parent.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return NULL to indicate that an empty relation set.
    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the set of current states.
    virtual css::uno::Reference<css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual css::lang::Locale SAL_CALL
        getLocale()
        throw (css::uno::RuntimeException,
            css::accessibility::IllegalAccessibleComponentStateException, std::exception) override;

    ///=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
        throw (css::uno::RuntimeException, std::exception) override;

    //  Remove an existing event listener.
    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleEventListener  ========================================

    virtual void SAL_CALL
        disposing( const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        notifyEvent(
        const css::accessibility::AccessibleEventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService(const OUString& sServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext and Accessible service.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

     /// returns the possible types
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception);

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName()
        throw (css::uno::RuntimeException,
               std::exception);

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (css::uno::RuntimeException, std::exception);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox() const
        throw (css::uno::RuntimeException, std::exception);

public:
    /// Calls all Listener to tell they the change.
    void
        CommitChange(const css::accessibility::AccessibleEventObject& rEvent) const;

protected:
    /// Calls all FocusListener to tell they that the focus is gained.
    void CommitFocusGained() const;

    /// Calls all FocusListener to tell they that the focus is lost.
    void CommitFocusLost() const;

    bool IsDefunc() const { return rBHelper.bDisposed; }

    void IsObjectValid() const
        throw (css::lang::DisposedException);

    /// Use this method to set initial Name without notification
    void SetName(const OUString& rName) { msName = rName; }
    /// Use this method to set initial Description without notification
    void SetDescription(const OUString& rDesc) { msDescription = rDesc; }

    /// Reference to the parent object.
    css::uno::Reference<css::accessibility::XAccessible> mxParent;

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
