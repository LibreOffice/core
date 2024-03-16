/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/FontWeight.hpp>
#include <swmodeltestbase.hxx>
#include <itabenum.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <expfld.hxx>
#include <drawdoc.hxx>
#include <view.hxx>
#include <swacorr.hxx>
#include <swmodule.hxx>
#include <charatr.hxx>
#include <editeng/acorrcfg.hxx>
#include <unotbl.hxx>
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include <com/sun/star/awt/FontUnderline.hpp>

#include <svx/hdft.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>

#include <i18nutil/searchopt.hxx>
#include <txtftn.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <unofldmid.h>
#include <UndoManager.hxx>
#include <textsh.hxx>
#include <frmatr.hxx>
#include <frmmgr.hxx>

#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <osl/file.hxx>
#include <drawfont.hxx>
#include <txtfrm.hxx>
#include <txttypes.hxx>
#include <SwPortionHandler.hxx>
#include <comphelper/propertysequence.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/docfilt.hxx>
#include <vcl/scheduler.hxx>
#include <config_features.h>
#include <config_fonts.h>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <officecfg/Office/Writer.hxx>
#include <vcl/idletask.hxx>

namespace
{
int CountFilesInDirectory(const OUString& rURL)
{
    int nRet = 0;

    osl::Directory aDir(rURL);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aDir.open());

    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_Type);
    while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        if (aFileStatus.getFileType() != osl::FileStatus::Directory)
            ++nRet;
    }

    return nRet;
}

} //namespace

