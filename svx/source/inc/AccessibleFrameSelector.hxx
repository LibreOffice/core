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
#include <tools/resary.hxx>
#include <tools/rc.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
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
                >,
    public Resource
{
public:
    explicit            AccFrameSelector( FrameSelector& rFrameSel, FrameBorderType eBorder );

    virtual             ~AccFrameSelector();

    //XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener ( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener ( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener) throw (css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    void    Invalidate();
    void    NotifyFocusListeners(bool bGetFocus);
    void    NotifyAccessibleEvent( const sal_Int16 _nEventId, const css::uno::Any& _rOldValue, const css::uno::Any& _rNewValue );

protected:
    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );

    void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

private:
    void                IsValid() throw (css::uno::RuntimeException);
    void                RemoveFrameSelEventListener();

    VclPtr<FrameSelector>  mpFrameSel;
    ::osl::Mutex        maFocusMutex;
    ::osl::Mutex        maPropertyMutex;

    FrameBorderType     meBorder;

    ::cppu::OInterfaceContainerHelper maFocusListeners;
    ::cppu::OInterfaceContainerHelper maPropertyListeners;

    ResStringArray      maNames;
    ResStringArray      maDescriptions;

    ::comphelper::AccessibleEventNotifier::TClientId    mnClientId;
};



}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
