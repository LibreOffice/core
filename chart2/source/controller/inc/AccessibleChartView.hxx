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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ACCESSIBLECHARTVIEW_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ACCESSIBLECHARTVIEW_HXX

#include "AccessibleBase.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <memory>

namespace accessibility
{
class IAccessibleViewForwarder;
}

namespace chart
{

class ExplicitValueProvider;

namespace impl
{
typedef ::cppu::ImplInheritanceHelper<
        ::chart::AccessibleBase,
        css::lang::XInitialization,
        css::view::XSelectionChangeListener >
    AccessibleChartView_Base;
}

class AccessibleChartView :
        public impl::AccessibleChartView_Base
{
public:
    AccessibleChartView(SdrView* pView );
    virtual ~AccessibleChartView();

    AccessibleChartView() = delete;

    // ____ WeakComponentHelper (called from XComponent::dispose()) ____
    virtual void SAL_CALL disposing() override;

    // ____ lang::XInitialization ____
    // 0: view::XSelectionSupplier offers notifications for selection changes and access to the selection itself
    // 1: frame::XModel representing the chart model - offers access to object data
    // 2: lang::XInterface representing the normal chart view - offers access to some extra object data
    // 3: accessibility::XAccessible representing the parent accessible
    // 4: awt::XWindow representing the view's window (is a vcl Window)
    // all arguments are only valid until next initialization - don't keep them longer
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments )
        throw (css::uno::Exception,
               css::uno::RuntimeException, std::exception) override;

    // ____ view::XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // ________ XEventListener ________
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    // ________ XAccessibleContext ________
    virtual OUString SAL_CALL getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception) override;

    // ________ XAccessibleComponent ________
    virtual css::awt::Rectangle SAL_CALL getBounds() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() throw (css::uno::RuntimeException, std::exception) override;

protected:
    // ________ AccessibleChartElement ________
    virtual css::awt::Point   GetUpperLeftOnScreen() const override;

private: // methods
    /** @return the result that m_xWindow->getPosSize() _should_ return.  It
                returns (0,0) as upper left corner.  When calling
                getAccessibleParent, you get the parent's parent, which contains
                a decoration.  Thus you have an offset of (currently) (2,2)
                which isn't taken into account.
     */
    css::awt::Rectangle GetWindowPosSize() const;

    ExplicitValueProvider* getExplicitValueProvider();

private: // members
    css::uno::WeakReference< css::view::XSelectionSupplier >        m_xSelectionSupplier;
    css::uno::WeakReference< css::frame::XModel >                   m_xChartModel;
    css::uno::WeakReference< css::uno::XInterface >                 m_xChartView;
    css::uno::WeakReference< css::awt::XWindow >                    m_xWindow;
    css::uno::WeakReference< css::accessibility::XAccessible >      m_xParent;

    std::shared_ptr< ObjectHierarchy >                              m_spObjectHierarchy;
    AccessibleUniqueId                                              m_aCurrentSelectionOID;
    SdrView*                                                        m_pSdrView;
    ::accessibility::IAccessibleViewForwarder*                      m_pViewForwarder;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
