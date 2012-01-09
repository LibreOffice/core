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
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>

#include <rtl/oustringostreaminserter.hxx>
#include <rtl/string.hxx>



namespace ScSpreadsheetObj {

#define NUMBER_OF_TESTS 1

class ScXSpreadsheets2 : public UnoApiTest
{
public:

    virtual void setUp();
    virtual void tearDown();

    uno::Reference< sheet::XSpreadsheetDocument> getDoc(const rtl::OUString, uno::Reference< lang::XComponent >);
    uno::Reference< sheet::XNamedRanges> getNamedRanges(uno::Reference< sheet::XSpreadsheetDocument >);
    void importSheetToCopy();
    bool isExternalReference(const rtl::OUString aDestContent, const rtl::OUString aSrcContent );

    // XSpreadsheets2
    void testImportedSheetNameAndIndex();
    void testImportString();
    void testImportValue();
    void testImportFormulaBasicMath();
    void testImportFormulaWithNamedRange();
    void testImportOverExistingNamedRange();
    void testImportNamedRangeDefinedInSource();
    void testImportNamedRangeRedefinedInSource();
    void testImportNewNamedRange();
    void testImportCellStyle();

    CPPUNIT_TEST_SUITE(ScXSpreadsheets2);
    CPPUNIT_TEST(testImportedSheetNameAndIndex);
    CPPUNIT_TEST(testImportString);
    CPPUNIT_TEST(testImportValue);
    CPPUNIT_TEST(testImportFormulaBasicMath);
    CPPUNIT_TEST(testImportFormulaWithNamedRange);
    CPPUNIT_TEST(testImportOverExistingNamedRange);
    CPPUNIT_TEST(testImportNamedRangeDefinedInSource);
    CPPUNIT_TEST(testImportNamedRangeRedefinedInSource);
    CPPUNIT_TEST(testImportNewNamedRange);
    CPPUNIT_TEST(testImportCellStyle);

    CPPUNIT_TEST_SUITE_END();

private:
    static int nTest;
    static uno::Reference< lang::XComponent > xComponent;

