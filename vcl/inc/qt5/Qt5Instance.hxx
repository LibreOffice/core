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

#include <osl/conditn.hxx>

#include <QtCore/QObject>

#include <functional>

class QApplication;
class SalYieldMutex;
class SalFrame;

class VCLPLUG_QT5_PUBLIC Qt5Instance : public QObject,
                                       public SalGenericInstance,
                                       public SalUserEventList
{
    Q_OBJECT

    osl::Condition m_aWaitingYieldCond;
    int m_postUserEventId;
    const bool m_bUseCairo;
    std::unordered_map<OUString, css::uno::Reference<css::uno::XInterface>> m_aClipboards;

public:
    std::unique_ptr<QApplication> m_pQApplication;
    std::unique_ptr<char* []> m_pFakeArgvFreeable;
    std::unique_ptr<char* []> m_pFakeArgv;
    std::unique_ptr<int> m_pFakeArgc;

private Q_SLOTS:
    bool ImplYield(bool bWait, bool bHandleAllCurrentEvents);
    void ImplRunInMain();
    static void deleteObjectLater(QObject* pObject);

Q_SIGNALS:
    bool ImplYieldSignal(bool bWait, bool bHandleAllCurrentEvents);
    void ImplRunInMainSignal();
    void deleteObjectLaterSignal(QObject* pObject);

public:
    explicit Qt5Instance(bool bUseCairo = false);
    virtual ~Qt5Instance() override;

    void RunInMainThread(std::function<void()> func);

    virtual SalFrame* CreateFrame(SalFrame* pParent, SalFrameStyleFlags nStyle) override;
    virtual SalFrame* CreateChildFrame(SystemParentData* pParent,
                                       SalFrameStyleFlags nStyle) override;
    virtual void DestroyFrame(SalFrame* pFrame) override;

    virtual SalObject* CreateObject(SalFrame* pParent, SystemWindowData* pWindowData,
                                    bool bShow) override;
    virtual void DestroyObject(SalObject* pObject) override;

    virtual std::unique_ptr<SalVirtualDevice>
    CreateVirtualDevice(SalGraphics* pGraphics, long& nDX, long& nDY, DeviceFormat eFormat,
                        const SystemGraphicsData* pData = nullptr) override;

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

    virtual GenPspGraphics* CreatePrintGraphics() override;

    virtual bool IsMainThread() const override;

    virtual void TriggerUserEventProcessing() override;
    virtual void ProcessEvent(SalUserEvent aEvent) override;

    virtual css::uno::Reference<css::ui::dialogs::XFilePicker2>
    createFilePicker(const css::uno::Reference<css::uno::XComponentContext>&) override;

    virtual css::uno::Reference<css::ui::dialogs::XFolderPicker2>
    createFolderPicker(const css::uno::Reference<css::uno::XComponentContext>&) override;

    virtual css::uno::Reference<css::uno::XInterface>
    CreateClipboard(const css::uno::Sequence<css::uno::Any>& i_rArguments) override;
    virtual css::uno::Reference<css::uno::XInterface> CreateDragSource() override;
    virtual css::uno::Reference<css::uno::XInterface> CreateDropTarget() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
