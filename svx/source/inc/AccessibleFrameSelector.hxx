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

#ifndef INCLUDED_SVX_SOURCE_INC_ACCESSIBLEFRAMESELECTOR_HXX
#define INCLUDED_SVX_SOURCE_INC_ACCESSIBLEFRAMESELECTOR_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <tools/link.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <svx/framebordertype.hxx>
#include <vcl/vclptr.hxx>

class VclSimpleEvent;
class VclWindowEvent;

namespace svx {

class FrameSelector;

namespace a11y {


class AccFrameSelector :
    public ::cppu::WeakImplHelper<
                css::accessibility::XAccessible,
                css::accessibility::XAccessibleContext,
                css::accessibility::XAccessibleComponent,
                css::accessibility::XAccessibleEventBroadcaster,
                css::lang::XServiceInfo
                >
{
public:
    explicit            AccFrameSelector( FrameSelector& rFrameSel, FrameBorderType eBorder );

    virtual             ~AccFrameSelector() override;

    //XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener ( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener) override;
    virtual void SAL_CALL removeAccessibleEventListener ( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    void    Invalidate();
    void    NotifyFocusListeners(bool bGetFocus);
    void    NotifyAccessibleEvent( const sal_Int16 _nEventId, const css::uno::Any& _rOldValue, const css::uno::Any& _rNewValue );

protected:
    DECL_LINK( WindowEventListener, VclWindowEvent&, void );

private:
    /// @throws css::uno::RuntimeException
    void                IsValid();
    void                RemoveFrameSelEventListener();

    VclPtr<FrameSelector>  mpFrameSel;
    ::osl::Mutex        maFocusMutex;
    ::osl::Mutex        maPropertyMutex;

    FrameBorderType     meBorder;

    ::comphelper::OInterfaceContainerHelper2 maFocusListeners;
    ::comphelper::OInterfaceContainerHelper2 maPropertyListeners;

    ::comphelper::AccessibleEventNotifier::TClientId    mnClientId;
};


}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
