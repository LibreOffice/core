/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <swmodeltestbase.hxx>

typedef std::map<OUString, com::sun::star::uno::Sequence< com::sun::star::table::BorderLine> > AllBordersMap;
typedef std::pair<OUString, com::sun::star::uno::Sequence< com::sun::star::table::BorderLine> > StringSequencePair;

class Test : public SwModelTestBase
{
public:
    void testEmptySvgFamilyName();
    void testHideAllSections();
    void testOdtBorders();
    void testPageStyleLayoutDefault();
    void testPageStyleLayoutRight();
    void testFdo61952();
    void testFdo60842();
    void testFdo56272();
    void testFdo55814();
    void testFdo68839();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(run);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void run();
};

void Test::run()
{
    MethodEntry<Test> aMethods[] = {
        {"empty-svg-family-name.odt", &Test::testEmptySvgFamilyName},
        {"fdo53210.odt", &Test::testHideAllSections},
        {"borders_ooo33.odt", &Test::testOdtBorders},
        {"hello.odt", &Test::testPageStyleLayoutDefault},
        {"hello.odt", &Test::testPageStyleLayoutRight},
        {"hello.odt", &Test::testFdo61952},
        {"fdo60842.odt", &Test::testFdo60842},
        {"fdo56272.odt", &Test::testFdo56272},
        {"fdo55814.odt", &Test::testFdo55814},
        {"fdo68839.odt", &Test::testFdo68839},
    };
    header();
    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aMethods); ++i)
    {
        MethodEntry<Test>& rEntry = aMethods[i];
        load("/sw/qa/extras/odfimport/data/",  rEntry.pName);
        (this->*rEntry.pMethod)();
        finish();
    }
}

void Test::testEmptySvgFamilyName()
{
    // .odt import did crash on the empty font list (which I think is valid according SVG spec)
}

