/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/drawing/XShapes.hpp>

#include <svx/svdpage.hxx>
#include <svx/svdotext.hxx>

using namespace css;

/// Shape / SdrObject import and export tests
class ShapeImportExportTest : public SdModelTestBaseXML
{
public:
    void testTextDistancesOOXML();
    void testTextDistancesOOXML_LargerThanTextAreaSpecialCase();
    void testTextDistancesOOXML_Export();

    CPPUNIT_TEST_SUITE(ShapeImportExportTest);
    CPPUNIT_TEST(testTextDistancesOOXML);
    CPPUNIT_TEST(testTextDistancesOOXML_LargerThanTextAreaSpecialCase);
    CPPUNIT_TEST(testTextDistancesOOXML_Export);
    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
        XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    }
};

namespace
{
SdrObject* searchObject(SdrPage const* pPage, std::u16string_view rName)
{
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        SdrObject* pCurrent = pPage->GetObj(i);
        if (pCurrent->GetName() == rName)
            return pCurrent;
    }
    return nullptr;
}
}

/* Test text distances (insets) */
void ShapeImportExportTest::testTextDistancesOOXML()
{
    ::sd::DrawDocShellRef xDocShell
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/TextDistancesInsets1.pptx"), PPTX);

    SdrPage const* pPage = GetPage(1, xDocShell);
    // Bottom Margin = 4cm
    {
        std::array<std::u16string_view, 3> aObjectDesc = {
            u"T, BM - 4cm",
            u"M, BM - 4cm",
            u"B, BM - 4cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(-1292), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(2708), pTextObj->GetTextLowerDistance());
        }
    }

    // Bottom Margin = 1cm
    {
        std::array<std::u16string_view, 3> aObjectDesc = {
            u"T, BM - 1cm",
            u"M, BM - 1cm",
            u"B, BM - 1cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(1000), pTextObj->GetTextLowerDistance());
        }
    }

    // Top + Bottom Margin = 1cm
    {
        std::array<std::u16string_view, 3> aObjectDesc = {
            u"T, TM+BM - 1cm",
            u"M, TM+BM - 1cm",
            u"B, TM+BM - 1cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextLowerDistance());
        }
    }

    // No margin - Top + Bottom = 0cm
    {
        std::array<std::u16string_view, 3> aObjectDesc = {
            u"T",
            u"M",
            u"B",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextLowerDistance());
        }
    }

    // Top Margin = 1cm
    {
        std::array<std::u16string_view, 3> aObjectDesc = {
            u"T, TM - 1cm",
            u"M, TM - 1cm",
            u"B, TM - 1cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(1000), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextLowerDistance());
        }
    }

    // Top Margin = 4cm
    {
        std::array<std::u16string_view, 3> aObjectDesc = {
            u"T, TM - 4cm",
            u"M, TM - 4cm",
            u"B, TM - 4cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(2708), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(-1292), pTextObj->GetTextLowerDistance());
        }
    }

    xDocShell->DoClose();
}

