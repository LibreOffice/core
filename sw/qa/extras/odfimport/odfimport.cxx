/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <config_features.h>

#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/FootnoteLineStyle.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <editeng/boxitem.hxx>
#include <vcl/scheduler.hxx>

#include <IDocumentSettingAccess.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <olmenu.hxx>
#include <hintids.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <frmatr.hxx>

#if defined(_WIN32)
#include <officecfg/Office/Common.hxx>
#include <unotools/securityoptions.hxx>
#include <systools/win32/comtools.hxx>
#include <urlmon.h>
#endif

namespace
{
typedef std::map<OUString, css::uno::Sequence< css::table::BorderLine> > AllBordersMap;
typedef std::pair<OUString, css::uno::Sequence< css::table::BorderLine> > StringSequencePair;

class Test : public SwModelTestBase
{
    public:
        Test() : SwModelTestBase("/sw/qa/extras/odfimport/data/", "writer8") {}
};

CPPUNIT_TEST_FIXTURE(Test, testEmptySvgFamilyName)
{
    createSwDoc("empty-svg-family-name.odt");
    // .odt import did crash on the empty font list (which I think is valid according SVG spec)
}

CPPUNIT_TEST_FIXTURE(Test, testHideAllSections)
{
    createSwDoc("fdo53210.odt");
    // This document has a section that is conditionally hidden, but has no empty paragraph after it.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xMasters = xTextFieldsSupplier->getTextFieldMasters();
    // Set _CS_Allgemein to 0
    uno::Reference<beans::XPropertySet> xMaster(xMasters->getByName("com.sun.star.text.fieldmaster.User._CS_Allgemein"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Content", uno::Any(OUString("0")));
    // This used to crash
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY_THROW)->refresh();
}

CPPUNIT_TEST_FIXTURE(Test, testOdtBorders)
{
    createSwDoc("borders_ooo33.odt");
    AllBordersMap map;

    map.insert(StringSequencePair(OUString("B2"), { table::BorderLine(0, 2, 2, 35),     // left
                                                    table::BorderLine(0, 2, 2, 35),     // right
                                                    table::BorderLine(0, 2, 2, 35),     // top
                                                    table::BorderLine(0, 2, 2, 35) })); // bottom

    map.insert(StringSequencePair(OUString("D2"), { table::BorderLine(0, 0, 2, 0),
                                                    table::BorderLine(0, 0, 2, 0),
                                                    table::BorderLine(0, 0, 2, 0),
                                                    table::BorderLine(0, 0, 2, 0) }));

    map.insert(StringSequencePair(OUString("B4"), { table::BorderLine(0, 2, 2, 88),
                                                    table::BorderLine(0, 2, 2, 88),
                                                    table::BorderLine(0, 2, 2, 88),
                                                    table::BorderLine(0, 2, 2, 88) }));

    map.insert(StringSequencePair(OUString("D4"), { table::BorderLine(0, 0, 18, 0),
                                                    table::BorderLine(0, 0, 18, 0),
                                                    table::BorderLine(0, 0, 18, 0),
                                                    table::BorderLine(0, 0, 18, 0) }));

    map.insert(StringSequencePair(OUString("B6"), { table::BorderLine(0, 35, 35, 35),
                                                    table::BorderLine(0, 35, 35, 35),
                                                    table::BorderLine(0, 35, 35, 35),
                                                    table::BorderLine(0, 35, 35, 35) }));

    map.insert(StringSequencePair(OUString("D6"), { table::BorderLine(0, 0, 35, 0),
                                                    table::BorderLine(0, 0, 35, 0),
                                                    table::BorderLine(0, 0, 35, 0),
                                                    table::BorderLine(0, 0, 35, 0) }));

    map.insert(StringSequencePair(OUString("B8"), { table::BorderLine(0, 88, 88, 88),
                                                    table::BorderLine(0, 88, 88, 88),
                                                    table::BorderLine(0, 88, 88, 88),
                                                    table::BorderLine(0, 88, 88, 88) }));

    map.insert(StringSequencePair(OUString("D8"), { table::BorderLine(0, 0, 88, 0),
                                                    table::BorderLine(0, 0, 88, 0),
                                                    table::BorderLine(0, 0, 88, 0),
                                                    table::BorderLine(0, 0, 88, 0) }));

    map.insert(StringSequencePair(OUString("B10"), { table::BorderLine(0, 2, 35, 88),
                                                     table::BorderLine(0, 2, 35, 88),
                                                     table::BorderLine(0, 2, 35, 88),
                                                     table::BorderLine(0, 2, 35, 88) }));

    map.insert(StringSequencePair(OUString("D10"), { table::BorderLine(0, 0, 141, 0),
                                                     table::BorderLine(0, 0, 141, 0),
                                                     table::BorderLine(0, 0, 141, 0),
                                                     table::BorderLine(0, 0, 141, 0) }));

    map.insert(StringSequencePair(OUString("B12"), { table::BorderLine(0, 2, 88, 88),
                                                     table::BorderLine(0, 2, 88, 88),
                                                     table::BorderLine(0, 2, 88, 88),
                                                     table::BorderLine(0, 2, 88, 88) }));

    map.insert(StringSequencePair(OUString("D12"), { table::BorderLine(0, 0, 176, 0),
                                                     table::BorderLine(0, 0, 176, 0),
                                                     table::BorderLine(0, 0, 176, 0),
                                                     table::BorderLine(0, 0, 176, 0) }));

    map.insert(StringSequencePair(OUString("B14"), { table::BorderLine(0, 2, 141, 88),
                                                     table::BorderLine(0, 2, 141, 88),
                                                     table::BorderLine(0, 2, 141, 88),
                                                     table::BorderLine(0, 2, 141, 88) }));

    map.insert(StringSequencePair(OUString("B16"), { table::BorderLine(0, 35, 88, 88),
                                                     table::BorderLine(0, 35, 88, 88),
                                                     table::BorderLine(0, 35, 88, 88),
                                                     table::BorderLine(0, 35, 88, 88) }));

    map.insert(StringSequencePair(OUString("B18"), { table::BorderLine(0, 88, 35, 35),
                                                     table::BorderLine(0, 88, 35, 35),
                                                     table::BorderLine(0, 88, 35, 35),
                                                     table::BorderLine(0, 88, 35, 35) }));

    map.insert(StringSequencePair(OUString("B20"), { table::BorderLine(0, 88, 141, 88),
                                                     table::BorderLine(0, 88, 141, 88),
                                                     table::BorderLine(0, 88, 141, 88),
                                                     table::BorderLine(0, 88, 141, 88) }));

    map.insert(StringSequencePair(OUString("B22"), { table::BorderLine(0, 141, 88, 88),
                                                     table::BorderLine(0, 141, 88, 88),
                                                     table::BorderLine(0, 141, 88, 88),
                                                     table::BorderLine(0, 141, 88, 88) }));

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

                AllBordersMap::iterator it = map.begin();

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

CPPUNIT_TEST_FIXTURE(Test, testTdf41542_borderlessPadding)
{
    createSwDoc("tdf41542_borderlessPadding.odt");
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, testPageStyleLayoutDefault)
{
    createSwDoc("hello.odt");
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Page Style"), uno::UNO_QUERY);
    // This was style::PageStyleLayout_MIRRORED.
    CPPUNIT_ASSERT_EQUAL(style::PageStyleLayout_ALL, getProperty<style::PageStyleLayout>(xPropertySet, "PageStyleLayout"));
}

CPPUNIT_TEST_FIXTURE(Test, testTimeFormFormats)
{
    createSwDoc("timeFormFormats.odt");
    //FIXME: make it an ODFEXPORT_TEST. Validator fails with
    //attribute "form:current-value" has a bad value: "PT12H12M" does not satisfy the "time" type
    //See tdf#131127

    uno::Reference<frame::XModel> const xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<drawing::XDrawPageSupplier> const xDPS(xModel, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> const xDP = xDPS->getDrawPage();
    CPPUNIT_ASSERT(xDP.is());
    uno::Reference<form::XFormsSupplier> const xFS(xDP, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFS.is());
    uno::Reference<container::XIndexContainer> const xForms(xFS->getForms(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xForms.is());
    uno::Reference<form::XForm> xForm(xForms->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xForm.is());
    uno::Reference<container::XNameContainer> xFormNC(xForm, uno::UNO_QUERY);

    uno::Any aAny;
    uno::Reference<awt::XControlModel> xControlModel;
    uno::Reference<view::XControlAccess> xController;
    uno::Reference<awt::XControl> xControl;
    uno::Reference<awt::XWindowPeer> xWindowPeer;
    uno::Reference<awt::XTextComponent> xTextComponent;
    OUString aName = "Time Field ";

    static const char* const aExpectedResults[] = { "12:12", "12:12:00", "12:12PM", "06:00:00AM"};

    for (size_t i = 1; i <= 4; ++i)
    {
        aAny = xFormNC->getByName(aName + OUString::number(i));
        xControlModel.set(aAny, uno::UNO_QUERY);
        xController.set(xModel->getCurrentController(), uno::UNO_QUERY_THROW);
        xControl = xController->getControl(xControlModel);
        xWindowPeer = xControl->getPeer();
        xTextComponent.set(xWindowPeer, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8(aExpectedResults[i - 1]), xTextComponent->getText());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDateFormFormats)
{
    createSwDoc("dateFormFormats.odt");
    //FIXME: make it an ODFEXPORT_TEST. Validator fails with
    //unexpected attribute "form:input-required"
    //See tdf#131148

    uno::Reference<frame::XModel> const xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<drawing::XDrawPageSupplier> const xDPS(xModel, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> const xDP = xDPS->getDrawPage();
    CPPUNIT_ASSERT(xDP.is());
    uno::Reference<form::XFormsSupplier> const xFS(xDP, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFS.is());
    uno::Reference<container::XIndexContainer> const xForms(xFS->getForms(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xForms.is());
    uno::Reference<form::XForm> xForm(xForms->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xForm.is());
    uno::Reference<container::XNameContainer> xFormNC(xForm, uno::UNO_QUERY);

    uno::Any aAny;
    uno::Reference<awt::XControlModel> xControlModel;
    uno::Reference<view::XControlAccess> xController;
    uno::Reference<awt::XControl> xControl;
    uno::Reference<awt::XWindowPeer> xWindowPeer;
    uno::Reference<awt::XTextComponent> xTextComponent;
    OUString aName = "Date Field ";

    static const char* const aExpectedResults[] = { "03/04/20", "03/04/20", "03/04/2020",
        "Wednesday, March 4, 2020", "04/03/20", "03/04/20", "20/03/04", "04/03/2020", "03/04/2020",
        "2020/03/04", "20-03-04", "2020-03-04"};

    for (size_t i = 1; i <= 12; ++i)
    {
        aAny = xFormNC->getByName(aName + OUString::number(i));
        xControlModel.set(aAny, uno::UNO_QUERY);
        xController.set(xModel->getCurrentController(), uno::UNO_QUERY_THROW);
        xControl = xController->getControl(xControlModel);
        xWindowPeer = xControl->getPeer();
        xTextComponent.set(xWindowPeer, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8(aExpectedResults[i - 1]), xTextComponent->getText());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149978)
{
    createSwDoc("tdf149978.fodt");
    // on Linux the bug only reproduces if a document has been loaded previously
    createSwDoc("tdf149978.fodt");
    // this was nondeterministic so try 10 times
    for (int i = 1; i <= 10; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(getRun(getParagraph(i), 2, "bar"), "CharBackColor"));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf64038)
{
    createSwDoc("space.odt");
    // no space
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(4), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(4), 2)->getString());
    // one space
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(6), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(6), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(7), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(7), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(7), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(8), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" b"), getRun(getParagraph(8), 2)->getString());
    // two spaces
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(10), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(10), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(10), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(11), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(11), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(12), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(12), 2)->getString());
    // three spaces
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(14), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(14), 2)->getString());
    // no space
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(17), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(17), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(17), 3)->getString());
    // one space
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(19), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(19), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(19), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(20), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(20), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(20), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(21), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(21), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(21), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(22), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(22), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" b"), getRun(getParagraph(22), 3)->getString());
    // two spaces
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(24), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(24), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(24), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(25), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(25), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(25), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(26), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(26), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" b"), getRun(getParagraph(26), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(27), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("   "), getRun(getParagraph(27), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(27), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(28), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(28), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" b"), getRun(getParagraph(28), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(29), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(29), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(29), 3)->getString());
    // three spaces
    CPPUNIT_ASSERT_EQUAL(OUString("a"), getRun(getParagraph(31), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("   "), getRun(getParagraph(31), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(31), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(32), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(32), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(32), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(33), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getRun(getParagraph(33), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" b"), getRun(getParagraph(33), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(34), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(34), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(34), 3)->getString());
    // four spaces
    CPPUNIT_ASSERT_EQUAL(OUString("a "), getRun(getParagraph(36), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("  "), getRun(getParagraph(36), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), getRun(getParagraph(36), 3)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf74524)
{
    createSwDoc("tdf74524.odt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Any aField1 = xFields->nextElement();
    uno::Reference<lang::XServiceInfo> xServiceInfo1(aField1, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo1->supportsService("com.sun.star.text.textfield.PageNumber"));
    uno::Reference<beans::XPropertySet> xPropertySet(aField1, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(style::NumberingType::PAGE_DESCRIPTOR)), xPropertySet->getPropertyValue("NumberingType"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(0)), xPropertySet->getPropertyValue("Offset"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(text::PageNumberType_CURRENT), xPropertySet->getPropertyValue("SubType"));
    uno::Reference<text::XTextContent> xField1(aField1, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xField1->getAnchor()->getString());
    uno::Any aField2 = xFields->nextElement();
    uno::Reference<lang::XServiceInfo> xServiceInfo2(aField2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo2->supportsService("com.sun.star.text.textfield.Annotation"));
    uno::Reference<beans::XPropertySet> xPropertySet2(aField2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(OUString("Comment 1")), xPropertySet2->getPropertyValue("Content"));
    uno::Reference<text::XTextContent> xField2(aField2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello 1World"), xField2->getAnchor()->getString());
    CPPUNIT_ASSERT(!xFields->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testPageStyleLayoutRight)
{
    createSwDoc("hello.odt");
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Page Style"), uno::UNO_QUERY);
    // This caused a crash.
    xPropertySet->setPropertyValue("PageStyleLayout", uno::Any(style::PageStyleLayout_RIGHT));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo61952)
{
    createSwDoc("hello.odt");
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    xPara->setPropertyValue("PageDescName", uno::Any(OUString("Left Page")));
    xPara->setPropertyValue("PageDescName", uno::Any(OUString("Right Page")));
    xPara->setPropertyValue("PageDescName", uno::Any(OUString("Left Page")));
    xPara->setPropertyValue("PageDescName", uno::Any(OUString("Right Page")));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo60842)
{
    createSwDoc("fdo60842.odt");
    uno::Reference<text::XTextContent> const xTable(getParagraphOrTable(1));
    getCell(xTable, "A1", "");
    getCell(xTable, "B1", "18/02/2012");
    getCell(xTable, "C1", "USD"); // this is the cell with office:string-value
    getCell(xTable, "D1", "");
    getCell(xTable, "E1", "01/04/2012");
}

CPPUNIT_TEST_FIXTURE(Test, testFdo79269)
{
    createSwDoc("fdo79269.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // The problem was that the first-footer was shared.
    uno::Reference<beans::XPropertySet> xPropSet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPropSet, "FirstIsShared"));

    uno::Reference<text::XTextRange> xFooter1 = getProperty< uno::Reference<text::XTextRange> >(xPropSet, "FooterTextFirst");
    CPPUNIT_ASSERT_EQUAL(OUString("first"), xFooter1->getString());
    uno::Reference<text::XTextRange> xFooter = getProperty< uno::Reference<text::XTextRange> >(xPropSet, "FooterText");
    CPPUNIT_ASSERT_EQUAL(OUString("second"), xFooter->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo79269_header)
{
    createSwDoc("fdo79269_header.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    uno::Reference<beans::XPropertySet> xPropSet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPropSet, "FirstIsShared"));

    uno::Reference<text::XTextRange> xFooter1 = getProperty< uno::Reference<text::XTextRange> >(xPropSet, "HeaderTextFirst");
    CPPUNIT_ASSERT_EQUAL(OUString("first"), xFooter1->getString());
    uno::Reference<text::XTextRange> xFooter = getProperty< uno::Reference<text::XTextRange> >(xPropSet, "HeaderText");
    CPPUNIT_ASSERT_EQUAL(OUString("second"), xFooter->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testPageBackground)
{
    createSwDoc("PageBackground.odt");
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Page Style"), uno::UNO_QUERY);
    // The background image was lost
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xPropertySet, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sky"), getProperty<OUString>(xPropertySet, "FillBitmapName"));
    CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xPropertySet, "FillBitmapMode"));

    uno::Reference<beans::XPropertySet> xPropertySetOld(getStyles("PageStyles")->getByName("OldStandard"), uno::UNO_QUERY);
    // The background image was overridden by color
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xPropertySetOld, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sky"), getProperty<OUString>(xPropertySetOld, "FillBitmapName"));
    CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xPropertySetOld, "FillBitmapMode"));
}

CPPUNIT_TEST_FIXTURE(Test, testBibliographyEntryField)
{
    createSwDoc("BibliographyEntryField.odt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Bibliography entry"), xEnumerationAccess->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("[ABC]"), xEnumerationAccess->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo56272)
{
    createSwDoc("fdo56272.odt");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Vertical position was incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(422), xShape->getPosition().Y); // Was -2371
}

CPPUNIT_TEST_FIXTURE(Test, testIncorrectSum)
{
    createSwDoc("incorrectsum.odt");
    Scheduler::ProcessEventsToIdle();
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTextTable->getCellByName("C3"), uno::UNO_QUERY);
    // Use indexOf instead of exact match since the result contains an Euro sign which OUString doesn't like
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xCell->getString().indexOf("1,278"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128737)
{
    createSwDoc("tdf128737.odt");
    // Without the fix in place, this test would have crashed
    CPPUNIT_ASSERT_EQUAL(4, getPages());
    CPPUNIT_ASSERT_EQUAL(8, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testCalcFootnoteContent)
{
    createSwDoc("ooo32780-1.odt");
    //this was a CalcFootnoteContent crash
}

CPPUNIT_TEST_FIXTURE(Test, testMoveSubTree)
{
    createSwDoc("ooo77837-1.odt");
    //this was a MoveSubTree crash
}

CPPUNIT_TEST_FIXTURE(Test, testFdo75872_ooo33)
{
    createSwDoc("fdo75872_ooo33.odt");
    // graphics default style: line color and fill color changed
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK,
           getProperty<Color>(xShape, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(Color(0x99ccff),
           getProperty<Color>(xShape, "FillColor"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo75872_aoo40)
{
    createSwDoc("fdo75872_aoo40.odt");
    // graphics default style: line color and fill color changed
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(COL_GRAY,
           getProperty<Color>(xShape, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(Color(0xcfe7f5),
           getProperty<Color>(xShape, "FillColor"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo55814)
{
    createSwDoc("fdo55814.odt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);
    xField->setPropertyValue("Content", uno::Any(OUString("Yes")));
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY_THROW)->refresh();
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

CPPUNIT_TEST_FIXTURE(Test, testFdo68839)
{
    createSwDoc("fdo68839.odt");
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    // check names
    lcl_CheckShape(getShape(1), "FrameXXX");
    lcl_CheckShape(getShape(2), "ObjectXXX");
    lcl_CheckShape(getShape(3), "FrameY");
    lcl_CheckShape(getShape(4), "graphicsXXX");
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

CPPUNIT_TEST_FIXTURE(Test, testFdo82165)
{
    createSwDoc("fdo82165.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    // there was a duplicate shape in the left header
    lcl_CheckShape(getShape(1), "Picture 9");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf89802)
{
    createSwDoc("tdf89802.fodt");
    // the text frame was dropped during import
    uno::Reference<text::XTextFramesSupplier> const xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    uno::Reference<beans::XPropertySet> const xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x3f004586), getProperty<Color>(xFrame, "BackColor"));

}

CPPUNIT_TEST_FIXTURE(Test, testFdo37606)
{
    createSwDoc("fdo37606.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);

    {
        pWrtShell->SelAll(); // Selects A1.
        SwTextNode& rCellEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->GetNode());
        // fdo#72486 This was "Hello.", i.e. a single select-all selected the whole document, not just the cell only.
        CPPUNIT_ASSERT_EQUAL(OUString("A1"), rCellEnd.GetText());

        pWrtShell->SelAll(); // Selects the whole table.
        pWrtShell->SelAll(); // Selects the whole document.
        pShellCursor = pWrtShell->getShellCursor(false);

        SwTextNode& rStart = dynamic_cast<SwTextNode&>(pShellCursor->Start()->GetNode());
        CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetText());

        SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->GetNode());
        // This was "A1", i.e. Ctrl-A only selected the A1 cell of the table, not the whole document.
        CPPUNIT_ASSERT_EQUAL(OUString("Hello."), rEnd.GetText());
    }

    {
        pWrtShell->SttEndDoc(false); // Go to the end of the doc.
        pWrtShell->SelAll(); // And now that we're outside of the table, try Ctrl-A again.
        SwTextNode& rStart = dynamic_cast<SwTextNode&>(pShellCursor->Start()->GetNode());
        // This was "Hello", i.e. Ctrl-A did not select the starting table.
        CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetText());

        SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->GetNode());
        CPPUNIT_ASSERT_EQUAL(OUString("Hello."), rEnd.GetText());
    }

    {
        pWrtShell->Delete(); // Delete the selection
        // And make sure the table got deleted as well.
        SwNodes& rNodes = pWrtShell->GetDoc()->GetNodes();
        SwNodeIndex nNode(rNodes.GetEndOfExtras());
        SwContentNode* pContentNode = SwNodes::GoNext(&nNode);
        // First content node was in a table -> table wasn't deleted.
        CPPUNIT_ASSERT(!pContentNode->FindTableNode());
    }
}

#if HAVE_FEATURE_UI
CPPUNIT_TEST_FIXTURE(Test, testFdo37606Copy)
{
    createSwDoc("fdo37606.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    // Ctrl-A
    pWrtShell->SelAll(); // Selects A1.
    pWrtShell->SelAll(); // Selects the whole table.
    pWrtShell->SelAll(); // Selects the whole document.

    // Ctrl-C
    rtl::Reference<SwTransferable> xTransferable(new SwTransferable(*pWrtShell));
    xTransferable->Copy();

    pWrtShell->SttEndDoc(false); // Go to the end of the doc.

    // Ctrl-V
    TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromSystemClipboard(&pWrtShell->GetView().GetEditWin()));
    SwTransferable::Paste( *pWrtShell, aDataHelper );

    // Previously copy&paste failed to copy the table in case it was the document-starting one.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testFdo69862)
{
    createSwDoc("fdo69862.odt");
    // The test doc is special in that it starts with a table and it also has a footnote.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll(); // Selects A1.
    pWrtShell->SelAll(); // Selects the whole table.
    pWrtShell->SelAll(); // Selects the whole document.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    SwTextNode& rStart = dynamic_cast<SwTextNode&>(pShellCursor->Start()->GetNode());
    // This was "Footnote.", as Ctrl-A also selected footnotes, but it should not.
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetText());

    SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->GetNode());
    CPPUNIT_ASSERT_EQUAL(OUString("H" "\x01" "ello."), rEnd.GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo69979)
{
    createSwDoc("fdo69979.odt");
    // The test doc is special in that it starts with a table and it also has a header.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);

    pWrtShell->SelAll(); // Selects A1.
    pWrtShell->SelAll(); // Selects the whole table.
    pWrtShell->SelAll(); // Selects the whole document.
    SwTextNode& rStart = dynamic_cast<SwTextNode&>(pShellCursor->Start()->GetNode());
    // This was "", as Ctrl-A also selected headers, but it should not.
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), rStart.GetText());

    SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->GetNode());
    CPPUNIT_ASSERT_EQUAL(OUString("Hello."), rEnd.GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testSpellmenuRedline)
{
    createSwDoc("spellmenu-redline.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    uno::Reference<linguistic2::XSpellAlternatives> xAlt;
    SwSpellPopup aPopup(pWrtShell, xAlt, OUString());
    Menu& rMenu = aPopup.GetMenu();
    // Make sure that if we show the spellcheck popup menu (for the current
    // document, which contains redlines), then the last two entries will be
    // always 'go to next/previous change'.
    CPPUNIT_ASSERT_EQUAL(OUString("next"), rMenu.GetItemIdent(rMenu.GetItemId(rMenu.GetItemCount() - 2)));
    CPPUNIT_ASSERT_EQUAL(OUString("prev"), rMenu.GetItemIdent(rMenu.GetItemId(rMenu.GetItemCount() - 1)));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107776)
{
    createSwDoc("tdf107776.fodt");
    // Shape with a Graphics parent style name was imported as textbox.
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "TextBox"));
}

CPPUNIT_TEST_FIXTURE(Test, testAnnotationFormatting)
{
    createSwDoc("annotation-formatting.odt");
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 1), "TextField");
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XText> >(xTextField, "TextRange");
    // Make sure we test the right annotation.
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "Looses: bold");
    // Formatting was lost: the second text portion was NORMAL, not BOLD.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xParagraph, 2), "CharWeight"));
}

CPPUNIT_TEST_FIXTURE(Test, fdo81223)
{
    createSwDoc("fdo81223.odt");
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xFrame, "BackColor"));
}

