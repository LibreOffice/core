/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):  Michael Meeks <michael.meeks@novell.com>
 *                  Kohei Yoshida <kyoshida@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// TODO ...
//    officecfg: can we move this into our skeleton ?
//          Solve the Setup.xcu problem pleasantly [ custom version ? ]
//    deliver.pl
//          don't call regcomp if we don't have it.
//              In an ideal world
//              a) scp2 goes away and logic moved into the deliver d.lst
//              b) install set gets built incrementally as the build progresses
//              c) the new .xml component registration stuff then removes
//                 the need for manually calling regcomp and knowing what
//                 services we need, and in what .so they are implemented

#include "precompiled_sc.hxx"

#ifdef WNT
# include <prewin.h>
# include <postwin.h>
# undef ERROR
#endif

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/config.h>
#include <osl/file.hxx>
#include <osl/process.h>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/oslfile2streamwrap.hxx>

#include <vcl/svapp.hxx>
#include "scdll.hxx"
#include "document.hxx"
#include "stringutil.hxx"
#include "scmatrix.hxx"
#include "drwlayer.hxx"

#include "docsh.hxx"
#include "funcdesc.hxx"

#include "dpshttab.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"

#include "formula/IFunctionDescription.hxx"

#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>

#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>

#include <ucbhelper/contentbroker.hxx>

#include <iostream>
#include <vector>

#define MDDS_HASH_CONTAINER_BOOST 1
#include <mdds/mixed_type_matrix.hpp>

#define UCALC_DEBUG_OUTPUT 0

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::cout;
using ::std::cerr;
using ::std::endl;
using ::std::vector;

namespace {

::std::ostream& operator<< (::std::ostream& os, const OUString& str)
{
    return os << ::rtl::OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}

/**
 * Print nicely formatted sheet content to stdout.  Indispensable when
 * debugging the unit test code involving testing of sheet contents.
 */
class SheetPrinter
{
    typedef ::mdds::mixed_type_matrix<OUString, bool> MatrixType;
public:
    SheetPrinter(size_t rows, size_t cols) :
        maMatrix(rows, cols, ::mdds::matrix_density_sparse_empty) {}

    void set(size_t row, size_t col, const OUString& aStr)
    {
        maMatrix.set_string(row, col, new OUString(aStr));
    }

#if UCALC_DEBUG_OUTPUT
    void print(const char* header) const
    {
        if (header)
            cout << header << endl;

        MatrixType::size_pair_type ns = maMatrix.size();
        vector<sal_Int32> aColWidths(ns.second, 0);

        // Calculate column widths first.
        for (size_t row = 0; row < ns.first; ++row)
        {
            for (size_t col = 0; col < ns.second; ++col)
            {
                const OUString* p = maMatrix.get_string(row, col);
                if (aColWidths[col] < p->getLength())
                    aColWidths[col] = p->getLength();
            }
        }

        // Make the row separator string.
        OUStringBuffer aBuf;
        aBuf.appendAscii("+");
        for (size_t col = 0; col < ns.second; ++col)
        {
            aBuf.appendAscii("-");
            for (sal_Int32 i = 0; i < aColWidths[col]; ++i)
                aBuf.append(sal_Unicode('-'));
            aBuf.appendAscii("-+");
        }

        OUString aSep = aBuf.makeStringAndClear();

        // Now print to stdout.
        cout << aSep << endl;
        for (size_t row = 0; row < ns.first; ++row)
        {
            cout << "| ";
            for (size_t col = 0; col < ns.second; ++col)
            {
                const OUString* p = maMatrix.get_string(row, col);
                size_t nPadding = aColWidths[col] - p->getLength();
                aBuf.append(*p);
                for (size_t i = 0; i < nPadding; ++i)
                    aBuf.append(sal_Unicode(' '));
                cout << aBuf.makeStringAndClear() << " | ";
            }
            cout << endl;
            cout << aSep << endl;
        }
    }
#else
    void print(const char*) const {}
#endif

    /**
     * Print nested string array which can be copy-n-pasted into the test code
     * for content verification.
     */
    void printArray() const
    {
#if UCALC_DEBUG_OUTPUT
        MatrixType::size_pair_type ns = maMatrix.size();
        for (size_t row = 0; row < ns.first; ++row)
        {
            cout << "    { ";
            for (size_t col = 0; col < ns.second; ++col)
            {
                const OUString* p = maMatrix.get_string(row, col);
                if (p->getLength())
                    cout << "\"" << *p << "\"";
                else
                    cout << "0";
                if (col < ns.second - 1)
                    cout << ", ";
            }
            cout << " }";
            if (row < ns.first - 1)
                cout << ",";
            cout << endl;
        }
#endif
    }

    void clear() { maMatrix.clear(); }
    void resize(size_t rows, size_t cols) { maMatrix.resize(rows, cols); }

private:
    MatrixType maMatrix;
};

class Test : public CppUnit::TestFixture {
public:
    Test();
    ~Test();

    virtual void setUp();
    virtual void tearDown();

    bool testLoad(const rtl::OUString &rFilter, const rtl::OUString &rURL);

    void testCollator();
    void testInput();
    void testSUM();
    void testVolatileFunc();
    void testNamedRange();
    void testCSV();
    void testMatrix();
    void testDataPilot();
    void testSheetCopy();

