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
# include <tools/prewin.h>
# include <windows.h>
# include <tools/postwin.h>
#endif

#include "preextstl.h"
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "postextstl.h"

#include <sal/config.h>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/svapp.hxx>
#include <scdll.hxx>
#include <document.hxx>
#include <stringutil.hxx>
#include <scmatrix.hxx>

using namespace ::com::sun::star;

namespace {

class Test : public CppUnit::TestFixture {
public:
    Test();
    ~Test();

    virtual void setUp();
    virtual void tearDown();

    void testSUM();
    void testNamedRange();
    void testCSV();
    void testMatrix();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSUM);
    CPPUNIT_TEST(testNamedRange);
    CPPUNIT_TEST(testCSV);
#ifndef WNT
    CPPUNIT_TEST(testMatrix);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_xContext;
    ScDocument *m_pDoc;
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

    InitVCL(xSM);

    ScDLL::Init();
}

void Test::setUp()
{
    m_pDoc = new ScDocument;
}

void Test::tearDown()
{
    delete m_pDoc;
}

Test::~Test()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
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
    bool bSuccess = pNewRanges->Insert(pNew);
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