CPPUNIT_TEST_FIXTURE(Test, fdo90130_1)
{
    createSwDoc("fdo90130-1.odt");
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xff3333), getProperty<Color>(xFrame, "BackColor"));
}

CPPUNIT_TEST_FIXTURE(Test, fdo90130_2)
{
    createSwDoc("fdo90130-2.odt");
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nValue(0);
    xFrame->getPropertyValue("BackColorTransparency") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), nValue);
}

CPPUNIT_TEST_FIXTURE(Test, testBnc800714)
{
    createSwDoc("bnc800714.fodt");
    // Document's second paragraph wants to be together with the third one, but:
    // - it's in a section with multiple columns
    // - contains a single as-char anchored frame
    // This was a layout loop.
    CPPUNIT_ASSERT(getProperty< uno::Reference<text::XTextSection> >(getParagraph(2), "TextSection").is());
    CPPUNIT_ASSERT(getProperty<bool>(getParagraph(2), "ParaKeepTogether"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92586)
{
    createSwDoc("tdf92586.odt");
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    // This was BitmapMode_NO_REPEAT.
    CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_STRETCH, getProperty<drawing::BitmapMode>(aPageStyle, "FillBitmapMode"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103025)
{
    createSwDoc("tdf103025.odt");
    CPPUNIT_ASSERT_EQUAL(OUString("2014-01"), parseDump("/root/page[1]/header/tab[2]/row[2]/cell[3]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-01"), parseDump("/root/page[2]/header/tab[2]/row[2]/cell[3]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-02"), parseDump("/root/page[3]/header/tab[2]/row[2]/cell[3]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-03"), parseDump("/root/page[4]/header/tab[2]/row[2]/cell[3]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-03"), parseDump("/root/page[5]/header/tab[2]/row[2]/cell[3]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf76322_columnBreakInHeader)
{
    createSwDoc("tdf76322_columnBreakInHeader.docx");
// column breaks were ignored. First line should start in column 2
    CPPUNIT_ASSERT_EQUAL( OUString("Test1"), parseDump("/root/page[1]/header/section/column[2]/body/txt/text()"_ostr) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf76349_1columnBreak)
{
    createSwDoc("tdf76349_1columnBreak.odt");
    //single-column breaks should only be treated as page breaks for MS formats - should be only one page here.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96113)
{
    createSwDoc("tdf96113.odt");
    // Background of the formula frame was white (0xffffff), not green.
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(getShape(1), "BackColor"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47267)
{
    createSwDoc("fdo47267-3.odt");
    // This was a Style Families getByName() crash
}

CPPUNIT_TEST_FIXTURE(Test, testTdf75221)
{
    createSwDoc("tdf75221.odt");
    // When "Don't add space between paragraphs of the same style" setting set,
    // spacing between same-style paragraphs must be equal to their line spacing.
    // It used to be 0.
    OUString top = parseDump("/root/page/body/txt[2]/infos/prtBounds"_ostr, "top"_ostr);
    CPPUNIT_ASSERT(top.toInt32() > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101729)
{
    createSwDoc("tdf101729.odt");
    sal_Int32 l = parseDump("/root/page/body/tab/row/cell[1]/infos/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 w = parseDump("/root/page/body/tab/row/cell[1]/infos/bounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 x = parseDump("/root/page/body/tab/row/cell[1]/txt/infos/bounds"_ostr, "left"_ostr).toInt32();
    // Make sure the text does not go outside and verify it is centered roughly
    CPPUNIT_ASSERT( l + w / 4 < x  );
    CPPUNIT_ASSERT( x < l + 3 * w / 4);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107392)
{
    createSwDoc("tdf107392.odt");
    // Shapes from bottom to top were Frame, SVG, Bitmap, i.e. in the order as
    // they appeared in the document, not according to their requested z-index,
    // as sorting failed.
    // So instead of 0, 1, 2 these were 2, 0, 1.

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getShapeByName(u"Bitmap"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(getShapeByName(u"Frame"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(getShapeByName(u"SVG"), "ZOrder"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100033_1)
{
    createSwDoc("tdf100033_1.odt");
    // Test document have three duplicated frames with the same name and position/size -> import one frame
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testWordAsCharShape)
{
    createSwDoc("Word2010AsCharShape.odt");
    // As-char shape had VertOrient "from-top"/NONE default from GetVOrient()
    uno::Reference<drawing::XShape> const xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::TOP, getProperty<sal_Int16>(xShape, "VertOrient"));
    // also, the paragraph default fo:bottom-margin was wrongly applied to
    // the shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "BottomMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100033_2)
{
    createSwDoc("tdf100033_2.odt");
    // Test document have three different frames anchored to different paragraphs -> import all frames
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testI61225)
{
    createSwDoc("i61225.sxw");
    // Part of ooo61225-1.sxw from crashtesting.

    // This never returned.
    calcLayout();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109080_loext_ns)
{
    createSwDoc("tdf109080_loext_ns.odt");
    // Test we can import <loext:header-first> and <loext:footer-first>

    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page header"),
        parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page header"),
        parseDump("/root/page[2]/header/txt/text()"_ostr));

    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page footer"),
        parseDump("/root/page[1]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page footer"),
        parseDump("/root/page[2]/footer/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109080_style_ns)
{
    createSwDoc("tdf109080_style_ns.odt");
    // Test we can import <style:header-first> and <style:footer-first>
    // (produced by LibreOffice 4.0 - 5.x)

    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page header"),
        parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page header"),
        parseDump("/root/page[2]/header/txt/text()"_ostr));

    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page footer"),
        parseDump("/root/page[1]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page footer"),
        parseDump("/root/page[2]/footer/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109228)
{
    createSwDoc("tdf109228.odt");
    //  Embedded object with no frame name was imported incorrectly, it was anchored 'to character' instead of 'as character'
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94882)
{
    createSwDoc("tdf94882.odt");
    // Get the header of the page containing our content
    // (done this way to skip past any blank page inserted before it
    //  due to the page number being even)
    OUString headertext = parseDump(
        "/root/page[starts-with(body/txt/text(),'The paragraph style on this')]"
        "/header/txt/text()"_ostr
    );
    // This header should be the first page header
    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page header"), headertext);
}

CPPUNIT_TEST_FIXTURE(Test, testBlankBeforeFirstPage)
{
    createSwDoc("tdf94882.odt");
    // This document starts on page 50, which is even, but it should not have a
    // blank page inserted before it to make it a left page

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 1 pages output",
        OUString("1"), parseDump("count(/root/page)"_ostr)
    );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115079)
{
    createSwDoc("tdf115079.odt");
    // This document caused segfault when layouting
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108482)
{
    createSwDoc("tdf108482.odt");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The table on second page must have two rows",
        OUString("2"), parseDump("count(/root/page[2]/body/tab/row)"_ostr)
    );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The second page table's first row must be the repeated headline",
        OUString("Header"), parseDump("/root/page[2]/body/tab/row[1]/cell/txt"_ostr)
    );
    // The first (repeated headline) row with vertical text orientation must have non-zero height
    // (in my tests, it was 1135)
    CPPUNIT_ASSERT_GREATER(
        sal_Int32(1000), parseDump("/root/page[2]/body/tab/row[1]/infos/bounds"_ostr, "height"_ostr).toInt32()
    );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116195)
{
    createSwDoc("tdf116195.odt");
    // The image was set to zero height due to a regression
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(12960), parseDump("/root/page/anchored/fly/notxt/infos/bounds"_ostr, "height"_ostr).toInt32()
    );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf120677)
{
    createSwDoc("tdf120677.fodt");
    // The document used to hang the layout, consuming memory until OOM
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152015)
{
    // it hangs at import time
    createSwDoc("tdf152015.odt");

    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123829)
{
    createSwDoc("tdf123829.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Compatibility: collapse cell paras should not be set", false,
        pDoc->getIDocumentSettingAccess().get(DocumentSettingId::COLLAPSE_EMPTY_CELL_PARA));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf113289)
{
    createSwDoc("tdf113289.odt");
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int8>(style::FootnoteLineStyle::SOLID),
                         getProperty<sal_Int8>(aPageStyle, "FootnoteLineStyle"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123968)
{
    createSwDoc("tdf123968.odt");
    // The test doc is special in that it starts with a table and it also has a header.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);

    pWrtShell->SelAll();
    SwTextNode& rStart = dynamic_cast<SwTextNode&>(pShellCursor->Start()->GetNode());

    // The field is now editable like any text, thus the field content "New value" shows up for the cursor.
    // This field's variable is declared as string and used as string - typical.
    CPPUNIT_ASSERT_EQUAL(OUString("inputfield: " + OUStringChar(CH_TXT_ATR_INPUTFIELDSTART)
                                  + "New value" + OUStringChar(CH_TXT_ATR_INPUTFIELDEND)),
                         rStart.GetText());

    // This field's variable is declared as float and used as string - not
    // typical; this can easily happen if the input field is in a header/footer,
    // because only content.xml contains the variable-decls, styles.xml is
    // imported before content.xml, and apparently the default variable type is
    // numeric.
    SwTextNode& rEnd = dynamic_cast<SwTextNode&>(pShellCursor->End()->GetNode());
    CPPUNIT_ASSERT_EQUAL(OUString("inputfield: " + OUStringChar(CH_TXT_ATR_INPUTFIELDSTART)
                                  + "String input for num variable" + OUStringChar(CH_TXT_ATR_INPUTFIELDEND)),
                         rEnd.GetText());

}

CPPUNIT_TEST_FIXTURE(Test, testTdf133459)
{
    createSwDoc("tdf133459.odt");
    // Test that the number format was correctly imported, and used by both fields.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xFields(xTextFieldsSupplier->getTextFields()->createEnumeration());

    // First Field
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    const OUString sPresentation(xField->getPresentation(false));
    const sal_Int32 nFormat(getProperty<sal_Int32>(xField, "NumberFormat"));
    CPPUNIT_ASSERT_EQUAL(sal_True, getProperty<sal_Bool>(xField, "IsFixedLanguage"));

    // Second field
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sPresentation, xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(nFormat, getProperty<sal_Int32>(xField, "NumberFormat"));
    CPPUNIT_ASSERT_EQUAL(sal_True, getProperty<sal_Bool>(xField, "IsFixedLanguage"));

    // Test the number format itself
    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFormat(xNumberFormatsSupplier->getNumberFormats()->getByKey(nFormat));
    lang::Locale aLocale(getProperty<lang::Locale>(xFormat, "Locale"));
    CPPUNIT_ASSERT_EQUAL(OUString("ru"), aLocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("RU"), aLocale.Country);
    CPPUNIT_ASSERT_EQUAL(OUString("QQ YYYY"), getProperty<OUString>(xFormat, "FormatString"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134971)
{
    createSwDoc("tdf134971a.odt");
    // now insert 2nd file somewhere - insertDocumentFromURL should
    // _not_ touch pool defaults
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Name", uno::Any(createFileURL(u"tdf134971b.odt"))},
        {"Filter", uno::Any(OUString("writer8"))},
    });
    dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);

    // tdf134971b re-defines default font as "Liberation Sans" - make sure this stays
    // Arial in final doc:
    OUString sString;
    uno::Reference<container::XNameAccess> xParaStyles(getStyles("ParagraphStyles"));
    uno::Reference<beans::XPropertySet> xStyle1(xParaStyles->getByName(
            "Standard"), uno::UNO_QUERY);
    xStyle1->getPropertyValue("CharFontName") >>= sString;
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), sString);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf151375)
{
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<document::XDocumentInsertable> xCursor(xText->createTextCursorByRange(xText->getStart()), uno::UNO_QUERY);

    // Without the fix in place, this test would have crashed here
    xCursor->insertDocumentFromURL(createFileURL(u"tdf151375.ott"), {});

    uno::Reference<text::XTextFramesSupplier> const xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138879)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Name", uno::Any(createFileURL(u"tdf138879.odt"))},
        {"Filter", uno::Any(OUString("writer8"))},
    });
    dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);

    // Without the fix in place, this test would have failed with
    // - Expected: Heading 1
    // - Actual  : Standard
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
        getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(Test, testPasteFirstParaDirectFormat)
{
    createSwDoc();
    {
        // Set some direct formatting on the first paragraph, but leave paragraph adjust at its
        // default (left).
        uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
        xParagraph->setPropertyValue("PageNumberOffset", uno::Any(static_cast<sal_Int16>(0)));
    }

    // Paste from ODT.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<document::XDocumentInsertable> xCursor(xText->createTextCursorByRange(xText->getStart()), uno::UNO_QUERY);
    xCursor->insertDocumentFromURL(createFileURL(u"paste-first-para-direct-format.odt"), {});

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3 (center)
    // - Actual  : 0 (left)
    // i.e. the inserted document's first paragraph's paragraph formatting was lost.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(style::ParagraphAdjust_CENTER),
                         getProperty<sal_Int16>(xParagraph, "ParaAdjust"));
}

CPPUNIT_TEST_FIXTURE(Test, testVerticallyMergedCellBorder)
{
    // Given a document with two cells, vertically merged, when loading the document:
    createSwDoc("vmerge-cell-border.odt");
    SwDoc* pDoc = getSwDoc();

    // Then make sure that the first cell has a right border while the second has no right border:
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(/*bBlock=*/false);
    SwStartNode* pA1 = pShellCursor->Start()->GetNode().StartOfSectionNode();
    const SwAttrSet& rA1Set = pA1->GetTableBox()->GetFrameFormat()->GetAttrSet();
    CPPUNIT_ASSERT(rA1Set.GetBox().GetRight());
    SwNodeIndex aA2(*pA1->EndOfSectionNode(), 1);
    const SwAttrSet& rA2Set = aA2.GetNode().GetTableBox()->GetFrameFormat()->GetAttrSet();

    // Without the accompanying fix in place, this test would have failed, as the A2 cell also had a
    // right border, even if <table:covered-table-cell table:style-name="..."> explicitly disabled
    // it.
    CPPUNIT_ASSERT(!rA2Set.GetBox().GetRight());

    // Given this document model, when exporting to ODT:
    save("writer8");

    // Then make sure the covered cell has a style.
    xmlDocUniquePtr pXmlSettings = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - In <...>, XPath '//table:covered-table-cell' no attribute 'style-name' exist
    assertXPath(pXmlSettings, "//table:covered-table-cell"_ostr, "style-name"_ostr, "Table1.A2");
}

CPPUNIT_TEST_FIXTURE(Test, testPageAnchorZIndexFirstPage)
{
    createSwDoc("PageAnchorZIndexFirstPage.fodt");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image1 index not 0",
        sal_Int32(0), getProperty<sal_Int32>(getShapeByName(u"Image1"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image2 index not 1",
        sal_Int32(1), getProperty<sal_Int32>(getShapeByName(u"Image2"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape1 index not 2",
        sal_Int32(2), getProperty<sal_Int32>(getShapeByName(u"Shape1"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape2 index not 3",
        sal_Int32(3), getProperty<sal_Int32>(getShapeByName(u"Shape2"), "ZOrder"));
}

CPPUNIT_TEST_FIXTURE(Test, testPageAnchorZIndexSecondPage)
{
    createSwDoc("PageAnchorZIndexSecondPage.fodt");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image1 index not 0",
        sal_Int32(0), getProperty<sal_Int32>(getShapeByName(u"Image1"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image2 index not 1",
        sal_Int32(1), getProperty<sal_Int32>(getShapeByName(u"Image2"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape1 index not 2",
        sal_Int32(2), getProperty<sal_Int32>(getShapeByName(u"Shape1"), "ZOrder"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape2 index not 3",
        sal_Int32(3), getProperty<sal_Int32>(getShapeByName(u"Shape2"), "ZOrder"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146257)
{
    createSwDoc("tdf146257.odt");

    // First list should contain no labels
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));
    }
    // For second list ensure second level empty and third level should not contain extra dots
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(7), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146597)
{
    createSwDoc("tdf146597.odt");
    // This was crashed before.
}

// just care that it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(Test, testForcepoint101)
{
    createSwDoc("forcepoint101.fodt");
}

// just care that it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(Test, testForcepoint108)
{
    //load would assert because it fails to load entirely, like testMathMalformedXml
    mxComponent = mxDesktop->loadComponentFromURL(createFileURL(u"forcepoint108.fodt"), "_default", 0, {});
}

#ifdef _WIN32
template <class T>
void runWindowsFileZoneTests(css::uno::Reference<css::frame::XDesktop2> const & aDesktop,
                             const OUString& sFileName, sal_Int32 configValue, sal_Int32 zoneId,
                             bool expectedResult)
{
    // Set desired configuration params
    auto xChanges = comphelper::ConfigurationChanges::create();
    T::set(configValue, xChanges);
    xChanges->commit();

    // Set Windows Security Zone for temp file
    sal::systools::COMReference<IZoneIdentifier> pZoneId;
    pZoneId.CoCreateInstance(CLSID_PersistentZoneIdentifier);

    // ignore setting of Zone 0, since at least for Windows Server
    // setups, that always leads to E_ACCESSDENIED - presumably since
    // the file is already local?
    //
    // See below for the workaround (calling tests for ZONE_LOCAL
    // first)
    if( zoneId != 0 )
    {
        CPPUNIT_ASSERT(SUCCEEDED(pZoneId->SetId(zoneId)));
        sal::systools::COMReference<IPersistFile> pPersist(pZoneId, sal::systools::COM_QUERY_THROW);
        OUString sTempFileWinPath;
        osl::FileBase::getSystemPathFromFileURL(sFileName, sTempFileWinPath);
        CPPUNIT_ASSERT(
            SUCCEEDED(pPersist->Save(reinterpret_cast<LPCOLESTR>(sTempFileWinPath.getStr()), TRUE)));
    }

    // Load doc with default for UI settings: do not suppress macro
    uno::Sequence<beans::PropertyValue> aLoadArgs{ comphelper::makePropertyValue(
        "MacroExecutionMode", css::document::MacroExecMode::USE_CONFIG) };
    auto aComponent = aDesktop->loadComponentFromURL(sFileName, "_default", 0, aLoadArgs);

    // Are macro enabled in doc?
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(aComponent.get());
    CPPUNIT_ASSERT_EQUAL(expectedResult, bool(pTextDoc->getAllowMacroExecution()));

    aComponent->dispose();
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testWindowsFileZone)
{
// This makes sense only for Windows
#ifdef _WIN32
    // Create a temp copy of zone test file
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    SvStream& aStreamDst = *aTempFile.GetStream(StreamMode::WRITE);
    SvFileStream aStreamSrc(createFileURL(u"ZoneMacroTest.odt"), StreamMode::READ);
    aStreamDst.WriteStream(aStreamSrc);
    aTempFile.CloseStream();

    // Tweak macro security to 1
    SvtSecurityOptions::SetMacroSecurityLevel(1);

    // Run all tests: set for temp file security zone and then check if macro are enabled
    // depending on configuration values for given zone
    // There is no easy way to check default (0) variant, so macro are disabled by default in these tests.

    // run tests for ZoneLocal first, since runWindowsFileZoneTests
    // ignores Zone 0 (see above) - assuming the initial file state is
    // local after a copy, we're still triggering the expected
    // behaviour
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneLocal>(
        mxDesktop, aTempFile.GetURL(), 0, 0, false);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneLocal>(
        mxDesktop, aTempFile.GetURL(), 1, 0, true);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneLocal>(
        mxDesktop, aTempFile.GetURL(), 2, 0, false);

    // run tests for other zones (these actually set the Windows
    // Security Zone at the file)
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneUntrusted>(
        mxDesktop, aTempFile.GetURL(), 0, 4, false);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneUntrusted>(
        mxDesktop, aTempFile.GetURL(), 1, 4, true);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneUntrusted>(
        mxDesktop, aTempFile.GetURL(), 2, 4, false);

    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneInternet>(
        mxDesktop, aTempFile.GetURL(), 0, 3, false);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneInternet>(
        mxDesktop, aTempFile.GetURL(), 1, 3, true);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneInternet>(
        mxDesktop, aTempFile.GetURL(), 2, 3, false);

    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneTrusted>(
        mxDesktop, aTempFile.GetURL(), 0, 2, false);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneTrusted>(
        mxDesktop, aTempFile.GetURL(), 1, 2, true);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneTrusted>(
        mxDesktop, aTempFile.GetURL(), 2, 2, false);

    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneIntranet>(
        mxDesktop, aTempFile.GetURL(), 0, 1, false);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneIntranet>(
        mxDesktop, aTempFile.GetURL(), 1, 1, true);
    runWindowsFileZoneTests<
        officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneIntranet>(
        mxDesktop, aTempFile.GetURL(), 2, 1, false);
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testEmptyTrailingSpans)
{
    createSwDoc("emptyParagraphLoosesFontHeight.fodt");

    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

    auto xPara2 = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(float(11), getProperty<float>(xPara2, "CharHeight"));
    auto xRun = getRun(xPara2, 1);
    CPPUNIT_ASSERT_EQUAL(float(8), getProperty<float>(xRun, "CharHeight"));
    // Both empty spans merge -> no more runs
    CPPUNIT_ASSERT_THROW(getRun(xPara2, 2), css::container::NoSuchElementException);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    auto height1 = getXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    auto height2 = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(height1, height2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(184, height2, 1); // allow a bit of room for rounding just in case
}

CPPUNIT_TEST_FIXTURE(Test, testBrokenPackage_Tdf159474)
{
    // Given an invalid ODF having a stream not referenced in manifest.xml
    const OUString url = createFileURL(u"unreferenced_stream.odt");
    // It expectedly fails to load normally:
    CPPUNIT_ASSERT_ASSERTION_FAIL(loadFromDesktop(url, {}, {}));
    // importing it must succeed with RepairPackage set to true.
    mxComponent
        = loadFromDesktop(url, {}, { comphelper::makePropertyValue(u"RepairPackage"_ustr, true) });
    // The document imports in repair mode; the original broken package is used as a template,
    // and the loaded document has no URL:
    CPPUNIT_ASSERT(mxComponent.queryThrow<frame::XModel>()->getURL().isEmpty());
    CPPUNIT_ASSERT_EQUAL(u"Empty document"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf161054)
{
    // Similar to testTdf161023 in sd/qa/unit/import-tests2.cxx
    // Given a shape with three paragraphs (18pt), all directly assigned a smaller font (10pt)
    createSwDoc("empty_line_in_text_box.fodt");
    auto shapeText = getShape(1).queryThrow<text::XTextRange>()->getText();

    // 1st paragraph, not empty
    {
        auto paragraph(getParagraphOfText(1, shapeText));
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, paragraph->getString());
        auto run(getRun(paragraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, run->getString());
        uno::Reference<beans::XPropertySet> xPropSet(run, uno::UNO_QUERY_THROW);
        double fCharHeight = 0;
        xPropSet->getPropertyValue("CharHeight") >>= fCharHeight;
        CPPUNIT_ASSERT_EQUAL(10.0, fCharHeight);
        // No more runs
        CPPUNIT_ASSERT_THROW(getRun(paragraph, 2), container::NoSuchElementException);
    }

    // Empty 2nd paragraph, consisting of a single span: this span was treated as "paragraph mark"
    {
        auto paragraph(getParagraphOfText(2, shapeText));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, paragraph->getString());
        auto run(getRun(paragraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, run->getString());
        uno::Reference<beans::XPropertySet> xPropSet(run, uno::UNO_QUERY_THROW);
        double fCharHeight = 0;
        xPropSet->getPropertyValue("CharHeight") >>= fCharHeight;
        // Without the fix, this would fail with
        // - Expected: 10
        // - Actual  : 18
        CPPUNIT_ASSERT_EQUAL(10.0, fCharHeight);
        // No more runs
        CPPUNIT_ASSERT_THROW(getRun(paragraph, 2), container::NoSuchElementException);
    }

    // 3rd paragraph, not empty
    {
        auto paragraph(getParagraphOfText(3, shapeText));
        CPPUNIT_ASSERT_EQUAL(u"c"_ustr, paragraph->getString());
        auto run(getRun(paragraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"c"_ustr, run->getString());
        uno::Reference<beans::XPropertySet> xPropSet(run, uno::UNO_QUERY_THROW);
        double fCharHeight = 0;
        xPropSet->getPropertyValue("CharHeight") >>= fCharHeight;
        CPPUNIT_ASSERT_EQUAL(10.0, fCharHeight);
        // No more runs
        CPPUNIT_ASSERT_THROW(getRun(paragraph, 2), container::NoSuchElementException);
    }
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