    /**
     * Make sure the sheet streams are invalidated properly.
     */
    void testStreamValid();

    /**
     * Test built-in cell functions to make sure their categories and order
     * are correct.
     */
    void testFunctionLists();

    void testGraphicsInGroup();

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCollator);
    CPPUNIT_TEST(testInput);
    CPPUNIT_TEST(testSUM);
    CPPUNIT_TEST(testVolatileFunc);
    CPPUNIT_TEST(testNamedRange);
    CPPUNIT_TEST(testCSV);
    CPPUNIT_TEST(testMatrix);
    CPPUNIT_TEST(testDataPilot);
    CPPUNIT_TEST(testSheetCopy);
    CPPUNIT_TEST(testGraphicsInGroup);
    CPPUNIT_TEST(testStreamValid);
    CPPUNIT_TEST(testFunctionLists);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_xContext;
    ScDocument *m_pDoc;
    ScDocShellRef m_xDocShRef;
    ::rtl::OUString m_aPWDURL;
};

Test::Test()
    : m_pDoc(0)
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    uno::Reference<lang::XMultiServiceFactory> xSM(xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);

    // initialise UCB-Broker
    uno::Sequence<uno::Any> aUcbInitSequence(2);
    aUcbInitSequence[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
    aUcbInitSequence[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
    bool bInitUcb = ucbhelper::ContentBroker::initialize(xSM, aUcbInitSequence);
    CPPUNIT_ASSERT_MESSAGE("Should be able to initialize UCB", bInitUcb);

    uno::Reference<ucb::XContentProviderManager> xUcb =
        ucbhelper::ContentBroker::get()->getContentProviderManagerInterface();
    uno::Reference<ucb::XContentProvider> xFileProvider(xSM->createInstance(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.FileContentProvider"))), uno::UNO_QUERY);
    xUcb->registerContentProvider(xFileProvider, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file")), sal_True);

    InitVCL(xSM);

    ScDLL::Init();

    oslProcessError err = osl_getProcessWorkingDir(&m_aPWDURL.pData);
    CPPUNIT_ASSERT_MESSAGE("no PWD!", err == osl_Process_E_None);
}

void Test::setUp()
{
    m_xDocShRef = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);

    m_pDoc = m_xDocShRef->GetDocument();
}

void Test::tearDown()
{
    m_xDocShRef.Clear();
}

Test::~Test()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}

void Test::testCollator()
{
    OUString s1(RTL_CONSTASCII_USTRINGPARAM("A"));
    OUString s2(RTL_CONSTASCII_USTRINGPARAM("B"));
    CollatorWrapper* p = ScGlobal::GetCollator();
    sal_Int32 nRes = p->compareString(s1, s2);
    CPPUNIT_ASSERT_MESSAGE("these strings are supposed to be different!", nRes != 0);
}

void Test::testInput()
{
    rtl::OUString aTabName(RTL_CONSTASCII_USTRINGPARAM("foo"));
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    OUString numstr(RTL_CONSTASCII_USTRINGPARAM("'10.5"));
    OUString str(RTL_CONSTASCII_USTRINGPARAM("'apple'"));
    OUString test;

    m_pDoc->SetString(0, 0, 0, numstr);
    m_pDoc->GetString(0, 0, 0, test);
    bool bTest = test.equalsAscii("10.5");
    CPPUNIT_ASSERT_MESSAGE("String number should have the first apostrophe stripped.", bTest);
    m_pDoc->SetString(0, 0, 0, str);
    m_pDoc->GetString(0, 0, 0, test);
    bTest = test.equalsAscii("'apple'");
    CPPUNIT_ASSERT_MESSAGE("Text content should have retained the first apostrophe.", bTest);

    m_pDoc->DeleteTab(0);
}

void Test::testSUM()
{
    rtl::OUString aTabName(RTL_CONSTASCII_USTRINGPARAM("foo"));
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));
    double val = 1;
    m_pDoc->SetValue (0, 0, 0, val);
    m_pDoc->SetValue (0, 1, 0, val);
    m_pDoc->SetString (0, 2, 0, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("=SUM(A1:A2)")));
    m_pDoc->CalcAll();
    double result;
    m_pDoc->GetValue (0, 2, 0, result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 2.0);

    m_pDoc->DeleteTab(0);
}

void Test::testVolatileFunc()
{
    rtl::OUString aTabName(RTL_CONSTASCII_USTRINGPARAM("foo"));
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    double val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->SetString(0, 1, 0, OUString(RTL_CONSTASCII_USTRINGPARAM("=IF(A1>0;NOW();0")));
    double now1;
    m_pDoc->GetValue(0, 1, 0, now1);
    CPPUNIT_ASSERT_MESSAGE("Value of NOW() should be positive.", now1 > 0.0);

    val = 0;
    m_pDoc->SetValue(0, 0, 0, val);
    m_xDocShRef->DoRecalc(true);
    double zero;
    m_pDoc->GetValue(0, 1, 0, zero);
    CPPUNIT_ASSERT_MESSAGE("Result should equal the 3rd parameter of IF, which is zero.", zero == 0.0);

    val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_xDocShRef->DoRecalc(true);
    double now2;
    m_pDoc->GetValue(0, 1, 0, now2);
    CPPUNIT_ASSERT_MESSAGE("Result should be the value of NOW() again.", (now2 - now1) >= 0.0);

    m_pDoc->DeleteTab(0);
}

