/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Laurent Godard lgodard.libre@laposte.net (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets2.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>

#include <rtl/oustringostreaminserter.hxx>

namespace ScSpreadsheetObj {

class ScXSpreadsheets2 : public UnoApiTest
{
    uno::Reference< sheet::XSpreadsheetDocument> getDoc(const rtl::OUString);
    uno::Reference< sheet::XNamedRanges> getNamedRanges(uno::Reference< sheet::XSpreadsheetDocument >);

    // XSpreadsheets2
    void testImportSheet();

    CPPUNIT_TEST_SUITE(ScXSpreadsheets2);
    // disable test, test fails, might be the ongoing copy refs work
    CPPUNIT_TEST(testImportSheet);
    CPPUNIT_TEST_SUITE_END();
};


void ScXSpreadsheets2::testImportSheet()
{
    const rtl::OUString aSrcFileBase(RTL_CONSTASCII_USTRINGPARAM("rangenamessrc.ods"));
    uno::Reference< sheet::XSpreadsheetDocument> xSrcDoc = getDoc(aSrcFileBase);
    CPPUNIT_ASSERT(xSrcDoc.is());

    uno::Reference< container::XNameAccess > xSrcNamedRangesNameAccess (getNamedRanges(xSrcDoc), UNO_QUERY_THROW);

    const rtl::OUString aDestFileBase(RTL_CONSTASCII_USTRINGPARAM("rangenames.ods"));
    uno::Reference< sheet::XSpreadsheetDocument> xDestDoc = getDoc(aDestFileBase);
    CPPUNIT_ASSERT(xDestDoc.is());
    uno::Reference< sheet::XSpreadsheets2 > xDest (xDestDoc->getSheets(), UNO_QUERY_THROW);

    uno::Reference< container::XNameAccess > xDestNamedRangesNameAccess(getNamedRanges(xDestDoc), UNO_QUERY_THROW);

    // import sheet
    const sal_Int32 nDestPos = 0;
    const rtl::OUString aSrcSheetName(RTL_CONSTASCII_USTRINGPARAM("SheetToCopy"));
    sal_Int32 nDestPosEffecive = xDest->importSheet(xSrcDoc, aSrcSheetName, nDestPos);

    //sheet index in dest is ok
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet index", nDestPosEffecive == nDestPos);

    uno::Reference< container::XNameAccess > xDestSheetNameAccess (xDestDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xDestSheet( xDestSheetNameAccess->getByName(aSrcSheetName), UNO_QUERY_THROW);

    uno::Reference< container::XNameAccess > xSrcSheetNameAccess (xSrcDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSrcSheet( xSrcSheetNameAccess->getByName(aSrcSheetName), UNO_QUERY_THROW);

    //new sheet name in dest is ok
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet name", xDestSheetNameAccess->hasByName(aSrcSheetName));

     uno::Reference< table::XCell > xDestCell;
     uno::Reference< table::XCell > xSrcCell;

    //imported string
    xSrcCell = xSrcSheet->getCellByPosition(0,0);
    uno::Reference< text::XTextRange > xSrcTextRange(xSrcCell, UNO_QUERY_THROW);
    rtl::OUString aSrcString = xSrcTextRange->getString();

    xDestCell = xDestSheet->getCellByPosition(0,0);
    uno::Reference< text::XTextRange > xDestTextRange(xDestCell, UNO_QUERY_THROW);
    rtl::OUString aDestString = xDestTextRange->getString();

    CPPUNIT_ASSERT_MESSAGE("Wrong string imported", aDestString.equals(aSrcString));

    //imported value
    xSrcCell = xSrcSheet->getCellByPosition(1,0);
    sal_Int32 aSrcValue = xSrcCell->getValue();

    xDestCell = xDestSheet->getCellByPosition(1,0);
    sal_Int32 aDestValue = xDestCell->getValue();

    CPPUNIT_ASSERT_MESSAGE("Wrong value imported", aSrcValue == aDestValue);

    //mathematical formula
    xSrcCell = xSrcSheet->getCellByPosition(2,0);
    rtl::OUString aSrcFormula = xSrcCell->getFormula();

    xDestCell = xDestSheet->getCellByPosition(2,0);
    rtl::OUString aDestFormula = xDestCell->getFormula();

    // potential problem later: formulas might be adjusted
    // add some tests that the formulas are correctly adjusted
    CPPUNIT_ASSERT_MESSAGE("Wrong formula imported", aDestFormula.equals(aSrcFormula));

    //formula with an already existing named range in dest - initial1
    xSrcCell = xSrcSheet->getCellByPosition(3,0);
    aSrcFormula = xSrcCell->getFormula();

    xDestCell = xDestSheet->getCellByPosition(3,0);
    aDestFormula = xDestCell->getFormula();

    CPPUNIT_ASSERT_MESSAGE("Wrong Namedrange formula imported", aDestFormula.equals(aSrcFormula));

    // initial NamedRange content is correct
    rtl::OUString aNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial1"));

    uno::Any aNr = xDestNamedRangesNameAccess->getByName(aNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNamedRange(aNr, UNO_QUERY_THROW);
    rtl::OUString aNrDestContent = xDestNamedRange->getContent();

    aNr = xSrcNamedRangesNameAccess->getByName(aNamedRangeString);
    uno::Reference< sheet::XNamedRange > xSrcNamedRange(aNr, UNO_QUERY_THROW);
    rtl::OUString aNrSrcContent = xSrcNamedRange->getContent();

    std::cout << "testImportSheet : initial1 aNrDestContent " << aNrDestContent << std::endl; // is $Sheet1.$B$1
    std::cout << "testImportSheet : initial1 aNrSrcContent " << aNrSrcContent << std::endl; // is $Sheet1.$B$2
    rtl::OUString aExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$B$1"));
    CPPUNIT_ASSERT_MESSAGE("Sheet name for initial1 has been changed in dest (expected  $Sheet1.$B$1) ", aNrDestContent.equals(aExpectedContent));

    // New range name defined in imported sheet $SheetToCopy.$A$7
    rtl::OUString aNewInSheetNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("InSheetRangeName"));
    CPPUNIT_ASSERT_MESSAGE("InSheetRangeName", xDestNamedRangesNameAccess->hasByName(aNewInSheetNamedRangeString));

    uno::Any aNewInSheetNr = xDestNamedRangesNameAccess->getByName(aNewInSheetNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNewInSheetNamedRange(aNewInSheetNr, UNO_QUERY_THROW);
    rtl::OUString aNewInSheetNrDestContent = xDestNewInSheetNamedRange->getContent();
    rtl::OUString aNewInSheetExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$SheetToCopy.$A$7"));
    std::cout << "testImportSheet : InSheetRangeName content " << aNewInSheetNrDestContent << std::endl;
    //CPPUNIT_ASSERT_MESSAGE("Wrong address for InSheetRangeName", aNewInSheetNrDestContent.equals(aNewInSheetExpectedContent));

    // the source file redefines an existing named range in the imported sheet --> the target should not be changed
    rtl::OUString aRedefinedInSheetNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial2"));
    CPPUNIT_ASSERT_MESSAGE("aRedefinedInSheetNamedRangeString", xDestNamedRangesNameAccess->hasByName(aRedefinedInSheetNamedRangeString));

    uno::Any aRedefinedInSheetNr = xDestNamedRangesNameAccess->getByName(aRedefinedInSheetNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestRedefinedInSheetNamedRange(aRedefinedInSheetNr, UNO_QUERY_THROW);
    rtl::OUString aRedefinedInSheetNrDestContent = xDestRedefinedInSheetNamedRange->getContent();
    rtl::OUString aRedefinedInSheetExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$B$2"));
    std::cout << "testImportSheet : initial2 content " << aRedefinedInSheetNrDestContent << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Wrong address for Redefined InSheet named range", aRedefinedInSheetNrDestContent.equals(aRedefinedInSheetExpectedContent));

    //formula with a non-existant named range in dest - new_rangename
    rtl::OUString aNewNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("new_rangename"));
    CPPUNIT_ASSERT_MESSAGE("New NamedRange not created", xDestNamedRangesNameAccess->hasByName(aNewNamedRangeString));

    // verify the content of this new namedrange, pointing on $Sheet1.$B$1 in source. This address is already defined in target as NR content

    uno::Any aNewNr = xDestNamedRangesNameAccess->getByName(aNewNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNewNamedRange(aNewNr, UNO_QUERY_THROW);
    rtl::OUString aNewNrDestContent = xDestNewNamedRange->getContent();

    rtl::OUString aNewExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$B$1"));
/*
    xSrcCell = xSrcSheet->getCellByPosition(4,0);
    uno::Reference< text::XTextRange > xSrcTextRange3(xSrcCell, UNO_QUERY_THROW);
    aSrcString = xSrcTextRange3->getString();

    xDestCell = xDestSheet->getCellByPosition(4,0);
    uno::Reference< text::XTextRange > xDestTextRange3(xDestCell, UNO_QUERY_THROW);
    aDestString = xDestTextRange3->getString();
*/
    std::cout << "testImportSheet : new_rangename aNewExpectedContent " << aNewExpectedContent << std::endl;
    std::cout << "testImportSheet : new_rangename aNewNrDestContent " << aNewNrDestContent << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Wrong New NamedRange formula string value", aNewExpectedContent.equals(aNewExpectedContent));

    //new style created in dest
    uno::Reference< beans::XPropertySet > xSrcCellPropSet (xSrcCell, UNO_QUERY_THROW);
    const rtl::OUString aCellProperty(RTL_CONSTASCII_USTRINGPARAM("CellStyle"));
    rtl::OUString aSrcStyleName;
    CPPUNIT_ASSERT(xSrcCellPropSet->getPropertyValue(aCellProperty) >>= aSrcStyleName);

    uno::Reference< beans::XPropertySet > xDestCellPropSet (xSrcCell, UNO_QUERY_THROW);
    rtl::OUString aDestStyleName;
    CPPUNIT_ASSERT(xDestCellPropSet->getPropertyValue(aCellProperty) >>= aDestStyleName);

    CPPUNIT_ASSERT_MESSAGE("Wrong imported Cell Style", aDestStyleName.equals(aSrcStyleName));

    uno::Reference< style::XStyleFamiliesSupplier > xFamiliesSupplier (xDestDoc, UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xFamiliesNameAccess (xFamiliesSupplier->getStyleFamilies(), UNO_QUERY_THROW);
    rtl::OUString aCellFamilyName(RTL_CONSTASCII_USTRINGPARAM("CellStyles"));
    uno::Any xCellStylesFamily = xFamiliesNameAccess->getByName(aCellFamilyName);
    uno::Reference< container::XNameContainer > xCellStylesFamilyNameAccess (xCellStylesFamily, UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("New cell style not present", xCellStylesFamilyNameAccess->hasByName(aDestStyleName));

    uno::Any aCellStyle = xCellStylesFamilyNameAccess->getByName(aDestStyleName);
    uno::Reference< beans::XPropertySet > xCellStyleProp (aCellStyle, UNO_QUERY_THROW);
    rtl::OUString aProperty(RTL_CONSTASCII_USTRINGPARAM("VertJustify"));
    sal_Int32 aVertJustify = 0;
    CPPUNIT_ASSERT(xCellStyleProp->getPropertyValue(aProperty) >>= aVertJustify);

    CPPUNIT_ASSERT_MESSAGE("New style: VertJustify not set", aVertJustify == table::CellVertJustify_CENTER);
}

uno::Reference< sheet::XSpreadsheetDocument> ScXSpreadsheets2::getDoc(const rtl::OUString aFileBase)
{
    rtl::OUString aFileURL;
    createFileURL(aFileBase, aFileURL);
    uno::Reference< lang::XComponent > xComponent;
    xComponent = loadFromDesktop(aFileURL);

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDoc.is());
    return xDoc;
}

uno::Reference< sheet::XNamedRanges> ScXSpreadsheets2::getNamedRanges(uno::Reference< sheet::XSpreadsheetDocument> xDoc)
{
    uno::Reference< beans::XPropertySet > xPropSet (xDoc, UNO_QUERY_THROW);
    rtl::OUString NamedRangesPropertyString(RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue(NamedRangesPropertyString), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNamedRanges.is());

    return xNamedRanges;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScXSpreadsheets2);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
