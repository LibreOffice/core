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
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/implbase6.hxx>
#include <cppuhelper/implbase7.hxx>
#include <comphelper/servicehelper.hxx>
#include <svx/rectenum.hxx>

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } }

class Rectangle;
class SvxRectCtl;
class SvxRectCtlChildAccessibleContext;
class Window;


typedef ::cppu::WeakAggComponentImplHelper6<
            ::com::sun::star::accessibility::XAccessible,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::accessibility::XAccessibleSelection,
            ::com::sun::star::lang::XServiceInfo >
            SvxRectCtlAccessibleContext_Base;

class SvxRectCtlAccessibleContext : public ::comphelper::OBaseMutex, public SvxRectCtlAccessibleContext_Base
{
public:
    //=====  internal  ========================================================
    SvxRectCtlAccessibleContext(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent,
        SvxRectCtl&             rRepresentation,
        const OUString*  pName = NULL,
        const OUString*  pDescription = NULL );
protected:
    virtual ~SvxRectCtlAccessibleContext();
public:
    //=====  XAccessible  =====================================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL
        containsPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Rectangle SAL_CALL
        getBounds() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Point SAL_CALL
        getLocation() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Point SAL_CALL
        getLocationOnScreen() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Size SAL_CALL
        getSize() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Bool SAL_CALL
        isShowing() throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        isVisible() throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        isFocusTraversable() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        addFocusListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        removeFocusListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        grabFocus() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Any SAL_CALL
        getAccessibleKeyBinding() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
        getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL
        getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild( sal_Int32 nIndex )
            throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Int16 SAL_CALL
        getAccessibleRole( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual OUString SAL_CALL
        getAccessibleDescription( void ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual OUString SAL_CALL
        getAccessibleName( void ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale( void )
            throw(  ::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::accessibility::IllegalAccessibleComponentStateException, std::exception );

    //=====  XAccessibleEventBroadcaster  =====================================

    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception );

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception );

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL
        getImplementationName( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //=====  XAccessibleSelection =============================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
            throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
            throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception );

    virtual void SAL_CALL
        clearAccessibleSelection() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual void SAL_CALL
        selectAllAccessibleChildren() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
            throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception );

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
            throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception );


protected:
    //=====  internals ========================================================

    void checkChildIndex( long nIndexOfChild ) throw( ::com::sun::star::lang::IndexOutOfBoundsException );

    void checkChildIndexOnSelection( long nIndexOfChild ) throw( ::com::sun::star::lang::IndexOutOfBoundsException );

    /** Selects a new child by index.

        <p>If the child was not selected before, the state of the child will
        be updated. If the index is invalid, the index will internally set to NOCHILDSELECTED</p>

        @param nIndexOfChild
            Index of the new child which should be selected.
    */
    void selectChild( long nIndexOfChild, sal_Bool bFireFocus = sal_True);

public:
    /** Selects a new child by point.

        <p>If the child was not selected before, the state of the child will
        be updated. If the point is not invalid, the index will internally set to NOCHILDSELECTED</p>

        @param eButton
            Button which belongs to the child which should be selected.
    */
    void selectChild( RECT_POINT ePoint, sal_Bool bFireFocus = sal_True );
    void FireChildFocus( RECT_POINT eButton );

private:
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUniqueId( void );
protected:

    /// @Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen( void ) throw( ::com::sun::star::uno::RuntimeException );

    /// @Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox( void ) throw( ::com::sun::star::uno::RuntimeException );

    /// Calls all Listener to tell they the change.
    void CommitChange( const com::sun::star::accessibility::AccessibleEventObject& rEvent );

    virtual void SAL_CALL disposing();

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive( void ) const;

    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive( void ) const;

    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive( void ) throw( ::com::sun::star::lang::DisposedException );

