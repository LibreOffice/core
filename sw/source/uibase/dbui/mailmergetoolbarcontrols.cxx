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
#include <vcl/InterimItemWindow.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <strings.hrc>
#include <mmconfigitem.hxx>
#include <swmodule.hxx>
#include <view.hxx>

using namespace css;

namespace {

class CurrentEdit final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::Entry> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
public:
    CurrentEdit(vcl::Window* pParent)
        : InterimItemWindow(pParent, "modules/swriter/ui/editbox.ui", "EditBox")
        , m_xWidget(m_xBuilder->weld_entry("entry"))
    {
        m_xWidget->connect_key_press(LINK(this, CurrentEdit, KeyInputHdl));
        SetSizePixel(m_xWidget->get_preferred_size());
    }

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual void GetFocus() override
    {
        if (m_xWidget)
            m_xWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    void set_sensitive(bool bSensitive)
    {
        Enable(bSensitive);
        m_xWidget->set_sensitive(bSensitive);
    }

    bool get_sensitive() const
    {
        return m_xWidget->get_sensitive();
    }

    void set_text(const OUString& rText)
    {
        m_xWidget->set_text(rText);
    }

    OUString get_text() const
    {
        return m_xWidget->get_text();
    }

    void connect_activate(const Link<weld::Entry&, bool>& rLink)
    {
        m_xWidget->connect_activate(rLink);
    }

    virtual ~CurrentEdit() override
    {
        disposeOnce();
    }
};

IMPL_LINK(CurrentEdit, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

/// Controller for .uno:MailMergeCurrentEntry toolbar checkbox: creates the checkbox & handles the value.
class MMCurrentEntryController : public svt::ToolboxController, public lang::XServiceInfo
{
    VclPtr<CurrentEdit> m_xCurrentEdit;

    DECL_LINK(CurrentEditUpdatedHdl, weld::Entry&, bool);

public:
    explicit MMCurrentEntryController(const uno::Reference<uno::XComponentContext>& rContext)
        : svt::ToolboxController(rContext, uno::Reference<frame::XFrame>(), ".uno:MailMergeCurrentEntry")
        , m_xCurrentEdit(nullptr)
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
        return "lo.writer.MMCurrentEntryController";
    }

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { "com.sun.star.frame.ToolbarController" };
    }

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XToolbarController
    virtual uno::Reference<awt::XWindow> SAL_CALL createItemWindow(const uno::Reference<awt::XWindow>& rParent) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& rEvent) override;
};

class ExcludeCheckBox final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::CheckButton> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
public:
    ExcludeCheckBox(vcl::Window* pParent)
        : InterimItemWindow(pParent, "modules/swriter/ui/checkbox.ui", "CheckBox")
        , m_xWidget(m_xBuilder->weld_check_button("checkbutton"))
    {
        m_xWidget->set_label(SwResId(ST_EXCLUDE));
        m_xWidget->connect_key_press(LINK(this, ExcludeCheckBox, KeyInputHdl));
        SetSizePixel(m_xWidget->get_preferred_size());
    }

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual void GetFocus() override
    {
        if (m_xWidget)
            m_xWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    void set_sensitive(bool bSensitive)
    {
        Enable(bSensitive);
        m_xWidget->set_sensitive(bSensitive);
    }

    void set_active(bool bActive)
    {
        m_xWidget->set_active(bActive);
    }

    void connect_toggled(const Link<weld::ToggleButton&, void>& rLink)
    {
        m_xWidget->connect_toggled(rLink);
    }

    virtual ~ExcludeCheckBox() override
    {
        disposeOnce();
    }
};