void Test::testNamedRange()
{
    rtl::OUString aTabName(RTL_CONSTASCII_USTRINGPARAM("Sheet1"));
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    m_pDoc->SetValue (0, 0, 0, 101);

    ScAddress aA1(0, 0, 0);
    ScRangeName* pNewRanges = new ScRangeName();
    ScRangeData* pNew = new ScRangeData(m_pDoc,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Divisor")),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$A$1:$A$1048576")), aA1, 0, formula::FormulaGrammar::GRAM_PODF_A1);
    bool bSuccess = pNewRanges->insert(pNew);
    CPPUNIT_ASSERT_MESSAGE ("insertion failed", bSuccess);

    m_pDoc->SetRangeName(pNewRanges);

    m_pDoc->SetString (1, 0, 0, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("=A1/Divisor")));

    m_pDoc->CalcAll();
    double result;
    m_pDoc->GetValue (1, 0, 0, result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 1.0);

    m_pDoc->DeleteTab(0);
}

void Test::testCSV()
{
    const int English = 0, European = 1;
    struct {
        const char *pStr; int eSep; bool bResult; double nValue;
    } aTests[] = {
        { "foo",       English,  false, 0.0 },
        { "1.0",       English,  true,  1.0 },
        { "1,0",       English,  false, 0.0 },
        { "1.0",       European, false, 0.0 },
        { "1.000",     European, true,  1000.0 },
        { "1,000",     European, true,  1.0 },
        { "1.000",     English,  true,  1.0 },
        { "1,000",     English,  true,  1000.0 },
        { " 1.0",      English,  true,  1.0 },
        { " 1.0  ",    English,  true,  1.0 },
        { "1.0 ",      European, false, 0.0 },
        { "1.000",     European, true,  1000.0 },
        { "1137.999",  English,  true,  1137.999 },
        { "1.000.00",  European, false, 0.0 }
    };
    for (sal_uInt32 i = 0; i < SAL_N_ELEMENTS(aTests); i++) {
        rtl::OUString aStr(aTests[i].pStr, strlen (aTests[i].pStr), RTL_TEXTENCODING_UTF8);
        double nValue = 0.0;
        bool bResult = ScStringUtil::parseSimpleNumber
                (aStr, aTests[i].eSep == English ? '.' : ',',
                 aTests[i].eSep == English ? ',' : '.',
                 nValue);
        CPPUNIT_ASSERT_MESSAGE ("CSV numeric detection failure", bResult == aTests[i].bResult);
        CPPUNIT_ASSERT_MESSAGE ("CSV numeric value failure", nValue == aTests[i].nValue);
    }
}

bool Test::testLoad(const rtl::OUString &rFilter, const rtl::OUString &rURL)
{
    SfxFilter aFilter(
        rFilter,
        rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
        rtl::OUString(), rtl::OUString() );

    ScDocShellRef xDocShRef = new ScDocShell;
    SfxMedium aSrcMed(rURL, STREAM_STD_READ, true);
    aSrcMed.SetFilter(&aFilter);
    return xDocShRef->DoLoad(&aSrcMed);
}

void Test::testCVEs()
{
    bool bResult;

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Quattro Pro 6.0")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2007-5745-1.wb2")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2007-5745 regression", bResult == true);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Quattro Pro 6.0")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2007-5745-2.wb2")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2007-5745 regression", bResult == true);

    bResult = testLoad(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Quattro Pro 6.0")),
        m_aPWDURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/CVE/CVE-2007-5747-1.wb2")));
    CPPUNIT_ASSERT_MESSAGE("CVE-2007-5747 regression", bResult == false);
}

template<typename Evaluator>
void checkMatrixElements(const ScMatrix& rMat)
{
    SCSIZE nC, nR;
    rMat.GetDimensions(nC, nR);
    Evaluator aEval;
    for (SCSIZE i = 0; i < nC; ++i)
    {
        for (SCSIZE j = 0; j < nR; ++j)
        {
            aEval(i, j, rMat.Get(i, j));
        }
    }
}

struct AllZeroMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of numeric type", rVal.nType == SC_MATVAL_VALUE);
        CPPUNIT_ASSERT_MESSAGE("element value must be zero", rVal.fVal == 0.0);
    }
};

struct PartiallyFilledZeroMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of numeric type", rVal.nType == SC_MATVAL_VALUE);
        if (1 <= nCol && nCol <= 2 && 2 <= nRow && nRow <= 8)
        {
            CPPUNIT_ASSERT_MESSAGE("element value must be 3.0", rVal.fVal == 3.0);
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("element value must be zero", rVal.fVal == 0.0);
        }
    }
};

struct AllEmptyMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of empty type", rVal.nType == SC_MATVAL_EMPTY);
        CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
    }
};

struct PartiallyFilledEmptyMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        if (nCol == 1 && nRow == 1)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of boolean type", rVal.nType == SC_MATVAL_BOOLEAN);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.fVal == 1.0);
        }
        else if (nCol == 4 && nRow == 5)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of value type", rVal.nType == SC_MATVAL_VALUE);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.fVal == -12.5);
        }
        else if (nCol == 8 && nRow == 2)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of value type", rVal.nType == SC_MATVAL_STRING);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.pS->EqualsAscii("Test"));
        }
        else if (nCol == 8 && nRow == 11)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of empty path type", rVal.nType == SC_MATVAL_EMPTYPATH);
            CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of empty type", rVal.nType == SC_MATVAL_EMPTY);
            CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
        }
    }
};

void Test::testMatrix()
{
    ScMatrixRef pMat;
    ScMatrix::DensityType eDT[2];

    // First, test the zero matrix types.
    eDT[0] = ScMatrix::FILLED_ZERO;
    eDT[1] = ScMatrix::SPARSE_ZERO;
    for (int i = 0; i < 2; ++i)
    {
        pMat = new ScMatrix(0, 0, eDT[i]);
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        CPPUNIT_ASSERT_MESSAGE("matrix is not empty", nC == 0 && nR == 0);
        pMat->Resize(4, 10);
        pMat->GetDimensions(nC, nR);
        CPPUNIT_ASSERT_MESSAGE("matrix size is not as expected", nC == 4 && nR == 10);
        CPPUNIT_ASSERT_MESSAGE("both 'and' and 'or' should evaluate to false",
                               !pMat->And() && !pMat->Or());

        // Resizing into a larger matrix should fill the void space with zeros.
        checkMatrixElements<AllZeroMatrix>(*pMat);

        pMat->FillDouble(3.0, 1, 2, 2, 8);
        checkMatrixElements<PartiallyFilledZeroMatrix>(*pMat);
        CPPUNIT_ASSERT_MESSAGE("matrix is expected to be numeric", pMat->IsNumeric());
        CPPUNIT_ASSERT_MESSAGE("partially non-zero matrix should evaluate false on 'and' and true on 'or",
                               !pMat->And() && pMat->Or());
        pMat->FillDouble(5.0, 0, 0, nC-1, nR-1);
        CPPUNIT_ASSERT_MESSAGE("fully non-zero matrix should evaluate true both on 'and' and 'or",
                               pMat->And() && pMat->Or());
    }

    // Now test the emtpy matrix types.
    eDT[0] = ScMatrix::FILLED_EMPTY;
    eDT[1] = ScMatrix::SPARSE_EMPTY;
    for (int i = 0; i < 2; ++i)
    {
        pMat = new ScMatrix(10, 20, eDT[i]);
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        CPPUNIT_ASSERT_MESSAGE("matrix size is not as expected", nC == 10 && nR == 20);
        checkMatrixElements<AllEmptyMatrix>(*pMat);

        pMat->PutBoolean(true, 1, 1);
        pMat->PutDouble(-12.5, 4, 5);
        rtl::OUString aStr(RTL_CONSTASCII_USTRINGPARAM("Test"));
        pMat->PutString(aStr, 8, 2);
        pMat->PutEmptyPath(8, 11);
        checkMatrixElements<PartiallyFilledEmptyMatrix>(*pMat);
    }
}

