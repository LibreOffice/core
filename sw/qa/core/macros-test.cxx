/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>

#include <i18nutil/searchopt.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>

#include <unotools/tempfile.hxx>

#include <unocrsr.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

typedef rtl::Reference<SwDocShell> SwDocShellRef;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class SwMacrosTest : public UnoApiTest
{
public:
    SwMacrosTest();

    void testVba();
    void testModernVBADelete();
    void testBookmarkDeleteAndJoin();
    void testBookmarkDeleteTdf90816();
    void testControlShapeGrouping();
    void testTdf151846();
    void testFdo55289();
    void testFdo68983();
    void testFdo87530();
    void testFindReplace();

    CPPUNIT_TEST_SUITE(SwMacrosTest);
    CPPUNIT_TEST(testVba);
    CPPUNIT_TEST(testModernVBADelete);
    CPPUNIT_TEST(testBookmarkDeleteAndJoin);
    CPPUNIT_TEST(testBookmarkDeleteTdf90816);
    CPPUNIT_TEST(testControlShapeGrouping);
    CPPUNIT_TEST(testTdf151846);
    CPPUNIT_TEST(testFdo55289);
    CPPUNIT_TEST(testFdo68983);
    CPPUNIT_TEST(testFdo87530);
    CPPUNIT_TEST(testFindReplace);
    CPPUNIT_TEST_SUITE_END();
};

void SwMacrosTest::testVba()
{
    TestMacroInfo testInfo[] = {
        {
            OUString("testVBA.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.testAll?language=Basic&location=document")
        },
        {
            OUString("testModernVBA.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.testAll?language=Basic&location=document")
        },
        {
            OUString("testFind.docm"),
            OUString("vnd.sun.Star.script:Project.Module1.testAll?language=Basic&location=document")
        },
        {
            OUString("testDocumentRange.docm"),
            OUString("vnd.sun.Star.script:Project.Module1.testAll?language=Basic&location=document")
        },
        /*{
            OUString("testSelectionFind.docm"),
            OUString("vnd.sun.Star.script:Project.Module1.testAll?language=Basic&location=document")
        },
        {
            //current working tests here!

            OUString("testFontColor.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.testAll?language=Basic&location=document")
        }
        // TODO - make these pass in Writer
        {
            OUString("testSentences.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.TestAll?language=Basic&location=document")
        },
        {
            OUString("testWords.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.TestAll?language=Basic&location=document")
        },
        {
            OUString("testParagraphFormat.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.TestAll?language=Basic&location=document")
        },*/
        {
            OUString("testTables.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.TestAll?language=Basic&location=document")
        }

    };
    for (auto const & [ sFileBaseName, sMacroUrl ] : testInfo)
    {
        OUString sFileName("docm/" + sFileBaseName);
        loadFromFile(sFileName);

        uno::Any aRet = executeMacro(sMacroUrl);
        OUString aStringRes;
        CPPUNIT_ASSERT_MESSAGE(sFileName.toUtf8().getStr(), aRet >>= aStringRes);
        CPPUNIT_ASSERT_EQUAL(OUString("OK"), aStringRes);
    }
}

void SwMacrosTest::testModernVBADelete()
{
    TestMacroInfo testInfo =
        {
            OUString("testModernVBADelete.docm"),
            OUString("vnd.sun.Star.script:Project.ThisDocument.testAll?language=Basic&location=document")
        };

    OUString sFileName("docm/" + testInfo.sFileBaseName);
    loadFromFile(sFileName);

    SwXTextDocument *const pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    SwDoc *const pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->GetIDocumentUndoRedo().DoUndo(true);
    CPPUNIT_ASSERT(!pDoc->GetIDocumentUndoRedo().GetUndoActionCount());

    uno::Any aRet = executeMacro(testInfo.sMacroUrl);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetIDocumentUndoRedo().GetUndoActionCount());

    OUString aStringRes;
    CPPUNIT_ASSERT(aRet >>= aStringRes);
    CPPUNIT_ASSERT_EQUAL(OUString("OK"), aStringRes);
}