class SwUiWriterTest7 : public SwModelTestBase
{
public:
    SwUiWriterTest7()
        : SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testExportToPicture)
{
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PixelWidth", uno::Any(sal_Int32(610)) },
                                           { "PixelHeight", uno::Any(sal_Int32(610)) } }));
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        { { "FilterName", uno::Any(OUString("writer_png_Export")) },
          { "FilterData", uno::Any(aFilterData) } }));
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), aDescriptor);
    bool extchk = maTempFile.IsValid();
    CPPUNIT_ASSERT_EQUAL(true, extchk);
    osl::File tmpFile(maTempFile.GetURL());
    tmpFile.open(sal_uInt32(osl_File_OpenFlag_Read));
    sal_uInt64 val;
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, tmpFile.getSize(val));
    CPPUNIT_ASSERT(val > 100);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf77340)
{
    createSwDoc();
    //Getting some paragraph style in our document
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(
        xFactory->createInstance("com.sun.star.style.ParagraphStyle"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue("ParaBackColor", uno::Any(sal_Int32(0xFF00FF)));
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNameAccess(xSupplier->getStyleFamilies());
    uno::Reference<container::XNameContainer> xNameCont;
    xNameAccess->getByName("ParagraphStyles") >>= xNameCont;
    xNameCont->insertByName("myStyle", uno::Any(xStyle));
    CPPUNIT_ASSERT_EQUAL(OUString("myStyle"), xStyle->getName());
    //Setting the properties with proper values
    xPropSet->setPropertyValue("PageDescName", uno::Any(OUString("First Page")));
    xPropSet->setPropertyValue("PageNumberOffset", uno::Any(sal_Int16(3)));
    //Getting the properties and checking that they have proper values
    CPPUNIT_ASSERT_EQUAL(uno::Any(OUString("First Page")),
                         xPropSet->getPropertyValue("PageDescName"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(3)), xPropSet->getPropertyValue("PageNumberOffset"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf79236)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    //Getting some paragraph style
    SwTextFormatColl* pTextFormat = pDoc->FindTextFormatCollByName(u"Body Text"_ustr);
    const SwAttrSet& rAttrSet = pTextFormat->GetAttrSet();
    std::unique_ptr<SfxItemSet> pNewSet = rAttrSet.Clone();
    sal_uInt16 initialCount = pNewSet->Count();
    SvxAdjustItem AdjustItem = rAttrSet.GetAdjust();
    SvxAdjust initialAdjust = AdjustItem.GetAdjust();
    //By default the adjust is LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, initialAdjust);
    //Changing the adjust to RIGHT
    AdjustItem.SetAdjust(SvxAdjust::Right);
    //Checking whether the change is made or not
    SvxAdjust modifiedAdjust = AdjustItem.GetAdjust();
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, modifiedAdjust);
    //Modifying the itemset, putting *one* item
    pNewSet->Put(AdjustItem);
    //The count should increment by 1
    sal_uInt16 modifiedCount = pNewSet->Count();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(initialCount + 1), modifiedCount);
    //Setting the updated item set on the style
    pDoc->ChgFormat(*pTextFormat, *pNewSet);
    //Checking the Changes
    SwTextFormatColl* pTextFormat2 = pDoc->FindTextFormatCollByName(u"Body Text"_ustr);
    const SwAttrSet& rAttrSet2 = pTextFormat2->GetAttrSet();
    const SvxAdjustItem& rAdjustItem2 = rAttrSet2.GetAdjust();
    SvxAdjust Adjust2 = rAdjustItem2.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, Adjust2);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat3 = pDoc->FindTextFormatCollByName(u"Body Text"_ustr);
    const SwAttrSet& rAttrSet3 = pTextFormat3->GetAttrSet();
    const SvxAdjustItem& rAdjustItem3 = rAttrSet3.GetAdjust();
    SvxAdjust Adjust3 = rAdjustItem3.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, Adjust3);
    //Redo the changes
    rUndoManager.Redo();
    SwTextFormatColl* pTextFormat4 = pDoc->FindTextFormatCollByName(u"Body Text"_ustr);
    const SwAttrSet& rAttrSet4 = pTextFormat4->GetAttrSet();
    const SvxAdjustItem& rAdjustItem4 = rAttrSet4.GetAdjust();
    SvxAdjust Adjust4 = rAdjustItem4.GetAdjust();
    //The adjust should be RIGHT as per the modifications made
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, Adjust4);
    //Undo the changes
    rUndoManager.Undo();
    SwTextFormatColl* pTextFormat5 = pDoc->FindTextFormatCollByName(u"Body Text"_ustr);
    const SwAttrSet& rAttrSet5 = pTextFormat5->GetAttrSet();
    const SvxAdjustItem& rAdjustItem5 = rAttrSet5.GetAdjust();
    SvxAdjust Adjust5 = rAdjustItem5.GetAdjust();
    //The adjust should be back to default, LEFT
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Left, Adjust5);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTextSearch)
{
    // Create a new empty Writer document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    // Insert some text
    rIDCO.InsertString(*pCursor, "Hello World This is a test");
    // Use cursor to select part of text
    for (int i = 0; i < 10; i++)
    {
        pCursor->Move(fnMoveBackward);
    }
    pCursor->SetMark();
    for (int i = 0; i < 4; i++)
    {
        pCursor->Move(fnMoveBackward);
    }
    //Checking that the proper selection is made
    CPPUNIT_ASSERT_EQUAL(OUString("This"), pCursor->GetText());
    // Apply a "Bold" attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    //making another selection of text
    for (int i = 0; i < 7; i++)
    {
        pCursor->Move(fnMoveBackward);
    }
    pCursor->SetMark();
    for (int i = 0; i < 5; i++)
    {
        pCursor->Move(fnMoveBackward);
    }
    //Checking that the proper selection is made
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), pCursor->GetText());
    // Apply a "Bold" attribute to selection
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    //Performing Search Operation and also covering the UNO coverage for setProperty
    uno::Reference<util::XSearchable> xSearch(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XSearchDescriptor> xSearchDes = xSearch->createSearchDescriptor();
    uno::Reference<util::XPropertyReplace> xProp(xSearchDes, uno::UNO_QUERY);
    //setting some properties
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        { { "CharWeight", uno::Any(float(css::awt::FontWeight::BOLD)) } }));
    xProp->setSearchAttributes(aDescriptor);
    //receiving the defined properties and asserting them with expected values, covering UNO
    uno::Sequence<beans::PropertyValue> aPropVal2(xProp->getSearchAttributes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aPropVal2.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("CharWeight"), aPropVal2[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(float(css::awt::FontWeight::BOLD)), aPropVal2[0].Value);
    //specifying the search attributes
    uno::Reference<beans::XPropertySet> xPropSet(xSearchDes, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue("SearchWords", uno::Any(true));
    xPropSet->setPropertyValue("SearchCaseSensitive", uno::Any(true));
    //this will search all the BOLD words
    uno::Reference<container::XIndexAccess> xIndex(xSearch->findAll(xSearchDes));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndex->getCount());
    //Replacing the searched string via XReplaceable
    uno::Reference<util::XReplaceable> xReplace(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XReplaceDescriptor> xReplaceDes = xReplace->createReplaceDescriptor();
    uno::Reference<util::XPropertyReplace> xProp2(xReplaceDes, uno::UNO_QUERY);
    xProp2->setReplaceAttributes(aDescriptor);
    //checking that the proper attributes are there or not
    uno::Sequence<beans::PropertyValue> aRepProp(xProp2->getReplaceAttributes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRepProp.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("CharWeight"), aRepProp[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(float(css::awt::FontWeight::BOLD)), aRepProp[0].Value);
    //setting strings for replacement
    xReplaceDes->setSearchString("test");
    xReplaceDes->setReplaceString("task");
    //checking the replaceString
    CPPUNIT_ASSERT_EQUAL(OUString("task"), xReplaceDes->getReplaceString());
    //this will replace *normal*test to *bold*task
    sal_Int32 ReplaceCount = xReplace->replaceAll(xReplaceDes);
    //There should be only 1 replacement since there is only one occurrence of "test" in the document
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    //Now performing search again for BOLD words, count should be 3 due to replacement
    uno::Reference<container::XIndexAccess> xIndex2(xReplace->findAll(xSearchDes));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndex2->getCount());
    // regex tests
    xSearchDes->setPropertyValue("SearchRegularExpression", uno::Any(true));
    // regex: test correct matching combined with attributes like BOLD
    xSearchDes->setSearchString(".*"); // should match all bold words in the text
    xIndex.set(xReplace->findAll(xSearchDes), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndex->getCount());
    uno::Reference<text::XTextRange> xFound(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xFound->getString());
    xFound.set(xIndex->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("This"), xFound->getString());
    xFound.set(xIndex->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("task"), xFound->getString());
    // regex: test anchor combined with attributes like BOLD
    xSearchDes->setSearchString("^.*|.*$"); // should match first and last words (they are bold)
    xIndex.set(xReplace->findAll(xSearchDes), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndex->getCount());
    xFound.set(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xFound->getString());
    xFound.set(xIndex->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("task"), xFound->getString());
    // regex: test look-ahead/look-behind assertions outside of the bold text
    xSearchDes->setSearchString("(?<= ).*(?= )"); // should match second bold word
    xIndex.set(xReplace->findAll(xSearchDes), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndex->getCount());
    xFound.set(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("This"), xFound->getString());
    xReplaceDes->setPropertyValue("SearchRegularExpression", uno::Any(true));
    // regex: test correct match of paragraph start
    xReplaceDes->setSearchString("^."); // should only match first character of the paragraph
    xReplaceDes->setReplaceString("C");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: test correct match of word start
    xReplaceDes->setSearchString("\\b\\w"); // should match all words' first characters
    xReplaceDes->setReplaceString("x&");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), ReplaceCount);
    // regex: test negative look-behind assertion
    xReplaceDes->setSearchString("(?<!xCelly xW)o"); // only "o" in "xCello", not in "xWorld"
    xReplaceDes->setReplaceString("y");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: test positive look-behind assertion
    xReplaceDes->setSearchString("(?<=xCelly xWorld xTh)i"); // only "i" in "xThis", not in "xis"
    xReplaceDes->setReplaceString("z");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: use capturing group to test reference
    xReplaceDes->setSearchString("\\b(\\w\\w\\w\\w)\\w");
    xReplaceDes->setReplaceString("$1q"); // only fifth characters in words should change
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), ReplaceCount);
    // check of the end result
    CPPUNIT_ASSERT_EQUAL(OUString("xCelqy xWorqd xThzq xis xa xtasq"),
                         pCursor->GetPointNode().GetTextNode()->GetText());
    // regex: use positive look-ahead assertion
    xReplaceDes->setSearchString("Wor(?=qd xThzq xis xa xtasq)");
    xReplaceDes->setReplaceString("&p"); // testing & reference
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), ReplaceCount);
    // regex: use negative look-ahead assertion
    xReplaceDes->setSearchString("x(?!Worpqd xThzq xis xa xtasq)");
    xReplaceDes->setReplaceString("m");
    ReplaceCount = xReplace->replaceAll(xReplaceDes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), ReplaceCount); // one of the 6 "x" must not be replaced
    // check of the end result
    CPPUNIT_ASSERT_EQUAL(OUString("mCelqy xWorpqd mThzq mis ma mtasq"),
                         pCursor->GetPointNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf147583_backwardSearch)
{
    createSwDoc("tdf147583_backwardSearch.odt");
    uno::Reference<util::XSearchable> xSearch(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XSearchDescriptor> xSearchDes = xSearch->createSearchDescriptor();
    uno::Reference<util::XPropertyReplace> xProp(xSearchDes, uno::UNO_QUERY);

    uno::Reference<container::XIndexAccess> xIndex;
    const sal_Int32 nParas = getParagraphs();

    //specifying the search attributes
    uno::Reference<beans::XPropertySet> xPropSet(xSearchDes, uno::UNO_QUERY_THROW);
    xSearchDes->setPropertyValue("SearchRegularExpression", uno::Any(true)); // regex
    xSearchDes->setSearchString("$"); // the end of the paragraph pilcrow marker

    // xSearchDes->setPropertyValue("SearchBackwards", uno::Any(false));
    // xIndex.set(xSearch->findAll(xSearchDes), uno::UNO_SET_THROW);
    // // all paragraphs (including the unselected last one) should be found
    // CPPUNIT_ASSERT_EQUAL(nParas, xIndex->getCount());

    xSearchDes->setPropertyValue("SearchBackwards", uno::Any(true));
    xIndex.set(xSearch->findAll(xSearchDes), uno::UNO_SET_THROW);
    // all paragraphs (except the troublesome last one) are found
    CPPUNIT_ASSERT_EQUAL(nParas - 1, xIndex->getCount());

    xSearchDes->setSearchString("^$"); // empty paragraphs
    xIndex.set(xSearch->findAll(xSearchDes), uno::UNO_SET_THROW);
    // should actually be 10 (including the empty para with the comment marker, and the last para)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xIndex->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf69282)
{
    createSwDoc();
    SwDoc* source = getSwDoc();
    uno::Reference<lang::XComponent> xSourceDoc = mxComponent;
    mxComponent.clear();
    createSwDoc();
    SwDoc* target = getSwDoc();
    SwPageDesc* sPageDesc = source->MakePageDesc("SourceStyle");
    SwPageDesc* tPageDesc = target->MakePageDesc("TargetStyle");
    sPageDesc->ChgFirstShare(false);
    CPPUNIT_ASSERT(!sPageDesc->IsFirstShared());
    SwFrameFormat& rSourceMasterFormat = sPageDesc->GetMaster();
    //Setting horizontal spaces on master
    SvxLRSpaceItem horizontalSpace(RES_LR_SPACE);
    horizontalSpace.SetLeft(11);
    horizontalSpace.SetRight(12);
    rSourceMasterFormat.SetFormatAttr(horizontalSpace);
    //Setting vertical spaces on master
    SvxULSpaceItem verticalSpace(RES_UL_SPACE);
    verticalSpace.SetUpper(13);
    verticalSpace.SetLower(14);
    rSourceMasterFormat.SetFormatAttr(verticalSpace);
    //Changing the style and copying it to target
    source->ChgPageDesc("SourceStyle", *sPageDesc);
    target->CopyPageDesc(*sPageDesc, *tPageDesc);
    //Checking the set values on all Formats in target
    SwFrameFormat& rTargetMasterFormat = tPageDesc->GetMaster();
    SwFrameFormat& rTargetLeftFormat = tPageDesc->GetLeft();
    SwFrameFormat& rTargetFirstMasterFormat = tPageDesc->GetFirstMaster();
    SwFrameFormat& rTargetFirstLeftFormat = tPageDesc->GetFirstLeft();
    //Checking horizontal spaces
    const SvxLRSpaceItem MasterLRSpace = rTargetMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), MasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), MasterLRSpace.GetRight());
    const SvxLRSpaceItem LeftLRSpace = rTargetLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), LeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), LeftLRSpace.GetRight());
    const SvxLRSpaceItem FirstMasterLRSpace = rTargetFirstMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstMasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstMasterLRSpace.GetRight());
    const SvxLRSpaceItem FirstLeftLRSpace = rTargetFirstLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstLeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstLeftLRSpace.GetRight());
    //Checking vertical spaces
    const SvxULSpaceItem MasterULSpace = rTargetMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), MasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), MasterULSpace.GetLower());
    const SvxULSpaceItem LeftULSpace = rTargetLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), LeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), LeftULSpace.GetLower());
    const SvxULSpaceItem FirstMasterULSpace = rTargetFirstMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstMasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstMasterULSpace.GetLower());
    const SvxULSpaceItem FirstLeftULSpace = rTargetFirstLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstLeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstLeftULSpace.GetLower());
    xSourceDoc->dispose();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf69282WithMirror)
{
    createSwDoc();
    SwDoc* source = getSwDoc();
    uno::Reference<lang::XComponent> xSourceDoc = mxComponent;
    mxComponent.clear();
    createSwDoc();
    SwDoc* target = getSwDoc();
    SwPageDesc* sPageDesc = source->MakePageDesc("SourceStyle");
    SwPageDesc* tPageDesc = target->MakePageDesc("TargetStyle");
    //Enabling Mirror
    sPageDesc->SetUseOn(UseOnPage::Mirror);
    SwFrameFormat& rSourceMasterFormat = sPageDesc->GetMaster();
    //Setting horizontal spaces on master
    SvxLRSpaceItem horizontalSpace(RES_LR_SPACE);
    horizontalSpace.SetLeft(11);
    horizontalSpace.SetRight(12);
    rSourceMasterFormat.SetFormatAttr(horizontalSpace);
    //Setting vertical spaces on master
    SvxULSpaceItem verticalSpace(RES_UL_SPACE);
    verticalSpace.SetUpper(13);
    verticalSpace.SetLower(14);
    rSourceMasterFormat.SetFormatAttr(verticalSpace);
    //Changing the style and copying it to target
    source->ChgPageDesc("SourceStyle", *sPageDesc);
    target->CopyPageDesc(*sPageDesc, *tPageDesc);
    //Checking the set values on all Formats in target
    SwFrameFormat& rTargetMasterFormat = tPageDesc->GetMaster();
    SwFrameFormat& rTargetLeftFormat = tPageDesc->GetLeft();
    SwFrameFormat& rTargetFirstMasterFormat = tPageDesc->GetFirstMaster();
    SwFrameFormat& rTargetFirstLeftFormat = tPageDesc->GetFirstLeft();
    //Checking horizontal spaces
    const SvxLRSpaceItem MasterLRSpace = rTargetMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), MasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), MasterLRSpace.GetRight());
    //mirror effect should be present
    const SvxLRSpaceItem LeftLRSpace = rTargetLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), LeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), LeftLRSpace.GetRight());
    const SvxLRSpaceItem FirstMasterLRSpace = rTargetFirstMasterFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstMasterLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstMasterLRSpace.GetRight());
    //mirror effect should be present
    const SvxLRSpaceItem FirstLeftLRSpace = rTargetFirstLeftFormat.GetLRSpace();
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetRight(), FirstLeftLRSpace.GetLeft());
    CPPUNIT_ASSERT_EQUAL(horizontalSpace.GetLeft(), FirstLeftLRSpace.GetRight());
    //Checking vertical spaces
    const SvxULSpaceItem MasterULSpace = rTargetMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), MasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), MasterULSpace.GetLower());
    const SvxULSpaceItem LeftULSpace = rTargetLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), LeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), LeftULSpace.GetLower());
    const SvxULSpaceItem FirstMasterULSpace = rTargetFirstMasterFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstMasterULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstMasterULSpace.GetLower());
    const SvxULSpaceItem FirstLeftULSpace = rTargetFirstLeftFormat.GetULSpace();
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetUpper(), FirstLeftULSpace.GetUpper());
    CPPUNIT_ASSERT_EQUAL(verticalSpace.GetLower(), FirstLeftULSpace.GetLower());
    xSourceDoc->dispose();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf78742)
{
    //testing with service type and any .ods file
    OUString path = createFileURL(u"calc-data-source.ods");
    SfxMedium aMedium(path, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher("com.sun.star.text.TextDocument");
    std::shared_ptr<const SfxFilter> pFilter;
    ErrCode filter = aMatcher.DetectFilter(aMedium, pFilter);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_IO_ABORT, filter);
    //it should not return any Filter
    CPPUNIT_ASSERT(!pFilter);
    //testing without service type and any .ods file
    SfxMedium aMedium2(path, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher2;
    std::shared_ptr<const SfxFilter> pFilter2;
    ErrCode filter2 = aMatcher2.DetectFilter(aMedium2, pFilter2);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, filter2);
    //Filter should be returned with proper Name
    CPPUNIT_ASSERT_EQUAL(OUString("calc8"), pFilter2->GetFilterName());
    //testing with service type and any .odt file
    OUString path2 = createFileURL(u"fdo69893.odt");
    SfxMedium aMedium3(path2, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
    SfxFilterMatcher aMatcher3("com.sun.star.text.TextDocument");
    std::shared_ptr<const SfxFilter> pFilter3;
    ErrCode filter3 = aMatcher3.DetectFilter(aMedium3, pFilter3);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, filter3);
    //Filter should be returned with proper Name
    CPPUNIT_ASSERT_EQUAL(OUString("writer8"), pFilter3->GetFilterName());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testUnoParagraph)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting some text content in the document
    pWrtShell->Insert("This is initial text in paragraph one");
    pWrtShell->SplitNode();
    //Inserting second paragraph
    pWrtShell->Insert("This is initial text in paragraph two");
    //now testing the SwXParagraph
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xTextDocument->getText());
    uno::Reference<container::XEnumerationAccess> xParaAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPara(xParaAccess->createEnumeration());
    //getting first paragraph
    uno::Reference<text::XTextContent> xFirstParaContent(xPara->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFirstPara(xFirstParaContent, uno::UNO_QUERY);
    //testing the initial text
    CPPUNIT_ASSERT_EQUAL(OUString("This is initial text in paragraph one"),
                         xFirstPara->getString());
    //changing the text content in first paragraph
    xFirstPara->setString("This is modified text in paragraph one");
    //testing the changes
    CPPUNIT_ASSERT_EQUAL(OUString("This is modified text in paragraph one"),
                         xFirstPara->getString());
    //getting second paragraph
    uno::Reference<text::XTextContent> xSecondParaContent(xPara->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xSecondPara(xSecondParaContent, uno::UNO_QUERY);
    //testing the initial text
    CPPUNIT_ASSERT_EQUAL(OUString("This is initial text in paragraph two"),
                         xSecondPara->getString());
    //changing the text content in second paragraph
    xSecondPara->setString("This is modified text in paragraph two");
    //testing the changes
    CPPUNIT_ASSERT_EQUAL(OUString("This is modified text in paragraph two"),
                         xSecondPara->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf72788)
{
    //Create a new empty Writer document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    //Insert some text - two paragraphs
    rIDCO.InsertString(*pCursor, "this is text");
    //Position of word              9876543210
    //Position of word            0123456789
    //Change Paragraph
    pWrtShell->SplitNode();
    //Insert second paragraph
    rIDCO.InsertString(*pCursor, "more text");
    //Position of word            012345678
    //Make the following selection *bold*
    //this[is text
    //more] text
    //Move cursor back
    for (int i = 0; i < 5; i++)
    {
        pCursor->Move(fnMoveBackward);
    }
    //Start selection
    pCursor->SetMark();
    for (int i = 0; i < 12; i++)
    {
        pCursor->Move(fnMoveBackward);
    }
    //Check the text selection
    CPPUNIT_ASSERT_EQUAL(OUString(u"is text" + OUStringChar(CH_TXTATR_NEWLINE) + u"more"),
                         pCursor->GetText());
    //Apply a *Bold* attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    SfxItemSet aSet(pDoc->GetAttrPool(), svl::Items<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT>);
    //Add selected text's attributes to aSet
    pCursor->GetPointNode().GetTextNode()->GetParaAttr(aSet, 5, 12);
    SfxPoolItem const* pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    //Check that bold is active on the selection and it's in aSet
    CPPUNIT_ASSERT_EQUAL(true, (*pPoolItem == aWeightItem));
    //Make selection to remove formatting in first paragraph
    //[this is text
    //]more text
    pWrtShell->StartOfSection();
    //Start selection
    pCursor->SetMark();
    for (int i = 0; i < 13; i++)
    {
        pCursor->Move(fnMoveForward);
    }
    //Clear all the Direct Formatting ( Ctrl + M )
    SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
    sal_Int32 nEnd = pTextNode->Len();
    pTextNode->RstTextAttr(0, nEnd);
    //In case of Regression RstTextAttr() call will result to infinite recursion
    //Check that bold is removed in first paragraph
    aSet.ClearItem();
    pTextNode->GetParaAttr(aSet, 5, 12);
    SfxPoolItem const* pPoolItem2 = aSet.GetItem(RES_CHRATR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(true, (*pPoolItem2 != aWeightItem));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf60967)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", true, true, true);
    //Inserting table
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 2, 2);
    //getting the cursor's position just after the table insert
    SwPosition aPosAfterTable(*(pCursor->GetPoint()));
    //moving cursor to B2 (bottom right cell)
    pCursor->Move(fnMoveBackward);
    SwPosition aPosInTable(*(pCursor->GetPoint()));
    //deleting paragraph following table with Ctrl+Shift+Del
    bool val = pWrtShell->DelToEndOfSentence();
    CPPUNIT_ASSERT_EQUAL(true, val);
    //getting the cursor's position just after the paragraph deletion
    SwPosition aPosAfterDel(*(pCursor->GetPoint()));
    //moving cursor forward to check whether there is any node following the table, BTW there should not be any such node
    pCursor->Move(fnMoveForward);
    SwPosition aPosMoveAfterDel(*(pCursor->GetPoint()));
    //checking the positions to verify that the paragraph is actually deleted
    CPPUNIT_ASSERT_EQUAL(aPosAfterDel, aPosInTable);
    CPPUNIT_ASSERT_EQUAL(aPosMoveAfterDel, aPosInTable);
    //Undo the changes
    rUndoManager.Undo();
    {
        //paragraph *text node* should be back
        SwPosition aPosAfterUndo(*(pCursor->GetPoint()));
        //after undo aPosAfterTable increases the node position by one, since this contains the position *text node* so aPosAfterUndo should be less than aPosAfterTable
        CPPUNIT_ASSERT(aPosAfterTable > aPosAfterUndo);
        //moving cursor forward to check whether there is any node following the paragraph, BTW there should not be any such node as paragraph node is the last one in header
        pCursor->Move(fnMoveForward);
        SwPosition aPosMoveAfterUndo(*(pCursor->GetPoint()));
        //checking positions to verify that paragraph node is the last one and we are paragraph node only
        CPPUNIT_ASSERT(aPosAfterTable > aPosMoveAfterUndo);
        CPPUNIT_ASSERT_EQUAL(aPosAfterUndo, aPosMoveAfterUndo);
    }
    //Redo the changes
    rUndoManager.Redo();
    //paragraph *text node* should not be there
    SwPosition aPosAfterRedo(*(pCursor->GetPoint()));
    //position should be exactly same as it was after deletion of *text node*
    CPPUNIT_ASSERT_EQUAL(aPosAfterRedo, aPosMoveAfterDel);
    //moving the cursor forward, but it should not actually move as there is no *text node* after the table due to this same position is expected after move as it was before move
    pCursor->Move(fnMoveForward);
    SwPosition aPosAfterUndoMove(*(pCursor->GetPoint()));
    CPPUNIT_ASSERT_EQUAL(aPosAfterRedo, aPosAfterUndoMove);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testSearchWithTransliterate)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    {
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        pDoc->getIDocumentContentOperations().InsertString(aPaM, "This is paragraph one");
        pWrtShell->SplitNode();
    }
    {
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        pDoc->getIDocumentContentOperations().InsertString(aPaM, "This is Other PARAGRAPH");
    }
    i18nutil::SearchOptions2 SearchOpt;
    SearchOpt.searchFlag = css::util::SearchFlags::ALL_IGNORE_CASE;
    SearchOpt.searchString = "other";
    SearchOpt.replaceString.clear();
    SearchOpt.changedChars = 0;
    SearchOpt.deletedChars = 0;
    SearchOpt.insertedChars = 0;
    SearchOpt.transliterateFlags = TransliterationFlags::IGNORE_DIACRITICS_CTL;
    SearchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;
    SearchOpt.WildcardEscapeCharacter = 0;
    //transliteration option set so that at least one of the search strings is not found
    sal_Int32 case1
        = pWrtShell->SearchPattern(SearchOpt, true, SwDocPositions::Start, SwDocPositions::End);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(true);
    CPPUNIT_ASSERT_EQUAL(OUString(), pShellCursor->GetText());
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(case1));
    SearchOpt.searchString = "paragraph";
    SearchOpt.transliterateFlags = TransliterationFlags::IGNORE_KASHIDA_CTL;
    //transliteration option set so that all search strings are found
    sal_Int32 case2
        = pWrtShell->SearchPattern(SearchOpt, true, SwDocPositions::Start, SwDocPositions::End);
    pShellCursor = pWrtShell->getShellCursor(true);
    CPPUNIT_ASSERT_EQUAL(OUString("paragraph"), pShellCursor->GetText());
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(case2));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf73660)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString aData1 = "First" + OUStringChar(CHAR_SOFTHYPHEN) + "Word";
    OUString aData2 = "Seco" + OUStringChar(CHAR_SOFTHYPHEN) + "nd";
    OUString aData3 = OUStringChar(CHAR_SOFTHYPHEN) + "Third";
    OUString aData4 = "Fourth" + OUStringChar(CHAR_SOFTHYPHEN);
    pWrtShell->Insert("We are inserting some text in the document to check the search feature ");
    pWrtShell->Insert(aData1 + " ");
    pWrtShell->Insert(aData2 + " ");
    pWrtShell->Insert(aData3 + " ");
    pWrtShell->Insert(aData4 + " ");
    pWrtShell->Insert("Fifth ");
    pWrtShell->Insert("Now we have enough text let's test search for all the cases");
    //searching for all 5 strings entered with soft-hyphen, search string contains no soft-hyphen
    i18nutil::SearchOptions2 searchOpt;
    searchOpt.searchFlag = css::util::SearchFlags::NORM_WORD_ONLY;
    searchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::REGEXP;
    //case 1
    searchOpt.searchString = "First";
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(1),
        pWrtShell->SearchPattern(searchOpt, true, SwDocPositions::Start, SwDocPositions::End));
    //case 2
    searchOpt.searchString = "Second";
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(1),
        pWrtShell->SearchPattern(searchOpt, true, SwDocPositions::Start, SwDocPositions::End));
    //case 3
    searchOpt.searchString = "Third";
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(1),
        pWrtShell->SearchPattern(searchOpt, true, SwDocPositions::Start, SwDocPositions::End));
    //case 4
    searchOpt.searchString = "Fourth";
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(1),
        pWrtShell->SearchPattern(searchOpt, true, SwDocPositions::Start, SwDocPositions::End));
    //case 5
    searchOpt.searchString = "Fifth";
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(1),
        pWrtShell->SearchPattern(searchOpt, true, SwDocPositions::Start, SwDocPositions::End));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testNewDocModifiedState)
{
    //creating a new doc
    SwDoc* pDoc = new SwDoc();
    //getting the state of the document via IDocumentState
    IDocumentState& rState(pDoc->getIDocumentState());
    //the state should not be modified, no modifications yet
    CPPUNIT_ASSERT(!(rState.IsModified()));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf77342)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    //inserting first footnote
    pWrtShell->InsertFootnote("");
    SwFieldType* pField = pWrtShell->GetFieldType(0, SwFieldIds::GetRef);
    SwGetRefFieldType* pRefType = static_cast<SwGetRefFieldType*>(pField);
    //moving cursor to the starting of document
    pWrtShell->StartOfSection();
    //inserting reference field 1
    SwGetRefField aField1(pRefType, "", "", REF_FOOTNOTE, sal_uInt16(0), sal_uInt16(0),
                          REF_CONTENT);
    pWrtShell->InsertField2(aField1);
    //inserting second footnote
    pWrtShell->InsertFootnote("");
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    //inserting reference field 2
    SwGetRefField aField2(pRefType, "", "", REF_FOOTNOTE, sal_uInt16(1), sal_uInt16(0),
                          REF_CONTENT);
    pWrtShell->InsertField2(aField2);
    //inserting third footnote
    pWrtShell->InsertFootnote("");
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting reference field 3
    SwGetRefField aField3(pRefType, "", "", REF_FOOTNOTE, sal_uInt16(2), sal_uInt16(0),
                          REF_CONTENT);
    pWrtShell->InsertField2(aField3);
    //updating the fields
    IDocumentFieldsAccess& rField(pDoc->getIDocumentFieldsAccess());
    rField.UpdateExpFields(nullptr, true);
    //creating new clipboard doc
    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);
    xClpDoc->getIDocumentFieldsAccess().LockExpFields();
    //selecting reference field 2 and reference field 3 and footnote 1 and footnote 2
    //selection is such that more than one and not all footnotes and ref fields are selected
    pCursor->Move(fnMoveBackward);
    pCursor->Move(fnMoveBackward);
    //start marking
    pCursor->SetMark();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //copying the selection to clipboard
    pWrtShell->Copy(*xClpDoc);
    //deleting selection mark after copy
    pCursor->DeleteMark();
    //checking that the footnotes reference fields have same values after copy operation
    uno::Any aAny;
    sal_uInt16 aFormat;
    //reference field 1
    pWrtShell->StartOfSection();
    SwField* pRef1 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef1->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef1->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(0)), aAny);
    //reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pRef2 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef2->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef2->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pRef3 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef3->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef3->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //moving cursor to the end of the document
    pWrtShell->EndOfSection();
    //pasting the copied selection at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
    //old reference field 1
    SwField* pOldRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef13 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef13->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef13->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //old footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd1 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote1
        = pTextNd1->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote1(pFootnote1->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnote1.GetNumber());
    SwTextFootnote* pTFNote1 = static_cast<SwTextFootnote*>(pFootnote1);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pTFNote1->GetSeqRefNo());
    //old footnote 2
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd2 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote2
        = pTextNd2->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote2(pFootnote2->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnote2.GetNumber());
    SwTextFootnote* pTFNote2 = static_cast<SwTextFootnote*>(pFootnote2);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pTFNote2->GetSeqRefNo());
    //old footnote 3
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd3 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote3
        = pTextNd3->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote3(pFootnote3->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), rFootnote3.GetNumber());
    SwTextFootnote* pTFNote3 = static_cast<SwTextFootnote*>(pFootnote3);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pTFNote3->GetSeqRefNo());
    //new reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pNewRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(3)), aAny);
    //new footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd4 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote4
        = pTextNd4->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote4(pFootnote4->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), rFootnote4.GetNumber());
    SwTextFootnote* pTFNote4 = static_cast<SwTextFootnote*>(pFootnote4);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pTFNote4->GetSeqRefNo());
    //moving the cursor to the starting of document
    pWrtShell->StartOfSection();
    //pasting the selection again at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
    //new reference field 1
    SwField* pNewRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(4)), aAny);
    //new footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd11 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote11
        = pTextNd11->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote11(pFootnote11->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnote11.GetNumber());
    SwTextFootnote* pTFNote11 = static_cast<SwTextFootnote*>(pFootnote11);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pTFNote11->GetSeqRefNo());
    //old reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pOldRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef23 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef23->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef23->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //old footnote 1
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd12 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote12
        = pTextNd12->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote12(pFootnote12->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnote12.GetNumber());
    SwTextFootnote* pTFNote12 = static_cast<SwTextFootnote*>(pFootnote12);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pTFNote12->GetSeqRefNo());
    //old footnote 2
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd13 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote13
        = pTextNd13->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote13(pFootnote13->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), rFootnote13.GetNumber());
    SwTextFootnote* pTFNote13 = static_cast<SwTextFootnote*>(pFootnote13);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pTFNote13->GetSeqRefNo());
    //old footnote 3
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd14 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote14
        = pTextNd14->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote14(pFootnote14->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), rFootnote14.GetNumber());
    SwTextFootnote* pTFNote14 = static_cast<SwTextFootnote*>(pFootnote14);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pTFNote14->GetSeqRefNo());
    //old reference field 4
    pCursor->Move(fnMoveForward);
    SwField* pOldRef24 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef24->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef24->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //old reference field 5
    pCursor->Move(fnMoveForward);
    SwField* pOldRef25 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef25->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef25->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(3)), aAny);
    //old footnote 4
    pCursor->Move(fnMoveForward);
    SwTextNode* pTextNd15 = pCursor->GetPointNode().GetTextNode();
    SwTextAttr* const pFootnote15
        = pTextNd15->GetTextAttrForCharAt(pCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FTN);
    const SwFormatFootnote& rFootnote15(pFootnote15->GetFootnote());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), rFootnote15.GetNumber());
    SwTextFootnote* pTFNote15 = static_cast<SwTextFootnote*>(pFootnote15);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pTFNote15->GetSeqRefNo());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf63553)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    //inserting sequence field 1
    SwSetExpFieldType* pSeqType = static_cast<SwSetExpFieldType*>(
        pWrtShell->GetFieldType(SwFieldIds::SetExp, "Illustration"));
    SwSetExpField aSetField1(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->InsertField2(aSetField1);
    SwGetRefFieldType* pRefType
        = static_cast<SwGetRefFieldType*>(pWrtShell->GetFieldType(0, SwFieldIds::GetRef));
    //moving cursor to the starting of document
    pWrtShell->StartOfSection();
    //inserting reference field 1
    SwGetRefField aGetField1(pRefType, "Illustration", "", REF_SEQUENCEFLD, sal_uInt16(0),
                             sal_uInt16(0), REF_CONTENT);
    pWrtShell->InsertField2(aGetField1);
    //now we have ref1-seq1
    //moving the cursor
    pCursor->Move(fnMoveForward);
    //inserting sequence field 2
    SwSetExpField aSetField2(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->InsertField2(aSetField2);
    //moving the cursor
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    //inserting reference field 2
    SwGetRefField aGetField2(pRefType, "Illustration", "", REF_SEQUENCEFLD, sal_uInt16(1),
                             sal_uInt16(0), REF_CONTENT);
    pWrtShell->InsertField2(aGetField2);
    //now we have ref1-ref2-seq1-seq2
    //moving the cursor
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting sequence field 3
    SwSetExpField aSetField3(pSeqType, "", SVX_NUM_ARABIC);
    pWrtShell->InsertField2(aSetField3);
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //inserting reference field 3
    SwGetRefField aGetField3(pRefType, "Illustration", "", REF_SEQUENCEFLD, sal_uInt16(2),
                             sal_uInt16(0), REF_CONTENT);
    pWrtShell->InsertField2(aGetField3);
    //now after insertion we have ref1-ref2-ref3-seq1-seq2-seq3
    //updating the fields
    IDocumentFieldsAccess& rField(pDoc->getIDocumentFieldsAccess());
    rField.UpdateExpFields(nullptr, true);
    //creating new clipboard doc
    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);
    xClpDoc->getIDocumentFieldsAccess().LockExpFields();
    //selecting reference field 2 and 3 and sequence field 1 and 2
    //selection is such that more than one and not all sequence fields and reference fields are selected
    //ref1-[ref2-ref3-seq1-seq2]-seq3
    pWrtShell->StartOfSection();
    pCursor->Move(fnMoveForward);
    //start marking
    pCursor->SetMark();
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    pCursor->Move(fnMoveForward);
    //copying the selection to clipboard
    pWrtShell->Copy(*xClpDoc);
    //deleting selection mark after copy
    pCursor->DeleteMark();
    //checking whether the sequence and reference fields have same values after copy operation
    uno::Any aAny;
    sal_uInt16 aFormat;
    //reference field 1
    pWrtShell->StartOfSection();
    SwField* pRef1 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef1->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef1->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(0)), aAny);
    //reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pRef2 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef2->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef2->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pRef3 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pRef3->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pRef3->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeqF1
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pSeqF1->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeqF1->GetFieldName());
    //sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeqF2
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pSeqF2->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeqF2->GetFieldName());
    //sequence field 3
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeqF3
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pSeqF3->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeqF3->GetFieldName());
    //moving cursor to the end of the document
    pWrtShell->EndOfSection();
    //pasting the copied selection at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
    //now we have ref1-ref2-ref3-seq1-seq2-seq3-nref1-nref2-nseq1-nseq2
    //old reference field 1
    SwField* pOldRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef13 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef13->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef13->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //old sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeq1
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pSeq1->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeq1->GetFieldName());
    //old sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeq2
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pSeq2->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeq2->GetFieldName());
    //old sequence field 3
    pCursor->Move(fnMoveForward);
    SwSetExpField* pSeq3
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pSeq3->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pSeq3->GetFieldName());
    //new reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pNewRef11 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef11->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef11->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(4)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef12 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef12->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef12->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //new sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq1
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pNewSeq1->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq1->GetFieldName());
    //new sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq2
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pNewSeq2->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq2->GetFieldName());
    //moving the cursor to the starting of document
    pWrtShell->StartOfSection();
    //pasting the selection again at current cursor position
    pWrtShell->Paste(*xClpDoc);
    //checking the fields, both new and old, for proper values
    pWrtShell->StartOfSection();
    //now we have [nnref1-nnref2-nnseq1-nnseq2]-ref1-[ref2-ref3-seq1-seq2]-seq3-[nref1-nref2-nseq1-nseq2]
    //new reference field 1
    SwField* pNewRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(6)), aAny);
    //new reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pNewRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pNewRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pNewRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //new sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq11
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), pNewSeq11->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq11->GetFieldName());
    //new sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pNewSeq12
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), pNewSeq12->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pNewSeq12->GetFieldName());
    //old reference field 1
    pCursor->Move(fnMoveForward);
    SwField* pOldRef21 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef21->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef21->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(0)), aAny);
    //old reference field 2
    pCursor->Move(fnMoveForward);
    SwField* pOldRef22 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef22->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef22->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(1)), aAny);
    //old reference field 3
    pCursor->Move(fnMoveForward);
    SwField* pOldRef23 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef23->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef23->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //old sequence field 1
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq11
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pOldSeq11->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq11->GetFieldName());
    //old sequence field 2
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq12
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pOldSeq12->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq12->GetFieldName());
    //old sequence field 3
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq13
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pOldSeq13->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq13->GetFieldName());
    //old reference field 4
    pCursor->Move(fnMoveForward);
    SwField* pOldRef24 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef24->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef24->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(4)), aAny);
    //old reference field 5
    pCursor->Move(fnMoveForward);
    SwField* pOldRef25 = SwCursorShell::GetFieldAtCursor(pCursor, true);
    aFormat = pOldRef25->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_CONTENT), aFormat);
    pOldRef25->QueryValue(aAny, sal_uInt16(FIELD_PROP_SHORT1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_uInt16(2)), aAny);
    //old sequence field 4
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq14
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pOldSeq14->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq14->GetFieldName());
    //old sequence field 5
    pCursor->Move(fnMoveForward);
    SwSetExpField* pOldSeq15
        = static_cast<SwSetExpField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pOldSeq15->GetSeqNumber());
    CPPUNIT_ASSERT_EQUAL(OUString("Number range Illustration"), pOldSeq15->GetFieldName());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf74230)
{
    createSwDoc();
    //exporting the empty document to ODT via TempFile
    save("writer8");
    CPPUNIT_ASSERT(maTempFile.IsValid());
    //loading an XML DOM of the "styles.xml" of the TempFile
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    //pXmlDoc should not be null
    CPPUNIT_ASSERT(pXmlDoc);
    //asserting XPath in loaded XML DOM
    assertXPath(pXmlDoc, "//office:styles/style:default-style[@style:family='graphic']/"
                         "style:graphic-properties[@svg:stroke-color='#3465a4']"_ostr);
    assertXPath(pXmlDoc, "//office:styles/style:default-style[@style:family='graphic']/"
                         "style:graphic-properties[@draw:fill-color='#729fcf']"_ostr);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf80663)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting 2x2 Table
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 2, 2);
    //Checking for the number of rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the first row
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
    pWrtShell->SelTableRow(); //selects the first row
    pWrtShell->DeleteRow();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the second row
    pWrtShell->GoNextCell(); //moves the cursor to next cell
    pWrtShell->SelTableRow(); //selects the second row
    pWrtShell->DeleteRow();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the first column
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
    pWrtShell->SelTableCol(); //selects first column
    pWrtShell->DeleteCol();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Deleting the second column
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
    pWrtShell->GoNextCell(); //moves the cursor to next cell
    pWrtShell->SelTableCol(); //selects second column
    pWrtShell->DeleteCol();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf57197)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //Inserting 1x1 Table
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 1, 1);
    //Checking for the number of rows and columns
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one row before the existing row
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
    pWrtShell->InsertRow(1, false);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one row after the existing row
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
    pWrtShell->InsertRow(1, true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one column before the existing column
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
    pWrtShell->InsertCol(1, false);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Inserting one column after the existing column
    pWrtShell->StartOfSection(); //moves the cursor to the start of Doc
    pWrtShell->InsertCol(1, true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    //Redo changes
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    //Undo changes
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf131990)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT(!pWrtShell->Up(false, 1, true));
    CPPUNIT_ASSERT(!pWrtShell->Down(false, 1, true));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf90808)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xTextDocument->getText();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XParagraphCursor> xCursor(xText->createTextCursor(), uno::UNO_QUERY);
    //inserting text into document so that the paragraph is not empty
    xText->setString("Hello World!");
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    //creating bookmark 1
    uno::Reference<text::XTextContent> xHeadingBookmark1(
        xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xHeadingName1(xHeadingBookmark1, uno::UNO_QUERY);
    xHeadingName1->setName("__RefHeading__1");
    //moving cursor to the starting of paragraph
    xCursor->gotoStartOfParagraph(false);
    //inserting the bookmark in paragraph
    xText->insertTextContent(xCursor, xHeadingBookmark1, true);
    //creating bookmark 2
    uno::Reference<text::XTextContent> xHeadingBookmark2(
        xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xHeadingName2(xHeadingBookmark2, uno::UNO_QUERY);
    xHeadingName2->setName("__RefHeading__2");
    //inserting the bookmark in same paragraph, at the end
    //only one bookmark of this type is allowed in each paragraph an exception of com.sun.star.lang.IllegalArgumentException must be thrown when inserting the other bookmark in same paragraph
    xCursor->gotoEndOfParagraph(true);
    CPPUNIT_ASSERT_THROW(xText->insertTextContent(xCursor, xHeadingBookmark2, true),
                         css::lang::IllegalArgumentException);
    //now testing for __RefNumPara__
    //creating bookmark 1
    uno::Reference<text::XTextContent> xNumBookmark1(
        xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNumName1(xNumBookmark1, uno::UNO_QUERY);
    xNumName1->setName("__RefNumPara__1");
    //moving cursor to the starting of paragraph
    xCursor->gotoStartOfParagraph(false);
    //inserting the bookmark in paragraph
    xText->insertTextContent(xCursor, xNumBookmark1, true);
    //creating bookmark 2
    uno::Reference<text::XTextContent> xNumBookmark2(
        xFact->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNumName2(xNumBookmark2, uno::UNO_QUERY);
    xNumName2->setName("__RefNumPara__2");
    //inserting the bookmark in same paragraph, at the end
    //only one bookmark of this type is allowed in each paragraph an exception of com.sun.star.lang.IllegalArgumentException must be thrown when inserting the other bookmark in same paragraph
    xCursor->gotoEndOfParagraph(true);
    CPPUNIT_ASSERT_THROW(xText->insertTextContent(xCursor, xNumBookmark2, true),
                         css::lang::IllegalArgumentException);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf97601)
{
    // Instructions from the bugreport to trigger an infinite loop.
    createSwDoc("tdf97601.odt");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xEmbeddedObjectsSupplier(mxComponent,
                                                                                uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEmbeddedObjects
        = xEmbeddedObjectsSupplier->getEmbeddedObjects();
    uno::Reference<beans::XPropertySet> xChart;
    xEmbeddedObjects->getByName("myChart") >>= xChart;
    uno::Reference<chart2::data::XDataSource> xChartComponent;
    xChart->getPropertyValue("Component") >>= xChartComponent;
    uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>> aDataSequences
        = xChartComponent->getDataSequences();
    uno::Reference<document::XEmbeddedObjectSupplier2> xChartState(xChart, uno::UNO_QUERY);
    xChartState->getExtendedControlOverEmbeddedObject()->changeState(1);
    uno::Reference<util::XModifiable> xDataSequenceModifiable(aDataSequences[2]->getValues(),
                                                              uno::UNO_QUERY);
    xDataSequenceModifiable->setModified(true);

    // Make sure that the chart is marked as modified.
    uno::Reference<util::XModifiable> xModifiable(xChartComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xModifiable->isModified()));
    calcLayout();
    // This never returned.
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf75137)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(true);
    pWrtShell->InsertFootnote("This is first footnote");
    SwNodeOffset firstIndex = pShellCursor->GetPointNode().GetIndex();
    pShellCursor->GotoFootnoteAnchor();
    pWrtShell->InsertFootnote("This is second footnote");
    pWrtShell->Up(false);
    SwNodeOffset secondIndex = pShellCursor->GetPointNode().GetIndex();
    pWrtShell->Down(false);
    SwNodeOffset thirdIndex = pShellCursor->GetPointNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(firstIndex, thirdIndex);
    CPPUNIT_ASSERT(firstIndex != secondIndex);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf83798)
{
    createSwDoc("tdf83798.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->Move(fnMoveBackward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("Table of Contents"), pCursor->GetText());
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->DeleteMark();
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pCursor->GetText());
    pCursor->DeleteMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("1.A"), pCursor->GetText());
    pCursor->DeleteMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pCursor->GetText());
    pCursor->DeleteMark();
    pCursor->Move(fnMoveForward, GoInNode);
    pCursor->SetMark();
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    pCursor->Move(fnMoveForward, GoInContent);
    CPPUNIT_ASSERT_EQUAL(OUString("2.A"), pCursor->GetText());
    pCursor->DeleteMark();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf89714)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<uno::XInterface> xInterface(xFact->createInstance("com.sun.star.text.Defaults"),
                                               uno::UNO_QUERY);
    uno::Reference<beans::XPropertyState> xPropState(xInterface, uno::UNO_QUERY);
    //enabled Paragraph Orphan and Widows by default starting in LO5.1
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int8(2)), xPropState->getPropertyDefault("ParaOrphans"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int8(2)), xPropState->getPropertyDefault("ParaWidows"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf130287)
{
    //create a new writer document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    //insert a 1-cell table in the newly created document
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 1, 1);
    //checking for the row and column
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    uno::Reference<text::XText> xCellText(xCell, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xCellText);
    // they were 2 (orphan/widow control enabled unnecessarily in Table Contents paragraph style)
    CPPUNIT_ASSERT_EQUAL(sal_Int8(0), getProperty<sal_Int8>(xParagraph, "ParaOrphans"));
    CPPUNIT_ASSERT_EQUAL(sal_Int8(0), getProperty<sal_Int8>(xParagraph, "ParaWidows"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testPropertyDefaults)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<uno::XInterface> xInterface(xFact->createInstance("com.sun.star.text.Defaults"),
                                               uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertyState> xPropState(xInterface, uno::UNO_QUERY);
    //testing CharFontName from style::CharacterProperties
    //getting property default
    uno::Any aCharFontName = xPropState->getPropertyDefault("CharFontName");
    //asserting property default and defaults received from "css.text.Defaults" service
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharFontName"), aCharFontName);
    //changing the default value
    xPropSet->setPropertyValue("CharFontName", uno::Any(OUString("Symbol")));
    CPPUNIT_ASSERT_EQUAL(uno::Any(OUString("Symbol")), xPropSet->getPropertyValue("CharFontName"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharFontName");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharFontName"), aCharFontName);
    //testing CharHeight from style::CharacterProperties
    //getting property default
    uno::Any aCharHeight = xPropState->getPropertyDefault("CharHeight");
    //asserting property default and defaults received from "css.text.Defaults" service
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharHeight"), aCharHeight);
    //changing the default value
    xPropSet->setPropertyValue("CharHeight", uno::Any(float(14)));
    CPPUNIT_ASSERT_EQUAL(uno::Any(float(14)), xPropSet->getPropertyValue("CharHeight"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharHeight");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharHeight"), aCharHeight);
    //testing CharWeight from style::CharacterProperties
    uno::Any aCharWeight = xPropSet->getPropertyValue("CharWeight");
    //changing the default value
    xPropSet->setPropertyValue("CharWeight", uno::Any(float(awt::FontWeight::BOLD)));
    CPPUNIT_ASSERT_EQUAL(uno::Any(float(awt::FontWeight::BOLD)),
                         xPropSet->getPropertyValue("CharWeight"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharWeight");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharWeight"), aCharWeight);
    //testing CharUnderline from style::CharacterProperties
    uno::Any aCharUnderline = xPropSet->getPropertyValue("CharUnderline");
    //changing the default value
    xPropSet->setPropertyValue("CharUnderline", uno::Any(sal_Int16(awt::FontUnderline::SINGLE)));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(awt::FontUnderline::SINGLE)),
                         xPropSet->getPropertyValue("CharUnderline"));
    //resetting the value to default
    xPropState->setPropertyToDefault("CharUnderline");
    CPPUNIT_ASSERT_EQUAL(xPropSet->getPropertyValue("CharUnderline"), aCharUnderline);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTableBackgroundColor)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(TableOpt, 3, 3); //Inserting Table
    //Checking Rows and Columns of Inserted Table
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
    pWrtShell->StartOfSection();
    pWrtShell->SelTableRow(); //Selecting First Row
    pWrtShell->ClearMark();
    //Modifying the color of Table Box
    pWrtShell->SetBoxBackground(
        SvxBrushItem(Color(sal_Int32(0xFF00FF)), sal_Int16(RES_BACKGROUND)));
    //Checking cells for background color only A1 should be modified
    uno::Reference<table::XCell> xCell;
    xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(Color(0xFF00FF), getProperty<Color>(xCell, "BackColor"));
    xCell = xTable->getCellByName("A2");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("A3");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("B1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("B2");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("B3");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("C1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("C2");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell = xTable->getCellByName("C3");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xCell, "BackColor"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf88899)
{
    createSwDoc();
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xProps(
        xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Reference<beans::XPropertyContainer> xUserProps = xProps->getUserDefinedProperties();
    css::util::DateTime aDateTime
        = { sal_uInt32(1234567), sal_uInt16(3),  sal_uInt16(3),    sal_uInt16(3),
            sal_uInt16(10),      sal_uInt16(11), sal_uInt16(2014), true };
    xUserProps->addProperty("dateTime", sal_Int16(beans::PropertyAttribute::OPTIONAL),
                            uno::Any(aDateTime));
    uno::Reference<lang::XMultiServiceFactory> xFact(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextField> xTextField(
        xFact->createInstance("com.sun.star.text.textfield.docinfo.Custom"), uno::UNO_QUERY);
    //Setting Name Property
    uno::Reference<beans::XPropertySet> xPropSet(xTextField, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue("Name", uno::Any(OUString("dateTime")));
    //Setting NumberFormat
    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(mxComponent,
                                                                        uno::UNO_QUERY);
    uno::Reference<util::XNumberFormatTypes> xNumFormat(xNumberFormatsSupplier->getNumberFormats(),
                                                        uno::UNO_QUERY);
    css::lang::Locale alocale;
    alocale.Language = "en";
    alocale.Country = "US";
    sal_Int16 key = xNumFormat->getStandardFormat(util::NumberFormat::DATETIME, alocale);
    xPropSet->setPropertyValue("NumberFormat", uno::Any(key));
    //Inserting Text Content
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xTextDocument->getText();
    uno::Reference<text::XText> xText = xTextRange->getText();
    xText->insertTextContent(xTextRange, xTextField, true);
    //Retrieving the contents for verification
    CPPUNIT_ASSERT_EQUAL(OUString("11/10/14 03:03 AM"), xTextField->getPresentation(false));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf151605)
{
    createSwDoc("tdf151605.odt");

    // disable IncludeHiddenText
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::FilterFlags::ASCII::IncludeHiddenText::set(false, batch);
    officecfg::Office::Writer::Content::Display::ShowWarningHiddenSection::set(false, batch);
    batch->commit();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::STRING)) } });

    // Paste as Unformatted text
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Before"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("After"), getParagraph(2)->getString());

    // re-enable it
    officecfg::Office::Writer::FilterFlags::ASCII::IncludeHiddenText::set(true, batch);
    officecfg::Office::Writer::Content::Display::ShowWarningHiddenSection::set(true, batch);
    batch->commit();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf90362)
{
    createSwDoc("tdf90362.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Ensure correct initial setting
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Cursor::Option::IgnoreProtectedArea::set(false, batch);
    batch->commit();
    // First check if the end of the second paragraph is indeed protected.
    pWrtShell->EndPara();
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT_EQUAL(true, pWrtShell->HasReadonlySel());

    // Then enable ignoring of protected areas and make sure that this time the cursor is read-write.
    officecfg::Office::Writer::Cursor::Option::IgnoreProtectedArea::set(true, batch);
    batch->commit();
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->HasReadonlySel());
    // Clean up, otherwise following tests will have that option set
    officecfg::Office::Writer::Cursor::Option::IgnoreProtectedArea::set(false, batch);
    batch->commit();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testUndoDelAsCharTdf107512)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    sw::UndoManager& rUndoManager(pDoc->GetUndoManager());
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    SwCursorShell* pShell(pDoc->GetEditShell());
    SfxItemSet frameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SfxItemSet grfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>);
    rIDCO.InsertString(*pShell->GetCursor(), "foo");
    pShell->ClearMark();
    SwFormatAnchor anchor(RndStdIds::FLY_AS_CHAR);
    frameSet.Put(anchor);
    Graphic grf;
    pShell->SttEndDoc(true);
    CPPUNIT_ASSERT(rIDCO.InsertGraphic(*pShell->GetCursor(), OUString(), OUString(), &grf,
                                       &frameSet, &grfSet, nullptr));
    pShell->SttEndDoc(false);
    CPPUNIT_ASSERT(rIDCO.InsertGraphic(*pShell->GetCursor(), OUString(), OUString(), &grf,
                                       &frameSet, &grfSet, nullptr));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    SvxCharHiddenItem hidden(true, RES_CHRATR_HIDDEN);
    pShell->SelectTextModel(1, 4);
    rIDCO.InsertPoolItem(*pShell->GetCursor(), hidden);
    // now we have "\1foo\1" with the "foo" hidden
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(
        OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)),
        pShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    SfxPoolItem const* pItem;
    SfxItemSet query(pDoc->GetAttrPool(), svl::Items<RES_CHRATR_HIDDEN, RES_CHRATR_HIDDEN>);
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);

    // delete from the start
    pShell->SelectTextModel(0, 4);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(
        OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)),
        pShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(
        OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)),
        pShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 0, 1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);

    // delete from the end
    pShell->SelectTextModel(1, 5);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(
        OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)),
        pShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Redo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    rUndoManager.Undo();
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
        4, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    CPPUNIT_ASSERT_EQUAL(
        OUString(OUStringChar(CH_TXTATR_BREAKWORD) + u"foo" + OUStringChar(CH_TXTATR_BREAKWORD)),
        pShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 4, 5);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                         query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    query.ClearItem(RES_CHRATR_HIDDEN);
    pShell->GetCursor()->GetPointNode().GetTextNode()->GetParaAttr(query, 1, 4);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, query.GetItemState(RES_CHRATR_HIDDEN, false, &pItem));
    CPPUNIT_ASSERT(static_cast<SvxCharHiddenItem const*>(pItem)->GetValue());
    query.ClearItem(RES_CHRATR_HIDDEN);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testUndoCharAttribute)
{
    // Create a new empty Writer document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    // Insert some text
    rIDCO.InsertString(*pCursor, "This will be bolded");
    // Position of word                   9876543210
    // Use cursor to select part of text
    pCursor->SetMark();
    for (int i = 0; i < 9; i++)
    {
        pCursor->Move(fnMoveBackward);
    }
    // Check that correct text was selected
    CPPUNIT_ASSERT_EQUAL(OUString("be bolded"), pCursor->GetText());
    // Apply a "Bold" attribute to selection
    SvxWeightItem aWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    rIDCO.InsertPoolItem(*pCursor, aWeightItem);
    SfxItemSet aSet(pDoc->GetAttrPool(), svl::Items<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT>);
    // Adds selected text's attributes to aSet
    pCursor->GetPointNode().GetTextNode()->GetParaAttr(aSet, 10, 19);
    SfxPoolItem const* pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    // Check that bold is active on the selection; checks if it's in aSet
    CPPUNIT_ASSERT_EQUAL(true, (*pPoolItem == aWeightItem));
    // Invoke Undo
    rUndoManager.Undo();
    // Check that bold is no longer active
    aSet.ClearItem(RES_CHRATR_WEIGHT);
    pCursor->GetPointNode().GetTextNode()->GetParaAttr(aSet, 10, 19);
    pPoolItem = aSet.GetItem(RES_CHRATR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(false, (*pPoolItem == aWeightItem));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testUndoDelAsChar)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    sw::UndoManager& rUndoManager(pDoc->GetUndoManager());
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    SwCursorShell* pShell(pDoc->GetEditShell());
    SfxItemSet frameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SfxItemSet grfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>);
    SwFormatAnchor anchor(RndStdIds::FLY_AS_CHAR);
    frameSet.Put(anchor);
    Graphic grf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphic(*pShell->GetCursor(), OUString(), OUString(), &grf,
                                       &frameSet, &grfSet, nullptr));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    pShell->SetMark();
    pShell->Left(1, SwCursorSkipMode::Chars);
    rIDCO.DeleteAndJoin(*pShell->GetCursor());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(!pShell->GetCursor()->GetPointNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(!pShell->GetCursor()->GetPointNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT(pShell->GetCursor()->GetPointNode().GetTextNode()->HasHints());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pShell->GetCursor()->GetPointNode().GetTextNode()->Len());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf86639)
{
    createSwDoc("tdf86639.rtf");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextFormatColl* pColl = pDoc->FindTextFormatCollByName(u"Heading"_ustr);
    pWrtShell->SetTextFormatColl(pColl);
    OUString aExpected = pColl->GetAttrSet().GetFont().GetFamilyName();
    // This was Calibri, should be Liberation Sans.
    CPPUNIT_ASSERT_EQUAL(aExpected,
                         getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf90883TableBoxGetCoordinates)
{
    createSwDoc("tdf90883.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(true);
    SwSelBoxes aBoxes;
    ::GetTableSel(*pWrtShell, aBoxes);
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(aBoxes.size()));
    Point pos(aBoxes[0]->GetCoordinates());
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(pos.X()));
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(pos.Y()));
    pos = aBoxes[1]->GetCoordinates();
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(pos.X()));
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(pos.Y()));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testEmbeddedDataSource)
{
    // Initially no data source.
    uno::Reference<uno::XComponentContext> xComponentContext(
        comphelper::getProcessComponentContext());
    uno::Reference<sdb::XDatabaseContext> xDatabaseContext
        = sdb::DatabaseContext::create(xComponentContext);
    CPPUNIT_ASSERT(!xDatabaseContext->hasByName("calc-data-source"));

    // Load: should have a component and a data source, too.
    // Path with "#" must not cause issues
    createSwDoc("hash%23path/embedded-data-source.odt");
    CPPUNIT_ASSERT(xDatabaseContext->hasByName("calc-data-source"));

    // Data source has a table named Sheet1.
    uno::Reference<sdbc::XDataSource> xDataSource(xDatabaseContext->getByName("calc-data-source"),
                                                  uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDataSource.is());
    auto xConnection = xDataSource->getConnection("", "");
    uno::Reference<container::XNameAccess> xTables
        = css::uno::Reference<css::sdbcx::XTablesSupplier>(xConnection, uno::UNO_QUERY_THROW)
              ->getTables();
    CPPUNIT_ASSERT(xTables.is());
    CPPUNIT_ASSERT(xTables->hasByName("Sheet1"));
    xConnection->close();

    // Reload: should still have a component and a data source, too.
    saveAndReload("writer8");
    CPPUNIT_ASSERT(mxComponent.is());
    CPPUNIT_ASSERT(xDatabaseContext->hasByName("calc-data-source"));

    // Data source has a table named Sheet1 after saving to a different directory.
    xDataSource.set(xDatabaseContext->getByName("calc-data-source"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDataSource.is());
    xConnection = xDataSource->getConnection("", "");
    xTables = css::uno::Reference<css::sdbcx::XTablesSupplier>(xConnection, uno::UNO_QUERY_THROW)
                  ->getTables();
    CPPUNIT_ASSERT(xTables.is());
    CPPUNIT_ASSERT(xTables->hasByName("Sheet1"));
    xConnection->close();

    // Close: should not have a data source anymore.
    mxComponent->dispose();
    mxComponent.clear();
    CPPUNIT_ASSERT(!xDatabaseContext->hasByName("calc-data-source"));

    // Now open again the saved result, and instead of 'save as', just 'save'.
    mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->store();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testUnoCursorPointer)
{
    auto xDocComponent(
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));
    auto pxDocDocument(dynamic_cast<SwXTextDocument*>(xDocComponent.get()));
    CPPUNIT_ASSERT(pxDocDocument);
    SwDoc* const pDoc(pxDocDocument->GetDocShell()->GetDoc());
    std::unique_ptr<SwNodeIndex> pIdx(new SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1));
    std::unique_ptr<SwPosition> pPos(new SwPosition(*pIdx));
    sw::UnoCursorPointer pCursor(pDoc->CreateUnoCursor(*pPos));
    CPPUNIT_ASSERT(static_cast<bool>(pCursor));
    pPos.reset(); // we need to kill the SwPosition before disposing
    pIdx.reset(); // we need to kill the SwNodeIndex before disposing
    xDocComponent->dispose();
    CPPUNIT_ASSERT(!static_cast<bool>(pCursor));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTextTableCellNames)
{
    sal_Int32 nCol, nRow2;
    SwXTextTable::GetCellPosition(u"z1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(51), nCol);
    SwXTextTable::GetCellPosition(u"AA1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(52), nCol);
    SwXTextTable::GetCellPosition(u"AB1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(53), nCol);
    SwXTextTable::GetCellPosition(u"BB1", nCol, nRow2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(105), nCol);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testShapeAnchorUndo)
{
    createSwDoc("draw-anchor-undo.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    tools::Rectangle aOrigLogicRect(pObject->GetLogicRect());

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.StartUndo(SwUndoId::START, nullptr);

    pWrtShell->SelectObj(Point(), 0, pObject);

    pWrtShell->GetDrawView()->MoveMarkedObj(Size(100, 100));
    pWrtShell->ChgAnchor(RndStdIds::FLY_AT_PARA, true);

    rUndoManager.EndUndo(SwUndoId::END, nullptr);

    CPPUNIT_ASSERT(aOrigLogicRect != pObject->GetLogicRect());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(pObject->GetLogicRect(), aOrigLogicRect);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf127635)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    emulateTyping(*pXTextDocument, u"a b");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);

    //Select 'a'
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    // hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    emulateTyping(*pXTextDocument, u"c d");

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // Without the fix in place, this test would have failed with
    // - Expected: C d b
    // - Actual  : Cd  b
    CPPUNIT_ASSERT_EQUAL(OUString("C d b"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testDde)
{
#if HAVE_FEATURE_UI
    // Type asdf and copy it.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("asdf");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 4, /*bBasicCall=*/false);
    uno::Sequence<beans::PropertyValue> aPropertyValues;
    dispatchCommand(mxComponent, ".uno:Copy", aPropertyValues);

    // Go before the selection and paste as a DDE link.
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::LINK)) } });
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    // Make sure that the document starts with a field now, and its expanded string value contains asdf.
    const uno::Reference<text::XTextRange> xField = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xField, "TextPortionType"));
    CPPUNIT_ASSERT(xField->getString().endsWith("asdf"));
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testDocModState)
{
    //creating a new writer document via the XDesktop(to have more shells etc.)
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    //checking the state of the document via IDocumentState
    IDocumentState& rState(pDoc->getIDocumentState());
    //the state should not be modified
    CPPUNIT_ASSERT(!(rState.IsModified()));
    //checking the state of the document via SfxObjectShell
    SwDocShell* pShell(pDoc->GetDocShell());
    CPPUNIT_ASSERT(!(pShell->IsModified()));

    IdleTask::waitUntilIdleDispatched();

    //again checking for the state via IDocumentState
    CPPUNIT_ASSERT(!(rState.IsModified()));
    //again checking for the state via SfxObjectShell
    CPPUNIT_ASSERT(!(pShell->IsModified()));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf94804)
{
    //create new writer document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    //get cursor for making bookmark at a particular location
    SwPaM* pCrsr = pDoc->GetEditShell()->GetCursor();
    IDocumentMarkAccess* pIDMAccess(pDoc->getIDocumentMarkAccess());
    //make first bookmark, CROSSREF_HEADING, with *empty* name
    sw::mark::IMark* pMark1(
        pIDMAccess->makeMark(*pCrsr, "", IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK,
                             ::sw::mark::InsertMode::New));
    //get the new(autogenerated) bookmark name
    OUString bookmark1name = pMark1->GetName();
    //match the bookmark name, it should be like "__RefHeading__**"
    CPPUNIT_ASSERT(bookmark1name.match("__RefHeading__"));
    //make second bookmark, CROSSREF_NUMITEM, with *empty* name
    sw::mark::IMark* pMark2(
        pIDMAccess->makeMark(*pCrsr, "", IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK,
                             ::sw::mark::InsertMode::New));
    //get the new(autogenerated) bookmark name
    OUString bookmark2name = pMark2->GetName();
    //match the bookmark name, it should be like "__RefNumPara__**"
    CPPUNIT_ASSERT(bookmark2name.match("__RefNumPara__"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testUnicodeNotationToggle)
{
    createSwDoc("unicodeAltX.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString sOriginalDocString;
    OUString sDocString;
    OUString sExpectedString;
    uno::Sequence<beans::PropertyValue> aPropertyValues;

    pWrtShell->EndPara();
    sOriginalDocString = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL(OUString("uU+002b"), sOriginalDocString);

    dispatchCommand(mxComponent, ".uno:UnicodeNotationToggle", aPropertyValues);
    sExpectedString = "u+";
    sDocString = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL(sDocString, sExpectedString);

    dispatchCommand(mxComponent, ".uno:UnicodeNotationToggle", aPropertyValues);
    sDocString = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText();
    CPPUNIT_ASSERT_EQUAL(sDocString, sOriginalDocString);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf34957)
{
    createSwDoc("tdf34957.odt");
    // table with "keep with next" always started on a new page if the table was large,
    // regardless of whether it was already kept with the previous paragraph,
    // or whether the following paragraph actually fit on the same page (MAB 3.6 - 5.0)
    CPPUNIT_ASSERT_EQUAL(OUString("Row 1"),
                         parseDump("/root/page[2]/body/tab[1]/row[2]/cell[1]/txt"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Row 1"),
                         parseDump("/root/page[4]/body/tab[1]/row[2]/cell[1]/txt"_ostr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf89954)
{
    createSwDoc("tdf89954.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    emulateTyping(*pXTextDocument, u"test.");

    SwNodeIndex aNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    // Placeholder character for the comment anchor was ^A (CH_TXTATR_BREAKWORD), not <fff9> (CH_TXTATR_INWORD).
    // As a result, autocorrect did not turn the 't' input into 'T'.
    CPPUNIT_ASSERT_EQUAL(u"Tes\uFFF9t. Test."_ustr, aNodeIndex.GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf89720)
{
    createSwDoc("tdf89720.odt");
    SwDoc* pDoc = getSwDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (std::unique_ptr<SwSidebarItem> const& pItem : *pPostItMgr)
    {
        if (pItem->mpPostIt->IsFollow())
            // This was non-0: reply comments had a text range overlay,
            // resulting in unexpected dark color.
            CPPUNIT_ASSERT(!pItem->mpPostIt->TextRange());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf88986)
{
    // Create a text shell.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwTextShell aShell(*pView);

    // Create the item set that is normally passed to the insert frame dialog.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    SfxItemSet aSet = aShell.CreateInsertFrameItemSet(aMgr);

    // This was missing along with the gradient and other tables.
    CPPUNIT_ASSERT(aSet.HasItem(SID_COLOR_TABLE));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf78150)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("foobar");

    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:DelToStartOfWord", {});

    // Without the fix, test fails with:
    // equality assertion failed
    //  - Expected:
    //  - Actual  : f
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf138873)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("A B C");

    CPPUNIT_ASSERT_EQUAL(OUString("A B C"), getParagraph(1)->getString());

    // Select B
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    pWrtShell->Insert("DDD");

    CPPUNIT_ASSERT_EQUAL(OUString("A DDD C"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("A B C"), getParagraph(1)->getString());

    // Select B and C
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, ".uno:Copy", {});

    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without the fix in place, this test would have failed with
    // - Expected: A B C
    // - Actual  : A  CB CB
    CPPUNIT_ASSERT_EQUAL(OUString("A B C"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf87922)
{
    // Create an SwDrawTextInfo.
    createSwDoc("tdf87922.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwScriptInfo* pScriptInfo = nullptr;
    // Get access to the single paragraph in the document.
    SwNodeIndex aNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1);
    const OUString& rText = aNodeIndex.GetNode().GetTextNode()->GetText();
    sal_Int32 nLength = rText.getLength();
    SwDrawTextInfo aDrawTextInfo(pWrtShell, *pWrtShell->GetOut(), pScriptInfo, rText,
                                 TextFrameIndex(0), TextFrameIndex(nLength));
    // Root -> page -> body -> text.
    SwTextFrame* pTextFrame
        = static_cast<SwTextFrame*>(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());
    aDrawTextInfo.SetFrame(pTextFrame);

    // If no color background color is found, assume white.
    Color* pColor = sw::GetActiveRetoucheColor();
    *pColor = COL_WHITE;

    // Make sure that automatic color on black background is white, not black.
    vcl::Font aFont;
    aDrawTextInfo.ApplyAutoColor(&aFont);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aFont.GetColor());
}

#if HAVE_MORE_FONTS
namespace
{
struct PortionItem
{
    PortionItem(OUString const& sItemType, sal_Int32 nLength, PortionType nTextType)
        : msItemType(sItemType)
        , mnLength(nLength)
        , mnTextType(nTextType)
    {
    }

    OUString msItemType;
    sal_Int32 mnLength;
    PortionType mnTextType;
};

class PortionHandler : public SwPortionHandler
{
public:
    std::vector<PortionItem> mPortionItems;
    explicit PortionHandler()
        : SwPortionHandler()
    {
    }

    void clear() { mPortionItems.clear(); }

    virtual void Text(TextFrameIndex nLength, PortionType nType) override
    {
        mPortionItems.emplace_back("text", sal_Int32(nLength), nType);
    }

    virtual void Special(TextFrameIndex nLength, const OUString& /*rText*/,
                         PortionType nType) override
    {
        mPortionItems.emplace_back("special", sal_Int32(nLength), nType);
    }

    virtual void LineBreak() override
    {
        mPortionItems.emplace_back("line_break", 0, PortionType::NONE);
    }

    virtual void Skip(TextFrameIndex nLength) override
    {
        mPortionItems.emplace_back("skip", sal_Int32(nLength), PortionType::NONE);
    }

    virtual void Finish() override { mPortionItems.emplace_back("finish", 0, PortionType::NONE); }
};
}
#endif

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf77014)
{
#if HAVE_MORE_FONTS
    // The problem described in the bug tdf#77014 is that the input
    // field text ("ThisIsAllOneWord") is broken up on linebreak, but
    // it should be in one piece (like normal text).

    // This test checks that the input field is in one piece and if the
    // input field has more words, it is broken up at the correct place.

    createSwDoc("tdf77014.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    SwTextFrame* pTextFrame
        = static_cast<SwTextFrame*>(pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());

    PortionHandler aHandler;
    pTextFrame->VisitPortions(aHandler);

    {
        // Input Field - "One Two Three Four Five" = 25 chars
        CPPUNIT_ASSERT_EQUAL(OUString("text"), aHandler.mPortionItems[0].msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(25), aHandler.mPortionItems[0].mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField, aHandler.mPortionItems[0].mnTextType);

        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), aHandler.mPortionItems[1].msItemType);

        CPPUNIT_ASSERT_EQUAL(OUString("finish"), aHandler.mPortionItems[2].msItemType);
    }

    aHandler.clear();

    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());
    pTextFrame->VisitPortions(aHandler);

    {
        // Input Field - "ThisIsAllOneWord" = 18 chars
        CPPUNIT_ASSERT_EQUAL(OUString("text"), aHandler.mPortionItems[0].msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(18), aHandler.mPortionItems[0].mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField, aHandler.mPortionItems[0].mnTextType);

        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), aHandler.mPortionItems[1].msItemType);

        CPPUNIT_ASSERT_EQUAL(OUString("finish"), aHandler.mPortionItems[2].msItemType);
    }

    aHandler.clear();

    // skip empty paragraph
    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());

    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());
    pTextFrame->VisitPortions(aHandler);

    {
        // Text "The purpose of this report is to summarize the results of the existing bug in the LO suite"
        // = 91 chars
        auto& rPortionItem = aHandler.mPortionItems[0];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(91), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Text, rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[1];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        // Input Field: "ThisIsAllOneWord" = 18 chars
        // which is 16 chars + 2 hidden chars (start & end input field) = 18 chars
        // If this is correct then the input field is in one piece
        rPortionItem = aHandler.mPortionItems[2];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(18), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField, rPortionItem.mnTextType);

        // Text "."
        rPortionItem = aHandler.mPortionItems[3];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Text, rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[4];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        rPortionItem = aHandler.mPortionItems[5];
        CPPUNIT_ASSERT_EQUAL(OUString("finish"), rPortionItem.msItemType);
    }

    aHandler.clear();

    pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetNext());
    pTextFrame->VisitPortions(aHandler);
    {
        printf("Portions:\n");

        for (const auto& rPortionItem : aHandler.mPortionItems)
        {
            printf("-- Type: %s length: %" SAL_PRIdINT32 " text type: %d\n",
                   rPortionItem.msItemType.toUtf8().getStr(), rPortionItem.mnLength,
                   sal_uInt16(rPortionItem.mnTextType));
        }

        // Text "The purpose of this report is to summarize the results of the existing bug in the LO suite"
        // 91 chars
        auto& rPortionItem = aHandler.mPortionItems[0];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(91), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Text, rPortionItem.mnTextType);

        // The input field here has more words ("One Two Three Four Five")
        // and it should break after "Two".
        // Input Field: "One Two" = 7 chars + 1 start input field hidden character = 8 chars
        rPortionItem = aHandler.mPortionItems[1];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField, rPortionItem.mnTextType);

        rPortionItem = aHandler.mPortionItems[2];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Hole, rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[3];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        // Input Field:  "Three Four Five" = 16 chars + 1 end input field hidden character = 16 chars
        rPortionItem = aHandler.mPortionItems[4];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::InputField, rPortionItem.mnTextType);

        // Text "."
        rPortionItem = aHandler.mPortionItems[5];
        CPPUNIT_ASSERT_EQUAL(OUString("text"), rPortionItem.msItemType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rPortionItem.mnLength);
        CPPUNIT_ASSERT_EQUAL(PortionType::Text, rPortionItem.mnTextType);

        // NEW LINE
        rPortionItem = aHandler.mPortionItems[6];
        CPPUNIT_ASSERT_EQUAL(OUString("line_break"), rPortionItem.msItemType);

        rPortionItem = aHandler.mPortionItems[7];
        CPPUNIT_ASSERT_EQUAL(OUString("finish"), rPortionItem.msItemType);
    }
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf92648)
{
    createSwDoc("tdf92648.docx");
    SwDoc* pDoc = getSwDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // Make sure we have ten draw shapes.
    // Yes, we have if the left/right pages have different header/footer,
    // but if not we have only nine of them:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), SwTextBoxHelper::getCount(pPage));
    // and the text boxes haven't got zero height
    sal_Int32 nCount = 0;
    for (const SwFrameFormat* pFormat : *pDoc->GetSpzFrameFormats())
    {
        if (!SwTextBoxHelper::isTextBox(pFormat, RES_FLYFRMFMT))
            continue;
        SwFormatFrameSize aSize(pFormat->GetFrameSize());
        CPPUNIT_ASSERT(aSize.GetHeight() != 0);
        ++nCount;
    }
    // and we have had five of them.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nCount);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf103978_backgroundTextShape)
{
    createSwDoc("tdf103978_backgroundTextShape.docx");
    SwDoc* pDoc = getSwDoc();

    // there is only one shape. It has an attached textbox
    bool bShapeIsOpaque = getProperty<bool>(getShape(1), "Opaque");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in the foreground", false, bShapeIsOpaque);
    sal_Int32 nCount = 0;
    for (const SwFrameFormat* pFormat : *pDoc->GetSpzFrameFormats())
    {
        if (!SwTextBoxHelper::isTextBox(pFormat, RES_FLYFRMFMT))
            continue;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox syncs the shape's transparency", bShapeIsOpaque,
                                     pFormat->GetOpaque().GetValue());
        ++nCount;
    }
    //ensure that we don't skip the for loop without an error
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of TextBoxes", sal_Int32(1), nCount);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf117225)
{
    // Test that saving a document with an embedded object does not leak
    // tempfiles in the directory of the target file.
    OUString aTargetDirectory
        = m_directories.getURLFromWorkdir(u"/CppunitTest/sw_uiwriter7.test.user/");
    OUString aTargetFile = aTargetDirectory + "tdf117225.odt";
    OUString aSourceFile = createFileURL(u"tdf117225.odt");
    osl::File::copy(aSourceFile, aTargetFile);
    mxComponent = loadFromDesktop(aTargetFile);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    int nExpected = CountFilesInDirectory(aTargetDirectory);
    xStorable->store();
    int nActual = CountFilesInDirectory(aTargetDirectory);
    // nActual was nExpected + 1, i.e. we leaked a tempfile.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);

    OUString aTargetFileSaveAs = aTargetDirectory + "tdf117225-save-as.odt";
    xStorable->storeAsURL(aTargetFileSaveAs, {});
    ++nExpected;
    nActual = CountFilesInDirectory(aTargetDirectory);
    // nActual was nExpected + 1, i.e. we leaked a tempfile.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf149184)
{
    createSwDoc("simplefooter.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Removing the footer for all styles
    pWrtShell->ChangeHeaderOrFooter(u"", false, false, false);

    // export to simplefooter.doc

    // Without the fix in place, the test fails with:
    // [CUT] sw_uiwriter7
    // Segmentation fault (core dumped)
    // [_RUN_____] testTdf149184::TestBody
    save("MS Word 97");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest7, testTdf149089)
{
    createSwDoc("tdf149089.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nPorLen1 = getXPath(pXmlDoc, "(//SwLinePortion)[1]"_ostr, "length"_ostr).toInt32();
    sal_Int32 nPorLen2 = getXPath(pXmlDoc, "(//SwLinePortion)[2]"_ostr, "length"_ostr).toInt32();
    sal_Int32 nPorLen3 = getXPath(pXmlDoc, "(//SwLinePortion)[3]"_ostr, "length"_ostr).toInt32();
    // Two SwTextPortion and one SwKernPortion
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nPorLen1); // SwTextPortion "&#x4E00;&#x4E00; "
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), nPorLen2); // SwTextPortion "BUG 11111111"
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPorLen3); // SwKernPortion

    sal_Int32 nPorWidth1 = getXPath(pXmlDoc, "(//SwLinePortion)[1]"_ostr, "width"_ostr).toInt32();
    sal_Int32 nPorWidth2 = getXPath(pXmlDoc, "(//SwLinePortion)[2]"_ostr, "width"_ostr).toInt32();
    sal_Int32 nPorWidth3 = getXPath(pXmlDoc, "(//SwLinePortion)[3]"_ostr, "width"_ostr).toInt32();
    sal_Int32 nGridWidth1 = nPorWidth1 / 3;
    sal_Int32 nGridWidth2 = (nPorWidth2 + nPorWidth3) / 7;
    CPPUNIT_ASSERT_EQUAL(nGridWidth1, nGridWidth2);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
