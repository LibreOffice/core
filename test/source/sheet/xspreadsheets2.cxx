/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xspreadsheets2.hxx>

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
#include <com/sun/star/container/XIndexAccess.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {

XSpreadsheets2::XSpreadsheets2():
    aSrcSheetName("SheetToCopy"),
    aSrcFileName("rangenamessrc.ods"),
    aDestFileBase("ScNamedRangeObj.ods")
{
}

XSpreadsheets2::~XSpreadsheets2()
{
    if (xDestDoc.is())
    {
        uno::Reference<util::XCloseable> xCloseable(xDestDoc, UNO_QUERY_THROW);
        xCloseable->close(true);
    }
}

void XSpreadsheets2::testImportedSheetNameAndIndex()
{
/**
    Verfiy that the imported sheet has the correct name and is placed at the right requested index
*/

    importSheetToCopy();

    uno::Reference< container::XNameAccess > xDestSheetNameAccess(xDestDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet name", xDestSheetNameAccess->hasByName(aSrcSheetName));

}

void XSpreadsheets2::testImportString()
{
/**
    tests the cell A1 containing a string correctly imported
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(0,0);
    uno::Reference< text::XTextRange > xSrcTextRange(xSrcCell, UNO_QUERY_THROW);
    OUString aSrcString = xSrcTextRange->getString();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(0,0);
    uno::Reference< text::XTextRange > xDestTextRange(xDestCell, UNO_QUERY_THROW);
    OUString aDestString = xDestTextRange->getString();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong string imported", aDestString, aSrcString);
}

void XSpreadsheets2::testImportValue()
{
/**
    tests the cell B1 containing a value correctly imported
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(1,0);
    sal_Int32 aSrcValue = xSrcCell->getValue();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(1,0);
    sal_Int32 aDestValue = xDestCell->getValue();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value imported", aSrcValue, aDestValue);
}

void XSpreadsheets2::testImportFormulaBasicMath()
{
/**
    tests the cell C1 containing an arithmetic formula correctly imported
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(2,0);
    OUString aSrcFormula = xSrcCell->getFormula();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(2,0);
    OUString aDestFormula = xDestCell->getFormula();

    // potential problem later: formulas might be adjusted
    // add some tests that the formulas are correctly adjusted
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula imported", aDestFormula, aSrcFormula);
}

void XSpreadsheets2::testImportFormulaWithNamedRange()
{
/**
    tests the cell D1 containing a formula that uses a NamedRange expression
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(3,0);
    OUString aSrcFormula = xSrcCell->getFormula();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(3,0);
    OUString aDestFormula = xDestCell->getFormula();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Namedrange formula imported", aDestFormula, aSrcFormula);
}

void XSpreadsheets2::testImportOverExistingNamedRange()
{
/**
    Both Source and Target file define the named range initial1
    in Source, initial1 is defined outside the copied sheet
    In Target, after import sheet, initial1 should point on its initial definition $Sheet1.$B$1

    NEED MORE WORK
*/
    importSheetToCopy();

    OUString aNamedRangeString("initial1");

    uno::Reference< container::XNameAccess > xDestNamedRangesNameAccess(getNamedRanges(xDestDoc), UNO_QUERY_THROW);
    uno::Any aNr = xDestNamedRangesNameAccess->getByName(aNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNamedRange(aNr, UNO_QUERY_THROW);
    OUString aNrDestContent = xDestNamedRange->getContent();

    OUString aExpectedContent("$Sheet1.$B$1");

    std::cout << "testImportSheet : initial1 aNrDestContent " << aNrDestContent << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong address for initial1", aNrDestContent, aExpectedContent);

}

void XSpreadsheets2::testImportNamedRangeDefinedInSource()
{
/**
    in Source file, InSheetRangeName named range is defined in the copied sheet
    it does not exists in target file
    test that the range named is created in target and that it points in the target copied sheet
*/
    importSheetToCopy();

    // New range name defined in imported sheet $SheetToCopy.$A$7
    OUString aNewInSheetNamedRangeString("InSheetRangeName");
    uno::Reference< container::XNameAccess > xDestNamedRangesNameAccess(getNamedRanges(xDestDoc), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("InSheetRangeName", xDestNamedRangesNameAccess->hasByName(aNewInSheetNamedRangeString));

    uno::Any aNewInSheetNr = xDestNamedRangesNameAccess->getByName(aNewInSheetNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNewInSheetNamedRange(aNewInSheetNr, UNO_QUERY_THROW);
    OUString aNewInSheetNrDestContent = xDestNewInSheetNamedRange->getContent();
    OUString aNewInSheetExpectedContent("$SheetToCopy.$A$7");

    std::cout << "testImportSheet : InSheetRangeName content " << aNewInSheetNrDestContent << std::endl;
    std::cout << "testImportSheet : InSheetRangeName expected " << aNewInSheetExpectedContent << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong address for InSheetRangeName", aNewInSheetNrDestContent, aNewInSheetExpectedContent);
}

void XSpreadsheets2::testImportNamedRangeRedefinedInSource()
{
/**
    in Source file, initial2 named range is defined in the copied sheet
    it is defined in another sheet of target file
    test that the range named points in the target copied sheet
*/
    importSheetToCopy();

    // the source file redefines an existing named range in the imported sheet --> the target should not be changed
    OUString aRedefinedInSheetNamedRangeString("initial2");
    uno::Reference< container::XNameAccess > xDestNamedRangesNameAccess(getNamedRanges(xDestDoc), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("aRedefinedInSheetNamedRangeString", xDestNamedRangesNameAccess->hasByName(aRedefinedInSheetNamedRangeString));

    uno::Any aRedefinedInSheetNr = xDestNamedRangesNameAccess->getByName(aRedefinedInSheetNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestRedefinedInSheetNamedRange(aRedefinedInSheetNr, UNO_QUERY_THROW);
    OUString aRedefinedInSheetNrDestContent = xDestRedefinedInSheetNamedRange->getContent();
    OUString aRedefinedInSheetExpectedContent("$Sheet1.$B$2");
    std::cout << "testImportSheet : initial2 content " << aRedefinedInSheetNrDestContent << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong address for Redefined InSheet named range", aRedefinedInSheetNrDestContent, aRedefinedInSheetExpectedContent);
}

void XSpreadsheets2::testImportNewNamedRange()
{
/**
    in Source file, new_rangename range named is defined outside the copied sheet
    it does not exists in target file test that new_rangename is created and its
    content points to source file as an external reference
*/
    importSheetToCopy();

    //formula with a non-existent named range in dest - new_rangename
    OUString aNewNamedRangeString("new_rangename");
    uno::Reference< container::XNameAccess > xDestNamedRangesNameAccess(getNamedRanges(xDestDoc), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("New NamedRange not created", xDestNamedRangesNameAccess->hasByName(aNewNamedRangeString));

    // verify the content of this new namedrange, pointing on $Sheet1.$B$1 in source. This address is already defined in target as NR content

    uno::Any aNewNr = xDestNamedRangesNameAccess->getByName(aNewNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNewNamedRange(aNewNr, UNO_QUERY_THROW);
    OUString aNewNrDestContent = xDestNewNamedRange->getContent();

    OUString aNewExpectedContent("$Sheet1.$B$1");

    std::cout << "testImportSheet : new_rangename aNewExpectedContent " << aNewExpectedContent << std::endl;
    std::cout << "testImportSheet : new_rangename aNewNrDestContent " << aNewNrDestContent << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Wrong New NamedRange formula string value", isExternalReference(aNewNrDestContent, aNewExpectedContent));
}

void XSpreadsheets2::testImportCellStyle()
{
/**
    in source file, imported sheet uses a cellstyle that does not exists in target
    test that
        - an imported cell D1 uses the right cellStyle
        - the cellStyle is created in CellStyles family
        - a property of the cellStyle (VertJustify) is correctly set
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(3,0);
    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(3,0);

    //new style created in dest
    uno::Reference< beans::XPropertySet > xSrcCellPropSet (xSrcCell, UNO_QUERY_THROW);
    const OUString aCellProperty("CellStyle");
    OUString aSrcStyleName;
    CPPUNIT_ASSERT(xSrcCellPropSet->getPropertyValue(aCellProperty) >>= aSrcStyleName);

    uno::Reference< beans::XPropertySet > xDestCellPropSet (xSrcCell, UNO_QUERY_THROW);
    OUString aDestStyleName;
    CPPUNIT_ASSERT(xDestCellPropSet->getPropertyValue(aCellProperty) >>= aDestStyleName);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong imported Cell Style", aDestStyleName, aSrcStyleName);

    uno::Reference< style::XStyleFamiliesSupplier > xFamiliesSupplier (xDestDoc, UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xFamiliesNameAccess (xFamiliesSupplier->getStyleFamilies(), UNO_QUERY_THROW);
    OUString aCellFamilyName("CellStyles");
    uno::Any xCellStylesFamily = xFamiliesNameAccess->getByName(aCellFamilyName);
    uno::Reference< container::XNameContainer > xCellStylesFamilyNameAccess (xCellStylesFamily, UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("New cell style not present", xCellStylesFamilyNameAccess->hasByName(aDestStyleName));

    uno::Any aCellStyle = xCellStylesFamilyNameAccess->getByName(aDestStyleName);
    uno::Reference< beans::XPropertySet > xCellStyleProp (aCellStyle, UNO_QUERY_THROW);
    OUString aProperty("VertJustify");
    sal_Int32 aVertJustify = 0;
    CPPUNIT_ASSERT(xCellStyleProp->getPropertyValue(aProperty) >>= aVertJustify);

    CPPUNIT_ASSERT_MESSAGE("New style: VertJustify not set", aVertJustify == table::CellVertJustify_CENTER);
}

uno::Reference< sheet::XSpreadsheetDocument> XSpreadsheets2::getDoc(const OUString& aFileBase, uno::Reference< lang::XComponent >& xComp)
{
    OUString aFileURL;
    createFileURL(aFileBase, aFileURL);

    if (!xComp.is())
        xComp = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT(xComp.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComp, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDoc.is());
    return xDoc;
}

uno::Reference< sheet::XNamedRanges> XSpreadsheets2::getNamedRanges(uno::Reference< sheet::XSpreadsheetDocument> xDoc)
{
    uno::Reference< beans::XPropertySet > xPropSet (xDoc, UNO_QUERY_THROW);
    OUString NamedRangesPropertyString("NamedRanges");
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue(NamedRangesPropertyString), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNamedRanges.is());

    return xNamedRanges;
}

void XSpreadsheets2::importSheetToCopy()
{
    uno::Reference< container::XNameAccess> xSrcNameAccess(init(),UNO_QUERY_THROW);
    xSrcSheet.set( xSrcNameAccess->getByName(aSrcSheetName), UNO_QUERY_THROW);

    uno::Reference< lang::XComponent > xDestComponent;
    if (!xDestComponent.is())
    {
        xDestDoc = getDoc(aDestFileBase, xDestComponent);
        CPPUNIT_ASSERT(xDestDoc.is());

        // import sheet
        uno::Reference< sheet::XSpreadsheets2 > xDestSheets (xDestDoc->getSheets(), UNO_QUERY_THROW);
        sal_Int32 nDestPos = 0;
        sal_Int32 nDestPosEffective = xDestSheets->importSheet(xDocument, aSrcSheetName, nDestPos);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet index", nDestPosEffective, nDestPos);
    }
    else
    {
        xDestDoc.set(xDestComponent,UNO_QUERY_THROW);
    }

    uno::Reference< container::XNameAccess > xDestSheetNameAccess (xDestDoc->getSheets(), UNO_QUERY_THROW);
    xDestSheet.set( xDestSheetNameAccess->getByName(aSrcSheetName), UNO_QUERY_THROW);
}

bool XSpreadsheets2::isExternalReference(const OUString& aDestContent, const OUString& aSrcContent )
{
    OUString aStart("'file://");

    CPPUNIT_ASSERT(aDestContent.startsWith(aStart));

    return  (aDestContent.endsWithIgnoreAsciiCase(aSrcContent) // same cell address
            && aDestContent.indexOf(aSrcFileName)>0); // contains source file name
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