void Test::testDataPilot()
{
    m_pDoc->InsertTab(0, OUString(RTL_CONSTASCII_USTRINGPARAM("Data")));
    m_pDoc->InsertTab(1, OUString(RTL_CONSTASCII_USTRINGPARAM("Table")));

    // Dimension definition
    struct {
        const char* pName; sheet::DataPilotFieldOrientation eOrient;
    } aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW },
        { "Group", sheet::DataPilotFieldOrientation_COLUMN },
        { "Score", sheet::DataPilotFieldOrientation_DATA }
    };

    // Raw data
    struct {
        const char* pName; const char* pGroup; int nScore;
    } aData[] = {
        { "Andy",    "A", 30 },
        { "Bruce",   "A", 20 },
        { "Charlie", "B", 45 },
        { "David",   "B", 12 },
        { "Edward",  "C",  8 },
        { "Frank",   "C", 15 },
    };

    // Expected output table content.  0 = empty cell
    const char* aOutputCheck[][5] = {
        { "Sum - Score", "Group", 0, 0, 0 },
        { "Name", "A", "B", "C", "Total Result" },
        { "Andy", "30", 0, 0, "30" },
        { "Bruce", "20", 0, 0, "20" },
        { "Charlie", 0, "45", 0, "45" },
        { "David", 0, "12", 0, "12" },
        { "Edward", 0, 0, "8", "8" },
        { "Frank", 0, 0, "15", "15" },
        { "Total Result", "50", "57", "23", "130" }
    };

    sal_uInt32 nFieldCount = SAL_N_ELEMENTS(aFields);
    sal_uInt32 nDataCount = SAL_N_ELEMENTS(aData);

    // Insert field names in row 0.
    for (sal_uInt32 i = 0; i < nFieldCount; ++i)
        m_pDoc->SetString(static_cast<SCCOL>(i), 0, 0, OUString(aFields[i].pName, strlen(aFields[i].pName), RTL_TEXTENCODING_UTF8));

    // Insert data into row 1 and downward.
    for (sal_uInt32 i = 0; i < nDataCount; ++i)
    {
        SCROW nRow = static_cast<SCROW>(i) + 1;
        m_pDoc->SetString(0, nRow, 0, OUString(aData[i].pName, strlen(aData[i].pName), RTL_TEXTENCODING_UTF8));
        m_pDoc->SetString(1, nRow, 0, OUString(aData[i].pGroup, strlen(aData[i].pGroup), RTL_TEXTENCODING_UTF8));
        m_pDoc->SetValue(2, nRow, 0, aData[i].nScore);
    }

    SCROW nRow1 = 0, nRow2 = 0;
    SCCOL nCol1 = 0, nCol2 = 0;
    m_pDoc->GetDataArea(0, nCol1, nRow1, nCol2, nRow2, true, false);
    CPPUNIT_ASSERT_MESSAGE("Data is expected to start from (col=0,row=0).", nCol1 == 0 && nRow1 == 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected data range.",
                           nCol2 == static_cast<SCCOL>(nFieldCount - 1) && nRow2 == static_cast<SCROW>(nDataCount));

    SheetPrinter printer(nRow2 - nRow1 + 1, nCol2 - nCol1 + 1);
    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            String aVal;
            m_pDoc->GetString(nCol, nRow, 0, aVal);
            printer.set(nRow, nCol, aVal);
        }
    }
    printer.print("Data sheet content");
    printer.clear();

    ScSheetSourceDesc aSheetDesc(m_pDoc);
    aSheetDesc.SetSourceRange(ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0));
    ScDPObject* pDPObj = new ScDPObject(m_pDoc);
    pDPObj->SetSheetDesc(aSheetDesc);
    pDPObj->SetOutRange(ScAddress(0, 0, 1));
    ScPivotParam aParam;
    pDPObj->FillOldParam(aParam, false);
    for (sal_uInt32 i = 0; i < nFieldCount; ++i)
    {
        vector<ScDPLabelData::Member> aMembers;
        pDPObj->GetMembers(i, 0, aMembers);
    }

    ScDPSaveData aSaveData;
    // Set data pilot table output options.
    aSaveData.SetIgnoreEmptyRows(false);
    aSaveData.SetRepeatIfEmpty(false);
    aSaveData.SetColumnGrand(true);
    aSaveData.SetRowGrand(true);
    aSaveData.SetFilterButton(false);
    aSaveData.SetDrillDown(true);

    // Check the sanity of the source range.
    const ScRange& rSrcRange = aSheetDesc.GetSourceRange();
    nCol1 = rSrcRange.aStart.Col();
    nRow1 = rSrcRange.aStart.Row();
    nRow2 = rSrcRange.aEnd.Row();
    CPPUNIT_ASSERT_MESSAGE("source range contains no data!", nRow2 - nRow1 > 1);

    // Set the dimension information.
    for (sal_uInt32 i = 0; i < nFieldCount; ++i)
    {
        OUString aDimName(aFields[i].pName, strlen(aFields[i].pName), RTL_TEXTENCODING_UTF8);
        ScDPSaveDimension* pDim = aSaveData.GetDimensionByName(aDimName);
        pDim->SetOrientation(aFields[i].eOrient);
        pDim->SetUsedHierarchy(0);
        pDim->SetShowEmpty(true);

        if (aFields[i].eOrient == sheet::DataPilotFieldOrientation_DATA)
        {
            pDim->SetFunction(sheet::GeneralFunction_SUM);
            pDim->SetReferenceValue(NULL);
        }
        else
        {
            sheet::DataPilotFieldSortInfo aSortInfo;
            aSortInfo.IsAscending = true;
            aSortInfo.Mode = 2;
            pDim->SetSortInfo(&aSortInfo);

            sheet::DataPilotFieldLayoutInfo aLayInfo;
            aLayInfo.LayoutMode = 0;
            aLayInfo.AddEmptyLines = false;
            pDim->SetLayoutInfo(&aLayInfo);
            sheet::DataPilotFieldAutoShowInfo aShowInfo;
            aShowInfo.IsEnabled = false;
            aShowInfo.ShowItemsMode = 0;
            aShowInfo.ItemCount = 0;
            pDim->SetAutoShowInfo(&aShowInfo);

//          USHORT nFuncs[] = { sheet::GeneralFunction_AUTO };
//          pDim->SetSubTotals(1, nFuncs);
        }

        for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
        {
            SCCOL nCol = nCol1 + static_cast<SCCOL>(i);
            String aVal;
            m_pDoc->GetString(nCol, nRow, 0, aVal);
            // This call is just to populate the member list for each dimension.
            ScDPSaveMember* pMem = pDim->GetMemberByName(aVal);
            pMem->SetShowDetails(true);
            pMem->SetIsVisible(true);
        }
    }

    // Don't forget the data layout dimension.
    ScDPSaveDimension* pDim = aSaveData.GetDataLayoutDimension();
    pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
    pDim->SetShowEmpty(true);

    pDPObj->SetSaveData(aSaveData);
    pDPObj->SetAlive(true);
    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->InvalidateData();
    pDPObj->SetName(pDPs->CreateNewName());

    bool bOverFlow = false;
    ScRange aOutRange = pDPObj->GetNewOutputRange(bOverFlow);
    CPPUNIT_ASSERT_MESSAGE("Table overflow!?", !bOverFlow);

    pDPObj->Output(aOutRange.aStart);
    aOutRange = pDPObj->GetOutRange();
    const ScAddress& s = aOutRange.aStart;
    const ScAddress& e = aOutRange.aEnd;
    printer.resize(e.Row() - s.Row() + 1, e.Col() - s.Col() + 1);
    SCROW nOutRowSize = SAL_N_ELEMENTS(aOutputCheck);
    SCCOL nOutColSize = SAL_N_ELEMENTS(aOutputCheck[0]);
    CPPUNIT_ASSERT_MESSAGE("Row size of the table output is not as expected.",
                           nOutRowSize == (e.Row()-s.Row()+1));
    CPPUNIT_ASSERT_MESSAGE("Column size of the table output is not as expected.",
                           nOutColSize == (e.Col()-s.Col()+1));
    for (SCROW nRow = 0; nRow < nOutRowSize; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < nOutColSize; ++nCol)
        {
            String aVal;
            m_pDoc->GetString(nCol + s.Col(), nRow + s.Row(), s.Tab(), aVal);
            printer.set(nRow, nCol, aVal);
            const char* p = aOutputCheck[nRow][nCol];
            if (p)
            {
                OUString aCheckVal = OUString::createFromAscii(p);
                bool bEqual = aCheckVal.equals(aVal);
                if (!bEqual)
                {
                    cerr << "Expected: " << aCheckVal << "  Actual: " << aVal << endl;
                    CPPUNIT_ASSERT_MESSAGE("Unexpected cell content.", false);
                }
            }
            else
                CPPUNIT_ASSERT_MESSAGE("Empty cell expected.", aVal.Len() == 0);
        }
    }
    printer.print("DataPilot table output");
    printer.clear();

    // Now, delete the datapilot object.
    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any data pilot table stored with the document.",
                           pDPs->GetCount() == 0);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSheetCopy()
{
    OUString aTabName(RTL_CONSTASCII_USTRINGPARAM("TestTab"));
    m_pDoc->InsertTab(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount() == 1);
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Copy and test the result.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount() == 2);
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);
    m_pDoc->DeleteTab(1);

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);

    // Copy the sheet once again.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount() == 2);
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);
    m_pDoc->DeleteTab(0);
}