    static bool bIsSheetImported;
    static uno::Reference< sheet::XSpreadsheetDocument> xSrcDoc;
    static uno::Reference< sheet::XSpreadsheetDocument> xDestDoc;
    static uno::Reference< container::XNameAccess > xSrcNamedRangesNameAccess;
    static uno::Reference< container::XNameAccess > xDestNamedRangesNameAccess;
    static uno::Reference< container::XNameAccess > xDestSheetNameAccess;
    static uno::Reference< sheet::XSpreadsheet > xDestSheet;
    static uno::Reference< sheet::XSpreadsheet > xSrcSheet;
    static sal_Int32 nDestPosEffective;
    static rtl::OUString aSrcSheetName;
    static rtl::OUString aSrcFileBase;
    static rtl::OUString aDestFileBase;
    static sal_Int32 nDestPos;
};

int ScXSpreadsheets2::nTest = 0;
uno::Reference< lang::XComponent > ScXSpreadsheets2::xComponent;

bool ScXSpreadsheets2::bIsSheetImported = false;
uno::Reference< sheet::XSpreadsheetDocument> ScXSpreadsheets2::xSrcDoc;
uno::Reference< sheet::XSpreadsheetDocument> ScXSpreadsheets2::xDestDoc;
uno::Reference< container::XNameAccess > ScXSpreadsheets2::xSrcNamedRangesNameAccess;
uno::Reference< container::XNameAccess > ScXSpreadsheets2::xDestNamedRangesNameAccess;
uno::Reference< container::XNameAccess > ScXSpreadsheets2::xDestSheetNameAccess;
uno::Reference< sheet::XSpreadsheet > ScXSpreadsheets2::xDestSheet;
uno::Reference< sheet::XSpreadsheet > ScXSpreadsheets2::xSrcSheet;
rtl::OUString ScXSpreadsheets2::aSrcSheetName(RTL_CONSTASCII_USTRINGPARAM("SheetToCopy"));
rtl::OUString ScXSpreadsheets2::aSrcFileBase(RTL_CONSTASCII_USTRINGPARAM("rangenamessrc.ods"));
rtl::OUString ScXSpreadsheets2::aDestFileBase(RTL_CONSTASCII_USTRINGPARAM("rangenames.ods"));
sal_Int32 ScXSpreadsheets2::nDestPos = 0;
sal_Int32 ScXSpreadsheets2::nDestPosEffective;

void ScXSpreadsheets2::testImportedSheetNameAndIndex()
{
/**
    Verfiy that the imported sheet has the correct name and is placed at the right requested index
*/

    importSheetToCopy();

    CPPUNIT_ASSERT_MESSAGE("Wrong sheet name", xDestSheetNameAccess->hasByName(aSrcSheetName));
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet index", nDestPosEffective == nDestPos);

}

void ScXSpreadsheets2::testImportString()
{
/**
    tests the cell A1 containing a string correctly imported
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(0,0);
    uno::Reference< text::XTextRange > xSrcTextRange(xSrcCell, UNO_QUERY_THROW);
    rtl::OUString aSrcString = xSrcTextRange->getString();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(0,0);
    uno::Reference< text::XTextRange > xDestTextRange(xDestCell, UNO_QUERY_THROW);
    rtl::OUString aDestString = xDestTextRange->getString();

    CPPUNIT_ASSERT_MESSAGE("Wrong string imported", aDestString.equals(aSrcString));
}

void ScXSpreadsheets2::testImportValue()
{
/**
    tests the cell B1 containing a value correctly imported
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(1,0);
    sal_Int32 aSrcValue = xSrcCell->getValue();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(1,0);
    sal_Int32 aDestValue = xDestCell->getValue();

    CPPUNIT_ASSERT_MESSAGE("Wrong value imported", aSrcValue == aDestValue);
}

void ScXSpreadsheets2::testImportFormulaBasicMath()
{
/**
    tests the cell C1 containing an arithmetic formula correctly imported
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(2,0);
    rtl::OUString aSrcFormula = xSrcCell->getFormula();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(2,0);
    rtl::OUString aDestFormula = xDestCell->getFormula();

    // potential problem later: formulas might be adjusted
    // add some tests that the formulas are correctly adjusted
    CPPUNIT_ASSERT_MESSAGE("Wrong formula imported", aDestFormula.equals(aSrcFormula));
}

void ScXSpreadsheets2::testImportFormulaWithNamedRange()
{
/**
    tests the cell D1 containing a formula that uses a NamedRange expression
*/
    importSheetToCopy();

    uno::Reference< table::XCell > xSrcCell = xSrcSheet->getCellByPosition(3,0);
    rtl::OUString aSrcFormula = xSrcCell->getFormula();

    uno::Reference< table::XCell > xDestCell = xDestSheet->getCellByPosition(3,0);
    rtl::OUString aDestFormula = xDestCell->getFormula();

    CPPUNIT_ASSERT_MESSAGE("Wrong Namedrange formula imported", aDestFormula.equals(aSrcFormula));
}

void ScXSpreadsheets2::testImportOverExistingNamedRange()
{
/**
    Both Source and Target file define the named range initial1
    in Source, initial1 is defined outside the copied sheet
    In Target, after import sheet, initial1 should point on its initial definition $Sheet1.$B$1

    NEED MORE WORK
*/
    importSheetToCopy();

    rtl::OUString aNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial1"));

    uno::Any aNr = xDestNamedRangesNameAccess->getByName(aNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNamedRange(aNr, UNO_QUERY_THROW);
    rtl::OUString aNrDestContent = xDestNamedRange->getContent();

    rtl::OUString aExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$B$1"));

    std::cout << "testImportSheet : initial1 aNrDestContent " << aNrDestContent << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Wrong address for initial1", aNrDestContent.equals(aExpectedContent));

}

void ScXSpreadsheets2::testImportNamedRangeDefinedInSource()
{
/**
    in Source file, InSheetRangeName named range is defined in the copied sheet
    it does not exists in target file
    test that the range named is created in target and that it points in the target copied sheet
*/
    importSheetToCopy();

    // New range name defined in imported sheet $SheetToCopy.$A$7
    rtl::OUString aNewInSheetNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("InSheetRangeName"));
    CPPUNIT_ASSERT_MESSAGE("InSheetRangeName", xDestNamedRangesNameAccess->hasByName(aNewInSheetNamedRangeString));

    uno::Any aNewInSheetNr = xDestNamedRangesNameAccess->getByName(aNewInSheetNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNewInSheetNamedRange(aNewInSheetNr, UNO_QUERY_THROW);
    rtl::OUString aNewInSheetNrDestContent = xDestNewInSheetNamedRange->getContent();
    rtl::OUString aNewInSheetExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$SheetToCopy.$A$7"));

    std::cout << "testImportSheet : InSheetRangeName content " << aNewInSheetNrDestContent << std::endl;
    std::cout << "testImportSheet : InSheetRangeName expected " << aNewInSheetExpectedContent << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Wrong address for InSheetRangeName", aNewInSheetNrDestContent.equals(aNewInSheetExpectedContent));
}

void ScXSpreadsheets2::testImportNamedRangeRedefinedInSource()
{
/**
    in Source file, initial2 named range is defined in the copied sheet
    it is defined in another sheet of target file
    test that the range named points in the target copied sheet
*/
    importSheetToCopy();

    // the source file redefines an existing named range in the imported sheet --> the target should not be changed
    rtl::OUString aRedefinedInSheetNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial2"));
    CPPUNIT_ASSERT_MESSAGE("aRedefinedInSheetNamedRangeString", xDestNamedRangesNameAccess->hasByName(aRedefinedInSheetNamedRangeString));

    uno::Any aRedefinedInSheetNr = xDestNamedRangesNameAccess->getByName(aRedefinedInSheetNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestRedefinedInSheetNamedRange(aRedefinedInSheetNr, UNO_QUERY_THROW);
    rtl::OUString aRedefinedInSheetNrDestContent = xDestRedefinedInSheetNamedRange->getContent();
    rtl::OUString aRedefinedInSheetExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$B$2"));
    std::cout << "testImportSheet : initial2 content " << aRedefinedInSheetNrDestContent << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Wrong address for Redefined InSheet named range", aRedefinedInSheetNrDestContent.equals(aRedefinedInSheetExpectedContent));
}

void ScXSpreadsheets2::testImportNewNamedRange()
{
/**
    in Soucre file, new_rangename range named is defined outside the copied sheet
    it does not exists in target file
    test that new_rangename is created and its content points to source file as an external reference
*/
    importSheetToCopy();

    //formula with a non-existant named range in dest - new_rangename
    rtl::OUString aNewNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("new_rangename"));
    CPPUNIT_ASSERT_MESSAGE("New NamedRange not created", xDestNamedRangesNameAccess->hasByName(aNewNamedRangeString));

    // verify the content of this new namedrange, pointing on $Sheet1.$B$1 in source. This address is already defined in target as NR content

    uno::Any aNewNr = xDestNamedRangesNameAccess->getByName(aNewNamedRangeString);
    uno::Reference< sheet::XNamedRange > xDestNewNamedRange(aNewNr, UNO_QUERY_THROW);
    rtl::OUString aNewNrDestContent = xDestNewNamedRange->getContent();

    rtl::OUString aNewExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$B$1"));

    std::cout << "testImportSheet : new_rangename aNewExpectedContent " << aNewExpectedContent << std::endl;
    std::cout << "testImportSheet : new_rangename aNewNrDestContent " << aNewNrDestContent << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Wrong New NamedRange formula string value", isExternalReference(aNewNrDestContent, aNewExpectedContent));
}

void ScXSpreadsheets2::testImportCellStyle()
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

uno::Reference< sheet::XSpreadsheetDocument> ScXSpreadsheets2::getDoc(const rtl::OUString aFileBase, uno::Reference< lang::XComponent > xComp)
{
    rtl::OUString aFileURL;
    createFileURL(aFileBase, aFileURL);

    if (!xComp.is())
        xComp = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT(xComp.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComp, UNO_QUERY_THROW);
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

void ScXSpreadsheets2::importSheetToCopy()
{
    if (!bIsSheetImported)
    {
        xSrcDoc = getDoc(aSrcFileBase, xComponent);
        CPPUNIT_ASSERT(xSrcDoc.is());
        xSrcNamedRangesNameAccess = uno::Reference< container::XNameAccess> (getNamedRanges(xSrcDoc), UNO_QUERY_THROW);

        uno::Reference< lang::XComponent > xDestComponent;
        xDestDoc = getDoc(aDestFileBase, xDestComponent);
        CPPUNIT_ASSERT(xDestDoc.is());
        xDestNamedRangesNameAccess = uno::Reference< container::XNameAccess > (getNamedRanges(xDestDoc), UNO_QUERY_THROW);

        // import sheet
        uno::Reference< sheet::XSpreadsheets2 > xDestSheets (xDestDoc->getSheets(), UNO_QUERY_THROW);
        nDestPosEffective = xDestSheets->importSheet(xSrcDoc, aSrcSheetName, nDestPos);

        uno::Reference< container::XNameAccess > xSrcSheetNameAccess (xSrcDoc->getSheets(), UNO_QUERY_THROW);
        xSrcSheet = uno::Reference< sheet::XSpreadsheet > (xSrcSheetNameAccess->getByName(aSrcSheetName), UNO_QUERY_THROW);

        xDestSheetNameAccess = uno::Reference< container::XNameAccess > (xDestDoc->getSheets(), UNO_QUERY_THROW);
        xDestSheet = uno::Reference< sheet::XSpreadsheet > ( xDestSheetNameAccess->getByName(aSrcSheetName), UNO_QUERY_THROW);

        bIsSheetImported = true;

        // store destFile
        //uno::Reference< frame::XStorable > xStore (xDestDoc, UNO_QUERY_THROW);
        //xStore->store();

    }
}

bool ScXSpreadsheets2::isExternalReference(rtl::OUString aDestContent, rtl::OUString aSrcContent )
{
    rtl::OUString aStart(RTL_CONSTASCII_USTRINGPARAM("'file://"));
    const sal_Char* sSrcContent = rtl::OUStringToOString( aSrcContent, RTL_TEXTENCODING_UTF8 ).getStr();
/*
    std::cout << "aDestContent " << aDestContent << std::endl;
    std::cout << "aSrcContent " << aSrcContent << std::endl;
    std::cout << "endsWithIgnoreAsciiCaseAsciiL " << aDestContent.endsWithIgnoreAsciiCaseAsciiL(sSrcContent, aSrcContent.getLength()) << std::endl;
    std::cout << "indexOf(aStart) " << aDestContent.indexOf(aStart) << std::endl;
    std::cout << "indexOf(aSrcFileBase) " << aDestContent.indexOf(aSrcFileBase) << std::endl;
*/
    return  (aDestContent.endsWithIgnoreAsciiCaseAsciiL(sSrcContent, aSrcContent.getLength()) // same cell address
            && aDestContent.indexOf(aStart)==0 // starts with 'file://
            && aDestContent.indexOf(aSrcFileBase)>0); // contains source file name
}

void ScXSpreadsheets2::setUp()
{
    nTest += 1;
    UnoApiTest::setUp();
}

void ScXSpreadsheets2::tearDown()
{
    //closing the document fails ATM
    if (nTest == NUMBER_OF_TESTS)
    {
        //uno::Reference< util::XCloseable > xCloseable(xComponent, UNO_QUERY_THROW);
        //xCloseable->close( false );
    }

    UnoApiTest::tearDown();

    if (nTest == NUMBER_OF_TESTS)
    {
        //mxDesktop->terminate();
        //uno::Reference< lang::XComponent>(m_xContext, UNO_QUERY_THROW)->dispose();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScXSpreadsheets2);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
