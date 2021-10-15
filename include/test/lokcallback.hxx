/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <test/testdllapi.hxx>
#include <LibreOfficeKit/LibreOfficeKitTypes.h>
#include <sfx2/lokcallback.hxx>
#include <vcl/idle.hxx>

#include <vector>

/**
A helper to convert SfxLokCallbackInterface to a LIbreOfficeKitCallback for tests.

It reimplements the specialized callbacks and converts them to the generic type/payload
callback.
*/
class OOO_DLLPUBLIC_TEST TestLokCallbackWrapper final : public SfxLokCallbackInterface, public Idle
{
public:
    TestLokCallbackWrapper(LibreOfficeKitCallback callback, void* data);
    /// Discard all possibly still held events.
    void clear();
    /// Set the view id of the associated SfxViewShell.
    void setLOKViewId(int viewId) { m_viewId = viewId; }
    virtual void libreOfficeKitViewCallback(int nType, const char* pPayload) override;
    virtual void libreOfficeKitViewCallbackWithViewId(int nType, const char* pPayload,
                                                      int nViewId) override;
    virtual void libreOfficeKitViewInvalidateTilesCallback(const tools::Rectangle* pRect,
                                                           int nPart) override;
    virtual void libreOfficeKitViewUpdatedCallback(int nType) override;
    virtual void libreOfficeKitViewUpdatedCallbackPerViewId(int nType, int nViewId,
                                                            int nSourceViewId) override;
    virtual void Invoke() override;

private:
    void callCallback(int nType, const char* pPayload, int nViewId);
    void startTimer();
    void flushLOKData();
    void discardUpdatedTypes(int nType, int nViewId);
    LibreOfficeKitCallback m_callback;
    void* m_data;
    int m_viewId = -1; // the associated SfxViewShell
    std::vector<int> m_updatedTypes; // value is type
    struct PerViewIdData
    {
        int type;
        int viewId;
        int sourceViewId;
    };
    std::vector<PerViewIdData> m_updatedTypesPerViewId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
