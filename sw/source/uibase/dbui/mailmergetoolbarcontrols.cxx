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

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <dbui.hrc>
#include <strings.hrc>
#include <mmconfigitem.hxx>
#include <swmodule.hxx>
#include <view.hxx>

using namespace css;

namespace {

/// Controller for .uno:MailMergeCurrentEntry toolbar checkbox: creates the checkbox & handles the value.
class MMCurrentEntryController : public svt::ToolboxController, public lang::XServiceInfo
{
    VclPtr<Edit> m_pCurrentEdit;

    DECL_LINK(CurrentEditUpdatedHdl, Edit&, void);

public:
    explicit MMCurrentEntryController(const uno::Reference<uno::XComponentContext>& rContext)
        : svt::ToolboxController(rContext, uno::Reference<frame::XFrame>(), ".uno:MailMergeCurrentEntry")
        , m_pCurrentEdit(nullptr)
    {
    }

    // XInterface
    virtual uno::Any SAL_CALL queryInterface(const uno::Type& aType) override
    {
        uno::Any a(ToolboxController::queryInterface(aType));
        if (a.hasValue())
            return a;

        return ::cppu::queryInterface(aType, static_cast<lang::XServiceInfo*>(this));
    }

    void SAL_CALL acquire() throw () override
    {
        ToolboxController::acquire();
    }

    void SAL_CALL release() throw () override
    {
        ToolboxController::release();
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("lo.writer.MMCurrentEntryController");
    }

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        uno::Sequence<OUString> aServices { "com.sun.star.frame.ToolbarController" };
        return aServices;
    }

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XToolbarController
    virtual uno::Reference<awt::XWindow> SAL_CALL createItemWindow(const uno::Reference<awt::XWindow>& rParent) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& rEvent) override;
};

/// Controller for .uno:MailMergeExcludeEntry toolbar checkbox: creates the checkbox & handles the value.
class MMExcludeEntryController : public svt::ToolboxController, public lang::XServiceInfo
{
    VclPtr<CheckBox> m_pExcludeCheckbox;

    DECL_STATIC_LINK(MMExcludeEntryController, ExcludeHdl, CheckBox&, void);

public:
    explicit MMExcludeEntryController(const uno::Reference<uno::XComponentContext>& rContext)
        : svt::ToolboxController(rContext, uno::Reference<frame::XFrame>(), ".uno:MailMergeExcludeEntry")
        , m_pExcludeCheckbox(nullptr)
    {
    }

    // XInterface
    virtual uno::Any SAL_CALL queryInterface(const uno::Type& aType) override
    {
        uno::Any a(ToolboxController::queryInterface(aType));
        if (a.hasValue())
            return a;

        return ::cppu::queryInterface(aType, static_cast<lang::XServiceInfo*>(this));
    }

    void SAL_CALL acquire() throw () override
    {
        ToolboxController::acquire();
    }

    void SAL_CALL release() throw () override
    {
        ToolboxController::release();
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("lo.writer.MMExcludeEntryController");
    }

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        uno::Sequence<OUString> aServices { "com.sun.star.frame.ToolbarController" };
        return aServices;
    }

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XToolbarController
    virtual uno::Reference<awt::XWindow> SAL_CALL createItemWindow(const uno::Reference<awt::XWindow>& rParent) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& rEvent) override;
};

void MMCurrentEntryController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();
    m_pCurrentEdit.disposeAndClear();
}

uno::Reference<awt::XWindow> MMCurrentEntryController::createItemWindow(const uno::Reference<awt::XWindow>& rParent)
{
    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent.get());
    if (pToolbar)
    {
        // make it visible
        m_pCurrentEdit = VclPtr<Edit>::Create(pToolbar);
        m_pCurrentEdit->SetWidthInChars(4);
        m_pCurrentEdit->SetSizePixel(m_pCurrentEdit->GetOptimalSize());

        m_pCurrentEdit->SetModifyHdl(LINK(this, MMCurrentEntryController, CurrentEditUpdatedHdl));
    }

    return uno::Reference<awt::XWindow>(VCLUnoHelper::GetInterface(m_pCurrentEdit));
}

IMPL_LINK(MMCurrentEntryController, CurrentEditUpdatedHdl, Edit&, rEdit, void)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (!xConfigItem)
        return;

    OUString aText(rEdit.GetText());
    sal_Int32 nEntry = aText.toInt32();
    if (!aText.isEmpty() && nEntry != xConfigItem->GetResultSetPosition())
    {
        xConfigItem->MoveResultSet(nEntry);
        // notify about the change
        dispatchCommand(".uno:MailMergeCurrentEntry", uno::Sequence<beans::PropertyValue>());
    }
};

void MMCurrentEntryController::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    if (!m_pCurrentEdit)
        return;

    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (!xConfigItem || !rEvent.IsEnabled)
    {
        m_pCurrentEdit->Disable();
        m_pCurrentEdit->SetText("");
    }
    else
    {
        sal_Int32 nEntry = m_pCurrentEdit->GetText().toInt32();
        if (!m_pCurrentEdit->IsEnabled() || nEntry != xConfigItem->GetResultSetPosition())
        {
            m_pCurrentEdit->Enable();
            m_pCurrentEdit->SetText(OUString::number(xConfigItem->GetResultSetPosition()));
        }
    }
}

void MMExcludeEntryController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();
    m_pExcludeCheckbox.disposeAndClear();
}

uno::Reference<awt::XWindow> MMExcludeEntryController::createItemWindow(const uno::Reference<awt::XWindow>& rParent)
{
    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent.get());
    if (pToolbar)
    {
        // make it visible
        m_pExcludeCheckbox = VclPtr<CheckBox>::Create(pToolbar);
        m_pExcludeCheckbox->SetText(SwResId(ST_EXCLUDE));
        m_pExcludeCheckbox->SetSizePixel(m_pExcludeCheckbox->GetOptimalSize());

        m_pExcludeCheckbox->SetToggleHdl(LINK(this, MMExcludeEntryController, ExcludeHdl));
    }

    return uno::Reference<awt::XWindow>(VCLUnoHelper::GetInterface(m_pExcludeCheckbox));
}

IMPL_STATIC_LINK(MMExcludeEntryController, ExcludeHdl, CheckBox&, rCheckbox, void)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (xConfigItem)
        xConfigItem->ExcludeRecord(xConfigItem->GetResultSetPosition(), rCheckbox.IsChecked());
};

void MMExcludeEntryController::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    if (!m_pExcludeCheckbox)
        return;

    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (!xConfigItem || !rEvent.IsEnabled)
    {
        m_pExcludeCheckbox->Disable();
        m_pExcludeCheckbox->Check(false);
    }
    else
    {
        m_pExcludeCheckbox->Enable();
        m_pExcludeCheckbox->Check(xConfigItem->IsRecordExcluded(xConfigItem->GetResultSetPosition()));
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface * SAL_CALL
lo_writer_MMCurrentEntryController_get_implementation(
    uno::XComponentContext *context,
    uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new MMCurrentEntryController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface * SAL_CALL
lo_writer_MMExcludeEntryController_get_implementation(
    uno::XComponentContext *context,
    uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new MMExcludeEntryController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
