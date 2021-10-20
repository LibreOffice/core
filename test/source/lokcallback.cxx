/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/lokcallback.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <rtl/strbuf.hxx>
#include <tools/gen.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>

TestLokCallbackWrapper::TestLokCallbackWrapper(LibreOfficeKitCallback callback, void* data)
    : Idle("TestLokCallbackWrapper flush timer")
    , m_callback(callback)
    , m_data(data)
{
    // Flushing timer needs to run with the lowest priority, so that all pending tasks
    // such as invalidations are processed before it.
    SetPriority(TaskPriority::LOWEST);
}

inline void TestLokCallbackWrapper::callCallback(int nType, const char* pPayload)
{
    m_callback(nType, pPayload, m_data);
    if (!IsActive())
        Start();
}

void TestLokCallbackWrapper::libreOfficeKitViewCallback(int nType, const char* pPayload)
{
    callCallback(nType, pPayload);
}

void TestLokCallbackWrapper::libreOfficeKitViewCallbackWithViewId(int nType, const char* pPayload,
                                                                  int /*nViewId*/)
{
    callCallback(nType, pPayload); // the view id is also included in payload
}

void TestLokCallbackWrapper::libreOfficeKitViewInvalidateTilesCallback(
    const tools::Rectangle* pRect, int nPart)
{
    OStringBuffer buf(64);
    if (pRect)
        buf.append(pRect->toString());
    else
        buf.append("EMPTY");
    if (comphelper::LibreOfficeKit::isPartInInvalidation())
    {
        buf.append(", ");
        buf.append(nPart);
    }
    callCallback(LOK_CALLBACK_INVALIDATE_TILES, buf.makeStringAndClear().getStr());
}

void TestLokCallbackWrapper::Invoke()
{
    // Timer timeout, flush any possibly pending data.
    for (SfxViewShell* viewShell = SfxViewShell::GetFirst(false); viewShell != nullptr;
         viewShell = SfxViewShell::GetNext(*viewShell, false))
    {
        viewShell->flushPendingLOKInvalidateTiles();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
