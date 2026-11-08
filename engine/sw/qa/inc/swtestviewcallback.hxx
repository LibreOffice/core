/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_INC_SWTESTVIEWCALLBACK_HXX
#define INCLUDED_SW_QA_INC_SWTESTVIEWCALLBACK_HXX

#include <boost/property_tree/ptree.hpp>

#include <rtl/string.hxx>
#include <test/kitcallback.hxx>
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
    /// Whether the last own cursor payload marked a drop position.
    bool m_bOwnCursorAtDropPosition = false;
    /// Whether the last own cursor payload had no rectangle.
    bool m_bOwnCursorEmpty = false;
    /// The type and the text of the last tooltip, both empty while none arrived.
    std::string m_aTooltipType;
    std::string m_aTooltipText;
    tools::Rectangle m_aOwnCursor;
    bool m_bViewCursorInvalidated;
    /// The view named as having moved the other view's caret, or -1 when none was named.
    int m_nViewCursorMovedBy;
    tools::Rectangle m_aViewCursor;
    bool m_bOwnSelectionSet;
    /// The rectangles of the last text selection of this view, empty when nothing is selected.
    OString m_aOwnSelection;
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
    int m_nCommentCallbackCount = 0;
    /// Accessibility focused paragraph payload.
    boost::property_tree::ptree m_aA11yFocusedParagraph;
    /// Accessibility caret position in the focused paragraph, from the last focus or caret payload.
    int m_nA11yCaretPosition = -1;
    /// Accessibility focused table cell payload.
    boost::property_tree::ptree m_aA11yFocusedCell;
    /// Accessibility selected object payload.
    boost::property_tree::ptree m_aA11ySelection;
    int m_nA11ySelectionCallbackCount = 0;
    std::vector<OString> m_aStateChanges;
    /// The view ids named by each notice that recording was turned on for all views, in order.
    std::vector<int> m_aRecordingForAllViewsBy;
    TestKitCallbackWrapper m_callbackWrapper;
    OString m_aExportFile;
    /// The payload of the last error report.
    OString m_aErrorReport;
    bool m_bCursorVisible = false;

    SwTestViewCallback(SfxViewShell* pViewShell = nullptr,
                       std::function<void(SwTestViewCallback&)> const& rBeforeInstallFunc = {});

    ~SwTestViewCallback();

    static void callback(COKitCallbackType eType, const char* pPayload, void* pData);

    void callbackImpl(COKitCallbackType eType, const char* pPayload);
};

#endif // INCLUDED_SW_QA_INC_SWTESTVIEWCALLBACK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
