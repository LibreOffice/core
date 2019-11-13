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
#include <desktop/crashreport.hxx>
#include <officecfg/Office/Common.hxx>

#if !HAVE_FEATURE_SKIA
bool SkiaHelper::isVCLSkiaEnabled() { return false; }

#else

static bool supportsVCLSkia()
{
    static bool bDisableSkia = !!getenv("SAL_DISABLESKIA");
    bool bBlacklisted = false; // TODO isDeviceBlacklisted();

    return !bDisableSkia && !bBlacklisted;
}

bool SkiaHelper::isVCLSkiaEnabled()
{
    /**
     * The !bSet part should only be called once! Changing the results in the same
     * run will mix Skia and normal rendering.
     */

    static bool bSet = false;
    static bool bEnable = false;
    static bool bForceSkia = false;

    // No hardware rendering, so no Skia
    // TODO SKIA
    if (Application::IsBitmapRendering())
        return false;

    if (bSet)
    {
        return bForceSkia || bEnable;
    }

    /*
     * There are a number of cases that these environment variables cover:
     *  * SAL_FORCESKIA forces Skia independent of any other option
     *  * SAL_DISABLESKIA avoids the use of Skia if SAL_FORCESKIA is not set
     */

    bSet = true;
    bForceSkia = !!getenv("SAL_FORCESKIA") || officecfg::Office::Common::VCL::ForceSkia::get();

    bool bRet = false;
    bool bSupportsVCLSkia = supportsVCLSkia();
    // TODO SKIA always call supportsVCLOpenGL to de-zombie the glxtest child process on X11
    if (bForceSkia)
    {
        bRet = true;
    }
    else if (bSupportsVCLSkia)
    {
        static bool bEnableSkiaEnv = !!getenv("SAL_ENABLESKIA");

        bEnable = bEnableSkiaEnv;

        if (officecfg::Office::Common::VCL::UseSkia::get())
            bEnable = false;

        // Force disable in safe mode
        if (Application::IsSafeModeEnabled())
            bEnable = false;

        bRet = bEnable;
    }

    CrashReporter::addKeyValue("UseSkia", OUString::boolean(bRet), CrashReporter::Write);

    return bRet;
}

static SkiaHelper::RenderMethod methodToUse = SkiaHelper::RenderRaster;

static bool initRenderMethodToUse()
{
    if (const char* env = getenv("SAL_SKIA"))
    {
        if (strcmp(env, "raster") == 0)
        {
            methodToUse = SkiaHelper::RenderRaster;
            return true;
        }
    }
    methodToUse = SkiaHelper::RenderVulkan;
    return true;
}

SkiaHelper::RenderMethod SkiaHelper::renderMethodToUse()
{
    static bool methodToUseInited = initRenderMethodToUse();
    (void)methodToUseInited; // Used just to ensure thread-safe one-time init.
    return methodToUse;
}

void SkiaHelper::disableRenderMethod(RenderMethod method)
{
    if (renderMethodToUse() != method)
        return;
    // Choose a fallback, right now always raster.
    methodToUse = RenderRaster;
}

#endif // HAVE_FEATURE_SKIA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
