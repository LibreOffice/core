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


#ifndef INCLUDED_SVX_SOURCE_INC_SVXRECTCTACCESSIBLECONTEXT_HXX
#define INCLUDED_SVX_SOURCE_INC_SVXRECTCTACCESSIBLECONTEXT_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/compbase7.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/servicehelper.hxx>
#include <svx/rectenum.hxx>
#include <vcl/vclptr.hxx>
#include <tools/gen.hxx>

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } }

namespace tools { class Rectangle; }
class SvxRectCtl;
class SvxRectCtlChildAccessibleContext;
namespace vcl { class Window; }


typedef ::cppu::WeakAggComponentImplHelper6<
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleEventBroadcaster,
            css::accessibility::XAccessibleSelection,
            css::lang::XServiceInfo >
            SvxRectCtlAccessibleContext_Base;

class SvxRectCtlAccessibleContext : public ::cppu::BaseMutex, public SvxRectCtlAccessibleContext_Base
{
public:
    // internal
    SvxRectCtlAccessibleContext(
        const css::uno::Reference< css::accessibility::XAccessible>& rxParent,
        SvxRectCtl&      rRepresentation );
protected:
    virtual ~SvxRectCtlAccessibleContext() override;
public:
    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL
        containsPoint( const css::awt::Point& rPoint ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    virtual css::awt::Rectangle SAL_CALL
        getBounds() override;

    virtual css::awt::Point SAL_CALL
        getLocation() override;

    virtual css::awt::Point SAL_CALL
        getLocationOnScreen() override;

    virtual css::awt::Size SAL_CALL
        getSize() override;

    /// @throws css::uno::RuntimeException
    bool SAL_CALL
        isVisible();

    virtual void SAL_CALL
        grabFocus() override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild( sal_Int32 nIndex ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() override;

    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    virtual OUString SAL_CALL
        getAccessibleName() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() override;

    virtual css::lang::Locale SAL_CALL
        getLocale() override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL
        getImplementationName() override;

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& sServiceName ) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

    // XAccessibleSelection
    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex ) override;

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex ) override;

    virtual void SAL_CALL
        clearAccessibleSelection() override;

    virtual void SAL_CALL
        selectAllAccessibleChildren() override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) override;


protected:
    // internals
    /// @throws css::lang::IndexOutOfBoundsException
    void checkChildIndex( long nIndexOfChild );

    /** Selects a new child by index.

        <p>If the child was not selected before, the state of the child will
        be updated. If the index is invalid, the index will internally set to NOCHILDSELECTED</p>

        @param nIndexOfChild
            Index of the new child which should be selected.
    */
    void selectChild( long nIndexOfChild );

public:
    /** Selects a new child by point.

        <p>If the child was not selected before, the state of the child will
        be updated. If the point is not invalid, the index will internally set to NOCHILDSELECTED</p>

        @param eButton
            Button which belongs to the child which should be selected.
    */
    void selectChild( RectPoint ePoint );
    void FireChildFocus( RectPoint eButton );

protected:

    /// @Return the object's current bounding box relative to the desktop.
    ///
    /// @throws css::uno::RuntimeException
    tools::Rectangle GetBoundingBoxOnScreen();

    /// @Return the object's current bounding box relative to the parent object.
    ///
    /// @throws css::uno::RuntimeException
    tools::Rectangle GetBoundingBox();

    virtual void SAL_CALL disposing() override;

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

    /// @throws css::lang::DisposedException if it's not alive
    void ThrowExceptionIfNotAlive();

private:
    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    OUString                     msDescription;

    /** Name of this object.
    */
    OUString                     msName;

    /// Reference to the parent object.
    css::uno::Reference< css::accessibility::XAccessible >
                                        mxParent;

    /// pointer to internal representation
    VclPtr<SvxRectCtl>                  mpRepr;

    /// array for all possible children
    SvxRectCtlChildAccessibleContext**  mpChildren;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

    /// actual selected child
    long                                mnSelectedChild;
};

inline bool SvxRectCtlAccessibleContext::IsAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

typedef ::cppu::WeakAggComponentImplHelper7<
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleEventBroadcaster,
            css::accessibility::XAccessibleValue,
            css::accessibility::XAccessibleAction,
            css::lang::XServiceInfo >
            SvxRectCtlChildAccessibleContext_Base;

class SvxRectCtlChildAccessibleContext : public SvxRectCtlChildAccessibleContext_Base
{
public:
    SvxRectCtlChildAccessibleContext(
        const css::uno::Reference< css::accessibility::XAccessible>& rxParent,
        const vcl::Window& rParentWindow,
        const OUString& rName, const OUString& rDescription,
        const tools::Rectangle& rBoundingBox,
        long nIndexInParent );
protected:
    virtual ~SvxRectCtlChildAccessibleContext() override;
public:
    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL
        containsPoint( const css::awt::Point& rPoint ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    virtual css::awt::Rectangle SAL_CALL
        getBounds() override;

    virtual css::awt::Point SAL_CALL
        getLocation() override;

    virtual css::awt::Point SAL_CALL
        getLocationOnScreen() override;

    virtual css::awt::Size SAL_CALL
        getSize() override;

    virtual void SAL_CALL
        grabFocus() override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleChild( sal_Int32 nIndex ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleParent() override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() override;

    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    virtual OUString SAL_CALL
        getAccessibleName() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() override;

    virtual css::lang::Locale SAL_CALL
        getLocale() override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL
        getCurrentValue() override;

    virtual sal_Bool SAL_CALL
        setCurrentValue( const css::uno::Any& aNumber ) override;

    virtual css::uno::Any SAL_CALL
        getMaximumValue() override;

    virtual css::uno::Any SAL_CALL
        getMinimumValue() override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;
    // XServiceInfo
    virtual OUString SAL_CALL
        getImplementationName() override;

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& sServiceName ) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

    // internal
    /// Sets the checked status
    void setStateChecked(bool bChecked);
    void FireFocusEvent();

protected:
    /// @throws css::uno::RuntimeException
    tools::Rectangle GetBoundingBoxOnScreen();

    /// @throws css::uno::RuntimeException
    tools::Rectangle const & GetBoundingBox();

    void CommitChange( const css::accessibility::AccessibleEventObject& rEvent );

    virtual void SAL_CALL disposing() override;

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

    /// @throws css::lang::DisposedException if it's not alive
    void ThrowExceptionIfNotAlive();

    /// Mutex guarding this object.
    ::osl::Mutex                        maMutex;

private:

    /** Description of this object.  This is not a constant because it can
        be set from the outside.  Furthermore, it changes according to the
        draw page's display mode.
    */
    OUString                     msDescription;

    /** Name of this object.  It changes according the draw page's
        display mode.
    */
    OUString                     msName;

    /// Reference to the parent object.
    css::uno::Reference< css::accessibility::XAccessible >
                                        mxParent;

    /// Bounding box
    tools::Rectangle                         maBoundingBox;

    /// window of parent
    const vcl::Window&                       mrParentWindow;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32                          mnClientId;

    /// index of child in parent
    long                                mnIndexInParent;

    /// Indicates, if object is checked
    bool                            mbIsChecked;
};

inline bool SvxRectCtlChildAccessibleContext::IsAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
