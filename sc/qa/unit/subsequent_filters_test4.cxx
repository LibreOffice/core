/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_fonts.h>

#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <dbdata.hxx>
#include <dbdocfun.hxx>
#include <inputopt.hxx>
#include <globalnames.hxx>
#include <userdat.hxx>
#include <scmod.hxx>
#include <stlsheet.hxx>
#include <colorscale.hxx>
#include <patattr.hxx>
#include <scitems.hxx>
#include <editutil.hxx>
#include <attrib.hxx>
#include <fillinfo.hxx>
#include <scopetools.hxx>
#include <stlpool.hxx>
#include <postit.hxx>
#include <sortparam.hxx>
#include <undomanager.hxx>
#include <tabprotection.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <orcusfilters.hxx>
#include <filter.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Filters test */

class ScFiltersTest4 : public ScModelTestBase
{
public:
    ScFiltersTest4()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }

protected:
    void testImportCrash(const char* rFileName)
    {
        createScDoc(rFileName);
        ScDocument* pDoc = getScDoc();
        pDoc->CalcAll(); // perform hard re-calculation.
    }
};

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testPasswordNew)
{
    //tests opening a file with new password algorithm
    createScDoc("ods/password.ods", /*pPassword*/ "test");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testPasswordOld)
{
    //tests opening a file with old password algorithm
    createScDoc("ods/passwordOld.ods", /*pPassword*/ "test");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testPasswordWrongSHA)
{
    //tests opening a file wrongly using the new password algorithm
    //in a sxc with the key algorithm missing
    createScDoc("ods/passwordWrongSHA.ods", /*pPassword*/ "test");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testControlImport)
{
    createScDoc("xlsx/singlecontrol.xlsx");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf131575)
{
    // It expectedly fails to load normally
    CPPUNIT_ASSERT_ASSERTION_FAIL(createScDoc("xlsx/tdf131575.xlsx"));

    // importing it must succeed with RepairPackage set to true.
    uno::Sequence<beans::PropertyValue> aParams
        = { comphelper::makePropertyValue(u"RepairPackage"_ustr, true) };
    loadWithParams(createFileURL(u"xlsx/tdf131575.xlsx"), aParams);
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"ETAT DES SORTIES"_ustr, pDoc->GetString(1, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf76115)
{
    // It expectedly fails to load normally
    CPPUNIT_ASSERT_ASSERTION_FAIL(createScDoc("xlsx/tdf76115.xlsx"));

    // importing it must succeed with RepairPackage set to true.
    uno::Sequence<beans::PropertyValue> aParams
        = { comphelper::makePropertyValue(u"RepairPackage"_ustr, true) };
    loadWithParams(createFileURL(u"xlsx/tdf76115.xlsx"), aParams);
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"Filial"_ustr, pDoc->GetString(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testLegacyOptionButtonGroupBox)
{
    createScDoc("xls/tdf79542_radioGroupBox.xls");
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);

    OUString sGroupName;
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(1),
                                                         UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(),
                                                     uno::UNO_QUERY_THROW);
    // The radio buttons are grouped by GroupBoxes - so the name comes from the group shape name
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName;
    CPPUNIT_ASSERT_EQUAL(u"Casella di gruppo 1"_ustr, sGroupName);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testActiveXOptionButtonGroup)
{
    createScDoc("xlsx/tdf111980_radioButtons.xlsx");
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);

    OUString sGroupName;
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(),
                                                     uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName;
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, sGroupName);

    // Optionbuttons (without Group names) were not grouped.
    // The two optionbuttons should have the same auto-generated group name.
    OUString sGroupName2; //ActiveX controls
    xControlShape.set(xIA_DrawPage->getByIndex(2), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName2;
    CPPUNIT_ASSERT_EQUAL(false, sGroupName2.isEmpty());

    OUString sGroupName3;
    xControlShape.set(xIA_DrawPage->getByIndex(3), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName3;
    CPPUNIT_ASSERT_EQUAL(sGroupName2, sGroupName3);
    CPPUNIT_ASSERT(sGroupName != sGroupName3);

    OUString sGroupName4; //Form controls
    xControlShape.set(xIA_DrawPage->getByIndex(4), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName4;
    CPPUNIT_ASSERT_EQUAL(false, sGroupName4.isEmpty());

    OUString sGroupName5;
    xControlShape.set(xIA_DrawPage->getByIndex(5), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName5;
    CPPUNIT_ASSERT_EQUAL(sGroupName4, sGroupName5);
    CPPUNIT_ASSERT(sGroupName2 != sGroupName5);
    CPPUNIT_ASSERT(sGroupName != sGroupName5);

    OUString sGroupName7; //Form radiobutton autogrouped by GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(7), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName7;
    CPPUNIT_ASSERT_EQUAL(u"autoGroup_Group Box 7"_ustr, sGroupName7);

    OUString sGroupName8;
    xControlShape.set(xIA_DrawPage->getByIndex(8), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName8;
    CPPUNIT_ASSERT_EQUAL(sGroupName7, sGroupName8);
    CPPUNIT_ASSERT(sGroupName4 != sGroupName8);
    CPPUNIT_ASSERT(sGroupName2 != sGroupName8);
    CPPUNIT_ASSERT(sGroupName != sGroupName8);

    OUString sGroupName9; //Form radiobutton not fully inside GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(9), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName9;
    CPPUNIT_ASSERT_EQUAL(sGroupName4, sGroupName9);

    OUString sGroupName10; //ActiveX unaffected by GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(10), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue(u"GroupName"_ustr) >>= sGroupName10;
    CPPUNIT_ASSERT_EQUAL(sGroupName, sGroupName10);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testChartImportODS)
{
    createScDoc("ods/chart-import-basic.ods");

    ScDocument* pDoc = getScDoc();

    // Ensure that the document contains "Empty", "Chart", "Data" and "Title" sheets in this exact order.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 4 sheets in this document.", sal_Int16(4),
                                 pDoc->GetTableCount());
    OUString aName;
    pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(u"Empty"_ustr, aName);
    pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Chart"_ustr, aName);
    pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(u"Data"_ustr, aName);
    pDoc->GetName(3, aName);
    CPPUNIT_ASSERT_EQUAL(u"Title"_ustr, aName);

    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(*pDoc, *pOleObj);

    CPPUNIT_ASSERT_MESSAGE("Data series title cell not found.",
                           aRanges.Contains(ScAddress(1, 0, 3))); // B1 on Title
    CPPUNIT_ASSERT_MESSAGE("Data series label range not found.",
                           aRanges.Contains(ScRange(0, 1, 2, 0, 3, 2))); // A2:A4 on Data
    CPPUNIT_ASSERT_MESSAGE("Data series value range not found.",
                           aRanges.Contains(ScRange(1, 1, 2, 1, 3, 2))); // B2:B4 on Data
}

#if HAVE_MORE_FONTS
CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testChartImportXLS)
{
    createScDoc("xls/chartx.xls");

    ScDocument* pDoc = getScDoc();

    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 0);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    CPPUNIT_ASSERT_EQUAL(tools::Long(11137), pOleObj->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT(8640L > pOleObj->GetLogicRect().getOpenHeight());
}
#endif

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testNumberFormatHTML)
{
    setImportFilterName(u"calc_HTML_WebQuery"_ustr);
    createScDoc("html/numberformat.html");

    ScDocument* pDoc = getScDoc();

    // Check the header just in case.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", u"Product"_ustr,
                                 pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", u"Price"_ustr,
                                 pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", u"Note"_ustr,
                                 pDoc->GetString(2, 0, 0));

    // B2 should be imported as a value cell.
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect value.", 199.98, pDoc->GetValue(1, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testNumberFormatCSV)
{
    createScDoc("csv/numberformat.csv");

    ScDocument* pDoc = getScDoc();

    // Check the header just in case.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", u"Product"_ustr,
                                 pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", u"Price"_ustr,
                                 pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", u"Note"_ustr,
                                 pDoc->GetString(2, 0, 0));

    // B2 should be imported as a value cell.
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect value.", 199.98, pDoc->GetValue(1, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCellAnchoredShapesODS)
{
    createScDoc("ods/cell-anchored-shapes.ods");

    // There are two cell-anchored objects on the first sheet.
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 2 objects.", static_cast<size_t>(2), nCount);
    for (size_t i = 0; i < nCount; ++i)
    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
        ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
        CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
        CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                               !pData->getShapeRect().IsEmpty());
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCellAnchoredHiddenShapesXLSX)
{
    createScDoc("xlsx/cell-anchored-hidden-shapes.xlsx");

    // There are two cell-anchored objects on the first sheet.
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_MESSAGE("There should be 2 shapes.", !(nCount == 2));

    SdrObject* pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    CPPUNIT_ASSERT_MESSAGE(
        "The shape having same twocellanchor from and to attribute values, is visible.",
        !pObj->IsVisible());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testRowHeightODS)
{
    createScDoc("ods/row-height-import.ods");

    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument* pDoc = getScDoc();

    // The first 3 rows have manual heights.
    int nHeight = pDoc->GetRowHeight(nRow, nTab, false);
    bool bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(600, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);
    nHeight = pDoc->GetRowHeight(++nRow, nTab, false);
    bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(1200, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);
    nHeight = pDoc->GetRowHeight(++nRow, nTab, false);
    bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(1800, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);

    // This one should have an automatic row height.
    bManual = pDoc->IsManualRowHeight(++nRow, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    // Followed by a row with manual height.
    nHeight = pDoc->GetRowHeight(++nRow, nTab, false);
    bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(2400, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);

    // And all the rest should have automatic heights.
    bManual = pDoc->IsManualRowHeight(++nRow, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    bManual = pDoc->IsManualRowHeight(pDoc->MaxRow(), nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testRichTextContentODS)
{
    createScDoc("ods/rich-text-cells.ods");
    ScDocument* pDoc = getScDoc();

    OUString aTabName;
    CPPUNIT_ASSERT_MESSAGE("Failed to get the name of the first sheet.",
                           pDoc->GetName(0, aTabName));

    // All tested cells are in the first column.
    ScAddress aPos(0, 0, 0);

    // Normal simple string with no formatting.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"Normal"_ustr, pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Normal string with bold applied to the whole cell.
    {
        aPos.IncRow();
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
        CPPUNIT_ASSERT_EQUAL(u"All bold"_ustr, pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));
        const ScPatternAttr* pAttr = pDoc->GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
        CPPUNIT_ASSERT_MESSAGE("Failed to get cell attribute.", pAttr);
        const SvxWeightItem& rWeightItem = pAttr->GetItem(ATTR_FONT_WEIGHT);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeightItem.GetWeight());
    }

    // This cell has an unformatted but multi-line content. Multi-line text is
    // stored in edit cell even if it has no formatting applied.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    const EditTextObject* pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pEditText->GetParagraphCount());
    OUString aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(u"one"_ustr, aParaText);
    aParaText = pEditText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(u"two"_ustr, aParaText);
    aParaText = pEditText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(u"three"_ustr, aParaText);

    // Cell with sheet name field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Sheet name is "));
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.",
                           pEditText->HasField(text::textfield::Type::TABLE));
    CPPUNIT_ASSERT_EQUAL(u"Sheet name is Test."_ustr, ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(u"Sheet name is ?."_ustr, ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with URL field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("URL: "));
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.",
                           pEditText->HasField(text::textfield::Type::URL));
    CPPUNIT_ASSERT_EQUAL(u"URL: http://libreoffice.org"_ustr,
                         ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(u"URL: http://libreoffice.org"_ustr,
                         ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with Date field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Date: "));
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.",
                           pEditText->HasField(text::textfield::Type::DATE));
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with pDoc->",
                           ScEditUtil::GetString(*pEditText, pDoc).indexOf("/20") > 0);
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with NULL.",
                           ScEditUtil::GetString(*pEditText, nullptr).indexOf("/20") > 0);

    // Cell with DocInfo title field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Title: "));
    CPPUNIT_ASSERT_MESSAGE("DocInfo title field item not found.",
                           pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));
    CPPUNIT_ASSERT_EQUAL(u"Title: Test Document"_ustr, ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(u"Title: ?"_ustr, ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with sentence with both bold and italic sequences.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(u"Sentence with bold and italic."_ustr, aParaText);
    std::vector<EECharAttrib> aAttribs;
    pEditText->GetCharAttribs(0, aAttribs);
    {
        bool bHasBold = false, bHasItalic = false;
        for (const auto& rAttrib : aAttribs)
        {
            OUString aSeg = aParaText.copy(rAttrib.nStart, rAttrib.nEnd - rAttrib.nStart);
            const SfxPoolItem* pAttr = rAttrib.pAttr;
            if (aSeg == "bold" && pAttr->Which() == EE_CHAR_WEIGHT && !bHasBold)
            {
                const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>(*pAttr);
                bHasBold = (rItem.GetWeight() == WEIGHT_BOLD);
            }
            else if (aSeg == "italic" && pAttr->Which() == EE_CHAR_ITALIC && !bHasItalic)
            {
                const SvxPostureItem& rItem = static_cast<const SvxPostureItem&>(*pAttr);
                bHasItalic = (rItem.GetPosture() == ITALIC_NORMAL);
            }
        }
        CPPUNIT_ASSERT_MESSAGE("This sentence is expected to have both bold and italic sequences.",
                               bHasBold);
        CPPUNIT_ASSERT_MESSAGE("This sentence is expected to have both bold and italic sequences.",
                               bHasItalic);
    }

    // Cell with multi-line content with formatting applied.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(u"bold"_ustr, aParaText);
    aParaText = pEditText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(u"italic"_ustr, aParaText);
    aParaText = pEditText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(u"underlined"_ustr, aParaText);

    // first line is bold.
    pEditText->GetCharAttribs(0, aAttribs);
    {
        bool bHasBold
            = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
                  return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT
                         && static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight()
                                == WEIGHT_BOLD;
              });
        CPPUNIT_ASSERT_MESSAGE("First line should be bold.", bHasBold);
    }

    // second line is italic.
    pEditText->GetCharAttribs(1, aAttribs);
    bool bHasItalic
        = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
              return rAttrib.pAttr->Which() == EE_CHAR_ITALIC
                     && static_cast<const SvxPostureItem&>(*rAttrib.pAttr).GetPosture()
                            == ITALIC_NORMAL;
          });
    CPPUNIT_ASSERT_MESSAGE("Second line should be italic.", bHasItalic);

    // third line is underlined.
    pEditText->GetCharAttribs(2, aAttribs);
    bool bHasUnderline
        = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
              return rAttrib.pAttr->Which() == EE_CHAR_UNDERLINE
                     && static_cast<const SvxUnderlineItem&>(*rAttrib.pAttr).GetLineStyle()
                            == LINESTYLE_SINGLE;
          });
    CPPUNIT_ASSERT_MESSAGE("Second line should be underlined.", bHasUnderline);

    // URL with formats applied.  For now, we'll check whether or not the
    // field objects gets imported.  Later we should add checks for the
    // formats.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.",
                           pEditText->HasField(text::textfield::Type::URL));

    // Sheet name with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.",
                           pEditText->HasField(text::textfield::Type::TABLE));

    // Date with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.",
                           pEditText->HasField(text::textfield::Type::DATE));

    // Document title with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.",
                           pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));

    // URL for a file in the same directory. It should be converted into an absolute URL on import.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get the URL data.", text::textfield::Type::URL,
                                 pData->GetClassId());
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT_MESSAGE("URL is not absolute with respect to the file system.",
                           pURLData->GetURL().startsWith("file:///"));

    // Embedded spaces as <text:s text:c='4' />, normal text
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"one     two"_ustr, pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Leading space as <text:s />.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u" =3+4"_ustr, pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Embedded spaces with <text:s text:c='4' /> inside a <text:span>, text
    // partly bold.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(u"one     two"_ustr, aParaText);
    pEditText->GetCharAttribs(0, aAttribs);
    {
        auto it = std::find_if(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
            return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT
                   && static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight() == WEIGHT_BOLD;
        });
        bool bHasBold = (it != aAttribs.end());
        if (bHasBold)
        {
            OUString aSeg = aParaText.copy(it->nStart, it->nEnd - it->nStart);
            CPPUNIT_ASSERT_EQUAL(u"e     t"_ustr, aSeg);
        }
        CPPUNIT_ASSERT_MESSAGE("Expected a bold sequence.", bHasBold);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testDataBarODS)
{
    createScDoc("ods/databar.ods");

    ScDocument* pDoc = getScDoc();
    testDataBar_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testDataBarXLSX)
{
    createScDoc("xlsx/databar.xlsx");

    ScDocument* pDoc = getScDoc();
    testDataBar_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testColorScaleODS)
{
    createScDoc("ods/colorscale.ods");
    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testColorScaleXLSX)
{
    createScDoc("xlsx/colorscale.xlsx");
    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testNewCondFormatODS)
{
    createScDoc("ods/new_cond_format_test.ods");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test.csv");
    testCondFile(aCSVPath, pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testNewCondFormatXLSX)
{
    createScDoc("xlsx/new_cond_format_test.xlsx");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test.csv");
    testCondFile(aCSVPath, pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCondFormatImportCellIs)
{
    createScDoc("xlsx/condFormat_cellis.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    OUString aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"$Sheet2.$A$2"_ustr, aStr);

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"$Sheet2.$A$1"_ustr, aStr);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCondFormatThemeColorXLSX)
{
    createScDoc("xlsx/condformat_theme_color.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, *pDataBarFormatData->mxNegativeColor);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(1)->size());
    pFormat = pDoc->GetCondFormat(0, 0, 1);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());
    const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pColorScale->size());
    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(0);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(255, 230, 153), pColorScaleEntry->GetColor());

    pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pColorScaleEntry->GetColor());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCondFormatThemeColor2XLSX)
{
    // negative bar color and axis color
    createScDoc("xlsx/cond_format_theme_color2.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(5, 5, 0);
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(99, 142, 198), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(Color(217, 217, 217), *pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(Color(197, 90, 17), pDataBarFormatData->maAxisColor);
}

namespace
{
void checkDatabarPositiveColor(const ScConditionalFormat* pFormat, const Color& rColor)
{
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(rColor, pDataBarFormatData->maPositiveColor);
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCondFormatThemeColor3XLSX)
{
    // theme index 2 and 3 are switched
    createScDoc("xlsx/cond_format_theme_color3.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(1, 3, 0);
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());
    const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>(pEntry);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pColorScale->size());
    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(0);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(175, 171, 171), pColorScaleEntry->GetColor());

    pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(51, 63, 80), pColorScaleEntry->GetColor());

    pFormat = pDoc->GetCondFormat(3, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(59, 56, 56));

    pFormat = pDoc->GetCondFormat(5, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(173, 185, 202));

    pFormat = pDoc->GetCondFormat(7, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(89, 89, 89));

    pFormat = pDoc->GetCondFormat(9, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(217, 217, 217));
}

