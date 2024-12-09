/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <unotxdoc.hxx>
#include <itabenum.hxx>
#include <wrtsh.hxx>
#include <com/sun/star/text/XTextTable.hpp>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <swmodule.hxx>

namespace
{
/**
 * Covers sw/source/filter/ascii/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/filter/ascii/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf144576_ascii)
{
    // Given a document with a 2x2 table
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/2);

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTableNames = xTablesSupplier->getTextTables();
    CPPUNIT_ASSERT(xTableNames->hasByName(u"Table1"_ustr));
    uno::Reference<text::XTextTable> xTable1(xTableNames->getByName(u"Table1"_ustr),
                                             uno::UNO_QUERY);

    pWrtShell->GotoTable(UIName(u"Table1"_ustr));
    pWrtShell->Insert(u"A"_ustr);
    pWrtShell->GoNextCell(false);
    pWrtShell->Insert(u"B"_ustr);
    pWrtShell->GoNextCell(false);
    pWrtShell->Insert(u"C"_ustr);
    pWrtShell->GoNextCell(false);
    pWrtShell->Insert(u"D"_ustr);

    // Select the whole table
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    rtl::Reference<SwTransferable> xTransferable(new SwTransferable(*pWrtShell));
    xTransferable->Copy(); // Ctl-C
    xTransferable.get();

    // Get the plain text version of the selection
    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = "text/plain;charset=utf-16";
    aFlavor.DataType = cppu::UnoType<OUString>::get();
    uno::Any aData = xTransferable->getTransferData(aFlavor);
    OUString aActual;
    aData >>= aActual;
    pWrtShell->ClearMark();

    CPPUNIT_ASSERT(aData.hasValue());

    OUString aExpected = u"A\tB"_ustr SAL_NEWLINE_STRING u"C\tD"_ustr SAL_NEWLINE_STRING u""_ustr;

    // Without the fix in place, the test fails with:
    // - Expected: A    B
    //             C    D
    //
    // - Actual  : A
    //             B
    //             C
    //             D
    // i.e. Each cell is separated by a tab
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    // Add some newlines in the first two cells
    // and test to see if they're handled correctly
    uno::Reference<text::XTextRange> xCellA1(xTable1->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    xCellA1->setString(u""_ustr);
    uno::Reference<text::XTextRange> xCellB1(xTable1->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    xCellB1->setString(u""_ustr);

    pWrtShell->GotoTable(UIName(u"Table1"_ustr));
    pWrtShell->Insert(u"A"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"A1"_ustr);
    pWrtShell->GoNextCell(false);
    pWrtShell->Insert(u"B"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"B1"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"B2"_ustr);

    aExpected
        = u"A"_ustr SAL_NEWLINE_STRING u"A1\tB"_ustr SAL_NEWLINE_STRING u"B1"_ustr SAL_NEWLINE_STRING u"B2"_ustr SAL_NEWLINE_STRING u"C\tD"_ustr SAL_NEWLINE_STRING u""_ustr;

    // Select the whole table
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Get the plain text version of the selection
    rtl::Reference<SwTransferable> xNewTransferable(new SwTransferable(*pWrtShell));
    xNewTransferable->Copy(); // Ctl-C
    xNewTransferable.get();
    aData = xNewTransferable->getTransferData(aFlavor);
    CPPUNIT_ASSERT(aData.hasValue());
    aData >>= aActual;

    // Without the fix in place, the test fails with:
    // - Expected: A
    //             A1   B
    //             B1
    //             B2
    //             C    D
    //
    // - Actual  : A
    //             A1
    //             B
    //             B1
    //             B2
    //             C
    //             D
    // i.e. Each cell is separated by a tab, a newline inside
    // a cell gets written to the next line in the furthest
    // left spot available
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf162180)
{
    createSwDoc("tdf162180.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Select the whole table
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, this test would have crashed here
    rtl::Reference<SwTransferable> xTransferable(new SwTransferable(*pWrtShell));
    xTransferable->Copy(); // Ctl-C
    xTransferable.get();

    // Get the plain text version of the selection
    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = "text/plain;charset=utf-16";
    aFlavor.DataType = cppu::UnoType<OUString>::get();
    uno::Any aData = xTransferable->getTransferData(aFlavor);
    CPPUNIT_ASSERT(aData.hasValue());

    OUString aActual;
    aData >>= aActual;

#if !defined(_WIN32) //FIXME
    OUString aExpected
        = u"2010\t2011"_ustr SAL_NEWLINE_STRING u"All Projects"_ustr SAL_NEWLINE_STRING u"Pending\t"_ustr SAL_NEWLINE_STRING u"USA\tWest\tApproved"_ustr SAL_NEWLINE_STRING u"\tCentral\tPending"_ustr SAL_NEWLINE_STRING u"\tEast\tApproved"_ustr;
#else
    OUString aExpected
        = u"2010\t2011"_ustr SAL_NEWLINE_STRING u"All Projects\nPending\t"_ustr SAL_NEWLINE_STRING u"USA\tWest\tApproved"_ustr SAL_NEWLINE_STRING u"\tCentral\tPending"_ustr SAL_NEWLINE_STRING u"\tEast\tApproved"_ustr;
#endif

    CPPUNIT_ASSERT_EQUAL(aExpected, aActual.trim());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