void Test::testStreamValid()
{
    m_pDoc->InsertTab(0, OUString(RTL_CONSTASCII_USTRINGPARAM("Sheet1")));
    m_pDoc->InsertTab(1, OUString(RTL_CONSTASCII_USTRINGPARAM("Sheet2")));
    m_pDoc->InsertTab(2, OUString(RTL_CONSTASCII_USTRINGPARAM("Sheet3")));
    m_pDoc->InsertTab(3, OUString(RTL_CONSTASCII_USTRINGPARAM("Sheet4")));
    CPPUNIT_ASSERT_MESSAGE("We should have 4 sheet instances.", m_pDoc->GetTableCount() == 4);

    OUString a1(RTL_CONSTASCII_USTRINGPARAM("A1"));
    OUString a2(RTL_CONSTASCII_USTRINGPARAM("A2"));
    OUString test;

    // Put values into Sheet1.
    m_pDoc->SetString(0, 0, 0, a1);
    m_pDoc->SetString(0, 1, 0, a2);
    m_pDoc->GetString(0, 0, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A1", test.equals(a1));
    m_pDoc->GetString(0, 1, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A2", test.equals(a2));

    // Put formulas into Sheet2 to Sheet4 to references values from Sheet1.

    m_pDoc->SetString(0, 0, 1, OUString(RTL_CONSTASCII_USTRINGPARAM("=Sheet1.A1")));
    m_pDoc->SetString(0, 1, 1, OUString(RTL_CONSTASCII_USTRINGPARAM("=Sheet1.A2")));
    m_pDoc->SetString(0, 0, 2, OUString(RTL_CONSTASCII_USTRINGPARAM("=Sheet1.A1")));
    m_pDoc->SetString(0, 0, 3, OUString(RTL_CONSTASCII_USTRINGPARAM("=Sheet1.A2")));

    m_pDoc->GetString(0, 0, 1, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet2.A1", test.equals(a1));
    m_pDoc->GetString(0, 1, 1, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet2.A2", test.equals(a2));
    m_pDoc->GetString(0, 0, 2, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet3.A1", test.equals(a1));
    m_pDoc->GetString(0, 0, 3, test);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet3.A1", test.equals(a2));

    // Set all sheet streams valid after all the initial cell values are in
    // place. In reality we need to have real XML streams stored in order to
    // claim they are valid, but we are just testing the flag values here.
    m_pDoc->SetStreamValid(0, true);
    m_pDoc->SetStreamValid(1, true);
    m_pDoc->SetStreamValid(2, true);
    m_pDoc->SetStreamValid(3, true);
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(0));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(1));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(2));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(3));

    // Now, insert a new row at row 2 position on Sheet1.  This will move cell
    // A2 downward but cell A1 remains unmoved.
    m_pDoc->InsertRow(0, 0, MAXCOL, 0, 1, 2);
    m_pDoc->GetString(0, 0, 0, test);
    CPPUNIT_ASSERT_MESSAGE("Cell A1 should not have moved.", test.equals(a1));
    m_pDoc->GetString(0, 3, 0, test);
    CPPUNIT_ASSERT_MESSAGE("the old cell A2 should now be at A4.", test.equals(a2));
    const ScBaseCell* pCell = m_pDoc->GetCell(ScAddress(0, 1, 0));
    CPPUNIT_ASSERT_MESSAGE("Cell A2 should be empty.", pCell == NULL);
    pCell = m_pDoc->GetCell(ScAddress(0, 2, 0));
    CPPUNIT_ASSERT_MESSAGE("Cell A3 should be empty.", pCell == NULL);

#if 0 // This currently fails.
    // After the move, Sheet1, Sheet2, and Sheet4 should have their stream
    // invalidated, whereas Sheet3's stream should still be valid.
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(0));
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(1));
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(3));
    CPPUNIT_ASSERT_MESSAGE("Stream should still be valid.", m_pDoc->IsStreamValid(2));
