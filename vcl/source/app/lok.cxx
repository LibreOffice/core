/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dbggui.hxx>
#include <salinst.hxx>
#include <svdata.hxx>

#include <LibreOfficeKit/LibreOfficeKit.hxx>
#include <tools/json_writer.hxx>
#include <vcl/lok.hxx>

namespace vcl::lok
{
void registerPollCallbacks(LibreOfficeKitPollCallback pPollCallback,
                           LibreOfficeKitWakeCallback pWakeCallback, void* pData)
{
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData)
    {
        pSVData->mpPollCallback = pPollCallback;
        pSVData->mpWakeCallback = pWakeCallback;
        pSVData->mpPollClosure = pData;
    }
}

void unregisterPollCallbacks()
{
    ImplSVData* pSVData = ImplGetSVData();
    if (!pSVData)
        return;

    // Not hyper-elegant - but in the case of Android & unipoll we need to detach
    // this thread from the JVM's clutches to avoid a crash closing document
    if (pSVData->mpPollClosure && pSVData->mpDefInst)
        pSVData->mpDefInst->releaseMainThread();

    // Just set mpPollClosure to null as that is what calling this means, that the callback data
    // points to an object that no longer exists. In particular, don't set
    // pSVData->mpPollCallback to nullptr as that is used to detect whether Unipoll is in use in
    // isUnipoll().
    pSVData->mpPollClosure = nullptr;
}

bool isUnipoll()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData && pSVData->mpPollCallback != nullptr;
}

void numberOfViewsChanged(int count)
{
    if (count == 0)
        return;
    ImplSVData* pSVData = ImplGetSVData();
    auto& rCache = pSVData->maGDIData.maScaleCache;
    // Normally the cache size is set to 10, scale according to the number of users.
    rCache.setMaxSize(count * 10);
}

void dumpState(rtl::OStringBuffer& rState)
{
    ImplSVData* pSVData = ImplGetSVData();
    if (!pSVData)
        return;

#ifndef NDEBUG
    // lo_dumpState deliberately doesn't take SolarMutexGuard
    // so disable these checks during dumpState
    DbgGUIDeInitSolarMutexCheck();
#endif

    rState.append("\nWindows:\t");
    rState.append(static_cast<sal_Int32>(Application::GetTopWindowCount()));

    vcl::Window* pWin = Application::GetFirstTopLevelWindow();
    while (pWin)
    {
        tools::JsonWriter aProps;

        rState.append("\n\tWindow: ");

        auto notifier = pWin->GetLOKNotifier();
        if (notifier)
        {
            rState.append(notifier->dumpNotifyState());
            rState.append(" ");
        }
        else
            rState.append("no notifier ");

        OString aPropStr = aProps.finishAndGetAsOString();
        if (aPropStr.getLength() > 256)
        {
            rState.append(aPropStr.subView(0, 256));
            rState.append("...");
        }
        else
            rState.append(aPropStr);

        pWin = Application::GetNextTopLevelWindow(pWin);
    }

    pSVData->dumpState(rState);

#ifndef NDEBUG
    DbgGUIInitSolarMutexCheck();
#endif
}

void trimMemory(int nTarget)
{
    if (nTarget >= 1000)
    {
        SolarMutexGuard aGuard;
        ImplSVData* pSVData = ImplGetSVData();
        if (!pSVData) // shutting down
            return;
        pSVData->dropCaches();
    }
    // else for now caches re-fill themselves as/when used.
}

} // namespace lok, namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
