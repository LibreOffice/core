/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <olmenu.hxx>
#include <cmdid.h>

typedef std::map<OUString, com::sun::star::uno::Sequence< com::sun::star::table::BorderLine> > AllBordersMap;
typedef std::pair<OUString, com::sun::star::uno::Sequence< com::sun::star::table::BorderLine> > StringSequencePair;

class Test : public SwModelTestBase
{
    public:
        Test() : SwModelTestBase("/sw/qa/extras/odfimport/data/", "writer8") {}
};

#define DECLARE_ODFIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, Test)

DECLARE_ODFIMPORT_TEST(testEmptySvgFamilyName, "empty-svg-family-name.odt")
{
    // .odt import did crash on the empty font list (which I think is valid according SVG spec)
}

DECLARE_ODFIMPORT_TEST(testHideAllSections, "fdo53210.odt")
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

DECLARE_ODFIMPORT_TEST(testOdtBorders, "borders_ooo33.odt")
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

DECLARE_ODFIMPORT_TEST(testPageStyleLayoutDefault, "hello.odt")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Style"), uno::UNO_QUERY);
    // This was style::PageStyleLayout_MIRRORED.
    CPPUNIT_ASSERT_EQUAL(style::PageStyleLayout_ALL, getProperty<style::PageStyleLayout>(xPropertySet, "PageStyleLayout"));
}

DECLARE_ODFIMPORT_TEST(testPageStyleLayoutRight, "hello.odt")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Style"), uno::UNO_QUERY);
    // This caused a crash.
    xPropertySet->setPropertyValue("PageStyleLayout", uno::makeAny(style::PageStyleLayout_RIGHT));
}

DECLARE_ODFIMPORT_TEST(testFdo61952, "hello.odt")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(0), uno::UNO_QUERY);
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Left Page")));
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Right Page")));
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Left Page")));
    xPara->setPropertyValue("PageDescName", uno::makeAny(OUString("Right Page")));
}

DECLARE_ODFIMPORT_TEST(testFdo60842, "fdo60842.odt")
{
    uno::Reference<text::XTextContent> const xTable(getParagraphOrTable(0));
    getCell(xTable, "A1", "");
    getCell(xTable, "B1", "18/02/2012");
    getCell(xTable, "C1", "USD"); // this is the cell with office:string-value
    getCell(xTable, "D1", "");
    getCell(xTable, "E1", "01/04/2012");
}

DECLARE_ODFIMPORT_TEST(testFdo56272, "fdo56272.odt")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Vertical position was incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(422), xShape->getPosition().Y); // Was -2371
}

DECLARE_ODFIMPORT_TEST(testFdo75872_ooo33, "fdo75872_ooo33.odt")
{
    // graphics default style: line color and fill color changed
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_BLACK),
           getProperty<sal_Int32>(xShape, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(RGB_COLORDATA(153, 204, 255)),
           getProperty<sal_Int32>(xShape, "FillColor"));
}

DECLARE_ODFIMPORT_TEST(testFdo75872_aoo40, "fdo75872_aoo40.odt")
{
    // graphics default style: line color and fill color changed
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(RGB_COLORDATA(128, 128, 128)),
           getProperty<sal_Int32>(xShape, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(RGB_COLORDATA(0xCF, 0xE7, 0xF5)),
           getProperty<sal_Int32>(xShape, "FillColor"));
}

DECLARE_ODFIMPORT_TEST(testFdo55814, "fdo55814.odt")
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

DECLARE_ODFIMPORT_TEST(testFdo68839, "fdo68839.odt")
{
    // check names
    lcl_CheckShape(getShape(1), "FrameXXX");
    lcl_CheckShape(getShape(2), "ObjectXXX");
    lcl_CheckShape(getShape(3), "FrameY");
    lcl_CheckShape(getShape(4), "graphicsXXX");
    try {
        uno::Reference<drawing::XShape> xShape = getShape(5);
        CPPUNIT_FAIL("IndexOutOfBoundsException expected");
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

DECLARE_ODFIMPORT_TEST(testFdo82165, "fdo82165.odt")
{
    // there was a duplicate shape in the left header
    lcl_CheckShape(getShape(1), "Picture 9");
    try {
        uno::Reference<drawing::XShape> xShape = getShape(2);
        CPPUNIT_FAIL("IndexOutOfBoundsException expected");
    } catch (lang::IndexOutOfBoundsException const&) { }
}

DECLARE_ODFIMPORT_TEST(testFdo37606, "fdo37606.odt")
{
    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    SwWrtShell* pWrtShell = pTxtDoc->GetDocShell()->GetWrtShell();
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);

    {
        pWrtShell->SelAll();
        SwTxtNode& rStart = dynamic_cast<SwTxtNode&>(pShellCrsr->Start()->nNode.GetNode());
        CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetTxt());

        SwTxtNode& rEnd = dynamic_cast<SwTxtNode&>(pShellCrsr->End()->nNode.GetNode());
        // This was "A1", i.e. Ctrl-A only selected the A1 cell of the table, not the whole document.
        CPPUNIT_ASSERT_EQUAL(OUString("Hello."), rEnd.GetTxt());
    }

    {
        pWrtShell->SttEndDoc(false); // Go to the end of the doc.
        pWrtShell->SelAll(); // And now that we're outside of the table, try Ctrl-A again.
        SwTxtNode& rStart = dynamic_cast<SwTxtNode&>(pShellCrsr->Start()->nNode.GetNode());
        // This was "Hello", i.e. Ctrl-A did not select the starting table.
        CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetTxt());

        SwTxtNode& rEnd = dynamic_cast<SwTxtNode&>(pShellCrsr->End()->nNode.GetNode());
        CPPUNIT_ASSERT_EQUAL(OUString("Hello."), rEnd.GetTxt());
    }

    {
        pWrtShell->Delete(); // Delete the selection
        // And make sure the table got deleted as well.
        SwNodes& rNodes = pWrtShell->GetDoc()->GetNodes();
        SwNodeIndex nNode(rNodes.GetEndOfExtras());
        SwCntntNode* pCntntNode = rNodes.GoNext(&nNode);
        // First content node was in a table -> table wasn't deleted.
        CPPUNIT_ASSERT(!pCntntNode->FindTableNode());
    }
}