#endif

    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFunctionLists()
{
    const char* aDataBase[] = {
        "DAVERAGE",
        "DCOUNT",
        "DCOUNTA",
        "DGET",
        "DMAX",
        "DMIN",
        "DPRODUCT",
        "DSTDEV",
        "DSTDEVP",
        "DSUM",
        "DVAR",
        "DVARP",
        0
    };

    const char* aDateTime[] = {
        "DATE",
        "DATEVALUE",
        "DAY",
        "DAYS",
        "DAYS360",
        "EASTERSUNDAY",
        "HOUR",
        "MINUTE",
        "MONTH",
        "NOW",
        "SECOND",
        "TIME",
        "TIMEVALUE",
        "TODAY",
        "WEEKDAY",
        "WEEKNUM",
        "YEAR",
        0
    };

    const char* aFinancial[] = {
        "CUMIPMT",
        "CUMPRINC",
        "DB",
        "DDB",
        "DURATION",
        "EFFECTIVE",
        "FV",
        "IPMT",
        "IRR",
        "ISPMT",
        "MIRR",
        "NOMINAL",
        "NPER",
        "NPV",
        "PMT",
        "PPMT",
        "PV",
        "RATE",
        "RRI",
        "SLN",
        "SYD",
        "VDB",
        0
    };

    const char* aInformation[] = {
        "CELL",
        "CURRENT",
        "FORMULA",
        "INFO",
        "ISBLANK",
        "ISERR",
        "ISERROR",
        "ISFORMULA",
        "ISLOGICAL",
        "ISNA",
        "ISNONTEXT",
        "ISNUMBER",
        "ISREF",
        "ISTEXT",
        "N",
        "NA",
        "TYPE",
        0
    };

    const char* aLogical[] = {
        "AND",
        "FALSE",
        "IF",
        "NOT",
        "OR",
        "TRUE",
        0
    };

    const char* aMathematical[] = {
        "ABS",
        "ACOS",
        "ACOSH",
        "ACOT",
        "ACOTH",
        "ASIN",
        "ASINH",
        "ATAN",
        "ATAN2",
        "ATANH",
        "CEILING",
        "COMBIN",
        "COMBINA",
        "CONVERT",
        "COS",
        "COSH",
        "COT",
        "COTH",
        "COUNTBLANK",
        "COUNTIF",
        "DEGREES",
        "EUROCONVERT",
        "EVEN",
        "EXP",
        "FACT",
        "FLOOR",
        "GCD",
        "INT",
        "ISEVEN",
        "ISODD",
        "LCM",
        "LN",
        "LOG",
        "LOG10",
        "MOD",
        "ODD",
        "PI",
        "POWER",
        "PRODUCT",
        "RADIANS",
        "RAND",
        "ROUND",
        "ROUNDDOWN",
        "ROUNDUP",
        "SIGN",
        "SIN",
        "SINH",
        "SQRT",
        "SUBTOTAL",
        "SUM",
        "SUMIF",
        "SUMSQ",
        "TAN",
        "TANH",
        "TRUNC",
        0
    };

    const char* aArray[] = {
        "FREQUENCY",
        "GROWTH",
        "LINEST",
        "LOGEST",
        "MDETERM",
        "MINVERSE",
        "MMULT",
        "MUNIT",
        "SUMPRODUCT",
        "SUMX2MY2",
        "SUMX2PY2",
        "SUMXMY2",
        "TRANSPOSE",
        "TREND",
        0
    };

    const char* aStatistical[] = {
        "AVEDEV",
        "AVERAGE",
        "AVERAGEA",
        "B",
        "BETADIST",
        "BETAINV",
        "BINOMDIST",
        "CHIDIST",
        "CHIINV",
        "CHISQDIST",
        "CHISQINV",
        "CHITEST",
        "CONFIDENCE",
        "CORREL",
        "COUNT",
        "COUNTA",
        "COVAR",
        "CRITBINOM",
        "DEVSQ",
        "EXPONDIST",
        "FDIST",
        "FINV",
        "FISHER",
        "FISHERINV",
        "FORECAST",
        "FTEST",
        "GAMMA",
        "GAMMADIST",
        "GAMMAINV",
        "GAMMALN",
        "GAUSS",
        "GEOMEAN",
        "HARMEAN",
        "HYPGEOMDIST",
        "INTERCEPT",
        "KURT",
        "LARGE",
        "LOGINV",
        "LOGNORMDIST",
        "MAX",
        "MAXA",
        "MEDIAN",
        "MIN",
        "MINA",
        "MODE",
        "NEGBINOMDIST",
        "NORMDIST",
        "NORMINV",
        "NORMSDIST",
        "NORMSINV",
        "PEARSON",
        "PERCENTILE",
        "PERCENTRANK",
        "PERMUT",
        "PERMUTATIONA",
        "PHI",
        "POISSON",
        "PROB",
        "QUARTILE",
        "RANK",
        "RSQ",
        "SKEW",
        "SLOPE",
        "SMALL",
        "STANDARDIZE",
        "STDEV",
        "STDEVA",
        "STDEVP",
        "STDEVPA",
        "STEYX",
        "TDIST",
        "TINV",
        "TRIMMEAN",
        "TTEST",
        "VAR",
        "VARA",
        "VARP",
        "VARPA",
        "WEIBULL",
        "ZTEST",
        0
    };

    const char* aSpreadsheet[] = {
        "ADDRESS",
        "AREAS",
        "CHOOSE",
        "COLUMN",
        "COLUMNS",
        "DDE",
        "ERRORTYPE",
        "GETPIVOTDATA",
        "HLOOKUP",
        "HYPERLINK",
        "INDEX",
        "INDIRECT",
        "LOOKUP",
        "MATCH",
        "OFFSET",
        "ROW",
        "ROWS",
        "SHEET",
        "SHEETS",
        "STYLE",
        "VLOOKUP",
        0
    };

    const char* aText[] = {
        "ARABIC",
        "ASC",
        "BAHTTEXT",
        "BASE",
        "CHAR",
        "CLEAN",
        "CODE",
        "CONCATENATE",
        "DECIMAL",
        "DOLLAR",
        "EXACT",
        "FIND",
        "FIXED",
        "JIS",
        "LEFT",
        "LEN",
        "LOWER",
        "MID",
        "PROPER",
        "REPLACE",
        "REPT",
        "RIGHT",
        "ROMAN",
        "SEARCH",
        "SUBSTITUTE",
        "T",
        "TEXT",
        "TRIM",
        "UNICHAR",
        "UNICODE",
        "UPPER",
        "VALUE",
        0
    };

    struct {
        const char* Category; const char** Functions;
    } aTests[] = {
        { "Database",     aDataBase },
        { "Date&Time",    aDateTime },
        { "Financial",    aFinancial },
        { "Information",  aInformation },
        { "Logical",      aLogical },
        { "Mathematical", aMathematical },
        { "Array",        aArray },
        { "Statistical",  aStatistical },
        { "Spreadsheet",  aSpreadsheet },
        { "Text",         aText },
        { "Add-in",       0 },
        { 0, 0 }
    };

    ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
    sal_uInt32 n = pFuncMgr->getCount();
    for (sal_uInt32 i = 0; i < n; ++i)
    {
        const formula::IFunctionCategory* pCat = pFuncMgr->getCategory(i);
        CPPUNIT_ASSERT_MESSAGE("Unexpected category name", pCat->getName().equalsAscii(aTests[i].Category));
        sal_uInt32 nFuncCount = pCat->getCount();
        for (sal_uInt32 j = 0; j < nFuncCount; ++j)
        {
            const formula::IFunctionDescription* pFunc = pCat->getFunction(j);
            CPPUNIT_ASSERT_MESSAGE("Unexpected function name", pFunc->getFunctionName().equalsAscii(aTests[i].Functions[j]));
        }
    }
}

