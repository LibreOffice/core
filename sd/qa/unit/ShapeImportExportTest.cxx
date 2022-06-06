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

#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>

#include <vcl/scheduler.hxx>
#include <osl/thread.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <sfx2/request.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsPageSelector.hxx>
#include <undo/undomanager.hxx>
#include <GraphicViewShell.hxx>
#include <chrono>
#include <sdpage.hxx>
#include <comphelper/base64.hxx>
#include <LayerTabBar.hxx>
#include <vcl/event.hxx>
#include <vcl/keycodes.hxx>
#include <svx/svdoashp.hxx>
#include <tools/gen.hxx>
#include <svx/view3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sdmetitm.hxx>

using namespace css;

/// Shape / SdrObject import and export tests
class ShapeImportExportTest : public SdModelTestBaseXML
{
public:
    void testTextDistancesOOXML();
    void testTextDistancesOOXML_LargerThanTextAreaSpecialCase();

    CPPUNIT_TEST_SUITE(ShapeImportExportTest);
    CPPUNIT_TEST(testTextDistancesOOXML);
    CPPUNIT_TEST(testTextDistancesOOXML_LargerThanTextAreaSpecialCase);
    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
    }
};

namespace
{
SdrObject* searchObjectByDescription(SdrPage const* pPage, OUString const& rDescription)
{
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        SdrObject* pCurrent = pPage->GetObj(i);
        if (pCurrent->GetDescription() == rDescription)
            return pCurrent;
    }
    return nullptr;
}

SdrObject* searchObjectByName(SdrPage const* pPage, OUString const& rName)
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
        std::array<OUString, 3> aObjectDesc = {
            u"T, BM - 4cm",
            u"M, BM - 4cm",
            u"B, BM - 4cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByDescription(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(-1292), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(2708), pTextObj->GetTextLowerDistance());
        }
    }

    // Bottom Margin = 1cm
    {
        std::array<OUString, 3> aObjectDesc = {
            u"T, BM - 1cm",
            u"M, BM - 1cm",
            u"B, BM - 1cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByDescription(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(1000), pTextObj->GetTextLowerDistance());
        }
    }

    // Top + Bottom Margin = 1cm
    {
        std::array<OUString, 3> aObjectDesc = {
            u"T, TM+BM - 1cm",
            u"M, TM+BM - 1cm",
            u"B, TM+BM - 1cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByDescription(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextLowerDistance());
        }
    }

    // No margin - Top + Bottom = 0cm
    {
        std::array<OUString, 3> aObjectDesc = {
            u"T",
            u"M",
            u"B",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByDescription(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextLowerDistance());
        }
    }

    // Top Margin = 1cm
    {
        std::array<OUString, 3> aObjectDesc = {
            u"T, TM - 1cm",
            u"M, TM - 1cm",
            u"B, TM - 1cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByDescription(pPage, rString));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(1000), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), pTextObj->GetTextLowerDistance());
        }
    }

    // Top Margin = 4cm
    {
        std::array<OUString, 3> aObjectDesc = {
            u"T, TM - 4cm",
            u"M, TM - 4cm",
            u"B, TM - 4cm",
        };

        for (auto const& rString : aObjectDesc)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByDescription(pPage, rString));
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
        std::array<OUString, 9> aObjectNames = {
            u"T_0_3", u"M_0_3", u"B_0_3", u"T_1_4", u"M_1_4",
            u"B_1_4", u"T_4_7", u"M_4_7", u"B_4_7",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByName(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(-792), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(2208), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 0cm/2cm, 1cm/3cm, 4cm/6cm - all should be converted to the same value in LO
    {
        std::array<OUString, 9> aObjectNames = {
            u"T_0_2", u"M_0_2", u"B_0_2", u"T_1_3", u"M_1_3",
            u"B_1_3", u"T_4_6", u"M_4_6", u"B_4_6",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByName(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(-292), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(1708), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 2cm/2cm, 3cm/3cm, 4cm/4cm - all should be converted to the same value in LO
    {
        std::array<OUString, 9> aObjectNames = {
            u"T_2_2", u"M_2_2", u"B_2_2", u"T_3_3", u"M_3_3",
            u"B_3_3", u"T_4_4", u"M_4_4", u"B_4_4",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByName(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(708), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 2cm/0cm, 3cm/1cm, 6cm/4cm - all should be converted to the same value in LO
    {
        std::array<OUString, 9> aObjectNames = {
            u"T_2_0", u"M_2_0", u"B_2_0", u"T_3_1", u"M_3_1",
            u"B_3_1", u"T_6_4", u"M_6_4", u"B_6_4",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByName(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(1708), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(-292), pTextObj->GetTextLowerDistance());
        }
    }

    // Top/Bottom 3cm/0cm, 4cm/1cm, 7cm/4cm - all should be converted to the same value in LO
    {
        std::array<OUString, 9> aObjectNames = {
            u"T_3_0", u"M_3_0", u"B_3_0", u"T_4_1", u"M_4_1",
            u"B_4_1", u"T_7_4", u"M_7_4", u"B_7_4",
        };

        for (auto const& rName : aObjectNames)
        {
            auto* pTextObj = dynamic_cast<SdrTextObj*>(searchObjectByName(pPage, rName));
            CPPUNIT_ASSERT(pTextObj);
            CPPUNIT_ASSERT_EQUAL(tools::Long(2208), pTextObj->GetTextUpperDistance());
            CPPUNIT_ASSERT_EQUAL(tools::Long(-792), pTextObj->GetTextLowerDistance());
        }
    }

    xDocShell->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ShapeImportExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
