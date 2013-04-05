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
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifdef WNT
# undef ERROR
#endif

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/config.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <i18nlangtag/lang.h>

#include "svl/zforlist.hxx"
#include "svl/zformat.hxx"

#define DEBUG_UNIT_TEST 0

#if DEBUG_UNIT_TEST
#include <iostream>
#endif

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

#if DEBUG_UNIT_TEST
using ::std::cout;
using ::std::endl;
#endif

namespace {

#if DEBUG_UNIT_TEST
::std::ostream& operator<< (::std::ostream& os, const OUString& str)
{
    return os << ::rtl::OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}
#endif

class Test : public CppUnit::TestFixture {
public:
    Test();
    ~Test();

    virtual void setUp();
    virtual void tearDown();

    void testNumberFormat();
    void testFdo60915();
    void testI116701();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testNumberFormat);
    CPPUNIT_TEST(testFdo60915);
    CPPUNIT_TEST(testI116701);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_xContext;
    void checkPreviewString(SvNumberFormatter& aFormatter,
                            const OUString& sCode,
                            double fPreviewNumber,
                            LanguageType eLang,
                            OUString& sExpected);
};

Test::Test()
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    uno::Reference<lang::XMultiServiceFactory> xSM(xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);
}

void Test::setUp()
{
}

void Test::tearDown()
{
}

Test::~Test()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}

void Test::testNumberFormat()
{
    LanguageType eLang = LANGUAGE_ENGLISH_US;

    const char* pNumber[] = {
        "General",
        "0",
        "0.00",
        "#,##0",
        "#,##0.00",
        "#,###.00",
        0
    };

    const char* pScientific[] = {
        "0.00E+000",
        "0.00E+00",
        0
    };

    const char* pPercent[] = {
        "0%",
        "0.00%",
        0
    };

    const char* pFraction[] = {
        "# \?/\?",
        "# \?\?/\?\?",
#if 0
// TODO: Followings aren't in range of NF_FRACTION_START and NF_FRACTION_END
// see enum NfIndexTableOffset in svl/inc/svl/zforlist.hxx
        "# \?/4",
        "# \?\?/100",
#endif
        0
    };

#if 0 // TODO: Find out why on some systems the last two currency format codes differ.
    const char* pCurrency[] = {
        "$#,##0;[RED]-$#,##0",
        "$#,##0.00;[RED]-$#,##0.00",
        "#,##0.00 CCC",
        "$#,##0.--;[RED]-$#,##0.--",
        "$#,##0;-$#,##0",
        "$#,##0;-$#,##0",
        0
    };
#endif

#if 0 // TODO: This currently fails
    const char* pDate[] = {
        "MM/DD/YY",
        "MM/DD/YYYY",
        "MMM D, YY",
        "MMM D, YYYY",
        "D. MMM. YYYY",
        "MMMM D, YYYY",
        "D. MMMM YYYY",
        "NN, MMM D, YY",
        "NN DD/MMM YY",
        "NN, MMMM D, YYYY",
        "NNNNMMMM D, YYYY",
        "MM-DD",
        "YY-MM-DD",
        "YYYY-MM-DD",
        "MM/YY",
        "MMM DD",
        "MMMM",
        "QQ YY",
        "WW",
        "MM/DD/YY",
        "WW",
        0
    };
#endif

    const char* pTime[] = {
        "HH:MM",
        "HH:MM:SS",
        "HH:MM AM/PM",
        "HH:MM:SS AM/PM",
        "[HH]:MM:SS",
        "MM:SS.00",
        "[HH]:MM:SS.00",
        0
    };

#if 0 // TODO: This currently fails
    const char* pDateTime[] = {
        "MM/DD/YY HH:MM AM/PM",
        "MM/DD/YY HH:MM AM/PM",
        0
    };
#endif

    const char* pBoolean[] = {
        "BOOLEAN",
        0
    };

    const char* pText[] = {
        "@",
        0
    };

    struct {
        NfIndexTableOffset eStart;
        NfIndexTableOffset eEnd;
        size_t nSize;
        const char** pCodes;
    } aTests[] = {
        { NF_NUMBER_START, NF_NUMBER_END, 6, pNumber },
        { NF_SCIENTIFIC_START, NF_SCIENTIFIC_END, 2, pScientific },
        { NF_PERCENT_START, NF_PERCENT_END, 2, pPercent },
        { NF_FRACTION_START, NF_FRACTION_END, 2, pFraction },
#if 0 // TODO: Find out why on some systems the last two currency format codes differ.
        { NF_CURRENCY_START, NF_CURRENCY_END, 6, pCurrency },
#endif
#if 0 // TODO: This currently fails
        { NF_DATE_START, NF_DATE_END, 21, pDate },
#endif
        { NF_TIME_START, NF_TIME_END, 7, pTime },
#if 0 // TODO: This currently fails
        { NF_DATETIME_START, NF_DATETIME_END, 2, pDateTime },
#endif
        { NF_BOOLEAN, NF_BOOLEAN, 1, pBoolean },
        { NF_TEXT, NF_TEXT, 1, pText }
    };

    SvNumberFormatter aFormatter(m_xContext, eLang);

    for (size_t i = 0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
        size_t nStart = aTests[i].eStart;
        size_t nEnd = aTests[i].eEnd;

        CPPUNIT_ASSERT_MESSAGE("Unexpected number of formats for this category.",
                               (nEnd - nStart + 1) == aTests[i].nSize);

        for (size_t j = nStart; j <= nEnd; ++j)
        {
            sal_uInt32 nIndex =
                aFormatter.GetFormatIndex(static_cast<NfIndexTableOffset>(j));
            const SvNumberformat* p = aFormatter.GetEntry(nIndex);

            CPPUNIT_ASSERT_MESSAGE("Number format entry is expected, but doesn't exist.", p);
            OUString aCode = p->GetFormatstring();
            bool bEqual = aCode.equalsAscii(aTests[i].pCodes[j-nStart]);
            CPPUNIT_ASSERT_MESSAGE("Unexpected number format code.", bEqual);
        }
    }

    sal_Int32 nPos;
    short nType = NUMBERFORMAT_DEFINED;
    sal_uInt32 nKey;
    OUString aCode;
    // Thai date format (implicit locale).
    aCode = "[$-1070000]d/mm/yyyy;@";
    if (!aFormatter.PutEntry(aCode, nPos, nType, nKey))
    {
        CPPUNIT_ASSERT_MESSAGE("failed to insert format code '[$-1070000]d/mm/yyyy;@'", false);
    }

    // Thai date format (explicit locale)
    aCode = "[$-107041E]d/mm/yyyy;@";
    if (!aFormatter.PutEntry(aCode, nPos, nType, nKey))
    {
        CPPUNIT_ASSERT_MESSAGE("failed to insert format code '[$-107041E]d/mm/yyyy;@'", false);
    }

    // Thai date format (using buddhist calendar type).
    aCode = "[~buddhist]D MMMM YYYY";
    if (!aFormatter.PutEntry(aCode, nPos, nType, nKey))
    {
        CPPUNIT_ASSERT_MESSAGE("failed to insert format code '[~buddhist]D MMMM YYYY'", false);
    }
}

