/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef _WIN32
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

#include <math.h>

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/sharedstringpool.hxx>
#include <svl/sharedstring.hxx>
#include <unotools/syslocale.hxx>

#include <memory>
#include <unicode/calendar.h>

using namespace ::com::sun::star;
using namespace svl;

namespace {

class Test : public CppUnit::TestFixture {
public:
    Test();
    virtual ~Test();

    virtual void tearDown() override;

    void testNumberFormat();
    void testSharedString();
    void testSharedStringPool();
    void testSharedStringPoolPurge();
    void testFdo60915();
    void testI116701();
    void testDateInput();
    void testIsNumberFormat();
    void testUserDefinedNumberFormats();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testNumberFormat);
    CPPUNIT_TEST(testSharedString);
    CPPUNIT_TEST(testSharedStringPool);
    CPPUNIT_TEST(testSharedStringPoolPurge);
    CPPUNIT_TEST(testFdo60915);
    CPPUNIT_TEST(testI116701);
    CPPUNIT_TEST(testDateInput);
    CPPUNIT_TEST(testIsNumberFormat);
    CPPUNIT_TEST(testUserDefinedNumberFormats);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_xContext;
    void checkPreviewString(SvNumberFormatter& aFormatter,
                            const OUString& sCode,
                            double fPreviewNumber,
                            LanguageType eLang,
                            OUString& sExpected);
    void checkDateInput( SvNumberFormatter& rFormatter, const char* pTimezone, const char* pIsoDate );
    std::unique_ptr<icu::TimeZone> m_pDefaultTimeZone;
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
    m_pDefaultTimeZone.reset(icu::TimeZone::createDefault());
}

void Test::tearDown()
{
    icu::TimeZone::setDefault(*m_pDefaultTimeZone);
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
        nullptr
    };

    const char* pScientific[] = {
        "0.00E+000",
        "0.00E+00",
        nullptr
    };

    const char* pPercent[] = {
        "0%",
        "0.00%",
        nullptr
    };

    const char* pFraction[] = {
        "# \?/\?",
        "# \?\?/\?\?",
        nullptr
    };

// Followings aren't in range of NF_FRACTION_START and NF_FRACTION_END
// see enum NfIndexTableOffset in svl/inc/svl/zforlist.hxx
    const char* pFractionExt[] = {
        "# \?\?\?/\?\?\?",
        "# \?/2",
        "# \?/4",
        "# \?/8",
        "# \?\?/16",
        "# \?\?/10",
        "# \?\?/100",
        nullptr
    };

    const char* pCurrency[] = {
        "$#,##0;-$#,##0",
        "$#,##0.00;-$#,##0.00",
        "$#,##0;[RED]-$#,##0",
        "$#,##0.00;[RED]-$#,##0.00",
        "#,##0.00 CCC",
        "$#,##0.--;[RED]-$#,##0.--",
        nullptr
    };

    const char* pDate[] = {
        "M/D/YY",
        "NNNNMMMM DD, YYYY",
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
        nullptr
    };

    const char* pTime[] = {
        "HH:MM",
        "HH:MM:SS",
        "HH:MM AM/PM",
        "HH:MM:SS AM/PM",
        "[HH]:MM:SS",
        "MM:SS.00",
        "[HH]:MM:SS.00",
        nullptr
    };

    const char* pDateTime[] = {
        "MM/DD/YY HH:MM AM/PM",
        "MM/DD/YYYY HH:MM:SS",
        nullptr
    };

    const char* pBoolean[] = {
        "BOOLEAN",
        nullptr
    };

    const char* pText[] = {
        "@",
        nullptr
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
        { NF_FRACTION_3D, NF_FRACTION_100, 7, pFractionExt },
        { NF_CURRENCY_START, NF_CURRENCY_END, 6, pCurrency },
        { NF_DATE_START, NF_DATE_END, 21, pDate },
        { NF_TIME_START, NF_TIME_END, 7, pTime },
        { NF_DATETIME_START, NF_DATETIME_END, 2, pDateTime },
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
            CPPUNIT_ASSERT_EQUAL( aCode.toUtf8() , OString( aTests[i].pCodes[j-nStart] ) );
        }
    }

    sal_Int32 nPos;
    short nType = css::util::NumberFormat::DEFINED;
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

void Test::testSharedString()
{
    // Use shared string as normal, non-shared string, which is allowed.
    SharedString aSS1("Test"), aSS2("Test");
    CPPUNIT_ASSERT_MESSAGE("Equality check should return true.", aSS1 == aSS2);
    SharedString aSS3("test");
    CPPUNIT_ASSERT_MESSAGE("Equality check is case sensitive.", aSS1 != aSS3);
}

void Test::testSharedStringPool()
{
    SvtSysLocale aSysLocale;
    svl::SharedStringPool aPool(aSysLocale.GetCharClassPtr());

    svl::SharedString p1, p2;
    p1 = aPool.intern("Andy");
    p2 = aPool.intern("Andy");
    CPPUNIT_ASSERT_EQUAL(p1.getData(), p2.getData());

    p2 = aPool.intern("Bruce");
    CPPUNIT_ASSERT_MESSAGE("They must differ.", p1.getData() != p2.getData());

    OUString aAndy("Andy");
    p1 = aPool.intern("Andy");
    p2 = aPool.intern(aAndy);
    CPPUNIT_ASSERT_MESSAGE("Identifier shouldn't be NULL.", p1.getData());
    CPPUNIT_ASSERT_MESSAGE("Identifier shouldn't be NULL.", p2.getData());
    CPPUNIT_ASSERT_EQUAL(p1.getData(), p2.getData());

    // Test case insensitive string ID's.
    OUString aAndyLower("andy"), aAndyUpper("ANDY");
    p1 = aPool.intern(aAndy);
    p2 = aPool.intern(aAndyLower);
    CPPUNIT_ASSERT_MESSAGE("Failed to intern strings.", p1.getData() && p2.getData());
    CPPUNIT_ASSERT_MESSAGE("These two ID's should differ.", p1.getData() != p2.getData());
    CPPUNIT_ASSERT_MESSAGE("These two ID's should be equal.", p1.getDataIgnoreCase() == p2.getDataIgnoreCase());
    p2 = aPool.intern(aAndyUpper);
    CPPUNIT_ASSERT_MESSAGE("Failed to intern string.", p2.getData());
    CPPUNIT_ASSERT_MESSAGE("These two ID's should differ.", p1.getData() != p2.getData());
    CPPUNIT_ASSERT_MESSAGE("These two ID's should be equal.", p1.getDataIgnoreCase() == p2.getDataIgnoreCase());
}

