/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <salinst.hxx>
#include <salmenu.hxx>
#include <salsession.hxx>
#include <salvtables.hxx>
#include <svdata.hxx>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/ColorChooserDialog.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weld.hxx>

SalInstance::SalInstance(std::unique_ptr<comphelper::SolarMutex> pMutex, SalData* pSalData,
                         const OUString& rToolkitName)
    : m_pYieldMutex(std::move(pMutex))
    , m_sToolkitName(rToolkitName)
{
    SetSalData(pSalData);
}

SalInstance::~SalInstance() {}

comphelper::SolarMutex* SalInstance::GetYieldMutex() { return m_pYieldMutex.get(); }

void SalInstance::DestroyInfoPrinter(SalInfoPrinter* pPrinter) { delete pPrinter; }

sal_uInt32 SalInstance::ReleaseYieldMutex(bool all) { return m_pYieldMutex->release(all); }

void SalInstance::AcquireYieldMutex(sal_uInt32 nCount) { m_pYieldMutex->acquire(nCount); }

std::unique_ptr<SalSession> SalInstance::CreateSalSession() { return nullptr; }

OpenGLContext* SalInstance::CreateOpenGLContext()
{
    assert(!m_bSupportsOpenGL);
    std::abort();
}

std::unique_ptr<SalMenu> SalInstance::CreateMenu(bool, Menu*)
{
    // default: no native menus
    return nullptr;
}

std::unique_ptr<SalMenuItem> SalInstance::CreateMenuItem(const SalItemParams&) { return nullptr; }

bool SalInstance::DoExecute(int&)
{
    // can't run on system event loop without implementing DoExecute and DoQuit
    if (Application::IsUseSystemEventLoop())
        std::abort();
    return false;
}

void SalInstance::DoQuit()
{
    if (Application::IsUseSystemEventLoop())
        std::abort();
}

std::unique_ptr<weld::Builder>
SalInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    vcl::Window* pParentWidget = pParentInstance ? pParentInstance->getWidget() : nullptr;
    return std::make_unique<SalInstanceBuilder>(pParentWidget, rUIRoot, rUIFile);
}

std::unique_ptr<weld::Builder> SalInstance::CreateInterimBuilder(vcl::Window* pParent,
                                                                 const OUString& rUIRoot,
                                                                 const OUString& rUIFile, bool,
                                                                 sal_uInt64)
{
    return std::make_unique<SalInstanceBuilder>(pParent, rUIRoot, rUIFile);
}

weld::MessageDialog* SalInstance::CreateMessageDialog(weld::Widget* pParent,
                                                      VclMessageType eMessageType,
                                                      VclButtonsType eButtonsType,
                                                      const OUString& rPrimaryMessage)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    SystemWindow* pParentWidget = pParentInstance ? pParentInstance->getSystemWindow() : nullptr;
    VclPtrInstance<MessageDialog> xMessageDialog(pParentWidget, rPrimaryMessage, eMessageType,
                                                 eButtonsType);
    return new SalInstanceMessageDialog(xMessageDialog, nullptr, true);
}

std::unique_ptr<weld::ColorChooserDialog>
SalInstance::CreateColorChooserDialog(weld::Window* pParent, vcl::ColorPickerMode eMode)
{
    std::unique_ptr<ColorPickerDialog> pColorPickerDialog
        = std::make_unique<ColorPickerDialog>(pParent, COL_BLACK, eMode);
    return std::make_unique<SalInstanceColorChooserDialog>(std::move(pColorPickerDialog));
}

weld::Window* SalInstance::GetFrameWeld(const css::uno::Reference<css::awt::XWindow>& rWindow)
{
    UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper();
    if (!pWrapper)
        return nullptr;
    VclPtr<vcl::Window> xWindow = pWrapper->GetWindow(rWindow);
    if (!xWindow)
        return nullptr;
    return xWindow->GetFrameWeld();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