void Test::checkPreviewString(SvNumberFormatter& aFormatter,
                              const OUString& sCode,
                              double fPreviewNumber,
                              LanguageType eLang,
                              OUString& sExpected)
{
    OUString sStr;
    Color* pColor = 0;
    Color** ppColor = &pColor;
    if (!aFormatter.GetPreviewString(sCode, fPreviewNumber, sStr, ppColor, eLang))
        CPPUNIT_FAIL("GetPreviewString() failed");
    CPPUNIT_ASSERT_EQUAL(sExpected, sStr);
}

void Test::testFdo60915()
{
    LanguageType eLang = LANGUAGE_THAI;
    OUString sCode, sExpected;
    double fPreviewNumber = 36486; // equals 1999-11-22 (2542 B.E.)
    SvNumberFormatter aFormatter(m_xContext, eLang);
    {
        sCode = "[~buddhist]D/MM/YYYY";
        sExpected = "22/11/2542";
        checkPreviewString(aFormatter, sCode, fPreviewNumber, eLang, sExpected);
    }
    {
        sCode = "[~buddhist]D/MM/YY";
        sExpected = "22/11/42";
        checkPreviewString(aFormatter, sCode, fPreviewNumber, eLang, sExpected);
    }
    {
        sCode = "[NatNum1][$-41E][~buddhist]D/MM/YYYY";
        sal_Unicode sTemp[] =
        {
            0x0E52, 0x0E52, 0x002F,
            0x0E51, 0x0E51, 0x002F,
            0x0E52, 0x0E55, 0x0E54, 0x0E52
        };
        sExpected = OUString(sTemp, SAL_N_ELEMENTS(sTemp));
        checkPreviewString(aFormatter, sCode, fPreviewNumber, eLang, sExpected);
    }
    {
        sCode = "[NatNum1][$-41E][~buddhist]D/MM/YY";
        sal_Unicode sTemp[] =
        {
            0x0E52, 0x0E52, 0x002F,
            0x0E51, 0x0E51, 0x002F,
            0x0E54, 0x0E52
        };
        sExpected = OUString(sTemp, SAL_N_ELEMENTS(sTemp));
        checkPreviewString(aFormatter, sCode, fPreviewNumber, eLang, sExpected);
    }
}

// https://issues.apache.org/ooo/show_bug.cgi?id=116701
void Test::testI116701()
{
    LanguageType eLang = LANGUAGE_CHINESE_TRADITIONAL;
    OUString sCode, sExpected;
    double fPreviewNumber = 40573; // equals 30/01/2011
    SvNumberFormatter aFormatter(m_xContext, eLang);
    // DateFormatskey25 in i18npool/source/localedata/data/zh_TW.xml
    sal_Unicode CODE1[] =
    {
        0x0047, 0x0047, 0x0047, 0x0045, 0x0045, // GGGEE
        0x0022, 0x5E74, 0x0022,
        0x004D, // M
        0x0022, 0x6708, 0x0022,
        0x0044, // D
        0x0022, 0x65E5, 0x0022
    };
    sCode = OUString(CODE1, SAL_N_ELEMENTS(CODE1));
    sal_Unicode EXPECTED[] =
    {
        0x4E2D, 0x83EF, 0x6C11, 0x570B,
        0x0031, 0x0030, 0x0030, // 100
        0x5E74,
        0x0031, // 1
        0x6708,
        0x0033, 0x0030, // 30
        0x65E5
    };
    sExpected = OUString(EXPECTED, SAL_N_ELEMENTS(EXPECTED));
    checkPreviewString(aFormatter, sCode, fPreviewNumber, eLang, sExpected);
    sal_Unicode CODE2[] =
    {
        0x0047, 0x0047, 0x0047, 0x0045, // GGGE
        0x0022, 0x5E74, 0x0022,
        0x004D, // M
        0x0022, 0x6708, 0x0022,
        0x0044, // D
        0x0022, 0x65E5, 0x0022
    };
    sCode = OUString(CODE2, SAL_N_ELEMENTS(CODE2));
    checkPreviewString(aFormatter, sCode, fPreviewNumber, eLang, sExpected);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
