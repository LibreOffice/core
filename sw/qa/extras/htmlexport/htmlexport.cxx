/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <rtl/byteseq.hxx>

#include <swmodule.hxx>
#include <usrpref.hxx>

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

using namespace rtl;

class Test : public SwModelTestBase
{
private:
    FieldUnit m_eUnit;

public:
    Test() :
        SwModelTestBase("/sw/qa/extras/htmlexport/data/", "HTML (StarWriter)"),
        m_eUnit(FUNIT_NONE)
    {}

protected:
    htmlDocPtr parseHtml()
    {
        SvFileStream aFileStream(m_aTempFile.GetURL(), STREAM_READ);
        sal_Size nSize = aFileStream.remainingSize();

        ByteSequence aBuffer(nSize + 1);
        aFileStream.Read(aBuffer.getArray(), nSize);

        aBuffer[nSize] = 0;
        return htmlParseDoc(reinterpret_cast<xmlChar*>(aBuffer.getArray()), NULL);
    }

private:
    bool mustCalcLayoutOf(const char* filename) SAL_OVERRIDE
    {
        return OString(filename) != "fdo62336.docx";
    }

    bool mustTestImportOf(const char* filename) const SAL_OVERRIDE
    {
        return OString(filename) != "fdo62336.docx";
    }

    void preTest(const char* filename) SAL_OVERRIDE
    {
        if (getTestName() == "testExportOfImagesWithSkipImageEnabled")
            setFilterOptions("SkipImages");
        else
            setFilterOptions("");

        if (OString(filename) == "charborder.odt" && SW_MOD())
        {
            // FIXME if padding-top gets exported as inches, not cms, we get rounding errors.
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            m_eUnit = pPref->GetMetric();
            pPref->SetMetric(FUNIT_CM);
        }
    }

    void postTest(const char* filename) SAL_OVERRIDE
    {
        if (OString(filename) == "charborder.odt" && SW_MOD())
        {
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            pPref->SetMetric(m_eUnit);
        }
    }
};

#define DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo62336, "fdo62336.docx")
{
    // The problem was essentially a crash during table export as docx/rtf/html
    // If either of no-calc-layout or no-test-import is enabled, the crash does not occur
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testCharacterBorder, "charborder.odt")
{

    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // Different Border
    {
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x6666FF,12,12,12,3,37), getProperty<table::BorderLine2>(xRun,"CharTopBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF9900,0,99,0,2,99), getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF0000,0,169,0,1,169), getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x0000FF,0,169,0,0,169), getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Different Padding
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(450), getProperty<sal_Int32>(xRun,"CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(550), getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // No shadow
}

#define DECLARE_HTMLEXPORT_TEST(TestName, filename) DECLARE_SW_EXPORT_TEST(TestName, filename, Test)

DECLARE_HTMLEXPORT_TEST(testExportOfImages, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml();
    if (pDoc)
    {
        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/p/img", 1);
    }
}

DECLARE_HTMLEXPORT_TEST(testExportOfImagesWithSkipImageEnabled, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml();
    if (pDoc)
    {
        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/p/img", 0);
    }
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
