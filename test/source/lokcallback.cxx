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

void TestLokCallbackWrapper::clear()
{
    m_viewId = -1;
    m_updatedTypes.clear();
    m_updatedTypesPerViewId.clear();
}

inline void TestLokCallbackWrapper::startTimer()
{
    if (!IsActive())
        Start();
}

constexpr int NO_VIEWID = -1;

inline void TestLokCallbackWrapper::callCallback(int nType, const char* pPayload, int nViewId)
{
    discardUpdatedTypes(nType, nViewId);
    m_callback(nType, pPayload, m_data);
    startTimer();
}

void TestLokCallbackWrapper::libreOfficeKitViewCallback(int nType, const char* pPayload)
{
    callCallback(nType, pPayload, NO_VIEWID);
}

void TestLokCallbackWrapper::libreOfficeKitViewCallbackWithViewId(int nType, const char* pPayload,
                                                                  int nViewId)
{
    callCallback(nType, pPayload, nViewId);
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
        buf.append(static_cast<sal_Int32>(nPart));
    }
    callCallback(LOK_CALLBACK_INVALIDATE_TILES, buf.makeStringAndClear().getStr(), NO_VIEWID);
}

// TODO This is probably a pointless code duplication with CallbackFlushHandler,
// and using this in unittests also means that CallbackFlushHandler does not get
// tested as thoroughly as it could. On the other hand, this class is simpler,
// so debugging those unittests should also be simpler. The proper solution
// is presumably this class using CallbackFlushHandler internally by default,
// but having an option to use this simpler code when needed.

void TestLokCallbackWrapper::libreOfficeKitViewUpdatedCallback(int nType)
{
    if (std::find(m_updatedTypes.begin(), m_updatedTypes.end(), nType) == m_updatedTypes.end())
    {
        m_updatedTypes.push_back(nType);
        startTimer();
    }
}

void TestLokCallbackWrapper::libreOfficeKitViewUpdatedCallbackPerViewId(int nType, int nViewId,
                                                                        int nSourceViewId)
{
    const PerViewIdData data{ nType, nViewId, nSourceViewId };
    auto& l = m_updatedTypesPerViewId;
    // The source view doesn't matter for uniqueness, just keep the latest one.
    auto it = std::find_if(l.begin(), l.end(), [data](const PerViewIdData& other) {
        return data.type == other.type && data.viewId == other.viewId;
    });
    if (it != l.end())
        *it = data;
    else
        l.push_back(data);
    startTimer();
}

void TestLokCallbackWrapper::discardUpdatedTypes(int nType, int nViewId)
{
    // If a callback is called directly with an event, drop the updated flag for it, since
    // the direct event replaces it.
    for (auto it = m_updatedTypes.begin(); it != m_updatedTypes.end();)
    {
        if (*it == nType)
            it = m_updatedTypes.erase(it);
        else
            ++it;
    }
    // If we do not have a specific view id, drop flag for all views.
    bool allViewIds = false;
    if (nViewId < 0)
        allViewIds = true;
    if (nType == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR
        && !comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
        allViewIds = true;
    for (auto it = m_updatedTypesPerViewId.begin(); it != m_updatedTypesPerViewId.end();)
    {
        if (it->type == nType && (allViewIds || it->viewId == nViewId))
            it = m_updatedTypesPerViewId.erase(it);
        else
            ++it;
    }
}

void TestLokCallbackWrapper::flushLOKData()
{
    if (m_updatedTypes.empty() && m_updatedTypesPerViewId.empty())
        return;
    // Ask for payloads of all the pending types that need updating, and call the generic callback with that data.
    assert(m_viewId >= 0);
    SfxViewShell* viewShell = SfxViewShell::GetFirst(false, [this](const SfxViewShell* shell) {
        return shell->GetViewShellId().get() == m_viewId;
    });
    assert(viewShell != nullptr);
    // First move data to local structures, so that notifyFromLOKCallback() doesn't modify it.
    std::vector<int> updatedTypes;
    std::swap(updatedTypes, m_updatedTypes);
    std::vector<PerViewIdData> updatedTypesPerViewId;
    std::swap(updatedTypesPerViewId, m_updatedTypesPerViewId);

    for (int type : updatedTypes)
    {
        OString payload = viewShell->getLOKPayload(type, m_viewId);
        if (!payload.isEmpty())
            libreOfficeKitViewCallback(type, payload.getStr());
    }
    for (const PerViewIdData& data : updatedTypesPerViewId)
    {
        viewShell = SfxViewShell::GetFirst(false, [data](const SfxViewShell* shell) {
            return shell->GetViewShellId().get() == data.sourceViewId;
        });
        assert(viewShell != nullptr);
        OString payload = viewShell->getLOKPayload(data.type, data.viewId);
        if (!payload.isEmpty())
            libreOfficeKitViewCallbackWithViewId(data.type, payload.getStr(), data.viewId);
    }
}

void TestLokCallbackWrapper::Invoke()
{
    // Timer timeout, flush any possibly pending data.
    for (SfxViewShell* viewShell = SfxViewShell::GetFirst(false); viewShell != nullptr;
         viewShell = SfxViewShell::GetNext(*viewShell, false))
    {
        viewShell->flushPendingLOKInvalidateTiles();
    }
    flushLOKData();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