void Test::testSharedStringPoolPurge()
{
    SvtSysLocale aSysLocale;
    svl::SharedStringPool aPool(aSysLocale.GetCharClassPtr());
    aPool.intern("Andy");
    aPool.intern("andy");
    aPool.intern("ANDY");

    CPPUNIT_ASSERT_MESSAGE("Wrong string count.", aPool.getCount() == 3);
    CPPUNIT_ASSERT_MESSAGE("Wrong case insensitive string count.", aPool.getCountIgnoreCase() == 1);

    // Since no string objects referencing the pooled strings exist, purging
    // the pool should empty it.
    aPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), aPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), aPool.getCountIgnoreCase());

    // Now, create string objects on the heap.
    std::unique_ptr<OUString> pStr1(new OUString("Andy"));
    std::unique_ptr<OUString> pStr2(new OUString("andy"));
    std::unique_ptr<OUString> pStr3(new OUString("ANDY"));
    std::unique_ptr<OUString> pStr4(new OUString("Bruce"));
    aPool.intern(*pStr1);
    aPool.intern(*pStr2);
    aPool.intern(*pStr3);
    aPool.intern(*pStr4);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPool.getCountIgnoreCase());

    // This shouldn't purge anything.
    aPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPool.getCountIgnoreCase());

    // Delete one heap string object, and purge. That should purge one string.
    pStr1.reset();
    aPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPool.getCountIgnoreCase());

    // Ditto...
    pStr3.reset();
    aPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPool.getCountIgnoreCase());

    // Again.
    pStr2.reset();
    aPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPool.getCountIgnoreCase());

    // Delete 'Bruce' and purge.
    pStr4.reset();
    aPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), aPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), aPool.getCountIgnoreCase());
}

