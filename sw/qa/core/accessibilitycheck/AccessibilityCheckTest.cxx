/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <wrtsh.hxx>
#include <AccessibilityCheck.hxx>

class AccessibilityCheckTest : public SwModelTestBase
{
public:
    SwDoc* createDoc(const char* pName = nullptr)
    {
        load("/sw/qa/core/accessibilitycheck/data/", pName);
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        return pTextDoc->GetDocShell()->GetDoc();
    }
};

CPPUNIT_TEST_FIXTURE(AccessibilityCheckTest, testCheck)
{
    SwDoc* pDoc = createDoc("DocumentTest.odt");
    CPPUNIT_ASSERT(pDoc);
    sw::AccessibilityCheck aCheck(pDoc);
    aCheck.check();
    auto& aIssues = aCheck.getIssueCollecton().getIssues();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aIssues.size());
    CPPUNIT_ASSERT_EQUAL(svx::AccessibilityIssueID::DOCUMENT_LANGUAGE, aIssues[0]->m_eIssueID);
    CPPUNIT_ASSERT_EQUAL(svx::AccessibilityIssueID::DOCUMENT_TITLE, aIssues[1]->m_eIssueID);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
