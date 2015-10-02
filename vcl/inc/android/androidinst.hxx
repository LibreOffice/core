/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_ANDROID_ANDROIDINST_HXX
#define INCLUDED_VCL_INC_ANDROID_ANDROIDINST_HXX

#include <jni.h>
#include <android/input.h>
#include <android/native_window.h>
#include <headless/svpinst.hxx>
#include <headless/svpframe.hxx>

class AndroidSalFrame;
class AndroidSalInstance : public SvpSalInstance
{
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
    SalFrame* CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle );
    SalFrame* CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle );

    // mainloop pieces
    virtual bool AnyInput( VclInputFlags nType );

    void      damaged(AndroidSalFrame *frame);
};

#endif // INCLUDED_VCL_INC_ANDROID_ANDROIDINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
