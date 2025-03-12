/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SD_QA_INC_SDTILEDRENDERINGTEST_HXX
#define INCLUDED_SD_QA_INC_SDTILEDRENDERINGTEST_HXX

#include <test/unoapixml_test.hxx>

#include <osl/conditn.hxx>
#include <test/lokcallback.hxx>

#include "sdqahelperdllapi.h"

class SdXImpressDocument;
class SfxViewShell;

class SDQAHELPER_DLLPUBLIC SdTiledRenderingTest : public UnoApiXmlTest
{
public:
    SdTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    SdXImpressDocument* createDoc(const char* pName,
                                  const css::uno::Sequence<css::beans::PropertyValue>& rArguments
                                  = css::uno::Sequence<css::beans::PropertyValue>());
    void setupLibreOfficeKitViewCallback(SfxViewShell& pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    xmlDocUniquePtr parseXmlDump();

    ::tools::Rectangle m_aInvalidation;
    std::vector<::tools::Rectangle> m_aSelection;
    bool m_bFound;
    sal_Int32 m_nPart;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;
    int m_nSearchResultCount;

    /// For document size changed callback.
    osl::Condition m_aDocumentSizeCondition;
    xmlBufferPtr m_pXmlBuffer;
    TestLokCallbackWrapper m_callbackWrapper;
};

#endif // INCLUDED_SD_QA_INC_SDTILEDRENDERINGTEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