namespace
{
// This function is used temporarily so we don't have to update test files.
void convertToOldCondFormat(const ScIconSetFormat* pIconSet)
{
    auto itr = pIconSet->begin();
    (*itr)->SetMode(ScConditionMode::Less);
    (*itr)->SetValue((*(itr + 1))->GetValue());
    (*itr)->SetType((*(++itr))->GetType());
    for (; itr != pIconSet->end(); ++itr)
        (*itr)->SetMode(ScConditionMode::EqGreater);
}

void testComplexIconSetsXLSX_Impl(const ScDocument& rDoc, SCCOL nCol, ScIconSetType eType)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(eType, pIconSet->GetIconSetData()->eIconSetType);
}

void testCustomIconSetsXLSX_Impl(const ScDocument& rDoc, SCCOL nCol, SCROW nRow,
                                 ScIconSetType eType, sal_Int32 nIndex)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, 1, 1);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    convertToOldCondFormat(pIconSet);
    std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(nCol, nRow, 1)));
    if (nIndex == -1)
        CPPUNIT_ASSERT(!pInfo);
    else
    {
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(eType, pInfo->eIconSetType);
    }
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testComplexIconSetsXLSX)
{
    createScDoc("xlsx/complex_icon_set.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(3), pDoc->GetCondFormList(0)->size());
    testComplexIconSetsXLSX_Impl(*pDoc, 1, IconSet_3Triangles);
    testComplexIconSetsXLSX_Impl(*pDoc, 3, IconSet_3Stars);
    testComplexIconSetsXLSX_Impl(*pDoc, 5, IconSet_5Boxes);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetCondFormList(1)->size());
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 1, IconSet_3ArrowsGray, 0);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 2, IconSet_3ArrowsGray, -1);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 3, IconSet_3Arrows, 1);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 4, IconSet_3ArrowsGray, -1);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 5, IconSet_3Arrows, 2);

    testCustomIconSetsXLSX_Impl(*pDoc, 3, 1, IconSet_4RedToBlack, 3);
    testCustomIconSetsXLSX_Impl(*pDoc, 3, 2, IconSet_3TrafficLights1, 1);
    testCustomIconSetsXLSX_Impl(*pDoc, 3, 3, IconSet_3Arrows, 2);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf101104)
{
    createScDoc("ods/tdf101104.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(1, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    convertToOldCondFormat(pIconSet);

    for (size_t i = 1; i < 10; ++i)
    {
        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(1, i, 0)));

        // Without the fix in place, this test would have failed here
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf64401)
{
    createScDoc("ods/tdf64401.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    convertToOldCondFormat(pIconSet);

    for (size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if (i >= 7) // B5 = 8
            nIndex = 2;
        else if (i >= 3) // B4 = 4
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }

    // Update values in B4 and B5
    pDoc->SetValue(ScAddress(1, 3, 0), 2.0);
    pDoc->SetValue(ScAddress(1, 4, 0), 9.0);

    for (size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if (i >= 8) // B5 = 9
            nIndex = 2;
        else if (i >= 1) // B4 = 2
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCondFormatParentXLSX)
{
    createScDoc("xlsx/cond_parent.xlsx");

    ScDocument* pDoc = getScDoc();
    const SfxItemSet* pCondSet = pDoc->GetCondResult(2, 5, 0);
    const ScPatternAttr* pPattern = pDoc->GetPattern(2, 5, 0);
    const SfxPoolItem& rPoolItem = pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet);
    const SvxVerJustifyItem& rVerJustify = static_cast<const SvxVerJustifyItem&>(rPoolItem);
    CPPUNIT_ASSERT_EQUAL(SvxCellVerJustify::Top, rVerJustify.GetValue());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testColorScaleNumWithRefXLSX)
{
    createScDoc("xlsx/colorscale_num_with_ref.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);

    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());

    const ScColorScaleFormat* pColorScale = dynamic_cast<const ScColorScaleFormat*>(pEntry);
    CPPUNIT_ASSERT(pColorScale);

    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT_EQUAL(u"=$A$1"_ustr,
                         pColorScaleEntry->GetFormula(formula::FormulaGrammar::GRAM_NATIVE));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf153514)
{
    ScDocument aDoc;
    OUString aPath = createFilePath(u"xml/tdf153514.xml");

    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    CPPUNIT_ASSERT(pOrcus);

    pOrcus->importODS_Styles(aDoc, aPath);
    ScStyleSheetPool* pStyleSheetPool = aDoc.GetStyleSheetPool();

    ScStyleSheet* pStyleSheet;
    // Case sensitive tests
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"aBcD"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_EQUAL(u"aBcD"_ustr, pStyleSheet->GetName());
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"abCd"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_EQUAL(u"abCd"_ustr, pStyleSheet->GetName());
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Abcd"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_EQUAL(u"Abcd"_ustr, pStyleSheet->GetName());
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"ABCD"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_EQUAL(u"ABCD"_ustr, pStyleSheet->GetName());
    // Case insensitive tests
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"abcd"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyleSheet);
    CPPUNIT_ASSERT(pStyleSheet->GetName().equalsIgnoreAsciiCase("abcd"));
    CPPUNIT_ASSERT(pStyleSheet->GetName() != "abcd");
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"ABCd"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyleSheet);
    CPPUNIT_ASSERT(pStyleSheet->GetName().equalsIgnoreAsciiCase("ABCd"));
    CPPUNIT_ASSERT(pStyleSheet->GetName() != "ABCd");
    // Not match tests
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"NotFound"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT(!pStyleSheet);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testOrcusODSStyleInterface)
{
    ScDocument aDoc;
    OUString aPath = createFilePath(u"xml/styles.xml");

    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    CPPUNIT_ASSERT(pOrcus);

    pOrcus->importODS_Styles(aDoc, aPath);
    ScStyleSheetPool* pStyleSheetPool = aDoc.GetStyleSheetPool();

    /* Test cases for Style "Name1"
     * Has Border and Fill.
     */
    ScStyleSheet* pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name1"_ustr, SfxStyleFamily::Para);
    const SfxPoolItem* pItem = nullptr;

    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Doesn't have Attribute background, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem));
    const SvxBrushItem* pBackground = static_cast<const SvxBrushItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(254, 255, 204), pBackground->GetColor());

    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Doesn't have Attribute border, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_BORDER, &pItem));
    const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetLeft()->GetColor());
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetRight()->GetColor());
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetTop()->GetColor());
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetBottom()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetLeft()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetRight()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetTop()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetBottom()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with left width", 1, pBoxItem->GetLeft()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with right width", 1, pBoxItem->GetRight()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with top width", 1, pBoxItem->GetTop()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with bottom width", 1, pBoxItem->GetBottom()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Has Attribute Protection, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Has Attribute font, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Has Attribute number format, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem));

    /* Test for Style "Name2"
     * Has 4 sided borders + Diagonal borders.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name2"_ustr, SfxStyleFamily::Para);

    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Doesn't have Attribute background, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_BORDER, &pItem));

    pBoxItem = static_cast<const SvxBoxItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pBoxItem->GetLeft()->GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pBoxItem->GetRight()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pBoxItem->GetLeft()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetRight()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with left width", 0, pBoxItem->GetLeft()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with right width", 14, pBoxItem->GetRight()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE(
        "Style Name2 : Doesn't have Attribute diagonal(tl-br) border, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_BORDER_TLBR, &pItem));

    const SvxLineItem* pTLBR = static_cast<const SvxLineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(18, 0, 0), pTLBR->GetLine()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASH_DOT, pTLBR->GetLine()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with diagonal tl-br width", 14,
                                 pTLBR->GetLine()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE(
        "Style Name2 : Doesn't have Attribute diagonal(bl-tr) border, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_BORDER_BLTR, &pItem));

    const SvxLineItem* pBLTR = static_cast<const SvxLineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 238), pBLTR->GetLine()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, pBLTR->GetLine()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with diagonal tl-br width", 34,
                                 pBLTR->GetLine()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Has Attribute background, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Has Attribute font, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Has Attribute number format, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem));

    /* Test for Style "Name3"
     * Hidden, protected and content is printed.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name3"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name3 : Doesn't have Attribute Protection, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name 3 : Error with Protection attribute.",
                           bool(ScProtectionAttr(true, false, true, true) == *pItem));

    /* Test for Style "Name4"
     * Hidden, protected and content is printed.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name4"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name4 : Doesn't have Attribute Protection, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name 4 : Error with Protection attribute.",
                           bool(ScProtectionAttr(true, true, false, false) == *pItem));

    /* Test for Style "Name5"
     * Hidden, protected and content is printed.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name5"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Doesn't have Attribute Protection, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Error with Protection attribute.",
                           bool(ScProtectionAttr(false, false, false, true) == *pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute Border, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_BORDER, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute background, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute font, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute number format, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem));

    /* Test for Style "Name6"
     * Has Font name, posture, weight, color, height
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name6"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));

    const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font name", u"Liberation Sans"_ustr,
                                 pFontItem->GetStyleName());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Height, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_HEIGHT, &pItem));

    const SvxFontHeightItem* pFontHeightItem = static_cast<const SvxFontHeightItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Height",
                                 static_cast<sal_uInt32>(480), pFontHeightItem->GetHeight());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Posture, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_POSTURE, &pItem));

    const SvxPostureItem* pFontPostureItem = static_cast<const SvxPostureItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Posture", ITALIC_NORMAL,
                                 pFontPostureItem->GetPosture());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Weight, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_WEIGHT, &pItem));

    const SvxWeightItem* pFontWeightItem = static_cast<const SvxWeightItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Weight", WEIGHT_BOLD,
                                 pFontWeightItem->GetWeight());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Color, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem));

    const SvxColorItem* pFontColorItem = static_cast<const SvxColorItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Color", COL_GRAY,
                                 pFontColorItem->GetValue());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Underline, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_UNDERLINE, &pItem));

    const SvxUnderlineItem* pUnderlineItem = static_cast<const SvxUnderlineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Underline Style", LINESTYLE_SINGLE,
                                 pUnderlineItem->GetLineStyle());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Underline Color", COL_GRAY,
                                 pUnderlineItem->GetColor());

    /* Test for Style Name "7"
     * Has strikethrough single
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name7"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name7 : Doesn't have Attribute Strikeout, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    const SvxCrossedOutItem* pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name7 :Error with Strikeout", STRIKEOUT_SINGLE,
                                 pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "8"
     * Has strikethrough bold
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name8"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name8 : Doesn't have Attribute Strikeout, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name7 :Error with Strikeout", STRIKEOUT_BOLD,
                                 pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "9"
     * Has strikethrough slash
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name9"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name9 : Doesn't have Attribute Strikeout, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name9 :Error with Strikeout", STRIKEOUT_SLASH,
                                 pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "10"
     * Has ver align, and hor align
     */

    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name10"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name10 : Doesn't have Attribute hor justify, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_HOR_JUSTIFY, &pItem));

    const SvxHorJustifyItem* pHorJustify = static_cast<const SvxHorJustifyItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name10 :Error with hor justify", SvxCellHorJustify::Right,
                                 pHorJustify->GetValue());

    pStyleSheet = pStyleSheetPool->FindCaseIns(u"Name10"_ustr, SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name10 : Doesn't have Attribute ver justify, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_VER_JUSTIFY, &pItem));

    const SvxVerJustifyItem* pVerJustify = static_cast<const SvxVerJustifyItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name10 :Error with ver justify", SvxCellVerJustify::Center,
                                 pVerJustify->GetValue());

    auto checkFontWeight = [pStyleSheetPool](const OUString& rName, FontWeight eExpected) {
        ScStyleSheet* pStyle = pStyleSheetPool->FindCaseIns(rName, SfxStyleFamily::Para);
        CPPUNIT_ASSERT(pStyle);

        const SfxPoolItem* p = nullptr;

        {
            std::ostringstream os;
            os << "Style named '" << rName << "' does not have a font weight attribute.";
            CPPUNIT_ASSERT_MESSAGE(os.str(), pStyle->GetItemSet().HasItem(ATTR_FONT_WEIGHT, &p));
        }

        const SvxWeightItem* pWeight = static_cast<const SvxWeightItem*>(p);
        FontWeight eActual = pWeight->GetWeight();
        {
            std::ostringstream os;
            os << "Wrong font weight value for style named '" << rName
               << "': expected=" << eExpected << "; actual=" << eActual;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(os.str(), eExpected, eActual);
        }
    };

    checkFontWeight(u"Accent"_ustr, WEIGHT_BOLD);
    checkFontWeight(u"Accent 1"_ustr, WEIGHT_BOLD); // inherits from 'Accent'
    checkFontWeight(u"Accent 2"_ustr, WEIGHT_BOLD); // inherits from 'Accent'
    checkFontWeight(u"Accent 3"_ustr, WEIGHT_BOLD); // inherits from 'Accent'
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testLiteralInFormulaXLS)
{
    createScDoc("xls/shared-string/literal-in-formula.xls");

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testFormulaDependency)
{
    createScDoc("ods/dependencyTree.ods");

    ScDocument* pDoc = getScDoc();

    // check if formula in A1 changes value
    double nVal = pDoc->GetValue(0, 0, 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, nVal, 1e-10);
    pDoc->SetValue(0, 1, 0, 0.0);
    nVal = pDoc->GetValue(0, 0, 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, nVal, 1e-10);

    // check that the number format is implicitly inherited
    // CPPUNIT_ASSERT_EQUAL(pDoc->GetString(0,4,0), rDoc.GetString(0,5,0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf153444)
{
    createScDoc("xml/tdf153444.xml");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"time, sec"_ustr, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Body_Right_Z, g"_ustr, pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Body_Left_Z, g"_ustr, pDoc->GetString(2, 0, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: ÄÖÜ, µm/m
    // - Actual  : ���, �m/m
    CPPUNIT_ASSERT_EQUAL(u"ÄÖÜ, µm/m"_ustr, pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"äöü, µm/m"_ustr, pDoc->GetString(4, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"ß, µm/m"_ustr, pDoc->GetString(5, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf151958)
{
    // Without the fix in place, this test would have failed with
    // sal_uInt64 ScCountIfCellIterator<accessType>::GetCount(): Assertion `false' failed.
    createScDoc("ods/tdf151958.ods");

    ScDocument* pDoc = getScDoc();

    double aValues[13] = { 17.0, 6.0, 6.0, 6.0, 5.0, 5.0, 4.0, 4.0, 4.0, 3.0, 3.0, 3.0, 2.0 };
    for (size_t i = 3; i < 15; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(aValues[i - 3], pDoc->GetValue(6, i, 0));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf151046)
{
    createScDoc("ods/tdf151046.ods");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 1, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf147955)
{
    createScDoc("xlsx/tdf147955.xlsx");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 892.75
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(892.75, pDoc->GetValue(1, 6, 0));
    CPPUNIT_ASSERT_EQUAL(130.25, pDoc->GetValue(1, 10, 0));
    CPPUNIT_ASSERT_EQUAL(10.98, pDoc->GetValue(1, 15, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testImportCrashes)
{
    testImportCrash("ods/tdf149752.ods");
    testImportCrash("xls/tdf158483.xls");
    testImportCrash("ods/tdf122643.ods");
    testImportCrash("ods/tdf132278.ods");
    testImportCrash("xlsx/tdf130959.xlsx");
    testImportCrash("ods/tdf129410.ods");
    testImportCrash("ods/tdf138507.ods");
    testImportCrash("xlsx/tdf131380.xlsx");
    testImportCrash("ods/tdf139782.ods");
    testImportCrash("ods/tdf136551.ods");
    testImportCrash("ods/tdf90391.ods");
    testImportCrash("xlsx/tdf121887.xlsx"); // 'Maximum number of rows per sheet' warning
    testImportCrash("xlsm/tdf111974.xlsm");
    testImportCrash("ods/tdf149679.ods");
    testImportCrash("xlsx/tdf124525.xlsx");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf129681)
{
    createScDoc("ods/tdf129681.ods");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"Lamb"_ustr, pDoc->GetString(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Beef"_ustr, pDoc->GetString(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Pork"_ustr, pDoc->GetString(ScAddress(4, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Goat"_ustr, pDoc->GetString(ScAddress(4, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Goat"_ustr, pDoc->GetString(ScAddress(4, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#VALUE!"_ustr, pDoc->GetString(ScAddress(4, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#VALUE!"_ustr, pDoc->GetString(ScAddress(4, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Deer"_ustr, pDoc->GetString(ScAddress(4, 9, 0)));

    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, pDoc->GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, pDoc->GetString(ScAddress(6, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pDoc->GetString(ScAddress(6, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pDoc->GetString(ScAddress(6, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(u"8"_ustr, pDoc->GetString(ScAddress(6, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#VALUE!"_ustr, pDoc->GetString(ScAddress(6, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#VALUE!"_ustr, pDoc->GetString(ScAddress(6, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#VALUE!"_ustr, pDoc->GetString(ScAddress(6, 9, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf149484)
{
    createScDoc("ods/tdf149484.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: -TRUE-
    // - Actual  : TRUE
    CPPUNIT_ASSERT_EQUAL(u"-TRUE-"_ustr, pDoc->GetString(0, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testEscapedUnicodeXLSX)
{
    createScDoc("xlsx/escape-unicode.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix, there would be "_x000D_" after every new-line char.
    CPPUNIT_ASSERT_EQUAL(u"Line 1\nLine 2\nLine 3\nLine 4"_ustr, pDoc->GetString(1, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf144758_DBDataDefaultOrientation)
{
    createScDoc("fods/tdf144758-dbdata-no-orientation.fods");
    ScDocument* pDoc = getScDoc();
    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT(pAnonDBData);

    ScSortParam aSortParam;
    pAnonDBData->GetSortParam(aSortParam);

    // Without the fix, the default value for bByRow (in absence of 'table:orientation' attribute
    // in 'table:database-range' element) was false
    CPPUNIT_ASSERT(aSortParam.bByRow);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testSharedFormulaXLS)
{
    createScDoc("xls/shared-formula/basic.xls");
    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = pDoc->GetValue(ScAddress(1, i, 0));
        double fCheck = i * 10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }

    ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(1, 18, 0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pCell);
    ScFormulaCellGroupRef xGroup = pCell->GetCellGroup();
    CPPUNIT_ASSERT_MESSAGE("This cell should be a part of a cell group.", xGroup);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(1),
                                 xGroup->mpTopCell->aPos.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(18), xGroup->mnLength);

    // The following file contains shared formula whose range is inaccurate.
    // Excel can easily mess up shared formula ranges, so we need to be able
    // to handle these wrong ranges that Excel stores.

    createScDoc("xls/shared-formula/gap.xls");
    pDoc = getScDoc();
    pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A1*20"_ustr, pDoc->GetFormula(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A2*20"_ustr, pDoc->GetFormula(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A3*20"_ustr, pDoc->GetFormula(1, 2, 0));

    // There is an intentional gap at row 4.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A5*20"_ustr, pDoc->GetFormula(1, 4, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A6*20"_ustr, pDoc->GetFormula(1, 5, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A7*20"_ustr, pDoc->GetFormula(1, 6, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", u"=A8*20"_ustr, pDoc->GetFormula(1, 7, 0));

    // We re-group formula cells on load. Let's check that as well.

    ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This should be the top cell in formula group.", pFC->IsSharedTop());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    pFC = pDoc->GetFormulaCell(ScAddress(1, 4, 0));
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This should be the top cell in formula group.", pFC->IsSharedTop());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testSharedFormulaXLSX)
{
    createScDoc("xlsx/shared-formula/basic.xlsx");
    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = pDoc->GetValue(ScAddress(1, i, 0));
        double fCheck = i * 10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }

    ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(1, 18, 0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pCell);
    ScFormulaCellGroupRef xGroup = pCell->GetCellGroup();
    CPPUNIT_ASSERT_MESSAGE("This cell should be a part of a cell group.", xGroup);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(1),
                                 xGroup->mpTopCell->aPos.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(18), xGroup->mnLength);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testSharedFormulaRefUpdateXLSX)
{
    createScDoc("xlsx/shared-formula/refupdate.xlsx");
    ScDocument* pDoc = getScDoc();
    sc::AutoCalcSwitch aACSwitch(*pDoc, true); // turn auto calc on.
    pDoc->DeleteRow(ScRange(0, 4, 0, pDoc->MaxCol(), 4, 0)); // delete row 5.

    struct TestCase
    {
        SCCOL nCol;
        SCROW nRow;
        SCTAB nTab;
        const char* pExpectedFormula;
        const char* pErrorMsg;
    };

    TestCase aCases[4] = {
        { 1, 0, 0, "=B29+1", "Wrong formula in B1" },
        { 2, 0, 0, "=C29+1", "Wrong formula in C1" },
        { 3, 0, 0, "=D29+1", "Wrong formula in D1" },
        { 4, 0, 0, "=E29+1", "Wrong formula in E1" },
    };

    for (size_t nIdx = 0; nIdx < 4; ++nIdx)
    {
        TestCase& rCase = aCases[nIdx];
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rCase.pErrorMsg,
                                     OUString::createFromAscii(rCase.pExpectedFormula),
                                     pDoc->GetFormula(rCase.nCol, rCase.nRow, rCase.nTab));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf153924)
{
    createScDoc("ods/tdf153924.ods");
    ScDocument* pDoc = getScDoc();

    double aCheck[6][2] = { { 148.0, 195.0 }, { 144.0, 200.0 }, { 151.0, 192.0 },
                            { 154.0, 189.0 }, { 165.0, 183.0 }, { 0.0, 0.0 } };
    for (size_t i = 0; i < 6; ++i)
    {
        for (size_t j = 0; j < 2; ++j)
        {
            // Without the fix in place, the output from LARGE()/SMALL() would have been 0
            CPPUNIT_ASSERT_EQUAL(aCheck[i][j], pDoc->GetValue(ScAddress(j, i + 8, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testSheetNamesXLSX)
{
    createScDoc("xlsx/sheet-names.xlsx");
    ScDocument* pDoc = getScDoc();

    std::vector<OUString> aTabNames = pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The document should have 5 sheets in total.", size_t(5),
                                 aTabNames.size());
    CPPUNIT_ASSERT_EQUAL(u"S&P"_ustr, aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(u"Sam's Club"_ustr, aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(u"\"The Sheet\""_ustr, aTabNames[2]);
    CPPUNIT_ASSERT_EQUAL(u"A<B"_ustr, aTabNames[3]);
    CPPUNIT_ASSERT_EQUAL(u"C>D"_ustr, aTabNames[4]);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf155046)
{
    createScDoc("xlsx/tdf155046.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: TRUE
    // - Actual  : FALSE
    CPPUNIT_ASSERT_EQUAL(u"TRUE"_ustr, pDoc->GetString(ScAddress(2, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf150599)
{
    createScDoc("dif/tdf150599.dif");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : #IND:?
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"32"_ustr, pDoc->GetString(ScAddress(31, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testCommentSize)
{
    createScDoc("ods/comment.ods");
    ScDocument* pDoc = getScDoc();

    ScAddress aPos(0, 0, 0);
    ScPostIt* pNote = pDoc->GetNote(aPos);
    CPPUNIT_ASSERT(pNote);

    pNote->ShowCaption(aPos, true);
    CPPUNIT_ASSERT(pNote->IsCaptionShown());

    SdrCaptionObj* pCaption = pNote->GetCaption();
    CPPUNIT_ASSERT(pCaption);

    // The values below depend on particular font and char size.
    // At least assert that the corresponding style was set:
    CPPUNIT_ASSERT_EQUAL(ScResId(STR_STYLENAME_NOTE), pCaption->GetStyleSheet()->GetName());

    const tools::Rectangle& rOldRect = pCaption->GetLogicRect();
    CPPUNIT_ASSERT_EQUAL(tools::Long(2899), rOldRect.getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(939), rOldRect.getOpenHeight());

    pNote->SetText(aPos, u"first\nsecond\nthird"_ustr);

    const tools::Rectangle& rNewRect = pCaption->GetLogicRect();
    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenWidth(), rNewRect.getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1386), rNewRect.getOpenHeight());

    pDoc->GetUndoManager()->Undo();

    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenWidth(), pCaption->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenHeight(), pCaption->GetLogicRect().getOpenHeight());
}

static void testEnhancedProtectionImpl(const ScDocument& rDoc)
{
    const ScTableProtection* pProt = rDoc.GetTabProtection(0);

    CPPUNIT_ASSERT(pProt);

    CPPUNIT_ASSERT(!pProt->isBlockEditable(ScRange(0, 0, 0, 0, 0, 0))); // locked
    CPPUNIT_ASSERT(pProt->isBlockEditable(ScRange(0, 1, 0, 0, 1, 0))); // editable without password
    CPPUNIT_ASSERT(pProt->isBlockEditable(ScRange(0, 2, 0, 0, 2, 0))); // editable without password
    CPPUNIT_ASSERT(
        !pProt->isBlockEditable(ScRange(0, 3, 0, 0, 3, 0))); // editable with password "foo"
    CPPUNIT_ASSERT(!pProt->isBlockEditable(ScRange(0, 4, 0, 0, 4, 0))); // editable with descriptor
    CPPUNIT_ASSERT(!pProt->isBlockEditable(
        ScRange(0, 5, 0, 0, 5, 0))); // editable with descriptor and password "foo"
    CPPUNIT_ASSERT(
        pProt->isBlockEditable(ScRange(0, 1, 0, 0, 2, 0))); // union of two different editables
    CPPUNIT_ASSERT(
        !pProt->isBlockEditable(ScRange(0, 0, 0, 0, 1, 0))); // union of locked and editable
    CPPUNIT_ASSERT(!pProt->isBlockEditable(
        ScRange(0, 2, 0, 0, 3, 0))); // union of editable and password editable
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testEnhancedProtectionXLS)
{
    createScDoc("xls/enhanced-protection.xls");
    ScDocument* pDoc = getScDoc();

    testEnhancedProtectionImpl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testEnhancedProtectionXLSX)
{
    createScDoc("xlsx/enhanced-protection.xlsx");
    ScDocument* pDoc = getScDoc();

    testEnhancedProtectionImpl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testSortWithSharedFormulasODS)
{
    createScDoc("ods/shared-formula/sort-crash.ods");
    ScDocument* pDoc = getScDoc();

    // E2:E10 should be shared.
    const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(4, 1, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedLength());

    // E12:E17 should be shared.
    pFC = pDoc->GetFormulaCell(ScAddress(4, 11, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(11), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());

    // Set A1:E17 as an anonymous database range to sheet, or else Calc would
    // refuse to sort the range.
    std::unique_ptr<ScDBData> pDBData(
        new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 4, 16, true, true));
    pDoc->SetAnonymousDBData(0, std::move(pDBData));

    // Sort ascending by Column E.

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 4;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 16;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 4;
    aSortData.maKeyState[0].bAscending = true;

    // Do the sorting.  This should not crash.
    ScDocShell* pDocSh = getScDocShell();
    ScDBDocFunc aFunc(*pDocSh);
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // After the sort, E2:E16 should be shared.
    pFC = pDoc->GetFormulaCell(ScAddress(4, 1, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedLength());
}

namespace
{
void testSortWithSheetExternalReferencesODS_Impl(ScDocShell& rDocSh, SCROW nRow1, SCROW nRow2,
                                                 bool bCheckRelativeInSheet)
{
    ScDocument& rDoc = rDocSh.GetDocument();

    // Check the original data is there.
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        double const aCheck[] = { 1, 2, 3, 4, 5 };
        CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(0, nRow, 0)));
    }
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = 1; nCol <= 3; ++nCol)
        {
            double const aCheck[] = { 1, 12, 123, 1234, 12345 };
            CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(nCol, nRow, 0)));
        }
    }

    // Set as an anonymous database range to sort.
    std::unique_ptr<ScDBData> pDBData(
        new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, nRow1, 3, nRow2, true, true));
    rDoc.SetAnonymousDBData(0, std::move(pDBData));

    // Sort descending by Column A.
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 3;
    aSortData.nRow1 = nRow1;
    aSortData.nRow2 = nRow2;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = false;

    // Do the sorting.
    ScDBDocFunc aFunc(rDocSh);
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);
    rDoc.CalcAll();

    // Check the sort and that all sheet references and external references are
    // adjusted to point to the original location.
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        double const aCheck[] = { 5, 4, 3, 2, 1 };
        CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(0, nRow, 0)));
    }
    // The last column (D) are in-sheet relative references.
    SCCOL nEndCol = (bCheckRelativeInSheet ? 3 : 2);
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = 1; nCol <= nEndCol; ++nCol)
        {
            double const aCheck[] = { 12345, 1234, 123, 12, 1 };
            CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(nCol, nRow, 0)));
        }
    }
}
}

// https://bugs.freedesktop.org/attachment.cgi?id=100089 from fdo#77018
// mentioned also in fdo#79441
// Document contains cached external references.
CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testSortWithSheetExternalReferencesODS)
{
    // We reset the SortRefUpdate value back to the original in tearDown().
    ScInputOptions aInputOption = SC_MOD()->GetInputOptions();
    bool bUpdateReferenceOnSort = aInputOption.GetSortRefUpdate();

    createScDoc("ods/sort-with-sheet-external-references.ods");
    ScDocument* pDoc = getScDoc();
    sc::AutoCalcSwitch aACSwitch(*pDoc, true); // turn auto calc on.
    pDoc->CalcAll();

    // The complete relative test only works with UpdateReferenceOnSort==true,
    // but the internal and external sheet references have to work in both
    // modes.

    aInputOption.SetSortRefUpdate(true);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A15:D20 with relative row references. UpdateReferenceOnSort==true
    // With in-sheet relative references.
    ScDocShell* pDocSh = getScDocShell();
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 14, 19, true);

    // Undo sort with relative references to perform same sort.
    pDoc->GetUndoManager()->Undo();
    pDoc->CalcAll();

    aInputOption.SetSortRefUpdate(false);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A15:D20 with relative row references. UpdateReferenceOnSort==false
    // Without in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 14, 19, false);

    // Undo sort with relative references to perform new sort.
    pDoc->GetUndoManager()->Undo();
    pDoc->CalcAll();

    // Sort with absolute references has to work in both UpdateReferenceOnSort
    // modes.

    aInputOption.SetSortRefUpdate(true);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A23:D28 with absolute row references. UpdateReferenceOnSort==true
    // With in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 22, 27, true);

    // Undo sort with absolute references to perform same sort.
    pDoc->GetUndoManager()->Undo();
    pDoc->CalcAll();

    aInputOption.SetSortRefUpdate(false);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A23:D28 with absolute row references. UpdateReferenceOnSort==false
    // With in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 22, 27, true);

    if (bUpdateReferenceOnSort != aInputOption.GetSortRefUpdate())
    {
        aInputOption.SetSortRefUpdate(bUpdateReferenceOnSort);
        SC_MOD()->SetInputOptions(aInputOption);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testSortWithFormattingXLS)
{
    createScDoc("xls/tdf129127.xls");
    ScDocument* pDoc = getScDoc();

    // Set as an anonymous database range to sort.
    std::unique_ptr<ScDBData> pDBData(
        new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 4, 9, false, false));
    pDoc->SetAnonymousDBData(0, std::move(pDBData));

    // Sort ascending by Row 1
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 4;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 9;
    aSortData.bHasHeader = false;
    aSortData.bByRow = false;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;

    // Do the sorting.
    ScDocShell* pDocSh = getScDocShell();
    ScDBDocFunc aFunc(*pDocSh);
    // Without the fix, sort would crash.
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);
}

// just needs to not crash on recalc
CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testForcepoint107)
{
    createScDoc("xlsx/forcepoint107.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf155402)
{
    createScDoc("xlsx/tdf155402.xlsx");
    ScDocument* pDoc = getScDoc();

    // Before the fix =CELL("FILENAME"; D2) returns 'file:///X:/dr/own.ods'#$Sheet1
    // After the fix =CELL("FILENAME"; D2) returns file :///X:/dr/[own.ods]Sheet1
    OUString aFilename = pDoc->GetString(1, 0, 0);
    sal_Int32 nPos = aFilename.lastIndexOf('/');
    aFilename = OUString::Concat(aFilename.subView(nPos));
    CPPUNIT_ASSERT_EQUAL(u"/[tdf155402.xlsx]Sheet1"_ustr, aFilename);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf142905)
{
    createScDoc("xlsx/tdf142905.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix, trims the spaces in cell C1 after reading the XLSX file.
    // After recalculation it is fixed.
    // - Expected: "     3M   "
    // - Actual  : "3M"
    CPPUNIT_ASSERT_EQUAL(u"     3M   "_ustr, pDoc->GetString(2, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testTdf125580)
{
    createScDoc("ods/tdf125580.ods");
    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: 11/22/23 04:30 PM
    // - Actual  : 11/22/23 04:29 PM
    // - At row 19
    for (SCROW i = 0; i <= 40; ++i)
    {
        OString sMessage = "At row " + OString::number(i + 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), pDoc->GetString(0, i, 0),
                                     pDoc->GetString(1, i, 0));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest4, testRowImportCellStyleIssue)
{
    // Test checks that the correct cell style is imported for the first 6 rows and then the rest of the rows.
    // Row 1 to 6 have no background color, after that light2 (background2) theme color.

    createScDoc("xlsx/RowImportCellStyleIssue.xlsx");
    ScDocument* pDoc = getScDoc();

    // Check cell A6 - should have no background color set
    {
        const ScPatternAttr* pAttr = pDoc->GetPattern(0, 5, 0); // A6
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(false, rBackground.isUsed());
    }

    // Check cell A7 - should have light2 (background2) theme color set
    {
        const ScPatternAttr* pAttr = pDoc->GetPattern(0, 6, 0); // A7
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(true, rBackground.isUsed());
        CPPUNIT_ASSERT_EQUAL(Color(0xe7e6e6), rBackground.GetColor());
        auto const& rComplexColor = rBackground.getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Light2, rComplexColor.getThemeColorType());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
