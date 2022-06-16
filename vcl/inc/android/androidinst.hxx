/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <jni.h>
#include <android/input.h>
#include <android/log.h>
#include <android/native_window.h>
#include <headless/svpinst.hxx>
#include <headless/svpframe.hxx>

#define LOGTAG "LibreOffice/androidinst"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__))

class AndroidSalFrame;
class AndroidSalInstance : public SvpSalInstance
{
    // This JNIEnv is valid only in the thread where this
    // AndroidSalInstance object is created, which is the "LO" thread
    // in which soffice_main() runs
    JNIEnv* m_pJNIEnv;

public:
    AndroidSalInstance(std::unique_ptr<SalYieldMutex> pMutex);
    virtual ~AndroidSalInstance();
    static AndroidSalInstance* getInstance();

    virtual SalSystem* CreateSalSystem();

    // frame management
    void GetWorkArea(tools::Rectangle& rRect);
    SalFrame* CreateFrame(SalFrame* pParent, SalFrameStyleFlags nStyle, vcl::Window&);
    SalFrame* CreateChildFrame(SystemParentData* pParent, SalFrameStyleFlags nStyle,
                               vcl::Window& rWin);

    // mainloop pieces
    virtual bool AnyInput(VclInputFlags nType);

    virtual void updateMainThread();
    virtual void releaseMainThread();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
