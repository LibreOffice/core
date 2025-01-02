/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_INC_SWTILEDRENDERINGTEST_HXX
#define INCLUDED_SW_QA_INC_SWTILEDRENDERINGTEST_HXX

#include "swmodeltestbase.hxx"

#include <test/lokcallback.hxx>

#include "swqahelperdllapi.h"

/// Testsuite for the SwXTextDocument methods implementing the vcl::ITiledRenderable interface.
class SWQAHELPER_DLLPUBLIC SwTiledRenderingTest : public SwModelTestBase
{
public:
    SwTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    SwXTextDocument* createDoc(const char* pName = nullptr);
    void setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    // First invalidation.
    tools::Rectangle m_aInvalidation;
    /// Union of all invalidations.
    tools::Rectangle m_aInvalidations;
    Size m_aDocumentSize;
    OString m_aTextSelection;
    bool m_bFound;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;
    int m_nInvalidations;
    int m_nRedlineTableSizeChanged;
    int m_nRedlineTableEntryModified;
    int m_nTrackedChangeIndex;
    bool m_bFullInvalidateSeen;
    OString m_sHyperlinkText;
    OString m_sHyperlinkLink;
    OString m_aFormFieldButton;
    OString m_aContentControl;
    OString m_ShapeSelection;
    struct
    {
        std::string text;
        std::string rect;
    } m_aTooltip;
    TestLokCallbackWrapper m_callbackWrapper;
};

#endif // INCLUDED_SW_QA_INC_SWTILEDRENDERINGTEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
