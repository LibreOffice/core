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
#pragma once

#include "AccessibleBase.hxx"
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

#include <memory>

namespace com::sun::star::accessibility { class XAccessible; }
namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::view { class XSelectionSupplier; }

namespace accessibility
{
class IAccessibleViewForwarder;
}

namespace chart
{

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
    virtual ~AccessibleChartView() override;

    AccessibleChartView() = delete;

    // ____ WeakComponentHelper (called from XComponent::dispose()) ____
    using AccessibleBase::disposing;

    // ____ lang::XInitialization ____
    // 0: view::XSelectionSupplier offers notifications for selection changes and access to the selection itself
    // 1: frame::XModel representing the chart model - offers access to object data
    // 2: lang::XInterface representing the normal chart view - offers access to some extra object data
    // 3: accessibility::XAccessible representing the parent accessible
    // 4: awt::XWindow representing the view's window (is a vcl Window)
    // all arguments are only valid until next initialization - don't keep them longer
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // ____ view::XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) override;

    // ________ XEventListener ________
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // ________ XAccessibleContext ________
    virtual OUString SAL_CALL getAccessibleDescription() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent() override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    // ________ XAccessibleComponent ________
    virtual css::awt::Rectangle SAL_CALL getBounds() override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;

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

private: // members
    css::uno::WeakReference< css::view::XSelectionSupplier >        m_xSelectionSupplier;
    css::uno::WeakReference< css::frame::XModel >                   m_xChartModel;
    css::uno::WeakReference< css::uno::XInterface >                 m_xChartView;
    css::uno::WeakReference< css::awt::XWindow >                    m_xWindow;
    css::uno::WeakReference< css::accessibility::XAccessible >      m_xParent;

    std::shared_ptr< ObjectHierarchy >                              m_spObjectHierarchy;
    AccessibleUniqueId                                              m_aCurrentSelectionOID;
    SdrView*                                                        m_pSdrView;
    std::unique_ptr<::accessibility::IAccessibleViewForwarder>      m_pViewForwarder;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
