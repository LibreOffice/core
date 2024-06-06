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
            u"testVBA.docm"_ustr,
            u"vnd.sun.Star.script:Project.ThisDocument.testAll?language=Basic&location=document"_ustr
        },
        {
            u"testModernVBA.docm"_ustr,
            u"vnd.sun.Star.script:Project.ThisDocument.testAll?language=Basic&location=document"_ustr
        },
        {
            u"testFind.docm"_ustr,
            u"vnd.sun.Star.script:Project.Module1.testAll?language=Basic&location=document"_ustr
        },
        {
            u"testDocumentRange.docm"_ustr,
            u"vnd.sun.Star.script:Project.Module1.testAll?language=Basic&location=document"_ustr
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
            u"testTables.docm"_ustr,
            u"vnd.sun.Star.script:Project.ThisDocument.TestAll?language=Basic&location=document"_ustr
        }

    };
    for (auto const & [ sFileBaseName, sMacroUrl ] : testInfo)
    {
        OUString sFileName("docm/" + sFileBaseName);
        loadFromFile(sFileName);

        uno::Any aRet = executeMacro(sMacroUrl);
        OUString aStringRes;
        CPPUNIT_ASSERT_MESSAGE(sFileName.toUtf8().getStr(), aRet >>= aStringRes);
        CPPUNIT_ASSERT_EQUAL(u"OK"_ustr, aStringRes);
    }
}

