/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Novell, Inc.
 *   Michael Meeks <michael.meeks@suse.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef ANDROID_SALINST_H
#define ANDROID_SALINST_H

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/input.h>
#include <android/native_window.h>
#include <headless/svpinst.hxx>
#include <headless/svpframe.hxx>

class AndroidSalFrame;
class AndroidSalInstance : public SvpSalInstance
{
    void BlitFrameToWindow(ANativeWindow_Buffer *pOutBuffer,
                           const basebmp::BitmapDeviceSharedPtr& aDev);
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
    virtual void Wakeup();
    virtual bool AnyInput( sal_uInt16 nType );

    // incoming android event handlers:
    void      onAppCmd     (struct android_app* app, int32_t cmd);
    int32_t   onInputEvent (struct android_app* app, AInputEvent* event);
    void RedrawWindows(ANativeWindow *pWindow, ANativeWindow_Buffer *pBuffer = NULL);
    SalFrame *getFocusFrame() const;

    void      damaged(AndroidSalFrame *frame, const Rectangle &rRect);
protected:
    virtual void DoReleaseYield( int nTimeoutMS );
    struct android_app *mpApp;
    Region maRedrawRegion;
    bool   mbQueueReDraw;
};

#endif // ANDROID_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