IMPL_LINK(ExcludeCheckBox, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

/// Controller for .uno:MailMergeExcludeEntry toolbar checkbox: creates the checkbox & handles the value.
class MMExcludeEntryController : public svt::ToolboxController, public lang::XServiceInfo
{
    VclPtr<ExcludeCheckBox> m_xExcludeCheckbox;

    DECL_STATIC_LINK(MMExcludeEntryController, ExcludeHdl, weld::ToggleButton&, void);

public:
    explicit MMExcludeEntryController(const uno::Reference<uno::XComponentContext>& rContext)
        : svt::ToolboxController(rContext, uno::Reference<frame::XFrame>(), ".uno:MailMergeExcludeEntry")
        , m_xExcludeCheckbox(nullptr)
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
        return "lo.writer.MMExcludeEntryController";
    }

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { "com.sun.star.frame.ToolbarController" };
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
    m_xCurrentEdit.disposeAndClear();
}

uno::Reference<awt::XWindow> MMCurrentEntryController::createItemWindow(const uno::Reference<awt::XWindow>& rParent)
{
    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent.get());
    if (pToolbar)
    {
        // make it visible
        m_xCurrentEdit = VclPtr<CurrentEdit>::Create(pToolbar);
        m_xCurrentEdit->connect_activate(LINK(this, MMCurrentEntryController, CurrentEditUpdatedHdl));
    }

    return VCLUnoHelper::GetInterface(m_xCurrentEdit);
}

IMPL_LINK(MMCurrentEntryController, CurrentEditUpdatedHdl, weld::Entry&, rEdit, bool)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (!xConfigItem)
        return true;

    OUString aText(rEdit.get_text());
    sal_Int32 nEntry = aText.toInt32();
    if (!aText.isEmpty() && nEntry != xConfigItem->GetResultSetPosition())
    {
        xConfigItem->MoveResultSet(nEntry);
        // notify about the change
        dispatchCommand(".uno:MailMergeCurrentEntry", uno::Sequence<beans::PropertyValue>());
    }
    return true;
};

void MMCurrentEntryController::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    if (!m_xCurrentEdit)
        return;

    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (!xConfigItem || !rEvent.IsEnabled)
    {
        m_xCurrentEdit->set_sensitive(false);
        m_xCurrentEdit->set_text("");
    }
    else
    {
        sal_Int32 nEntry = m_xCurrentEdit->get_text().toInt32();
        if (!m_xCurrentEdit->get_sensitive() || nEntry != xConfigItem->GetResultSetPosition())
        {
            m_xCurrentEdit->set_sensitive(true);
            m_xCurrentEdit->set_text(OUString::number(xConfigItem->GetResultSetPosition()));
        }
    }
}

void MMExcludeEntryController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();
    m_xExcludeCheckbox.disposeAndClear();
}

uno::Reference<awt::XWindow> MMExcludeEntryController::createItemWindow(const uno::Reference<awt::XWindow>& rParent)
{
    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent.get());
    if (pToolbar)
    {
        // make it visible
        m_xExcludeCheckbox = VclPtr<ExcludeCheckBox>::Create(pToolbar);
        m_xExcludeCheckbox->connect_toggled(LINK(this, MMExcludeEntryController, ExcludeHdl));
    }

    return VCLUnoHelper::GetInterface(m_xExcludeCheckbox);
}

IMPL_STATIC_LINK(MMExcludeEntryController, ExcludeHdl, weld::ToggleButton&, rCheckbox, void)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (xConfigItem)
        xConfigItem->ExcludeRecord(xConfigItem->GetResultSetPosition(), rCheckbox.get_active());
};

void MMExcludeEntryController::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    if (!m_xExcludeCheckbox)
        return;

    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
    if (pView)
        xConfigItem = pView->GetMailMergeConfigItem();

    if (!xConfigItem || !rEvent.IsEnabled)
    {
        m_xExcludeCheckbox->set_sensitive(false);
        m_xExcludeCheckbox->set_active(false);
    }
    else
    {
        m_xExcludeCheckbox->set_sensitive(false);
        m_xExcludeCheckbox->set_active(xConfigItem->IsRecordExcluded(xConfigItem->GetResultSetPosition()));
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface *
lo_writer_MMCurrentEntryController_get_implementation(
    uno::XComponentContext *context,
    uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new MMCurrentEntryController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface *
lo_writer_MMExcludeEntryController_get_implementation(
    uno::XComponentContext *context,
    uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new MMExcludeEntryController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