void SwMacrosTest::testModernVBADelete()
{
    TestMacroInfo testInfo =
        {
            u"testModernVBADelete.docm"_ustr,
            u"vnd.sun.Star.script:Project.ThisDocument.testAll?language=Basic&location=document"_ustr
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
    CPPUNIT_ASSERT_EQUAL(u"OK"_ustr, aStringRes);
}

void SwMacrosTest::testBookmarkDeleteAndJoin()
{
    rtl::Reference<SwDoc> const pDoc(new SwDoc);
    pDoc->GetIDocumentUndoRedo().DoUndo(true); // bug is in SwUndoDelete
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.AppendTextNode(*aPaM.GetPoint());
    rIDCO.InsertString(aPaM, u"A"_ustr);
    rIDCO.AppendTextNode(*aPaM.GetPoint());
    rIDCO.InsertString(aPaM, u"A"_ustr);
    rIDCO.AppendTextNode(*aPaM.GetPoint());
    aPaM.Move(fnMoveBackward, GoInNode);
    aPaM.Move(fnMoveBackward, GoInNode);
    aPaM.Move(fnMoveBackward, GoInContent);
    aPaM.SetMark();
    aPaM.Move(fnMoveForward, GoInDoc);
    IDocumentMarkAccess & rIDMA = *pDoc->getIDocumentMarkAccess();
    sw::mark::IMark *pMark =
        rIDMA.makeMark(aPaM, u"test"_ustr, IDocumentMarkAccess::MarkType::BOOKMARK,
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
    rIDCO.InsertString(aPaM, u"ABC"_ustr);
    aPaM.Move(fnMoveBackward, GoInContent);
    aPaM.SetMark();
    aPaM.Move(fnMoveBackward, GoInContent);
    IDocumentMarkAccess & rIDMA = *pDoc->getIDocumentMarkAccess();
    sw::mark::IMark *pMark =
        rIDMA.makeMark(aPaM, u"test"_ustr, IDocumentMarkAccess::MarkType::BOOKMARK,
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
    uno::Reference<drawing::XShapes> const xShapes(getMultiServiceFactory()->createInstance(u"com.sun.star.drawing.ShapeCollection"_ustr),
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
        xFactory->createInstance(u"com.sun.star.drawing.ControlShape"_ustr),
        UNO_QUERY);
    uno::Reference<awt::XControlModel> const xDateControlModel(
        xFactory->createInstance(u"com.sun.star.form.component.DateField"_ustr),
        UNO_QUERY);
    xDateShape->setControl(xDateControlModel);
    uno::Reference<beans::XPropertySet> xDateShapeProps(xDateShape, UNO_QUERY);
    xDateShapeProps->setPropertyValue(u"AnchorType"_ustr, Any(text::TextContentAnchorType_AT_PARAGRAPH));

    uno::Reference<drawing::XControlShape> const xTimeShape(
        xFactory->createInstance(u"com.sun.star.drawing.ControlShape"_ustr),
        UNO_QUERY);
    uno::Reference<awt::XControlModel> const xTimeControlModel(
        xFactory->createInstance(u"com.sun.star.form.component.TimeField"_ustr),
        UNO_QUERY);
    xTimeShape->setControl(xTimeControlModel);
    uno::Reference<beans::XPropertySet> xTimeShapeProps(xTimeShape, UNO_QUERY);
    xTimeShapeProps->setPropertyValue(u"AnchorType"_ustr, Any(text::TextContentAnchorType_AT_PARAGRAPH));

    xFormNC->insertByName(u"aDateCntrl"_ustr, Any(xDateControlModel));
    xDPShapes->add(xDateShape);
    xFormNC->insertByName(u"aTimeCntrl"_ustr, Any(xTimeControlModel));
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
    executeMacro(u"vnd.sun.Star.script:Standard.Module1.Main?language=Basic&location=document"_ustr);

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
            createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr),
        UNO_QUERY);
    xShape->setPropertyValue(u"AnchorType"_ustr,
            Any(text::TextContentAnchorType_AT_PAGE));
    xShapes->add(uno::Reference<drawing::XShape>(xShape, UNO_QUERY));
    xShape->setPropertyValue(u"AnchorType"_ustr,
            Any(text::TextContentAnchorType_AT_CHARACTER));
    xShape->setPropertyValue(u"AnchorType"_ustr,
            Any(text::TextContentAnchorType_AS_CHARACTER));
    xShape->setPropertyValue(u"AnchorType"_ustr,
            Any(text::TextContentAnchorType_AT_CHARACTER));
    xShape->setPropertyValue(u"AnchorType"_ustr,
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

    saveAndReload(u"writer8"_ustr);

    // check that password-protected library survived store and re-load
    Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
    Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
    Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xBasLibPwd->isLibraryPasswordProtected(u"Library1"_ustr));
    CPPUNIT_ASSERT(xBasLibPwd->verifyLibraryPassword(u"Library1"_ustr, u"foo"_ustr));
    xBasLib->loadLibrary(u"Library1"_ustr);
    CPPUNIT_ASSERT(xBasLib->isLibraryLoaded(u"Library1"_ustr));
}

void SwMacrosTest::testFdo87530()
{
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);

    {
        // insert initial password protected library
        Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
        Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
        Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
        Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
        Reference<container::XNameContainer> xLibrary(xBasLib->createLibrary(u"BarLibrary"_ustr));
        xLibrary->insertByName(u"BarModule"_ustr,
                uno::Any(u"Sub Main\nEnd Sub\n"_ustr));
        xBasLibPwd->changeLibraryPassword(u"BarLibrary"_ustr, u""_ustr, u"foo"_ustr);
    }

    saveAndReload(u"writer8"_ustr);

    {
        // check that password-protected library survived store and re-load
        Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
        Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
        Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
        Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xBasLibPwd->isLibraryPasswordProtected(u"BarLibrary"_ustr));
        CPPUNIT_ASSERT(xBasLibPwd->verifyLibraryPassword(u"BarLibrary"_ustr, u"foo"_ustr));
        xBasLib->loadLibrary(u"BarLibrary"_ustr);
        CPPUNIT_ASSERT(xBasLib->isLibraryLoaded(u"BarLibrary"_ustr));
        Reference<container::XNameContainer> xLibrary(xBasLib->getByName(u"BarLibrary"_ustr), UNO_QUERY);
        Any module(xLibrary->getByName(u"BarModule"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Sub Main\nEnd Sub\n"_ustr, module.get<OUString>());

        // add a second module now - tdf#87530 happened here
        Reference<container::XNameContainer> xFooLib(xBasLib->createLibrary(u"FooLibrary"_ustr));
        xFooLib->insertByName(u"FooModule"_ustr,
                uno::Any(u"Sub Main\nEnd Sub\n"_ustr));
        xBasLibPwd->changeLibraryPassword(u"FooLibrary"_ustr, u""_ustr, u"foo"_ustr);
    }

    saveAndReload(u"writer8"_ustr);

    // check that password-protected library survived store and re-load
    Reference<document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
    Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
    Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xBasLibPwd->isLibraryPasswordProtected(u"FooLibrary"_ustr));
    CPPUNIT_ASSERT(xBasLibPwd->verifyLibraryPassword(u"FooLibrary"_ustr, u"foo"_ustr));
    xBasLib->loadLibrary(u"FooLibrary"_ustr);
    CPPUNIT_ASSERT(xBasLib->isLibraryLoaded(u"FooLibrary"_ustr));
    Reference<container::XNameContainer> xLibrary(xBasLib->getByName(u"FooLibrary"_ustr), UNO_QUERY);
    Any module(xLibrary->getByName(u"FooModule"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Sub Main\nEnd Sub\n"_ustr, module.get<OUString>());
}


void SwMacrosTest::testFindReplace()
{
    // we need a full document with view and layout etc. because ::GetNode()
    mxComponent =
        loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);

    SwXTextDocument *const pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc *const pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    // use a UnoCursor so it will be corrected when deleting nodes
    auto pPaM(pDoc->CreateUnoCursor(SwPosition(aIdx)));

    IDocumentContentOperations & rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.InsertString(*pPaM, u"foo"_ustr);
    rIDCO.AppendTextNode(*pPaM->GetPoint());
    rIDCO.InsertString(*pPaM, u"bar"_ustr);
    rIDCO.AppendTextNode(*pPaM->GetPoint());
    rIDCO.InsertString(*pPaM, u"baz"_ustr);
    pPaM->Move(fnMoveBackward, GoInDoc);

    bool bCancel(false);
    i18nutil::SearchOptions2 opts(
            css::util::SearchFlags::LEV_RELAXED,
            u"$"_ustr,
            u""_ustr,
            lang::Locale(u"en"_ustr, u"US"_ustr, u""_ustr),
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

    rIDCO.ReplaceRange(*pPaM, u" "_ustr, true);

    pPaM->DeleteMark();
    pPaM->Move(fnMoveBackward, GoInDoc);

    // problem was that after the 2nd Find, the wrong newline was selected
    CPPUNIT_ASSERT_EQUAL(u"foo bar"_ustr,
            pPaM->Start()->GetNode().GetTextNode()->GetText());
    pPaM->Move(fnMoveForward, GoInNode);
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr,
            pPaM->End()->GetNode().GetTextNode()->GetText());
}

SwMacrosTest::SwMacrosTest()
      : UnoApiTest(u"/sw/qa/core/data/"_ustr)
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
