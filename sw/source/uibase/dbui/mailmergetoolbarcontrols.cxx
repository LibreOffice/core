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
#include <mmconfigitem.hxx>
#include <swmodule.hxx>
#include <view.hxx>

using namespace css;

namespace {

/// Controller for .uno:MailMergeExcludeEntry toolbar checkbox: creates the checkbox & handles the value.
class MMExcludeEntryController : public svt::ToolboxController, public lang::XServiceInfo
{
    VclPtr<CheckBox> m_pExcludeCheckbox;

    DECL_LINK_TYPED(ExcludeHdl, CheckBox&, void);

public:
    MMExcludeEntryController(const uno::Reference<uno::XComponentContext>& rContext)
        : svt::ToolboxController(rContext, uno::Reference<frame::XFrame>(), OUString(".uno:MailMergeExcludeEntry"))
        , m_pExcludeCheckbox(nullptr)
    {
    }

    virtual ~MMExcludeEntryController()
    {
    }

    // XInterface
    virtual uno::Any SAL_CALL queryInterface(const uno::Type& aType) throw (uno::RuntimeException, std::exception) override
    {
        uno::Any a(ToolboxController::queryInterface(aType));
        if (a.hasValue())
            return a;

        return ::cppu::queryInterface(aType, static_cast<lang::XServiceInfo*>(this));
    }

    void SAL_CALL acquire() throw ()
    {
        ToolboxController::acquire();
    }

    void SAL_CALL release() throw ()
    {
        ToolboxController::release();
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (uno::RuntimeException, std::exception) override
    {
        return OUString("lo.writer.MMExcludeEntryController");
    }

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) throw (uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw (uno::RuntimeException, std::exception) override
    {
        uno::Sequence<OUString> aServices { "com.sun.star.frame.ToolbarController" };
        return aServices;
    }

    // XComponent
    virtual void SAL_CALL dispose() throw (uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize(const uno::Sequence< uno::Any >& aArguments) throw (uno::Exception, uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual uno::Reference<awt::XWindow> SAL_CALL createItemWindow(const uno::Reference<awt::XWindow>& rParent) throw (uno::RuntimeException, std::exception) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& rEvent) throw (uno::RuntimeException, std::exception) override;
};

void MMExcludeEntryController::dispose() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();
    m_pExcludeCheckbox.disposeAndClear();
}

void MMExcludeEntryController::initialize(const uno::Sequence< uno::Any >& aArguments) throw (uno::Exception, uno::RuntimeException, std::exception)
{
    svt::ToolboxController::initialize(aArguments);
}

uno::Reference<awt::XWindow> MMExcludeEntryController::createItemWindow(const uno::Reference<awt::XWindow>& rParent) throw (uno::RuntimeException, std::exception)
{
    vcl::Window* pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent);
    if (pToolbar)
    {
        // make it visible
        m_pExcludeCheckbox = VclPtr<CheckBox>::Create(pToolbar);
        m_pExcludeCheckbox->SetText(SW_RES(ST_EXCLUDE));
        m_pExcludeCheckbox->SetSizePixel(m_pExcludeCheckbox->GetOptimalSize());

        m_pExcludeCheckbox->SetToggleHdl(LINK(this, MMExcludeEntryController, ExcludeHdl));
    }

    return uno::Reference<awt::XWindow>(VCLUnoHelper::GetInterface(m_pExcludeCheckbox));
}

IMPL_LINK_TYPED(MMExcludeEntryController, ExcludeHdl, CheckBox&, rCheckbox, void)
{
    SwView* pView = ::GetActiveView();
    SwMailMergeConfigItem* pConfigItem = pView->GetMailMergeConfigItem();

    if (pConfigItem)
        pConfigItem->ExcludeRecord(pConfigItem->GetResultSetPosition(), rCheckbox.IsChecked());
};

void MMExcludeEntryController::statusChanged(const frame::FeatureStateEvent& rEvent) throw (uno::RuntimeException, std::exception)
{
    if (!m_pExcludeCheckbox)
        return;

    SwView* pView = ::GetActiveView();
    SwMailMergeConfigItem* pConfigItem = pView->GetMailMergeConfigItem();

    if (!pConfigItem || !rEvent.IsEnabled)
    {
        m_pExcludeCheckbox->Disable();
        m_pExcludeCheckbox->Check(false);
    }
    else
    {
        m_pExcludeCheckbox->Enable();
        m_pExcludeCheckbox->Check(pConfigItem->IsRecordExcluded(pConfigItem->GetResultSetPosition()));
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface * SAL_CALL
lo_writer_MMExcludeEntryController_get_implementation(
    uno::XComponentContext *context,
    uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new MMExcludeEntryController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
