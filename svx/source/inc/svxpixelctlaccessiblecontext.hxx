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
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/basemutex.hxx>
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
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleEventBroadcaster,
            css::lang::XServiceInfo >
            SvxPixelCtlAccessibleChild_BASE;

class SvxPixelCtlAccessibleChild :
    public ::cppu::BaseMutex,
    public SvxPixelCtlAccessibleChild_BASE
{
    SvxPixelCtl& mrParentWindow;
    css::uno::Reference< css::accessibility::XAccessible > mxParent;
    bool m_bPixelColorOrBG;//Pixel Color Or BackGround Color
    Rectangle*  mpBoundingBox;
    /// index of child in parent
    long                                mnIndexInParent;
    /// Mutex guarding this object.
    ::osl::Mutex                        m_aMutex;
public:
    SvxPixelCtlAccessibleChild(
                SvxPixelCtl& rWindow,
                bool bPixelColorOrBG,
                const Rectangle& rBounds,
                const css::uno::Reference< css::accessibility::XAccessible >& xParent,
                long nIndexInParent );
    virtual ~SvxPixelCtlAccessibleChild();

    //XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;


    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    //Methods for XAccessibleEventBroadcaster
    sal_uInt32 mnClientId;

    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;
    void CommitChange( const css::accessibility::AccessibleEventObject& rEvent );

    //Solution: Add the event handling method
    void FireAccessibleEvent (short nEventId, const css::uno::Any& rOld, const css::uno::Any& rNew);
    virtual void SAL_CALL disposing() override;

    Rectangle GetBoundingBoxOnScreen() throw( css::uno::RuntimeException );

    Rectangle const & GetBoundingBox() throw( css::uno::RuntimeException );

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;
    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive() const;
    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive() throw( css::lang::DisposedException );


    void CheckChild();
    void SelectChild( bool bSelect);
    void ChangePixelColorOrBG(bool bPixelColorOrBG){ m_bPixelColorOrBG = bPixelColorOrBG ;}
    OUString GetName();
};


typedef ::cppu::WeakAggComponentImplHelper6<
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleEventBroadcaster,
            css::accessibility::XAccessibleSelection,
            css::lang::XServiceInfo >
            SvxPixelCtlAccessible_BASE;

class SvxPixelCtlAccessible :
    public ::cppu::BaseMutex,
    public SvxPixelCtlAccessible_BASE
{
    SvxPixelCtl& mrPixelCtl;

public:
    SvxPixelCtlAccessible(SvxPixelCtl& rWindow);
    virtual ~SvxPixelCtlAccessible();

    void ensureIsAlive() const throw ( css::lang::DisposedException );

    //XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;


    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;
    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL clearAccessibleSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL selectAllAccessibleChildren(  ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    //Methods for XAccessibleEventBroadcaster
    sal_uInt32 mnClientId;

    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override;
    void CommitChange( const css::accessibility::AccessibleEventObject& rEvent );
    //Solution: Add the event handling method
    void FireAccessibleEvent (short nEventId, const css::uno::Any& rOld, const css::uno::Any& rNew);
    virtual void SAL_CALL disposing() override;

    css::uno::Reference< css::accessibility::XAccessible >
         CreateChild (long nIndex, Point mPoint);

    void LoseFocus();

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

protected:
    css::uno::Reference< css::accessibility::XAccessible> m_xCurChild;
public:
    void NotifyChild(long nIndex, bool bSelect, bool bCheck);
};

inline bool SvxPixelCtlAccessible::IsAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvxPixelCtlAccessibleChild::IsAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvxPixelCtlAccessibleChild::IsNotAlive() const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