void SwMacrosTest::testBookmarkDeleteAndJoin()
{
    rtl::Reference<SwDoc> const pDoc(new SwDoc);
    pDoc->GetIDocumentUndoRedo().DoUndo(true); // bug is in SwUndoDelete
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.AppendTextNode(*aPaM.GetPoint());
    rIDCO.InsertString(aPaM, "A");
    rIDCO.AppendTextNode(*aPaM.GetPoint());
    rIDCO.InsertString(aPaM, "A");
    rIDCO.AppendTextNode(*aPaM.GetPoint());
    aPaM.Move(fnMoveBackward, GoInNode);
    aPaM.Move(fnMoveBackward, GoInNode);
    aPaM.Move(fnMoveBackward, GoInContent);
    aPaM.SetMark();
    aPaM.Move(fnMoveForward, GoInDoc);
    IDocumentMarkAccess & rIDMA = *pDoc->getIDocumentMarkAccess();
    sw::mark::IMark *pMark =
        rIDMA.makeMark(aPaM, "test", IDocumentMarkAccess::MarkType::BOOKMARK,
            ::sw::mark::InsertMode::New);
    CPPUNIT_ASSERT(pMark);
    // select so pMark start position is on a node that is fully deleted
    aPaM.Move(fnMoveBackward, GoInNode);
    // must leave un-selected content in last node to get the bJoinPrev flag!
    aPaM.Move(fnMoveBackward, GoInContent);
    aPaM.Exchange();
    aPaM.Move(fnMoveBackward, GoInDoc);
    // delete
    rIDCO.DeleteAndJoin(aPaM);

    for (IDocumentMarkAccess::const_iterator_t i = rIDMA.getAllMarksBegin(); i != rIDMA.getAllMarksEnd(); ++i)
    {
        // problem was that the nContent was pointing at deleted node
        CPPUNIT_ASSERT_EQUAL((*i)->GetMarkStart().GetContentNode(),
            static_cast<const SwContentNode*>((*i)->GetMarkStart().GetNode().GetContentNode()));
    }
}

void SwMacrosTest::testBookmarkDeleteTdf90816()
{
    rtl::Reference<SwDoc> const pDoc(new SwDoc);
    pDoc->GetIDocumentUndoRedo().DoUndo(true); // bug is in SwUndoDelete
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.AppendTextNode(*aPaM.GetPoint());
    rIDCO.InsertString(aPaM, "ABC");
    aPaM.Move(fnMoveBackward, GoInContent);
    aPaM.SetMark();
    aPaM.Move(fnMoveBackward, GoInContent);
    IDocumentMarkAccess & rIDMA = *pDoc->getIDocumentMarkAccess();
    sw::mark::IMark *pMark =
        rIDMA.makeMark(aPaM, "test", IDocumentMarkAccess::MarkType::BOOKMARK,
            ::sw::mark::InsertMode::New);
    CPPUNIT_ASSERT(pMark);

    // delete the same selection as the bookmark
    rIDCO.DeleteAndJoin(aPaM);

    // bookmark still there?
    auto iter = rIDMA.getAllMarksBegin();
    CPPUNIT_ASSERT_MESSAGE("the bookmark was deleted",
            iter != rIDMA.getAllMarksEnd());
    CPPUNIT_ASSERT_EQUAL((*iter)->GetMarkPos(), *aPaM.Start());
    CPPUNIT_ASSERT_EQUAL((*iter)->GetOtherMarkPos(), *aPaM.End());
}