/* Test text distances (insets) variants where top+bottom margin > text area*/
void ShapeImportExportTest::testTextDistancesOOXML_LargerThanTextAreaSpecialCase()
{
    ::sd::DrawDocShellRef xDocShell
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/TextDistancesInsets2.pptx"), PPTX);

    SdrPage const* pPage = GetPage(1, xDocShell);

    // Top/Bottom 0cm/3cm, 1cm/4cm, 4cm/7cm - all should be converted to the same value in LO
    {
        std::array<std::u16string_view, 9> aObjectNames = {
            u"T_0_3", u"M_0_3", u"B_0_3", u"T_1_4", u"M_1_4",
            u"B_1_4", u"T_4_7", u"M_4_7", u"B_4_7",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(-792), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(2208), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 0cm/2cm, 1cm/3cm, 4cm/6cm - all should be converted to the same value in LO
    {
        std::array<std::u16string_view, 9> aObjectNames = {
            u"T_0_2", u"M_0_2", u"B_0_2", u"T_1_3", u"M_1_3",
            u"B_1_3", u"T_4_6", u"M_4_6", u"B_4_6",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(-292), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(1708), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 2cm/2cm, 3cm/3cm, 4cm/4cm - all should be converted to the same value in LO
    {
        std::array<std::u16string_view, 9> aObjectNames = {
            u"T_2_2", u"M_2_2", u"B_2_2", u"T_3_3", u"M_3_3",
            u"B_3_3", u"T_4_4", u"M_4_4", u"B_4_4",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 2cm/0cm, 3cm/1cm, 6cm/4cm - all should be converted to the same value in LO
    {
        std::array<std::u16string_view, 9> aObjectNames = {
            u"T_2_0", u"M_2_0", u"B_2_0", u"T_3_1", u"M_3_1",
            u"B_3_1", u"T_6_4", u"M_6_4", u"B_6_4",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(1708), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(-292), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 3cm/0cm, 4cm/1cm, 7cm/4cm - all should be converted to the same value in LO
    {
        std::array<std::u16string_view, 9> aObjectNames = {
            u"T_3_0", u"M_3_0", u"B_3_0", u"T_4_1", u"M_4_1",
            u"B_4_1", u"T_7_4", u"M_7_4", u"B_7_4",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObject(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(2208), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(-792), pTextObj->GetTextLowerDistance());
        }
    }

    xDocShell->DoClose();
}

/* Test export of text distances (insets) - conversion back of special case */
void ShapeImportExportTest::testTextDistancesOOXML_Export()
{
    ::sd::DrawDocShellRef xDocShell
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/TextDistancesInsets3.pptx"), PPTX);

    utl::TempFile aTempFile;
    xDocShell = saveAndReload(xDocShell.get(), PPTX, &aTempFile);
    xDocShell->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(aTempFile, "ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    //Check shape Top/Bottom - 0cm, 4cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:nvSpPr/p:cNvPr", "name", "Text_TB_0_4");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr",
                     { { "tIns", "0" }, { "bIns", "1439640" } });

    //Check shape Top/Bottom - 4cm, 0cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:nvSpPr/p:cNvPr", "name", "Text_TB_4_0");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr",
                     { { "tIns", "1439640" }, { "bIns", "0" } });

    //Check shape Top/Bottom - 0cm, 3cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:nvSpPr/p:cNvPr", "name", "Text_TB_0_3");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:txBody/a:bodyPr",
                     { { "tIns", "0" }, { "bIns", "1079640" } });

    //Check shape Top/Bottom - 2cm, 1cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:nvSpPr/p:cNvPr", "name", "Text_TB_2_1");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:txBody/a:bodyPr",
                     { { "tIns", "720000" }, { "bIns", "360000" } });

    //Check shape Top/Bottom - 0cm, 2.5cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_0_2.5");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:txBody/a:bodyPr",
                     { { "tIns", "0" }, { "bIns", "899640" } });

    //Check shape Top/Bottom - 0cm, 2cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[6]/p:nvSpPr/p:cNvPr", "name", "Text_TB_0_2");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[6]/p:txBody/a:bodyPr",
                     { { "tIns", "0" }, { "bIns", "720000" } });

    //Check shape Top/Bottom - 0cm, 1.5cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[7]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_0_1.5");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[7]/p:txBody/a:bodyPr",
                     { { "tIns", "0" }, { "bIns", "540000" } });

    //Check shape Top/Bottom - 3cm, 0cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[8]/p:nvSpPr/p:cNvPr", "name", "Text_TB_3_0");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[8]/p:txBody/a:bodyPr",
                     { { "tIns", "1079640" }, { "bIns", "0" } });

    //Check shape Top/Bottom - 2.5cm, 0cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[9]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_2.5_0");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[9]/p:txBody/a:bodyPr",
                     { { "tIns", "899640" }, { "bIns", "0" } });

    //Check shape Top/Bottom - 2cm, 0cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[10]/p:nvSpPr/p:cNvPr", "name", "Text_TB_2_0");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[10]/p:txBody/a:bodyPr",
                     { { "tIns", "720000" }, { "bIns", "0" } });

    //Check shape Top/Bottom - 1.5cm, 0cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[11]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_1.5_0");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[11]/p:txBody/a:bodyPr",
                     { { "tIns", "540000" }, { "bIns", "0" } });

    //Check shape Top/Bottom - 1cm, 2cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[12]/p:nvSpPr/p:cNvPr", "name", "Text_TB_1_2");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[12]/p:txBody/a:bodyPr",
                     { { "tIns", "360000" }, { "bIns", "720000" } });

    //Check shape Top/Bottom - 2cm, 1.5cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[13]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_2_1.5");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[13]/p:txBody/a:bodyPr",
                     { { "tIns", "720000" }, { "bIns", "540000" } });

    //Check shape Top/Bottom - 1.5cm, 2cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[14]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_1.5_2");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[14]/p:txBody/a:bodyPr",
                     { { "tIns", "540000" }, { "bIns", "720000" } });

    //Check shape Top/Bottom - 2cm, 1.75cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[15]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_2_1.75");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[15]/p:txBody/a:bodyPr",
                     { { "tIns", "720000" }, { "bIns", "630000" } });

    //Check shape Top/Bottom - 1.75cm, 2cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[16]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_1.75_2");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[16]/p:txBody/a:bodyPr",
                     { { "tIns", "630000" }, { "bIns", "720000" } });

    //Check shape Top/Bottom - 2cm, 2cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[17]/p:nvSpPr/p:cNvPr", "name", "Text_TB_2_2");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[17]/p:txBody/a:bodyPr",
                     { { "tIns", "720000" }, { "bIns", "720000" } });

    //Check shape Top/Bottom - 1cm, 1cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[18]/p:nvSpPr/p:cNvPr", "name", "Text_TB_1_1");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[18]/p:txBody/a:bodyPr",
                     { { "tIns", "720000" }, { "bIns", "720000" } });

    //Check shape Top/Bottom - 0.5cm, 0.5cm
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[19]/p:nvSpPr/p:cNvPr", "name",
                "Text_TB_0.5_0.5");
    assertXPathAttrs(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[19]/p:txBody/a:bodyPr",
                     { { "tIns", "180000" }, { "bIns", "180000" } });
}

CPPUNIT_TEST_SUITE_REGISTRATION(ShapeImportExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
