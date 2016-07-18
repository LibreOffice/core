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

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } }

class Rectangle;
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
        SvxRectCtl&             rRepresentation,
        const OUString*  pName = nullptr,
        const OUString*  pDescription = nullptr );
protected:
    virtual ~SvxRectCtlAccessibleContext();
public:
    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL
        containsPoint( const css::awt::Point& rPoint ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& rPoint ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Rectangle SAL_CALL
        getBounds() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Point SAL_CALL
        getLocation() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Point SAL_CALL
        getLocationOnScreen() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Size SAL_CALL
        getSize() throw( css::uno::RuntimeException, std::exception ) override;

    bool SAL_CALL
        isVisible() throw( css::uno::RuntimeException );

    virtual void SAL_CALL
        grabFocus() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild( sal_Int32 nIndex )
            throw( css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() throw( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
        getAccessibleDescription() throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL
        getAccessibleName() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::lang::Locale SAL_CALL
        getLocale()
            throw(  css::uno::RuntimeException,
                    css::accessibility::IllegalAccessibleComponentStateException, std::exception ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL
        getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() throw( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleSelection
    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
            throw( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
            throw( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        clearAccessibleSelection() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
            throw( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
            throw( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;


protected:
    // internals
    void checkChildIndex( long nIndexOfChild ) throw( css::lang::IndexOutOfBoundsException );

    void checkChildIndexOnSelection( long nIndexOfChild ) throw( css::lang::IndexOutOfBoundsException );

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
    void selectChild( RECT_POINT ePoint );
    void FireChildFocus( RECT_POINT eButton );

protected:

    /// @Return the object's current bounding box relative to the desktop.
    Rectangle GetBoundingBoxOnScreen() throw( css::uno::RuntimeException );

    /// @Return the object's current bounding box relative to the parent object.
    Rectangle GetBoundingBox() throw( css::uno::RuntimeException );

    /// Calls all Listener to tell they the change.
    void CommitChange( const css::accessibility::AccessibleEventObject& rEvent );

    virtual void SAL_CALL disposing() override;

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive() const;

    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive() throw( css::lang::DisposedException );

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

    /// mode of control (true -> 8 points, false -> 9 points)
    bool                            mbAngleMode;
};

inline bool SvxRectCtlAccessibleContext::IsAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvxRectCtlAccessibleContext::IsNotAlive() const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
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
        const Rectangle& rBoundingBox,
        long nIndexInParent );
protected:
    virtual ~SvxRectCtlChildAccessibleContext();
public:
    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL
        containsPoint( const css::awt::Point& rPoint ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& rPoint ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Rectangle SAL_CALL
        getBounds() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Point SAL_CALL
        getLocation() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Point SAL_CALL
        getLocationOnScreen() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Size SAL_CALL
        getSize() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        grabFocus() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleChild( sal_Int32 nIndex )
            throw( css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleParent() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() throw( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
        getAccessibleDescription() throw( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
        getAccessibleName() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::lang::Locale SAL_CALL
        getLocale()
            throw( css::uno::RuntimeException, css::accessibility::IllegalAccessibleComponentStateException, std::exception ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL
        getCurrentValue() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
        setCurrentValue( const css::uno::Any& aNumber ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL
        getMaximumValue() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL
        getMinimumValue() throw( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    // XServiceInfo
    virtual OUString SAL_CALL
        getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() throw( css::uno::RuntimeException, std::exception ) override;

    // internal
    /// Sets the checked status
    void setStateChecked(bool bChecked);
    void FireFocusEvent();

protected:
    Rectangle GetBoundingBoxOnScreen() throw( css::uno::RuntimeException );

    Rectangle const & GetBoundingBox() throw( css::uno::RuntimeException );

    void CommitChange( const css::accessibility::AccessibleEventObject& rEvent );

    virtual void SAL_CALL disposing() override;

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive() const;

    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive() throw( css::lang::DisposedException );

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
    Rectangle*                          mpBoundingBox;

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

inline bool SvxRectCtlChildAccessibleContext::IsNotAlive() const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