void SwMacrosTest::testControlShapeGrouping()
{
    loadFromFile(u"odt/testControlShapeGrouping.odt");

    uno::Reference<frame::XModel> const xModel(mxComponent, UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<lang::XMultiServiceFactory> xFactory(xModel, UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> const xDPS(xModel, UNO_QUERY);
    uno::Reference<drawing::XDrawPage> const xDP = xDPS->getDrawPage();
    CPPUNIT_ASSERT(xDP.is());
    uno::Reference<drawing::XShapes> const xDPShapes = xDP;
    CPPUNIT_ASSERT(xDPShapes.is());
    uno::Reference<drawing::XShapes> const xShapes(getMultiServiceFactory()->createInstance("com.sun.star.drawing.ShapeCollection"),
                                                   UNO_QUERY);
    CPPUNIT_ASSERT(xShapes.is());
    uno::Reference<container::XIndexAccess> xShapesIC = xShapes;
    CPPUNIT_ASSERT(xShapesIC.is());

    // uno::Reference<beans::XPropertySet> xFormProps(xForm, UNO_QUERY);
    // xFormProps->setPropertyValue("Name", makeAny("aForm"));
    uno::Reference<form::XFormsSupplier> const xFS(xDP, UNO_QUERY);
    CPPUNIT_ASSERT(xFS.is());
    uno::Reference<container::XIndexContainer> const xForms(xFS->getForms(), UNO_QUERY);
    CPPUNIT_ASSERT(xForms.is());
    uno::Reference<form::XForm> xForm(xForms->getByIndex(0), UNO_QUERY);
    CPPUNIT_ASSERT(xForm.is());
    uno::Reference<container::XNameContainer> xFormNC(xForm, UNO_QUERY);

    uno::Reference<drawing::XControlShape> const xDateShape(
        xFactory->createInstance("com.sun.star.drawing.ControlShape"),
        UNO_QUERY);
    uno::Reference<awt::XControlModel> const xDateControlModel(
        xFactory->createInstance("com.sun.star.form.component.DateField"),
        UNO_QUERY);
    xDateShape->setControl(xDateControlModel);
    uno::Reference<beans::XPropertySet> xDateShapeProps(xDateShape, UNO_QUERY);
    xDateShapeProps->setPropertyValue("AnchorType", Any(text::TextContentAnchorType_AT_PARAGRAPH));

    uno::Reference<drawing::XControlShape> const xTimeShape(
        xFactory->createInstance("com.sun.star.drawing.ControlShape"),
        UNO_QUERY);
    uno::Reference<awt::XControlModel> const xTimeControlModel(
        xFactory->createInstance("com.sun.star.form.component.TimeField"),
        UNO_QUERY);
    xTimeShape->setControl(xTimeControlModel);
    uno::Reference<beans::XPropertySet> xTimeShapeProps(xTimeShape, UNO_QUERY);
    xTimeShapeProps->setPropertyValue("AnchorType", Any(text::TextContentAnchorType_AT_PARAGRAPH));

    xFormNC->insertByName("aDateCntrl", Any(xDateControlModel));
    xDPShapes->add(xDateShape);
    xFormNC->insertByName("aTimeCntrl", Any(xTimeControlModel));
    xDPShapes->add(xTimeShape);

    xShapes->add(xDateShape);
    xShapes->add(xTimeShape);
    uno::Reference<drawing::XShapeGrouper> const xDPGrouper(xDP, UNO_QUERY);
    CPPUNIT_ASSERT(xDPGrouper.is());
    uno::Reference<drawing::XShapeGroup> xGroup(xDPGrouper->group(xShapes));
    CPPUNIT_ASSERT(xGroup.is());

#if 0
    uno::Reference<container::XIndexAccess> xGroupIC(xGroup, UNO_QUERY);
    CPPUNIT_ASSERT(xDateShape->getControl().is());
    CPPUNIT_ASSERT_EQUAL(xDateShape->getControl(), xDateControlModel);
    CPPUNIT_ASSERT(xTimeShape->getControl().is());
    CPPUNIT_ASSERT_EQUAL(xTimeShape->getControl(), xTimeControlModel);

    {
        uno::Reference< uno::XInterface > xDI;
        xGroupIC->getByIndex(0) >>= xDI;
        CPPUNIT_ASSERT(xDI.is());
        uno::Reference< drawing::XControlShape > xDS(xDI, UNO_QUERY);
        CPPUNIT_ASSERT(xDS.is());
        CPPUNIT_ASSERT_EQUAL(xDS->getControl(), xDateControlModel);

        uno::Reference< uno::XInterface > xTI;
        xGroupIC->getByIndex(1) >>= xTI;
        CPPUNIT_ASSERT(xTI.is());
        uno::Reference< drawing::XControlShape > xTS(xTI, UNO_QUERY);
        CPPUNIT_ASSERT(xTS.is());
        CPPUNIT_ASSERT_EQUAL(xTS->getControl(), xTimeControlModel);
    }
    {
        uno::Reference< uno::XInterface > xDI;
        xShapesIC->getByIndex(0) >>= xDI;
        CPPUNIT_ASSERT(xDI.is());
        uno::Reference< drawing::XControlShape > xDS(xDI, UNO_QUERY);
        CPPUNIT_ASSERT(xDS.is());
        CPPUNIT_ASSERT_EQUAL(xDS->getControl(), xDateControlModel);

        uno::Reference< uno::XInterface > xTI;
        xShapesIC->getByIndex(1) >>= xTI;
        CPPUNIT_ASSERT(xTI.is());
        uno::Reference< drawing::XControlShape > xTS(xTI, UNO_QUERY);
        CPPUNIT_ASSERT(xTS.is());
        CPPUNIT_ASSERT_EQUAL(xTS->getControl(), xTimeControlModel);
    }
#endif
}

void SwMacrosTest::testTdf151846()
{
    loadFromFile(u"odt/tdf151846.odt");

    // Without the fix in place, this test would have failed with
    // Property or method not found: createDiagramByDataSource.
    executeMacro("vnd.sun.Star.script:Standard.Module1.Main?language=Basic&location=document");

    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTEOSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xAccess(xTEOSupplier->getEmbeddedObjects());
    uno::Sequence<OUString> aSeq(xAccess->getElementNames());

    // Check number of embedded objects.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeq.getLength());
}

void SwMacrosTest::testFdo55289()
{
    SwDoc* const pDoc = new SwDoc;
    SwDocShellRef pDocShell = new SwDocShell(*pDoc, SfxObjectCreateMode::EMBEDDED);
    // this needs to run with no layout to tickle the bugs in the special
    // cases in SwXShape re-anchoring
    assert(!pDoc->getIDocumentLayoutAccess().GetCurrentLayout());

    uno::Reference<frame::XModel> const xModel(pDocShell->GetModel());
    uno::Reference<drawing::XDrawPageSupplier> const xDPS(xModel, UNO_QUERY);
    uno::Reference<drawing::XShapes> const xShapes = xDPS->getDrawPage();
    uno::Reference<beans::XPropertySet> const xShape(
        uno::Reference<lang::XMultiServiceFactory>(xModel, UNO_QUERY_THROW)->
            createInstance("com.sun.star.drawing.GraphicObjectShape"),
        UNO_QUERY);
    xShape->setPropertyValue("AnchorType",
            Any(text::TextContentAnchorType_AT_PAGE));
    xShapes->add(uno::Reference<drawing::XShape>(xShape, UNO_QUERY));
    xShape->setPropertyValue("AnchorType",
            Any(text::TextContentAnchorType_AT_CHARACTER));
    xShape->setPropertyValue("AnchorType",
            Any(text::TextContentAnchorType_AS_CHARACTER));
    xShape->setPropertyValue("AnchorType",
            Any(text::TextContentAnchorType_AT_CHARACTER));
    xShape->setPropertyValue("AnchorType",
            Any(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextRange> const xEnd =
        uno::Reference<text::XTextDocument>(xModel, UNO_QUERY_THROW)->getText()->getEnd();
    uno::Reference<text::XTextContent> const xShapeContent(xShape, UNO_QUERY);
    xShapeContent->attach(xEnd);
    pDocShell->DoClose();
}

void SwMacrosTest::testFdo68983()
{
    loadFromFile(u"odt/fdo68983.odt");
    Reference< frame::XStorable > xDocStorable(mxComponent, UNO_QUERY_THROW);

    saveAndReload("writer8");

    // check that password-protected library survived store and re-load
    Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
    Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
    Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xBasLibPwd->isLibraryPasswordProtected("Library1"));
    CPPUNIT_ASSERT(xBasLibPwd->verifyLibraryPassword("Library1", "foo"));
    xBasLib->loadLibrary("Library1");
    CPPUNIT_ASSERT(xBasLib->isLibraryLoaded("Library1"));
}

void SwMacrosTest::testFdo87530()
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    {
        // insert initial password protected library
        Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
        Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
        Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
        Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
        Reference<container::XNameContainer> xLibrary(xBasLib->createLibrary("BarLibrary"));
        xLibrary->insertByName("BarModule",
                uno::Any(OUString("Sub Main\nEnd Sub\n")));
        xBasLibPwd->changeLibraryPassword("BarLibrary", "", "foo");
    }

    saveAndReload("writer8");

    {
        // check that password-protected library survived store and re-load
        Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
        Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
        Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
        Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xBasLibPwd->isLibraryPasswordProtected("BarLibrary"));
        CPPUNIT_ASSERT(xBasLibPwd->verifyLibraryPassword("BarLibrary", "foo"));
        xBasLib->loadLibrary("BarLibrary");
        CPPUNIT_ASSERT(xBasLib->isLibraryLoaded("BarLibrary"));
        Reference<container::XNameContainer> xLibrary(xBasLib->getByName("BarLibrary"), UNO_QUERY);
        Any module(xLibrary->getByName("BarModule"));
        CPPUNIT_ASSERT_EQUAL(OUString("Sub Main\nEnd Sub\n"), module.get<OUString>());

        // add a second module now - tdf#87530 happened here
        Reference<container::XNameContainer> xFooLib(xBasLib->createLibrary("FooLibrary"));
        xFooLib->insertByName("FooModule",
                uno::Any(OUString("Sub Main\nEnd Sub\n")));
        xBasLibPwd->changeLibraryPassword("FooLibrary", "", "foo");
    }

    saveAndReload("writer8");

    // check that password-protected library survived store and re-load
    Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
    Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
    Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xBasLibPwd->isLibraryPasswordProtected("FooLibrary"));
    CPPUNIT_ASSERT(xBasLibPwd->verifyLibraryPassword("FooLibrary", "foo"));
    xBasLib->loadLibrary("FooLibrary");
    CPPUNIT_ASSERT(xBasLib->isLibraryLoaded("FooLibrary"));
    Reference<container::XNameContainer> xLibrary(xBasLib->getByName("FooLibrary"), UNO_QUERY);
    Any module(xLibrary->getByName("FooModule"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sub Main\nEnd Sub\n"), module.get<OUString>());
}


void SwMacrosTest::testFindReplace()
{
    // we need a full document with view and layout etc. because ::GetNode()
    mxComponent =
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument *const pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc *const pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    // use a UnoCursor so it will be corrected when deleting nodes
    auto pPaM(pDoc->CreateUnoCursor(SwPosition(aIdx)));

    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.InsertString(*pPaM, "foo");
    rIDCO.AppendTextNode(*pPaM->GetPoint());
    rIDCO.InsertString(*pPaM, "bar");
    rIDCO.AppendTextNode(*pPaM->GetPoint());
    rIDCO.InsertString(*pPaM, "baz");
    pPaM->Move(fnMoveBackward, GoInDoc);

    bool bCancel(false);
    i18nutil::SearchOptions2 opts(
            css::util::SearchFlags::LEV_RELAXED,
            "$",
            "",
            lang::Locale("en", "US", ""),
            2,
            2,
            2,
            TransliterationFlags::IGNORE_CASE | TransliterationFlags::IGNORE_WIDTH |
            TransliterationFlags::IGNORE_KASHIDA_CTL | TransliterationFlags::IGNORE_DIACRITICS_CTL,
            util::SearchAlgorithms2::REGEXP,
            '\\');

    // find newline on 1st paragraph
    bool bFound = pPaM->Find_Text(
            opts, false, SwDocPositions::Curr, SwDocPositions::End, bCancel, FindRanges::InBody);
    CPPUNIT_ASSERT(bFound);
    CPPUNIT_ASSERT(pPaM->HasMark());
    CPPUNIT_ASSERT(pPaM->GetPoint()->GetNode() != pPaM->GetMark()->GetNode());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_NEWLINE)), pPaM->GetText());

    // now do another Find, inside the selection from the first Find
//    opts.searchFlags = 71680;
    bFound = pPaM->Find_Text(
            opts, false, SwDocPositions::Curr, SwDocPositions::End, bCancel, FindRanges::InSel);
    CPPUNIT_ASSERT(bFound);
    CPPUNIT_ASSERT(pPaM->HasMark());
    CPPUNIT_ASSERT(pPaM->GetPoint()->GetNode() != pPaM->GetMark()->GetNode());
    CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_NEWLINE)), pPaM->GetText());

    rIDCO.ReplaceRange(*pPaM, " ", true);

    pPaM->DeleteMark();
    pPaM->Move(fnMoveBackward, GoInDoc);

    // problem was that after the 2nd Find, the wrong newline was selected
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
            pPaM->Start()->GetNode().GetTextNode()->GetText());
    pPaM->Move(fnMoveForward, GoInNode);
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
            pPaM->End()->GetNode().GetTextNode()->GetText());
}

SwMacrosTest::SwMacrosTest()
      : UnoApiTest("/sw/qa/core/data/")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
