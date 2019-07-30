/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/skia/SkiaHelper.hxx>

#include <vcl/svapp.hxx>

bool SkiaHelper::isVCLSkiaEnabled()
{
    /**
     * The !bSet part should only be called once! Changing the results in the same
     * run will mix OpenGL and normal rendering.
     */

    static bool bSet = false;
    static bool bEnable = false;
    static bool bForceOpenGL = false;

    // No hardware rendering, so no OpenGL
    if (Application::IsBitmapRendering())
        return false;

    //tdf#106155, disable GL while loading certain bitmaps needed for the initial toplevel windows
    //under raw X (kde) vclplug
    // TODO
    //    if (bTempOpenGLDisabled)
    //        return false;

    if (bSet)
    {
        return bForceOpenGL || bEnable;
    }

    // TODO
    return true;
#if 0
    /*
     * There are a number of cases that these environment variables cover:
     *  * SAL_FORCEGL forces OpenGL independent of any other option
     *  * SAL_DISABLEGL or a blacklisted driver avoid the use of OpenGL if SAL_FORCEGL is not set
     */

    bSet = true;
    bForceOpenGL = !!getenv("SAL_FORCEGL") || officecfg::Office::Common::VCL::ForceOpenGL::get();

    bool bRet = false;
    bool bSupportsVCLOpenGL = supportsVCLOpenGL();
    // always call supportsVCLOpenGL to de-zombie the glxtest child process on X11
    if (bForceOpenGL)
    {
        bRet = true;
    }
    else if (bSupportsVCLOpenGL)
    {
        static bool bEnableGLEnv = !!getenv("SAL_ENABLEGL");

        bEnable = bEnableGLEnv;

        if (officecfg::Office::Common::VCL::UseOpenGL::get())
            bEnable = true;

        // Force disable in safe mode
        if (Application::IsSafeModeEnabled())
            bEnable = false;

        bRet = bEnable;
    }

    if (bRet)
    {
        if (!getenv("SAL_DISABLE_GL_WATCHDOG"))
            OpenGLWatchdogThread::start();
    }
    CrashReporter::AddKeyValue("UseOpenGL", OUString::boolean(bRet));

    return bRet;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