DECLARE_ODFIMPORT_TEST(testFdo37606Copy, "fdo37606.odt")
{
    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    SwWrtShell* pWrtShell = pTxtDoc->GetDocShell()->GetWrtShell();
    // Ctrl-A
    pWrtShell->SelAll();

    // Ctrl-C
    SwTransferable* pTransferable = new SwTransferable(*pWrtShell);
    uno::Reference<datatransfer::XTransferable> xTransferable(pTransferable);
    pTransferable->Copy();

    pWrtShell->SttEndDoc(false); // Go to the end of the doc.

    // Ctrl-V
    TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromSystemClipboard(&pWrtShell->GetView().GetEditWin()));
    SwTransferable::Paste( *pWrtShell, aDataHelper );

    // Previously copy&paste failed to copy the table in case it was the document-starting one.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

DECLARE_ODFIMPORT_TEST(testFdo69862, "fdo69862.odt")
{
    // The test doc is special in that it starts with a table and it also has a footnote.
    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    SwWrtShell* pWrtShell = pTxtDoc->GetDocShell()->GetWrtShell();
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);

    pWrtShell->SelAll();
    SwTxtNode& rStart = dynamic_cast<SwTxtNode&>(pShellCrsr->Start()->nNode.GetNode());
    // This was "Footnote.", as Ctrl-A also selected footnotes, but it should not.
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetTxt());

    SwTxtNode& rEnd = dynamic_cast<SwTxtNode&>(pShellCrsr->End()->nNode.GetNode());
    CPPUNIT_ASSERT_EQUAL(OUString("H" "\x01" "ello."), rEnd.GetTxt());
}

DECLARE_ODFIMPORT_TEST(testFdo69979, "fdo69979.odt")
{
    // The test doc is special in that it starts with a table and it also has a header.
    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    SwWrtShell* pWrtShell = pTxtDoc->GetDocShell()->GetWrtShell();
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);

    pWrtShell->SelAll();
    SwTxtNode& rStart = dynamic_cast<SwTxtNode&>(pShellCrsr->Start()->nNode.GetNode());
    // This was "", as Ctrl-A also selected headers, but it should not.
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetTxt());

    SwTxtNode& rEnd = dynamic_cast<SwTxtNode&>(pShellCrsr->End()->nNode.GetNode());
    CPPUNIT_ASSERT_EQUAL(OUString("Hello."), rEnd.GetTxt());
}

DECLARE_ODFIMPORT_TEST(testSpellmenuRedline, "spellmenu-redline.odt")
{
    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    SwWrtShell* pWrtShell = pTxtDoc->GetDocShell()->GetWrtShell();
    OUString aParaText;
    uno::Reference<linguistic2::XSpellAlternatives> xAlt;
    SwSpellPopup aPopup(pWrtShell, xAlt, aParaText);
    // Make sure that if we show the spellcheck popup menu (for the current
    // document, which contains redlines), then the last two entries will be
    // always 'go to next/previous change'.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(FN_REDLINE_NEXT_CHANGE), aPopup.GetItemId(aPopup.GetItemCount() - 2));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(FN_REDLINE_PREV_CHANGE), aPopup.GetItemId(aPopup.GetItemCount() - 1));
}

DECLARE_ODFIMPORT_TEST(testAnnotationFormatting, "annotation-formatting.odt")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 1), "TextField");
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XText> >(xTextField, "TextRange");
    // Make sure we test the right annotation.
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "Looses: bold");
    // Formatting was lost: the second text portion was NORMAL, not BOLD.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xParagraph, 2), "CharWeight"));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