void Test::checkPreviewString(SvNumberFormatter& aFormatter,
                              const OUString& sCode,
                              double fPreviewNumber,
                              LanguageType eLang,
                              OUString& sExpected)
{
    OUString sStr;
    Color* pColor = nullptr;
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

// https://bz.apache.org/ooo/show_bug.cgi?id=116701
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

void Test::testDateInput()
{
    const char* aData[][2] = {
        { "Europe/Paris", "1938-10-07" },      // i#76623
        { "Europe/Moscow", "1919-07-01" },     // i#86094
        { "America/St_Johns", "1935-03-30" },  // i#86094 i#90627
        { "Europe/Tallinn", "1790-03-01" },    // i#105864
        { "Australia/Perth", "2004-04-11" },   // i#17222
        { "America/Sao_Paulo", "1902-04-22" }, // tdf#44286
        { "Europe/Berlin", "1790-07-27" },
        { "US/Mountain", "1790-07-26" },
        { "Asia/Tehran", "1999-03-22" },

        // Data from https://bugs.documentfoundation.org/show_bug.cgi?id=63230
        // https://bugs.documentfoundation.org/attachment.cgi?id=79051
        // https://bugs.documentfoundation.org/show_bug.cgi?id=79663
        { "Africa/Accra", "1800-01-01" },
        { "Africa/Accra", "1800-04-10" },
        { "Africa/Addis_Ababa", "1870-01-01" },
        { "Africa/Addis_Ababa", "1936-05-05" },
        { "Africa/Algiers", "1956-01-29" },
        { "Africa/Algiers", "1981-05-01" },
        { "Africa/Asmara", "1936-05-05" },
        { "Africa/Asmera", "1936-05-05" },
        { "Africa/Bujumbura", "1890-01-01" },
        { "Africa/Casablanca", "1984-03-16" },
        { "Africa/Ceuta", "1984-03-16" },
        { "Africa/Dar_es_Salaam", "1931-01-01" },
        { "Africa/Dar_es_Salaam", "1961-01-01" },
        { "Africa/Djibouti", "1911-07-01" },
        { "Africa/Douala", "1912-01-01" },
        { "Africa/El_Aaiun", "1934-01-01" },
        { "Africa/Freetown", "1913-06-01" },
        { "Africa/Gaborone", "1885-01-01" },
        { "Africa/Johannesburg", "1903-03-01" },
        { "Africa/Kampala", "1928-07-01" },
        { "Africa/Kampala", "1948-01-01" },
        { "Africa/Kampala", "1957-01-01" },
        { "Africa/Lagos", "1919-09-01" },
        { "Africa/Libreville", "1912-01-01" },
        { "Africa/Luanda", "1911-05-26" },
        { "Africa/Lubumbashi", "1897-11-09" },
        { "Africa/Lusaka", "1903-03-01" },
        { "Africa/Malabo", "1963-12-15" },
        { "Africa/Maseru", "1903-03-01" },
        { "Africa/Mogadishu", "1957-01-01" },
        { "Africa/Monrovia", "1919-03-01" },
        { "Africa/Nairobi", "1928-07-01" },
        { "Africa/Nairobi", "1940-01-01" },
        { "Africa/Nairobi", "1960-01-01" },
        { "Africa/Niamey", "1960-01-01" },
        { "Africa/Porto-Novo", "1934-02-26" },
        { "Africa/Tripoli", "1920-01-01" },
        { "Africa/Tripoli", "1959-01-01" },
        { "Africa/Tripoli", "1990-05-04" },
        { "Africa/Tunis", "1911-03-11" },
        { "Africa/Windhoek", "1892-02-08" },
        { "Africa/Windhoek", "1903-03-01" },
        { "America/Antigua", "1912-03-02" },
        { "America/Argentina/Buenos_Aires", "1894-10-31" },
        { "America/Argentina/Catamarca", "1991-10-20" },
        { "America/Argentina/Catamarca", "2004-06-01" },
        { "America/Argentina/ComodRivadavia", "1991-10-20" },
        { "America/Argentina/ComodRivadavia", "2004-06-01" },
        { "America/Argentina/Cordoba", "1991-10-20" },
        { "America/Argentina/Jujuy", "1991-10-06" },
        { "America/Argentina/La_Rioja", "2004-06-01" },
        { "America/Argentina/Mendoza", "1992-10-18" },
        { "America/Argentina/Mendoza", "2004-05-23" },
        { "America/Argentina/Rio_Gallegos", "2004-06-01" },
        { "America/Argentina/Salta", "1991-10-20" },
        { "America/Argentina/San_Juan", "2004-05-31" },
        { "America/Argentina/San_Luis", "2004-05-31" },
        { "America/Argentina/San_Luis", "2008-01-21" },
        { "America/Argentina/Tucuman", "1991-10-20" },
        { "America/Argentina/Tucuman", "2004-06-01" },
        { "America/Argentina/Ushuaia", "2004-05-30" },
        { "America/Asuncion", "1931-10-10" },
        { "America/Asuncion", "1974-04-01" },
        { "America/Bahia", "1914-01-01" },
        { "America/Bahia_Banderas", "1930-11-15" },
        { "America/Bahia_Banderas", "1931-10-01" },
        { "America/Bahia_Banderas", "1942-04-24" },
        { "America/Bahia_Banderas", "1949-01-14" },
        { "America/Barbados", "1932-01-01" },
        { "America/Belize", "1912-04-01" },
        { "America/Blanc-Sablon", "1884-01-01" },
        { "America/Bogota", "1914-11-23" },
        { "America/Buenos_Aires", "1894-10-31" },
        { "America/Cambridge_Bay", "2000-11-05" },
        { "America/Campo_Grande", "1914-01-01" },
        { "America/Caracas", "1912-02-12" },
        { "America/Catamarca", "1991-10-20" },
        { "America/Catamarca", "2004-06-01" },
        { "America/Cayenne", "1911-07-01" },
        { "America/Chihuahua", "1930-11-15" },
        { "America/Chihuahua", "1931-10-01" },
        { "America/Cordoba", "1991-10-20" },
        { "America/Costa_Rica", "1921-01-15" },
        { "America/Cuiaba", "1914-01-01" },
        { "America/Danmarkshavn", "1916-07-28" },
        { "America/Detroit", "1905-01-01" },
        { "America/Eirunepe", "1914-01-01" },
        { "America/El_Salvador", "1921-01-01" },
        { "America/Ensenada", "1924-01-01" },
        { "America/Ensenada", "1930-11-15" },
        { "America/Fortaleza", "1914-01-01" },
        { "America/Glace_Bay", "1902-06-15" },
        { "America/Grand_Turk", "1890-01-01" },
        { "America/Guyana", "1991-01-01" },
        { "America/Havana", "1890-01-01" },
        { "America/Hermosillo", "1930-11-15" },
        { "America/Hermosillo", "1931-10-01" },
        { "America/Hermosillo", "1942-04-24" },
        { "America/Hermosillo", "1949-01-14" },
        { "America/Jujuy", "1991-10-06" },
        { "America/Lima", "1890-01-01" },
        { "America/Maceio", "1914-01-01" },
        { "America/Managua", "1890-01-01" },
        { "America/Managua", "1934-06-23" },
        { "America/Managua", "1975-02-16" },
        { "America/Managua", "1992-09-24" },
        { "America/Managua", "1997-01-01" },
        { "America/Mazatlan", "1930-11-15" },
        { "America/Mazatlan", "1931-10-01" },
        { "America/Mazatlan", "1942-04-24" },
        { "America/Mazatlan", "1949-01-14" },
        { "America/Mendoza", "1992-10-18" },
        { "America/Mendoza", "2004-05-23" },
        { "America/Merida", "1982-12-02" },
        { "America/Mexico_City", "1930-11-15" },
        { "America/Mexico_City", "1931-10-01" },
        { "America/Miquelon", "1911-05-15" },
        { "America/Moncton", "1883-12-09" },
        { "America/Montevideo", "1942-12-14" },
        { "America/Montevideo", "1974-12-22" },
        { "America/Montreal", "1884-01-01" },
        { "America/Ojinaga", "1930-11-15" },
        { "America/Ojinaga", "1931-10-01" },
        { "America/Panama", "1890-01-01" },
        { "America/Paramaribo", "1911-01-01" },
        { "America/Porto_Acre", "1914-01-01" },
        { "America/Recife", "1914-01-01" },
        { "America/Regina", "1905-09-01" },
        { "America/Rio_Branco", "1914-01-01" },
        { "America/Rosario", "1991-10-20" },
        { "America/Santa_Isabel", "1924-01-01" },
        { "America/Santa_Isabel", "1930-11-15" },
        { "America/Santarem", "1914-01-01" },
        { "America/Santiago", "1910-01-01" },
        { "America/Santiago", "1919-07-01" },
        { "America/Santo_Domingo", "1890-01-01" },
        { "America/Scoresbysund", "1916-07-28" },
        { "America/Scoresbysund", "1981-03-29" },
        { "America/Tegucigalpa", "1921-04-01" },
        { "America/Thunder_Bay", "1895-01-01" },
        { "America/Tijuana", "1924-01-01" },
        { "America/Tijuana", "1930-11-15" },
        { "Antarctica/Casey", "1969-01-01" },
        { "Antarctica/Casey", "2009-10-18" },
        { "Antarctica/Davis", "1957-01-13" },
        { "Antarctica/Davis", "1969-02-01" },
        { "Antarctica/Davis", "2010-03-11" },
        { "Antarctica/DumontDUrville", "1947-01-01" },
        { "Antarctica/DumontDUrville", "1956-11-01" },
        { "Antarctica/Macquarie", "1911-01-01" },
        { "Antarctica/Mawson", "1954-02-13" },
        { "Antarctica/McMurdo", "1956-01-01" },
        { "Antarctica/Palmer", "1982-05-01" },
        { "Antarctica/South_Pole", "1956-01-01" },
        { "Antarctica/Syowa", "1957-01-29" },
        { "Antarctica/Vostok", "1957-12-16" },
        { "Arctic/Longyearbyen", "1895-01-01" },
        { "Asia/Almaty", "1930-06-21" },
        { "Asia/Anadyr", "1924-05-02" },
        { "Asia/Anadyr", "1930-06-21" },
        { "Asia/Anadyr", "1992-01-19" },
        { "Asia/Anadyr", "2011-03-27" },
        { "Asia/Aqtau", "1924-05-02" },
        { "Asia/Aqtau", "1930-06-21" },
        { "Asia/Aqtau", "1981-10-01" },
        { "Asia/Aqtau", "2005-03-15" },
        { "Asia/Aqtobe", "1924-05-02" },
        { "Asia/Aqtobe", "1930-06-21" },
        { "Asia/Ashgabat", "1924-05-02" },
        { "Asia/Ashgabat", "1930-06-21" },
        { "Asia/Ashgabat", "1992-01-19" },
        { "Asia/Ashkhabad", "1924-05-02" },
        { "Asia/Ashkhabad", "1930-06-21" },
        { "Asia/Ashkhabad", "1992-01-19" },
        { "Asia/Baghdad", "1918-01-01" },
        { "Asia/Bahrain", "1920-01-01" },
        { "Asia/Baku", "1957-03-01" },
        { "Asia/Bangkok", "1920-04-01" },
        { "Asia/Bishkek", "1924-05-02" },
        { "Asia/Bishkek", "1930-06-21" },
        { "Asia/Brunei", "1933-01-01" },
        { "Asia/Calcutta", "1941-10-01" },
        { "Asia/Choibalsan", "1978-01-01" },
        { "Asia/Chongqing", "1980-05-01" },
        { "Asia/Chungking", "1980-05-01" },
        { "Asia/Colombo", "1880-01-01" },
        { "Asia/Colombo", "1906-01-01" },
        { "Asia/Colombo", "1942-09-01" },
        { "Asia/Colombo", "1996-05-25" },
        { "Asia/Dacca", "1941-10-01" },
        { "Asia/Dacca", "1942-09-01" },
        { "Asia/Dhaka", "1941-10-01" },
        { "Asia/Dhaka", "1942-09-01" },
        { "Asia/Dili", "2000-09-17" },
        { "Asia/Dubai", "1920-01-01" },
        { "Asia/Dushanbe", "1924-05-02" },
        { "Asia/Dushanbe", "1930-06-21" },
        { "Asia/Harbin", "1928-01-01" },
        { "Asia/Harbin", "1940-01-01" },
        { "Asia/Ho_Chi_Minh", "1912-05-01" },
        { "Asia/Hong_Kong", "1904-10-30" },
        { "Asia/Hong_Kong", "1941-12-25" },
        { "Asia/Hovd", "1978-01-01" },
        { "Asia/Irkutsk", "1920-01-25" },
        { "Asia/Irkutsk", "1930-06-21" },
        { "Asia/Irkutsk", "1992-01-19" },
        { "Asia/Irkutsk", "2011-03-27" },
        { "Asia/Istanbul", "1880-01-01" },
        { "Asia/Istanbul", "1910-10-01" },
        { "Asia/Istanbul", "1978-10-15" },
        { "Asia/Jakarta", "1932-11-01" },
        { "Asia/Jakarta", "1942-03-23" },
        { "Asia/Jakarta", "1948-05-01" },
        { "Asia/Jayapura", "1944-09-01" },
        { "Asia/Kabul", "1945-01-01" },
        { "Asia/Kamchatka", "1922-11-10" },
        { "Asia/Kamchatka", "1930-06-21" },
        { "Asia/Kamchatka", "1992-01-19" },
        { "Asia/Kamchatka", "2011-03-27" },
        { "Asia/Karachi", "1907-01-01" },
        { "Asia/Kashgar", "1928-01-01" },
        { "Asia/Kashgar", "1980-05-01" },
        { "Asia/Kathmandu", "1986-01-01" },
        { "Asia/Katmandu", "1986-01-01" },
        { "Asia/Kolkata", "1941-10-01" },
        { "Asia/Krasnoyarsk", "1930-06-21" },
        { "Asia/Krasnoyarsk", "1992-01-19" },
        { "Asia/Krasnoyarsk", "2011-03-27" },
        { "Asia/Kuala_Lumpur", "1901-01-01" },
        { "Asia/Kuala_Lumpur", "1905-06-01" },
        { "Asia/Kuala_Lumpur", "1941-09-01" },
        { "Asia/Kuala_Lumpur", "1942-02-16" },
        { "Asia/Kuala_Lumpur", "1982-01-01" },
        { "Asia/Kuching", "1926-03-01" },
        { "Asia/Kuching", "1933-01-01" },
        { "Asia/Kuching", "1942-02-16" },
        { "Asia/Macao", "1912-01-01" },
        { "Asia/Macau", "1912-01-01" },
        { "Asia/Magadan", "1930-06-21" },
        { "Asia/Magadan", "1992-01-19" },
        { "Asia/Magadan", "2011-03-27" },
        { "Asia/Makassar", "1932-11-01" },
        { "Asia/Makassar", "1942-02-09" },
        { "Asia/Manila", "1942-05-01" },
        { "Asia/Muscat", "1920-01-01" },
        { "Asia/Novokuznetsk", "1920-01-06" },
        { "Asia/Novokuznetsk", "1930-06-21" },
        { "Asia/Novokuznetsk", "1992-01-19" },
        { "Asia/Novokuznetsk", "2011-03-27" },
        { "Asia/Novosibirsk", "1930-06-21" },
        { "Asia/Novosibirsk", "1992-01-19" },
        { "Asia/Novosibirsk", "2011-03-27" },
        { "Asia/Omsk", "1919-11-14" },
        { "Asia/Omsk", "1930-06-21" },
        { "Asia/Omsk", "1992-01-19" },
        { "Asia/Omsk", "2011-03-27" },
        { "Asia/Oral", "1924-05-02" },
        { "Asia/Oral", "1930-06-21" },
        { "Asia/Oral", "2005-03-15" },
        { "Asia/Phnom_Penh", "1906-06-09" },
        { "Asia/Phnom_Penh", "1912-05-01" },
        { "Asia/Pontianak", "1932-11-01" },
        { "Asia/Pontianak", "1942-01-29" },
        { "Asia/Pontianak", "1948-05-01" },
        { "Asia/Pontianak", "1964-01-01" },
        { "Asia/Pyongyang", "1890-01-01" },
        { "Asia/Pyongyang", "1904-12-01" },
        { "Asia/Pyongyang", "1932-01-01" },
        { "Asia/Pyongyang", "1961-08-10" },
        { "Asia/Qatar", "1920-01-01" },
        { "Asia/Qyzylorda", "1930-06-21" },
        { "Asia/Qyzylorda", "1992-01-19" },
        { "Asia/Rangoon", "1920-01-01" },
        { "Asia/Rangoon", "1942-05-01" },
        { "Asia/Saigon", "1912-05-01" },
        { "Asia/Sakhalin", "1945-08-25" },
        { "Asia/Sakhalin", "1992-01-19" },
        { "Asia/Sakhalin", "2011-03-27" },
        { "Asia/Samarkand", "1930-06-21" },
        { "Asia/Seoul", "1890-01-01" },
        { "Asia/Seoul", "1904-12-01" },
        { "Asia/Seoul", "1932-01-01" },
        { "Asia/Seoul", "1961-08-10" },
        { "Asia/Seoul", "1968-10-01" },
        { "Asia/Singapore", "1905-06-01" },
        { "Asia/Singapore", "1941-09-01" },
        { "Asia/Singapore", "1942-02-16" },
        { "Asia/Singapore", "1982-01-01" },
        { "Asia/Tashkent", "1924-05-02" },
        { "Asia/Tashkent", "1930-06-21" },
        { "Asia/Tbilisi", "1924-05-02" },
        { "Asia/Tbilisi", "1957-03-01" },
        { "Asia/Tbilisi", "2005-03-27" },
        { "Asia/Tehran", "1946-01-01" },
        { "Asia/Tehran", "1977-11-01" },
        { "Asia/Thimbu", "1987-10-01" },
        { "Asia/Thimphu", "1987-10-01" },
        { "Asia/Ujung_Pandang", "1932-11-01" },
        { "Asia/Ujung_Pandang", "1942-02-09" },
        { "Asia/Ulaanbaatar", "1978-01-01" },
        { "Asia/Ulan_Bator", "1978-01-01" },
        { "Asia/Urumqi", "1928-01-01" },
        { "Asia/Urumqi", "1980-05-01" },
        { "Asia/Vientiane", "1906-06-09" },
        { "Asia/Vientiane", "1912-05-01" },
        { "Asia/Vladivostok", "1922-11-15" },
        { "Asia/Vladivostok", "1930-06-21" },
        { "Asia/Vladivostok", "1992-01-19" },
        { "Asia/Vladivostok", "2011-03-27" },
        { "Asia/Yakutsk", "1930-06-21" },
        { "Asia/Yakutsk", "1992-01-19" },
        { "Asia/Yakutsk", "2011-03-27" },
        { "Asia/Yekaterinburg", "1930-06-21" },
        { "Asia/Yekaterinburg", "1992-01-19" },
        { "Asia/Yekaterinburg", "2011-03-27" },
        { "Asia/Yerevan", "1924-05-02" },
        { "Asia/Yerevan", "1957-03-01" },
        { "Atlantic/Azores", "1884-01-01" },
        { "Atlantic/Azores", "1911-05-24" },
        { "Atlantic/Azores", "1942-04-25" },
        { "Atlantic/Azores", "1943-04-17" },
        { "Atlantic/Azores", "1944-04-22" },
        { "Atlantic/Azores", "1945-04-21" },
        { "Atlantic/Cape_Verde", "1907-01-01" },
        { "Atlantic/Jan_Mayen", "1895-01-01" },
        { "Atlantic/Madeira", "1942-04-25" },
        { "Atlantic/Madeira", "1943-04-17" },
        { "Atlantic/Madeira", "1944-04-22" },
        { "Atlantic/Madeira", "1945-04-21" },
        { "Atlantic/Reykjavik", "1837-01-01" },
        { "Atlantic/Stanley", "1912-03-12" },
        { "Australia/Adelaide", "1899-05-01" },
        { "Australia/Broken_Hill", "1895-02-01" },
        { "Australia/Broken_Hill", "1899-05-01" },
        { "Australia/Currie", "1895-09-01" },
        { "Australia/Darwin", "1895-02-01" },
        { "Australia/Darwin", "1899-05-01" },
        { "Australia/Eucla", "1895-12-01" },
        { "Australia/Hobart", "1895-09-01" },
        { "Australia/LHI", "1981-03-01" },
        { "Australia/Lindeman", "1895-01-01" },
        { "Australia/Lord_Howe", "1981-03-01" },
        { "Australia/Melbourne", "1895-02-01" },
        { "Australia/North", "1895-02-01" },
        { "Australia/North", "1899-05-01" },
        { "Australia/Perth", "1895-12-01" },
        { "Australia/South", "1899-05-01" },
        { "Australia/Tasmania", "1895-09-01" },
        { "Australia/Victoria", "1895-02-01" },
        { "Australia/West", "1895-12-01" },
        { "Australia/Yancowinna", "1895-02-01" },
        { "Australia/Yancowinna", "1899-05-01" },
        { "Brazil/Acre", "1914-01-01" },
        { "Canada/East-Saskatchewan", "1905-09-01" },
        { "Canada/Saskatchewan", "1905-09-01" },
        { "Chile/Continental", "1910-01-01" },
        { "Chile/Continental", "1919-07-01" },
        { "Chile/EasterIsland", "1932-09-01" },
        { "Cuba", "1890-01-01" },
        { "Eire", "1880-08-02" },
        { "Europe/Amsterdam", "1937-07-01" },
        { "Europe/Andorra", "1946-09-30" },
        { "Europe/Athens", "1916-07-28" },
        { "Europe/Athens", "1944-04-04" },
        { "Europe/Berlin", "1893-04-01" },
        { "Europe/Bratislava", "1891-10-01" },
        { "Europe/Brussels", "1914-11-08" },
        { "Europe/Bucharest", "1931-07-24" },
        { "Europe/Chisinau", "1931-07-24" },
        { "Europe/Copenhagen", "1894-01-01" },
        { "Europe/Dublin", "1880-08-02" },
        { "Europe/Gibraltar", "1941-05-04" },
        { "Europe/Gibraltar", "1942-04-05" },
        { "Europe/Gibraltar", "1943-04-04" },
        { "Europe/Gibraltar", "1944-04-02" },
        { "Europe/Gibraltar", "1945-04-02" },
        { "Europe/Gibraltar", "1947-04-13" },
        { "Europe/Helsinki", "1921-05-01" },
        { "Europe/Istanbul", "1880-01-01" },
        { "Europe/Istanbul", "1910-10-01" },
        { "Europe/Istanbul", "1978-10-15" },
        { "Europe/Kaliningrad", "1945-01-01" },
        { "Europe/Kaliningrad", "1946-01-01" },
        { "Europe/Kaliningrad", "2011-03-27" },
        { "Europe/Kiev", "1930-06-21" },
        { "Europe/Kiev", "1943-11-06" },
        { "Europe/Luxembourg", "1904-06-01" },
        { "Europe/Madrid", "1942-05-02" },
        { "Europe/Madrid", "1943-04-17" },
        { "Europe/Madrid", "1944-04-15" },
        { "Europe/Madrid", "1945-04-14" },
        { "Europe/Madrid", "1946-04-13" },
        { "Europe/Malta", "1893-11-02" },
        { "Europe/Mariehamn", "1921-05-01" },
        { "Europe/Minsk", "1924-05-02" },
        { "Europe/Minsk", "1930-06-21" },
        { "Europe/Minsk", "2011-03-27" },
        { "Europe/Monaco", "1941-05-05" },
        { "Europe/Monaco", "1942-03-09" },
        { "Europe/Monaco", "1943-03-29" },
        { "Europe/Monaco", "1944-04-03" },
        { "Europe/Monaco", "1945-04-02" },
        { "Europe/Moscow", "1916-07-03" },
        { "Europe/Moscow", "1919-05-31" },
        { "Europe/Moscow", "1930-06-21" },
        { "Europe/Moscow", "1992-01-19" },
        { "Europe/Moscow", "2011-03-27" },
        { "Europe/Oslo", "1895-01-01" },
        { "Europe/Paris", "1945-04-02" },
        { "Europe/Prague", "1891-10-01" },
        { "Europe/Riga", "1926-05-11" },
        { "Europe/Riga", "1940-08-05" },
        { "Europe/Riga", "1944-10-13" },
        { "Europe/Rome", "1893-11-01" },
        { "Europe/Samara", "1930-06-21" },
        { "Europe/Samara", "1991-10-20" },
        { "Europe/Samara", "2011-03-27" },
        { "Europe/San_Marino", "1893-11-01" },
        { "Europe/Simferopol", "1930-06-21" },
        { "Europe/Simferopol", "1994-05-01" },
        { "Europe/Sofia", "1880-01-01" },
        { "Europe/Sofia", "1894-11-30" },
        { "Europe/Tallinn", "1919-07-01" },
        { "Europe/Tallinn", "1921-05-01" },
        { "Europe/Tallinn", "1940-08-06" },
        { "Europe/Tiraspol", "1931-07-24" },
        { "Europe/Uzhgorod", "1945-06-29" },
        { "Europe/Vaduz", "1894-06-01" },
        { "Europe/Vatican", "1893-11-01" },
        { "Europe/Vilnius", "1917-01-01" },
        { "Europe/Vilnius", "1920-07-12" },
        { "Europe/Vilnius", "1940-08-03" },
        { "Europe/Volgograd", "1920-01-03" },
        { "Europe/Volgograd", "1930-06-21" },
        { "Europe/Volgograd", "1991-03-31" },
        { "Europe/Volgograd", "2011-03-27" },
        { "Europe/Zaporozhye", "1930-06-21" },
        { "Europe/Zaporozhye", "1943-10-25" },
        { "Europe/Zurich", "1894-06-01" },
        { "Hongkong", "1904-10-30" },
        { "Hongkong", "1941-12-25" },
        { "Iceland", "1837-01-01" },
        { "Indian/Chagos", "1907-01-01" },
        { "Indian/Chagos", "1996-01-01" },
        { "Indian/Cocos", "1900-01-01" },
        { "Indian/Comoro", "1911-07-01" },
        { "Indian/Kerguelen", "1950-01-01" },
        { "Indian/Mahe", "1906-06-01" },
        { "Indian/Maldives", "1960-01-01" },
        { "Indian/Mauritius", "1907-01-01" },
        { "Indian/Reunion", "1911-06-01" },
        { "Iran", "1946-01-01" },
        { "Iran", "1977-11-01" },
        { "Libya", "1920-01-01" },
        { "Libya", "1959-01-01" },
        { "Libya", "1990-05-04" },
        { "Mexico/BajaNorte", "1924-01-01" },
        { "Mexico/BajaNorte", "1930-11-15" },
        { "Mexico/BajaSur", "1930-11-15" },
        { "Mexico/BajaSur", "1931-10-01" },
        { "Mexico/BajaSur", "1942-04-24" },
        { "Mexico/BajaSur", "1949-01-14" },
        { "Mexico/General", "1930-11-15" },
        { "Mexico/General", "1931-10-01" },
        { "NZ-CHAT", "1957-01-01" },
        { "Pacific/Apia", "1911-01-01" },
        { "Pacific/Apia", "2011-12-30" },
        { "Pacific/Chatham", "1957-01-01" },
        { "Pacific/Easter", "1932-09-01" },
        { "Pacific/Enderbury", "1901-01-01" },
        { "Pacific/Enderbury", "1995-01-01" },
        { "Pacific/Fakaofo", "2011-12-30" },
        { "Pacific/Fiji", "1915-10-26" },
        { "Pacific/Funafuti", "1901-01-01" },
        { "Pacific/Galapagos", "1986-01-01" },
        { "Pacific/Gambier", "1912-10-01" },
        { "Pacific/Guadalcanal", "1912-10-01" },
        { "Pacific/Guam", "1901-01-01" },
        { "Pacific/Kiritimati", "1901-01-01" },
        { "Pacific/Kiritimati", "1995-01-01" },
        { "Pacific/Kosrae", "1901-01-01" },
        { "Pacific/Kosrae", "1969-10-01" },
        { "Pacific/Kwajalein", "1993-08-20" },
        { "Pacific/Majuro", "1969-10-01" },
        { "Pacific/Marquesas", "1912-10-01" },
        { "Pacific/Nauru", "1921-01-15" },
        { "Pacific/Nauru", "1944-08-15" },
        { "Pacific/Nauru", "1979-05-01" },
        { "Pacific/Niue", "1901-01-01" },
        { "Pacific/Niue", "1951-01-01" },
        { "Pacific/Norfolk", "1901-01-01" },
        { "Pacific/Norfolk", "1951-01-01" },
        { "Pacific/Pago_Pago", "1911-01-01" },
        { "Pacific/Palau", "1901-01-01" },
        { "Pacific/Pohnpei", "1901-01-01" },
        { "Pacific/Ponape", "1901-01-01" },
        { "Pacific/Port_Moresby", "1895-01-01" },
        { "Pacific/Rarotonga", "1978-11-12" },
        { "Pacific/Saipan", "1969-10-01" },
        { "Pacific/Samoa", "1911-01-01" },
        { "Pacific/Tahiti", "1912-10-01" },
        { "Pacific/Tarawa", "1901-01-01" },
        { "Pacific/Tongatapu", "1901-01-01" },
        { "Pacific/Tongatapu", "1941-01-01" },
        { "Pacific/Wake", "1901-01-01" },
        { "ROK", "1890-01-01" },
        { "ROK", "1904-12-01" },
        { "ROK", "1932-01-01" },
        { "ROK", "1961-08-10" },
        { "ROK", "1968-10-01" },
        { "Singapore", "1905-06-01" },
        { "Singapore", "1941-09-01" },
        { "Singapore", "1942-02-16" },
        { "Singapore", "1982-01-01" },
        { "Turkey", "1880-01-01" },
        { "Turkey", "1910-10-01" },
        { "Turkey", "1978-10-15" },
        { "US/Michigan", "1905-01-01" },
        { "US/Samoa", "1911-01-01" },
        { "W-SU", "1916-07-03" },
        { "W-SU", "1930-06-21" },
        { "W-SU", "1992-01-19" },
        { "W-SU", "2011-03-27" }
    };

    LanguageType eLang = LANGUAGE_ENGLISH_US;
    SvNumberFormatter aFormatter(m_xContext, eLang);

    for (size_t i=0; i < SAL_N_ELEMENTS(aData); ++i)
    {
        checkDateInput( aFormatter, aData[i][0], aData[i][1]);
    }
}

void Test::checkDateInput( SvNumberFormatter& rFormatter, const char* pTimezone, const char* pIsoDate )
{
    icu::TimeZone::adoptDefault( icu::TimeZone::createTimeZone( pTimezone));
    OUString aDate( OUString::createFromAscii(pIsoDate));
    sal_uInt32 nIndex = 0;
    double fVal = 0.0;
    bool bVal = rFormatter.IsNumberFormat( aDate, nIndex, fVal);
    CPPUNIT_ASSERT_MESSAGE( OString("Date not recognized: " +
                OString(pTimezone) + " " + OString(pIsoDate)).getStr(), bVal);
    CPPUNIT_ASSERT_MESSAGE("Format parsed is not date.",
            (rFormatter.GetType(nIndex) & css::util::NumberFormat::DATE));
    OUString aOutString;
    Color *pColor;
    rFormatter.GetOutputString( fVal, nIndex, aOutString, &pColor);
    CPPUNIT_ASSERT_EQUAL( aDate, aOutString);
}

void Test::testIsNumberFormat()
{
    LanguageType eLang = LANGUAGE_ENGLISH_US;
    SvNumberFormatter aFormatter(m_xContext, eLang);

    struct NumberFormatData
    {
        const char* pFormat;
        bool bIsNumber;
    } aTests[] = {
        { "20.3", true },
        { "2", true },
        { "test", false },
        { "Jan1", false },
        { "Jan1 2000", true },
        { "Jan 1", true },
        { "Jan 1 2000", true}
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
        sal_uInt32 nIndex = 0;
        double nNumber = 0;
        OUString aString = OUString::createFromAscii(aTests[i].pFormat);
        bool bIsNumber = aFormatter.IsNumberFormat(aString, nIndex, nNumber);
        CPPUNIT_ASSERT_EQUAL(aTests[i].bIsNumber, bIsNumber);

    }

    // Test Spanish "mar" short name ambiguity, day "martes" or month "marzo".
    // Day of week names are only parsed away, not evaluated if they actually
    // correspond to the date given.
    struct SpanishDate
    {
        const char* mpInput;
        bool mbNumber;
        const char* mpOutput;
    } aSpanishTests[] = {
        { "22/11/1999", true, "22/11/1999" },
        { "Lun 22/11/1999", true, "22/11/1999" },
        { "Mar 22/11/1999", true, "22/11/1999" },
        { "Abr 22/11/1999", false, "" },            // month name AND numeric month don't go along
        { "Lun Mar 22/11/1999", false, "" },        // month name AND numeric month don't go along
        { "Mar Mar 22/11/1999", false, "" },        // month name AND numeric month don't go along
        { "Lun Mar 22 1999", true, "22/03/1999" },
        { "Mar Mar 22 1999", true, "22/03/1999" },
        { "Mar Lun 22 1999", false, "" }            // day name only at the beginning (could change?)
    };

    sal_uInt32 nIndex = aFormatter.GetFormatIndex( NF_DATE_SYS_DDMMYYYY, LANGUAGE_SPANISH);
    for (size_t i = 0; i < SAL_N_ELEMENTS(aSpanishTests); ++i)
    {
        double fNumber = 0;
        OUString aString = OUString::createFromAscii( aSpanishTests[i].mpInput);
        bool bIsNumber = aFormatter.IsNumberFormat( aString, nIndex, fNumber);
        CPPUNIT_ASSERT_EQUAL( aSpanishTests[i].mbNumber, bIsNumber);
        if (aSpanishTests[i].mbNumber)
        {
            Color* pColor;
            aFormatter.GetOutputString( fNumber, nIndex, aString, &pColor);
            CPPUNIT_ASSERT_EQUAL( OUString::createFromAscii( aSpanishTests[i].mpOutput), aString);
        }
    }
}

void Test::testUserDefinedNumberFormats()
{
    LanguageType eLang = LANGUAGE_ENGLISH_US;
    OUString sCode, sExpected;
    SvNumberFormatter aFormatter(m_xContext, eLang);
    {  // tdf#97835: suppress decimal separator
        sCode = "0.##\" m\"";
        sExpected = "12 m";
        checkPreviewString(aFormatter, sCode, 12.0, eLang, sExpected);
    }
    {  // tdf#61996: skip quoted text
        sCode = "0.00\" ;\"";
        sExpected = "-12.00 ;";
        checkPreviewString(aFormatter, sCode, -12.0, eLang, sExpected);
    }
    {  // tdf#995339: detect SSMM as second minute
        sCode = "SS:MM:HH DD/MM/YY";
        sExpected = "54:23:03 02/01/00";
        checkPreviewString(aFormatter, sCode, M_PI, eLang, sExpected);
    }
    {  // tdf#99996: better algorithm for fraction representation
        sCode = "# ?/???";
        sExpected = "-575 540/697";
        checkPreviewString(aFormatter, sCode, -575.774749601315, eLang, sExpected);
    }
    {  // tdf#100594: forced denominator
        sCode = "# ?/100";
        sExpected = " 6/100";
        checkPreviewString(aFormatter, sCode, 0.06, eLang, sExpected);
    }
    {  // tdf#100754: forced denominator with text after fraction
        sCode = "# ?/16\" inch\"";
        sExpected = "2 6/16 inch";
        checkPreviewString(aFormatter, sCode, 2.379, eLang, sExpected);
    }
    {  // tdf#100842: text before/after fraction
        sCode = "\"before \"?/?\" after\"";
        sExpected = "before 11/9 after";
        checkPreviewString(aFormatter, sCode, 1.2345667, eLang, sExpected);
        sCode = "\"before \"# ?/?\" after\"";
        sExpected = "before 1 2/9 after";
        checkPreviewString(aFormatter, sCode, 1.2345667, eLang, sExpected);
        sCode = "\"before \"0.0\"inside\"0E+0\"middle\"0\" after\"";
        sExpected = "before 1.2inside3E+0middle4 after";
        checkPreviewString(aFormatter, sCode, 12345.667, eLang, sExpected);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