void Test::testHideAllSections()
{
    // This document has a section that is conditionally hidden, but has no empty paragraph after it.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xMasters = xTextFieldsSupplier->getTextFieldMasters();
    // Set _CS_Allgemein to 0
    uno::Reference<beans::XPropertySet> xMaster(xMasters->getByName("com.sun.star.text.fieldmaster.User._CS_Allgemein"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Content", uno::makeAny(OUString("0")));
    // This used to crash
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY)->refresh();
}

void Test::testOdtBorders()
{
    AllBordersMap map;
    uno::Sequence< table::BorderLine > tempSequence(4);

    tempSequence[0] = table::BorderLine(0, 2, 2, 35);   // left
    tempSequence[1] = table::BorderLine(0, 2, 2, 35);   // right
    tempSequence[2] = table::BorderLine(0, 2, 2, 35);   // top
    tempSequence[3] = table::BorderLine(0, 2, 2, 35);   // bottom
    map.insert(StringSequencePair(OUString("B2"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 0, 2, 0);
    tempSequence[1] = table::BorderLine(0, 0, 2, 0);
    tempSequence[2] = table::BorderLine(0, 0, 2, 0);
    tempSequence[3] = table::BorderLine(0, 0, 2, 0);
    map.insert(StringSequencePair(OUString("D2"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 2, 2, 88);
    tempSequence[1] = table::BorderLine(0, 2, 2, 88);
    tempSequence[2] = table::BorderLine(0, 2, 2, 88);
    tempSequence[3] = table::BorderLine(0, 2, 2, 88);
    map.insert(StringSequencePair(OUString("B4"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 0, 18, 0);
    tempSequence[1] = table::BorderLine(0, 0, 18, 0);
    tempSequence[2] = table::BorderLine(0, 0, 18, 0);
    tempSequence[3] = table::BorderLine(0, 0, 18, 0);
    map.insert(StringSequencePair(OUString("D4"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 35, 35, 35);
    tempSequence[1] = table::BorderLine(0, 35, 35, 35);
    tempSequence[2] = table::BorderLine(0, 35, 35, 35);
    tempSequence[3] = table::BorderLine(0, 35, 35, 35);
    map.insert(StringSequencePair(OUString("B6"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 0, 35, 0);
    tempSequence[1] = table::BorderLine(0, 0, 35, 0);
    tempSequence[2] = table::BorderLine(0, 0, 35, 0);
    tempSequence[3] = table::BorderLine(0, 0, 35, 0);
    map.insert(StringSequencePair(OUString("D6"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 88, 88, 88);
    tempSequence[1] = table::BorderLine(0, 88, 88, 88);
    tempSequence[2] = table::BorderLine(0, 88, 88, 88);
    tempSequence[3] = table::BorderLine(0, 88, 88, 88);
    map.insert(StringSequencePair(OUString("B8"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 0, 88, 0);
    tempSequence[1] = table::BorderLine(0, 0, 88, 0);
    tempSequence[2] = table::BorderLine(0, 0, 88, 0);
    tempSequence[3] = table::BorderLine(0, 0, 88, 0);
    map.insert(StringSequencePair(OUString("D8"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 2, 35, 88);
    tempSequence[1] = table::BorderLine(0, 2, 35, 88);
    tempSequence[2] = table::BorderLine(0, 2, 35, 88);
    tempSequence[3] = table::BorderLine(0, 2, 35, 88);
    map.insert(StringSequencePair(OUString("B10"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 0, 141, 0);
    tempSequence[1] = table::BorderLine(0, 0, 141, 0);
    tempSequence[2] = table::BorderLine(0, 0, 141, 0);
    tempSequence[3] = table::BorderLine(0, 0, 141, 0);
    map.insert(StringSequencePair(OUString("D10"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 2, 88, 88);
    tempSequence[1] = table::BorderLine(0, 2, 88, 88);
    tempSequence[2] = table::BorderLine(0, 2, 88, 88);
    tempSequence[3] = table::BorderLine(0, 2, 88, 88);
    map.insert(StringSequencePair(OUString("B12"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 0, 176, 0);
    tempSequence[1] = table::BorderLine(0, 0, 176, 0);
    tempSequence[2] = table::BorderLine(0, 0, 176, 0);
    tempSequence[3] = table::BorderLine(0, 0, 176, 0);
    map.insert(StringSequencePair(OUString("D12"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 2, 141, 88);
    tempSequence[1] = table::BorderLine(0, 2, 141, 88);
    tempSequence[2] = table::BorderLine(0, 2, 141, 88);
    tempSequence[3] = table::BorderLine(0, 2, 141, 88);
    map.insert(StringSequencePair(OUString("B14"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 35, 88, 88);
    tempSequence[1] = table::BorderLine(0, 35, 88, 88);
    tempSequence[2] = table::BorderLine(0, 35, 88, 88);
    tempSequence[3] = table::BorderLine(0, 35, 88, 88);
    map.insert(StringSequencePair(OUString("B16"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 88, 35, 35);
    tempSequence[1] = table::BorderLine(0, 88, 35, 35);
    tempSequence[2] = table::BorderLine(0, 88, 35, 35);
    tempSequence[3] = table::BorderLine(0, 88, 35, 35);
    map.insert(StringSequencePair(OUString("B18"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 88, 141, 88);
    tempSequence[1] = table::BorderLine(0, 88, 141, 88);
    tempSequence[2] = table::BorderLine(0, 88, 141, 88);
    tempSequence[3] = table::BorderLine(0, 88, 141, 88);
    map.insert(StringSequencePair(OUString("B20"), tempSequence));
    tempSequence[0] = table::BorderLine(0, 141, 88, 88);
    tempSequence[1] = table::BorderLine(0, 141, 88, 88);
    tempSequence[2] = table::BorderLine(0, 141, 88, 88);
    tempSequence[3] = table::BorderLine(0, 141, 88, 88);
    map.insert(StringSequencePair(OUString("B22"), tempSequence));

    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            if (xServiceInfo->supportsService("com.sun.star.text.TextTable"))
            {
                uno::Reference<text::XTextTable> const xTextTable(xServiceInfo, uno::UNO_QUERY_THROW);
                uno::Sequence<OUString> const cells = xTextTable->getCellNames();
                uno::Sequence< uno::Sequence< sal_Int32 > > borderWidthSequence;
                sal_Int32 nLength = cells.getLength();

                AllBordersMap::iterator it;
                it = map.begin();

                for (sal_Int32 i = 0; i < nLength; ++i)
                {
                    if(i%10==6 || (i%10==8 && i<60))    // only B and D cells have borders, every 2 rows
                    {
                        uno::Reference<table::XCell> xCell = xTextTable->getCellByName(cells[i]);
                        uno::Reference< beans::XPropertySet > xPropSet(xCell, uno::UNO_QUERY_THROW);
                        it = map.find(cells[i]);

                        uno::Any aLeftBorder = xPropSet->getPropertyValue("LeftBorder");
                        table::BorderLine aLeftBorderLine;
                        if (aLeftBorder >>= aLeftBorderLine)
                        {
                            sal_Int32 innerLineWidth = aLeftBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aLeftBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aLeftBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[0].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[0].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[0].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }

                        uno::Any aRightBorder = xPropSet->getPropertyValue("RightBorder");
                        table::BorderLine aRightBorderLine;
                        if (aRightBorder >>= aRightBorderLine)
                        {
                            sal_Int32 innerLineWidth = aRightBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aRightBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aRightBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[1].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[1].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[1].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }

                        uno::Any aTopBorder = xPropSet->getPropertyValue("TopBorder");
                        table::BorderLine aTopBorderLine;
                        if (aTopBorder >>= aTopBorderLine)
                        {
                            sal_Int32 innerLineWidth = aTopBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aTopBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aTopBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[2].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[2].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[2].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }

                        uno::Any aBottomBorder = xPropSet->getPropertyValue("BottomBorder");
                        table::BorderLine aBottomBorderLine;
                        if (aBottomBorder >>= aBottomBorderLine)
                        {
                            sal_Int32 innerLineWidth = aBottomBorderLine.InnerLineWidth;
                            sal_Int32 outerLineWidth = aBottomBorderLine.OuterLineWidth;
                            sal_Int32 lineDistance = aBottomBorderLine.LineDistance;

                            sal_Int32 perfectInner = it->second[3].InnerLineWidth;
                            sal_Int32 perfectOuter = it->second[3].OuterLineWidth;
                            sal_Int32 perfectDistance = it->second[3].LineDistance;
                            CPPUNIT_ASSERT_EQUAL(perfectInner, innerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectOuter, outerLineWidth);
                            CPPUNIT_ASSERT_EQUAL(perfectDistance, lineDistance);
                        }
                    }
                }   //end of the 'for' loop
            }
        }
    } while(xParaEnum->hasMoreElements());
}

void Test::testPageStyleLayoutDefault()
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Style"), uno::UNO_QUERY);
    // This was style::PageStyleLayout_MIRRORED.
    CPPUNIT_ASSERT_EQUAL(style::PageStyleLayout_ALL, getProperty<style::PageStyleLayout>(xPropertySet, "PageStyleLayout"));
}

void Test::testPageStyleLayoutRight()
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Style"), uno::UNO_QUERY);
    // This caused a crash.
    xPropertySet->setPropertyValue("PageStyleLayout", uno::makeAny(style::PageStyleLayout_RIGHT));
}

void Test::testFdo61952()
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(0), uno::UNO_QUERY);
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Left Page")));
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Right Page")));
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Left Page")));
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Right Page")));
}

void Test::testFdo60842()
{
    uno::Reference<text::XTextContent> const xTable(getParagraphOrTable(0));
    getCell(xTable, "A1", "");
    getCell(xTable, "B1", "18/02/2012");
    getCell(xTable, "C1", "USD"); // this is the cell with office:string-value
    getCell(xTable, "D1", "");
    getCell(xTable, "E1", "01/04/2012");
}

void Test::testFdo56272()
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Vertical position was incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(422), xShape->getPosition().Y); // Was -2371
}

void Test::testFdo55814()
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);
    xField->setPropertyValue("Content", uno::makeAny(OUString("Yes")));
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY)->refresh();
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was "0".
    CPPUNIT_ASSERT_EQUAL(OUString("Hide==\"Yes\""), getProperty<OUString>(xSections->getByIndex(0), "Condition"));
}

void lcl_CheckShape(
    uno::Reference<drawing::XShape> const& xShape, OUString const& rExpected)
{
    uno::Reference<container::XNamed> const xNamed(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xNamed.is());
    CPPUNIT_ASSERT_EQUAL(rExpected, xNamed->getName());
}

void Test::testFdo68839()
{
    // check names
    lcl_CheckShape(getShape(1), "FrameXXX");
    lcl_CheckShape(getShape(2), "ObjectXXX");
    lcl_CheckShape(getShape(3), "FrameY");
    lcl_CheckShape(getShape(4), "graphicsXXX");
    try {
        uno::Reference<drawing::XShape> xShape = getShape(5);
        CPPUNIT_ASSERT(!"IndexOutOfBoundsException expected");
    } catch (lang::IndexOutOfBoundsException const&) { }
    // check prev/next chain
    uno::Reference<beans::XPropertySet> xFrame1(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame2(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(),
            getProperty<OUString>(xFrame1, "ChainPrevName"));
    CPPUNIT_ASSERT_EQUAL(OUString("FrameY"),
            getProperty<OUString>(xFrame1, "ChainNextName"));
    CPPUNIT_ASSERT_EQUAL(OUString("FrameXXX"),
            getProperty<OUString>(xFrame2, "ChainPrevName"));
    CPPUNIT_ASSERT_EQUAL(OUString(),
            getProperty<OUString>(xFrame2, "ChainNextName"));
}


CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
