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
#ifndef INCLUDED_SVX_SOURCE_INC_SVXPIXELCTLACCESSIBLECONTEXT_HXX
#define INCLUDED_SVX_SOURCE_INC_SVXPIXELCTLACCESSIBLECONTEXT_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/compbase7.hxx>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/implbase6.hxx>
#include <comphelper/servicehelper.hxx>

#include <svx/rectenum.hxx>
#include <tools/gen.hxx>


namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } };
class SvxPixelCtl;


typedef ::cppu::WeakAggComponentImplHelper5<
            ::com::sun::star::accessibility::XAccessible,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::lang::XServiceInfo >
            SvxPixelCtlAccessibleChild_BASE;

class SvxPixelCtlAccessibleChild :
    public ::comphelper::OBaseMutex,
    public SvxPixelCtlAccessibleChild_BASE
{
    SvxPixelCtl& mrParentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > mxParent;
       void IsValid() throw (::com::sun::star::uno::RuntimeException);
    bool m_bPixelColorOrBG;//Pixel Color Or BackGround Color
    Point maPoint;
    Rectangle*  mpBoundingBox;
    /// index of child in parent
    long                                mnIndexInParent;
    /// Mutex guarding this object.
    ::osl::Mutex                        m_aMutex;
public:
    SvxPixelCtlAccessibleChild(
                SvxPixelCtl& rWindow,
                bool bPixelColorOrBG,
                const Point& aPoint,
                const Rectangle& rBounds,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xParent,
                long nIndexInParent );
    virtual ~SvxPixelCtlAccessibleChild();

    //XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //Methods for XAccessibleEventBroadcaster
    sal_uInt32 mnClientId;

    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    void CommitChange( const com::sun::star::accessibility::AccessibleEventObject& rEvent );

    //Solution: Add the event handling method
    void FireAccessibleEvent (short nEventId, const ::com::sun::star::uno::Any& rOld, const ::com::sun::star::uno::Any& rNew);
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    virtual Rectangle GetBoundingBoxOnScreen( void ) throw( ::com::sun::star::uno::RuntimeException );

    virtual Rectangle GetBoundingBox( void ) throw( ::com::sun::star::uno::RuntimeException );

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive( void ) const;
    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive( void ) const;
    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive( void ) throw( ::com::sun::star::lang::DisposedException );


    void CheckChild();
    void SelectChild( bool bSelect);
    void ChangePixelColorOrBG(bool bPixelColorOrBG){ m_bPixelColorOrBG = bPixelColorOrBG ;}
    OUString GetName();
};


typedef ::cppu::WeakAggComponentImplHelper6<
            ::com::sun::star::accessibility::XAccessible,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::accessibility::XAccessibleSelection,
//          ::com::sun::star::accessibility::XAccessibleTable,
            ::com::sun::star::lang::XServiceInfo >
            SvxPixelCtlAccessible_BASE;

class SvxPixelCtlAccessible :
    public ::comphelper::OBaseMutex,
    public SvxPixelCtlAccessible_BASE
{
    SvxPixelCtl& mrPixelCtl;

public:
    SvxPixelCtlAccessible(SvxPixelCtl& rWindow);
    virtual ~SvxPixelCtlAccessible();

    void ensureIsAlive() const throw ( ::com::sun::star::lang::DisposedException );

    //XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //Methods for XAccessibleEventBroadcaster
    sal_uInt32 mnClientId;

    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    void CommitChange( const com::sun::star::accessibility::AccessibleEventObject& rEvent );
    //Solution: Add the event handling method
    void FireAccessibleEvent (short nEventId, const ::com::sun::star::uno::Any& rOld, const ::com::sun::star::uno::Any& rNew);
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    /// select child
    void SelectChild( Point pos );
    void SelectChild( long nIndex);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
         CreateChild (long nIndex, Point mPoint);


    void LoseFocus();

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive( void ) const;
    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive( void ) const;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> m_xCurChild;
public:
    void NotifyChild(long nIndex, bool bSelect, bool bCheck);
};

inline bool SvxPixelCtlAccessible::IsAlive( void ) const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvxPixelCtlAccessible::IsNotAlive( void ) const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}

inline bool SvxPixelCtlAccessibleChild::IsAlive( void ) const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvxPixelCtlAccessibleChild::IsNotAlive( void ) const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