void Test::testGraphicsInGroup()
{
    OUString aTabName(RTL_CONSTASCII_USTRINGPARAM("TestTab"));
    m_pDoc->InsertTab(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount() == 1);
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != NULL);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != NULL);

    //Add a square
    Rectangle aOrigRect(2,2,100,100);
    SdrRectObj *pObj = new SdrRectObj(aOrigRect);
    pPage->InsertObject(pObj);
    const Rectangle &rNewRect = pObj->GetLogicRect();
    CPPUNIT_ASSERT_MESSAGE("must have equal position and size", aOrigRect == rNewRect);

    ScDrawLayer::SetPageAnchored(*pObj);

    //Use a range of rows guaranteed to include all of the square
    m_pDoc->ShowRows(0, 100, 0, false);
    CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);
    m_pDoc->ShowRows(0, 100, 0, true);
    CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);

    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
    CPPUNIT_ASSERT_MESSAGE("That shouldn't change size or positioning", aOrigRect == rNewRect);

    m_pDoc->ShowRows(0, 100, 0, false);
    CPPUNIT_ASSERT_MESSAGE("Left and Right should be unchanged",
        aOrigRect.nLeft == rNewRect.nLeft && aOrigRect.nRight == rNewRect.nRight);
    CPPUNIT_ASSERT_MESSAGE("Height should be minimum allowed height",
        (rNewRect.nBottom - rNewRect.nTop) <= 1);
    m_pDoc->ShowRows(0, 100, 0, true);
    CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
