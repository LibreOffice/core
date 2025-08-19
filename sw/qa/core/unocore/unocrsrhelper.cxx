/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <docsh.hxx>
#include <fchrfmt.hxx>
#include <redline.hxx>
#include <wrtsh.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sw/source/core/unocore/unocrsrhelper.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/unocore/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFormatCharStyleChangeDocxImport)
{
    // Given a document with a format redline, containing a char style change (strong -> quote):
    // When importing that document:
    createSwDoc("format-char-style-change.docx");

    // Then make sure the model has the new style name, the redline has the old style name:
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    SfxItemSetFixed<RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT> aSet(pDoc->GetAttrPool());
    pWrtShell->GetCurAttr(aSet);
    const SwFormatCharFormat& rNewCharFormat = aSet.Get(RES_TXTATR_CHARFMT);
    CPPUNIT_ASSERT_EQUAL(u"Quote Char"_ustr, rNewCharFormat.GetCharFormat()->GetName());
    const IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    const SwRedlineTable& rRedlineTable = rIDRA.GetRedlineTable();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the import result had no redlines.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlineTable.size());
    const SwRangeRedline* pRedline = rRedlineTable[0];
    auto pExtraData = dynamic_cast<const SwRedlineExtraData_FormatColl*>(pRedline->GetExtraData());
    CPPUNIT_ASSERT(pExtraData);
    std::shared_ptr<SfxItemSet> pRedlineSet = pExtraData->GetItemSet();
    CPPUNIT_ASSERT(pRedlineSet);
    const SwFormatCharFormat& rOldCharFormat = pRedlineSet->Get(RES_TXTATR_CHARFMT);
    CPPUNIT_ASSERT_EQUAL(u"Strong Emphasis"_ustr, rOldCharFormat.GetCharFormat()->GetName());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
