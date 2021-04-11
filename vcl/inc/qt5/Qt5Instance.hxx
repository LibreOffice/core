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

#include <vclpluginapi.h>
#include <unx/geninst.h>
#include <salusereventlist.hxx>
#include <vcl/timer.hxx>

#include <osl/conditn.hxx>

#include <QtCore/QObject>

#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

#include "Qt5FilePicker.hxx"

class Qt5Timer;

class QApplication;
class SalYieldMutex;
class SalFrame;

struct StdFreeCStr
{
    void operator()(char* arg) const noexcept { std::free(arg); }
};
using FreeableCStr = std::unique_ptr<char[], StdFreeCStr>;

class VCLPLUG_QT5_PUBLIC Qt5Instance : public QObject,
                                       public SalGenericInstance,
                                       public SalUserEventList
{
    Q_OBJECT

    osl::Condition m_aWaitingYieldCond;
    const bool m_bUseCairo;
    Qt5Timer* m_pTimer;
    bool m_bSleeping;
    std::unordered_map<OUString, css::uno::Reference<css::uno::XInterface>> m_aClipboards;

    std::unique_ptr<QApplication> m_pQApplication;
    std::vector<FreeableCStr> m_pFakeArgvFreeable;
    std::unique_ptr<char* []> m_pFakeArgv;
    std::unique_ptr<int> m_pFakeArgc;

    Timer m_aUpdateStyleTimer;
    bool m_bUpdateFonts;

    DECL_LINK(updateStyleHdl, Timer*, void);
    void AfterAppInit() override;

private Q_SLOTS:
    bool ImplYield(bool bWait, bool bHandleAllCurrentEvents);
    static void deleteObjectLater(QObject* pObject);
    static void localeChanged();

Q_SIGNALS:
    bool ImplYieldSignal(bool bWait, bool bHandleAllCurrentEvents);
    void deleteObjectLaterSignal(QObject* pObject);

protected:
    virtual rtl::Reference<Qt5FilePicker>
    createPicker(css::uno::Reference<css::uno::XComponentContext> const& context,
                 QFileDialog::FileMode);
    bool useCairo() const { return m_bUseCairo; }

public:
    explicit Qt5Instance(std::unique_ptr<QApplication>& pQApp, bool bUseCairo = false);
    virtual ~Qt5Instance() override;

    // handle common SalInstance setup
    static void AllocFakeCmdlineArgs(std::unique_ptr<char* []>& rFakeArgv,
                                     std::unique_ptr<int>& rFakeArgc,
                                     std::vector<FreeableCStr>& rFakeArgvFreeable);
    void MoveFakeCmdlineArgs(std::unique_ptr<char* []>& rFakeArgv, std::unique_ptr<int>& rFakeArgc,
                             std::vector<FreeableCStr>& rFakeArgvFreeable);
    static std::unique_ptr<QApplication> CreateQApplication(int& nArgc, char** pArgv);

    void RunInMainThread(std::function<void()> func);

    virtual SalFrame* CreateFrame(SalFrame* pParent, SalFrameStyleFlags nStyle) override;
    virtual SalFrame* CreateChildFrame(SystemParentData* pParent,
                                       SalFrameStyleFlags nStyle) override;
    virtual void DestroyFrame(SalFrame* pFrame) override;

    virtual SalObject* CreateObject(SalFrame* pParent, SystemWindowData* pWindowData,
                                    bool bShow) override;
    virtual void DestroyObject(SalObject* pObject) override;

    virtual std::unique_ptr<SalVirtualDevice>
    CreateVirtualDevice(SalGraphics* pGraphics, tools::Long& nDX, tools::Long& nDY,
                        DeviceFormat eFormat, const SystemGraphicsData* pData = nullptr) override;

    virtual SalInfoPrinter* CreateInfoPrinter(SalPrinterQueueInfo* pQueueInfo,
                                              ImplJobSetup* pSetupData) override;
    virtual void DestroyInfoPrinter(SalInfoPrinter* pPrinter) override;
    virtual std::unique_ptr<SalPrinter> CreatePrinter(SalInfoPrinter* pInfoPrinter) override;
    virtual void GetPrinterQueueInfo(ImplPrnQueueList* pList) override;
    virtual void GetPrinterQueueState(SalPrinterQueueInfo* pInfo) override;
    virtual OUString GetDefaultPrinter() override;
    virtual void PostPrintersChanged() override;

    virtual std::unique_ptr<SalMenu> CreateMenu(bool, Menu*) override;
    virtual std::unique_ptr<SalMenuItem> CreateMenuItem(const SalItemParams&) override;

    virtual SalTimer* CreateSalTimer() override;
    virtual SalSystem* CreateSalSystem() override;
    virtual std::shared_ptr<SalBitmap> CreateSalBitmap() override;

    virtual bool DoYield(bool bWait, bool bHandleAllCurrentEvents) override;
    virtual bool AnyInput(VclInputFlags nType) override;

    virtual OpenGLContext* CreateOpenGLContext() override;

    virtual OUString GetConnectionIdentifier() override;

    virtual void AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType,
                                         const OUString& rDocumentService) override;

    virtual std::unique_ptr<GenPspGraphics> CreatePrintGraphics() override;

    virtual bool IsMainThread() const override;

    virtual void TriggerUserEventProcessing() override;
    virtual void ProcessEvent(SalUserEvent aEvent) override;

    bool hasNativeFileSelection() const override { return true; }
    css::uno::Reference<css::ui::dialogs::XFilePicker2>
    createFilePicker(const css::uno::Reference<css::uno::XComponentContext>&) override;
    css::uno::Reference<css::ui::dialogs::XFolderPicker2>
    createFolderPicker(const css::uno::Reference<css::uno::XComponentContext>&) override;

    virtual css::uno::Reference<css::uno::XInterface>
    CreateClipboard(const css::uno::Sequence<css::uno::Any>& i_rArguments) override;
    virtual css::uno::Reference<css::uno::XInterface> CreateDragSource() override;
    virtual css::uno::Reference<css::uno::XInterface> CreateDropTarget() override;

    void UpdateStyle(bool bFontsChanged);

    void* CreateGStreamerSink(const SystemChildWindow*) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