private:
    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    OUString                     msDescription;

    /** Name of this object.
    */
    OUString                     msName;

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                        mxParent;

    /// pointer to internal representation
    SvxRectCtl*                         mpRepr;

    /// array for all possible children
    SvxRectCtlChildAccessibleContext**  mpChildren;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

    /// actual selected child
    long                                mnSelectedChild;

    /// mode of control (true -> 8 points, false -> 9 points)
    bool                            mbAngleMode;
};

inline bool SvxRectCtlAccessibleContext::IsAlive( void ) const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvxRectCtlAccessibleContext::IsNotAlive( void ) const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}

typedef ::cppu::WeakAggComponentImplHelper7<
            ::com::sun::star::accessibility::XAccessible,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::accessibility::XAccessibleValue,
            ::com::sun::star::accessibility::XAccessibleAction,
            ::com::sun::star::lang::XServiceInfo >
            SvxRectCtlChildAccessibleContext_Base;

class SvxRectCtlChildAccessibleContext : public SvxRectCtlChildAccessibleContext_Base
{
public:
    SvxRectCtlChildAccessibleContext(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent,
        const Window& rParentWindow,
        const OUString& rName, const OUString& rDescription,
        const Rectangle& rBoundingBox,
        long nIndexInParent );
protected:
    virtual ~SvxRectCtlChildAccessibleContext();
public:
    //=====  XAccessible  =====================================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL
        containsPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Rectangle SAL_CALL
        getBounds() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Point SAL_CALL
        getLocation() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Point SAL_CALL
        getLocationOnScreen() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::awt::Size SAL_CALL
        getSize() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Bool SAL_CALL
        isShowing() throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        isVisible() throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        isFocusTraversable() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        grabFocus() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Any SAL_CALL
        getAccessibleKeyBinding() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
        getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL
        getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleChild( sal_Int32 nIndex )
            throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleParent( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Int16 SAL_CALL
        getAccessibleRole( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual OUString SAL_CALL
        getAccessibleDescription( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual OUString SAL_CALL
        getAccessibleName( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale( void )
            throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::accessibility::IllegalAccessibleComponentStateException, std::exception );

    //=====  XAccessibleEventBroadcaster  =====================================

    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception );

    //=====  XAccessibleValue  ================================================

    virtual ::com::sun::star::uno::Any SAL_CALL
        getCurrentValue() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Bool SAL_CALL
        setCurrentValue( const ::com::sun::star::uno::Any& aNumber ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Any SAL_CALL
        getMaximumValue() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Any SAL_CALL
        getMinimumValue() throw( ::com::sun::star::uno::RuntimeException, std::exception );

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL
        getImplementationName( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );


    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId( void ) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //===== internal ==========================================================

    /// Sets the checked status
    void setStateChecked(bool bChecked, bool bFireFocus = true);
    void FireFocusEvent();

protected:
    virtual Rectangle GetBoundingBoxOnScreen( void ) throw( ::com::sun::star::uno::RuntimeException );

    virtual Rectangle GetBoundingBox( void ) throw( ::com::sun::star::uno::RuntimeException );

    void CommitChange( const com::sun::star::accessibility::AccessibleEventObject& rEvent );

    virtual void SAL_CALL disposing();

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive( void ) const;

    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive( void ) const;

    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive( void ) throw( ::com::sun::star::lang::DisposedException );

    /// Mutex guarding this object.
    ::osl::Mutex                        maMutex;

private:

    /** Description of this object.  This is not a constant because it can
        be set from the outside.  Furthermore, it changes according the the
        draw page's display mode.
    */
    OUString                     msDescription;

    /** Name of this object.  It changes according the draw page's
        display mode.
    */
    OUString                     msName;

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                        mxParent;

    /// Bounding box
    Rectangle*                          mpBoundingBox;

    /// window of parent
    const Window&                       mrParentWindow;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32                          mnClientId;

    /// index of child in parent
    long                                mnIndexInParent;

    /// Indicates, if object is checked
    bool                            mbIsChecked;
};

inline bool SvxRectCtlChildAccessibleContext::IsAlive( void ) const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvxRectCtlChildAccessibleContext::IsNotAlive( void ) const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
