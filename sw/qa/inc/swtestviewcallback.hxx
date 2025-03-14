/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_INC_SWTESTVIEWCALLBACK_HXX
#define INCLUDED_SW_QA_INC_SWTESTVIEWCALLBACK_HXX

#include <boost/property_tree/ptree.hpp>

#include <rtl/string.hxx>
#include <test/lokcallback.hxx>
#include <tools/gen.hxx>

#include "swqahelperdllapi.h"

class SfxViewShell;

/// A view callback tracks callbacks invoked on one specific view.
class SWQAHELPER_DLLPUBLIC SwTestViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;

public:
    bool m_bOwnCursorInvalidated;
    int m_nOwnCursorInvalidatedBy;
    bool m_bOwnCursorAtOrigin;
    tools::Rectangle m_aOwnCursor;
    bool m_bViewCursorInvalidated;
    tools::Rectangle m_aViewCursor;
    bool m_bOwnSelectionSet;
    bool m_bViewSelectionSet;
    OString m_aViewSelection;
    OString m_aViewRenderState;
    bool m_bTilesInvalidated;
    bool m_bViewCursorVisible;
    bool m_bGraphicViewSelection;
    bool m_bGraphicSelection;
    bool m_bViewLock;
    OString m_aDocColor;
    /// Set if any callback was invoked.
    bool m_bCalled;
    /// Redline table size changed payload
    boost::property_tree::ptree m_aRedlineTableChanged;
    /// Redline table modified payload
    boost::property_tree::ptree m_aRedlineTableModified;
    /// Post-it / annotation payload.
    boost::property_tree::ptree m_aComment;
    std::vector<OString> m_aStateChanges;
    TestLokCallbackWrapper m_callbackWrapper;
    OString m_aExportFile;
    bool m_bCursorVisible = false;

    SwTestViewCallback(SfxViewShell* pViewShell = nullptr,
                       std::function<void(SwTestViewCallback&)> const& rBeforeInstallFunc = {});

    ~SwTestViewCallback();

    static void callback(int nType, const char* pPayload, void* pData);

    void callbackImpl(int nType, const char* pPayload);
};

#endif // INCLUDED_SW_QA_INC_SWTESTVIEWCALLBACK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
