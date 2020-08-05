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

#include <ObjectIdentifier.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svtools/toolboxcontroller.hxx>

#include <vcl/InterimItemWindow.hxx>
#include <cppuhelper/weakref.hxx>

namespace chart
{

struct ListBoxEntryData
{
    OUString UIName;
    ObjectIdentifier OID;
    sal_Int32 nHierarchyDepth;

    ListBoxEntryData() : nHierarchyDepth(0)
    {
    }
};

class SelectorListBox final : public InterimItemWindow
{
public:
    SelectorListBox(vcl::Window* pParent);
    virtual void dispose() override;
    virtual ~SelectorListBox() override;

    void ReleaseFocus_Impl();

    void SetChartController( const css::uno::Reference< css::frame::XController >& xChartController );
    void UpdateChartElementsListAndSelection();

private:
    css::uno::WeakReference<css::frame::XController> m_xChartController;
    std::unique_ptr<weld::ComboBox> m_xWidget;

    std::vector<ListBoxEntryData> m_aEntries;

    bool m_bReleaseFocus;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
};

typedef ::cppu::ImplHelper1 < css::lang::XServiceInfo> ElementSelectorToolbarController_BASE;

class ElementSelectorToolbarController : public ::svt::ToolboxController
                                                , public ElementSelectorToolbarController_BASE
{
public:
    explicit ElementSelectorToolbarController();
    virtual ~ElementSelectorToolbarController() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XInterface
     virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
     virtual void SAL_CALL acquire() throw () override;
     virtual void SAL_CALL release() throw () override;

     // XStatusListener
     virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;
     // XToolbarController
     virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

private:
    VclPtr< SelectorListBox > m_apSelectorListBox;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
