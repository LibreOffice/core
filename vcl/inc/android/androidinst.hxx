/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ANDROID_SALINST_H
#define ANDROID_SALINST_H

#include <jni.h>
#include <android/input.h>
#include <android/native_window.h>
#include <headless/svpinst.hxx>
#include <headless/svpframe.hxx>

class AndroidSalFrame;
class AndroidSalInstance : public SvpSalInstance
{
    void BlitFrameToWindow(ANativeWindow_Buffer *pOutBuffer,
                           const basebmp::BitmapDeviceSharedPtr& aDev);

    // This JNIEnv is valid only in the thread where this
    // AndroidSalInstance object is created, which is the "LO" thread
    // in which soffice_main() runs
    JNIEnv *m_pJNIEnv;

public:
    AndroidSalInstance( SalYieldMutex *pMutex );
    virtual ~AndroidSalInstance();
    static AndroidSalInstance *getInstance();

    virtual SalSystem* CreateSalSystem();

    // frame management
    void GetWorkArea( Rectangle& rRect );
    SalFrame* CreateFrame( SalFrame* pParent, sal_uLong nStyle );
    SalFrame* CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle );

    // mainloop pieces
    virtual bool AnyInput( sal_uInt16 nType );

    void RedrawWindows(ANativeWindow_Buffer *pBuffer);
    SalFrame *getFocusFrame() const;

    void      damaged(AndroidSalFrame *frame);
};

#endif // ANDROID_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
