/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <sal/config.h>
#include <test/screenshot_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <svl/aeitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/svdtext.hxx>
#include <svx/svdotext.hxx>
#include <vcl/sound.hxx>

#include "drawdoc.hxx"
#include "DrawDocShell.hxx"

#include <osl/process.h>
#include <osl/thread.h>

#include "sdabstdlg.hxx"
#include <vcl/pngwrite.hxx>
#include "unomodel.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#include <app.hrc>
#include "stlsheet.hxx"
#include "sdattr.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "sdattr.hxx"

#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <unotest/macros_test.hxx>

using namespace ::com::sun::star;

/// Test opening a dialog in sd
class SdDialogsTest : public ScreenshotTest
{
private:
    /// Document and ComponentContext
    uno::Reference<lang::XComponent>        mxComponent;

    /// initially created SdAbstractDialogFactory and pointer to document
    SdAbstractDialogFactory*                mpFact;
    SdXImpressDocument*                     mpImpressDocument;

    /// on-demand created instances required for various dialogs to open
    ::sd::DrawDocShell*                     mpDocShell;
    ::sd::ViewShell*                        mpViewShell;
    ::sd::DrawView*                         mpDrawView;

    std::unique_ptr<SfxItemSet>             mpSfxItemSetFromSdrObject;
    std::unique_ptr<SfxItemSet>             mpEmptySfxItemSet;
    std::unique_ptr<SfxItemSet>             mpEmptyFillStyleSfxItemSet;

    /// helpers
    SdAbstractDialogFactory* getSdAbstractDialogFactory();
    SdXImpressDocument* getSdXImpressDocument();
    ::sd::DrawDocShell* getDocShell();
    ::sd::ViewShell* getViewShell();
    ::sd::DrawView* getDrawView();
    const SfxItemSet& getSfxItemSetFromSdrObject();
    const SfxItemSet& getEmptySfxItemSet();
    const SfxItemSet& getEmptyFillStyleSfxItemSet();

    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclPtr<VclAbstractDialog> createDialogByID(sal_uInt32 nID) override;

public:
    SdDialogsTest();

    virtual void setUp() override;

    void tearDown() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(SdDialogsTest);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

SdDialogsTest::SdDialogsTest()
:   mxComponent(),
    mpFact(nullptr),
    mpImpressDocument(nullptr),
    mpDocShell(nullptr),
    mpViewShell(nullptr),
    mpDrawView(nullptr),
    mpSfxItemSetFromSdrObject(nullptr),
    mpEmptySfxItemSet(nullptr),
    mpEmptyFillStyleSfxItemSet(nullptr)
{
}

void SdDialogsTest::setUp()
{
    ScreenshotTest::setUp();

    mpFact = SdAbstractDialogFactory::Create();
    mxComponent = loadFromDesktop("private:factory/simpress", "com.sun.star.presentation.PresentationDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    mpImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(mpImpressDocument);
}

void SdDialogsTest::tearDown()
{
    mpSfxItemSetFromSdrObject.reset();
    mpEmptySfxItemSet.reset();
    mpEmptyFillStyleSfxItemSet.reset();
    mxComponent->dispose();
    ScreenshotTest::tearDown();
}

SdAbstractDialogFactory* SdDialogsTest::getSdAbstractDialogFactory()
{
    return mpFact;
}

SdXImpressDocument* SdDialogsTest::getSdXImpressDocument()
{
    return mpImpressDocument;
}

::sd::DrawDocShell* SdDialogsTest::getDocShell()
{
    if (!mpDocShell)
    {
        mpDocShell = getSdXImpressDocument()->GetDocShell();
        CPPUNIT_ASSERT(mpDocShell);
    }

    return mpDocShell;
}

::sd::ViewShell* SdDialogsTest::getViewShell()
{
    if (!mpViewShell)
    {
        mpViewShell = getDocShell()->GetViewShell();
        CPPUNIT_ASSERT(mpViewShell);
    }

    return mpViewShell;
}

::sd::DrawView* SdDialogsTest::getDrawView()
{
    if (!mpDrawView)
    {
        mpDrawView = dynamic_cast<::sd::DrawView*>(getViewShell()->GetDrawView());
        CPPUNIT_ASSERT(mpDrawView);
    }

    return mpDrawView;
}

const SfxItemSet& SdDialogsTest::getSfxItemSetFromSdrObject()
{
    if (!mpSfxItemSetFromSdrObject)
    {
        SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
        CPPUNIT_ASSERT(pDrawDoc);
        SdPage* pSdPage = pDrawDoc->GetSdPage(0, PageKind::Standard);
        CPPUNIT_ASSERT(pSdPage);
        SdrObject* pSdrObj = pSdPage->GetObj(0);
        CPPUNIT_ASSERT(pSdrObj);
        mpSfxItemSetFromSdrObject.reset( new SfxItemSet( pSdrObj->GetMergedItemSet() ) );
        CPPUNIT_ASSERT(mpSfxItemSetFromSdrObject);
    }

    return *mpSfxItemSetFromSdrObject;
}

const SfxItemSet& SdDialogsTest::getEmptySfxItemSet()
{
    if (!mpEmptySfxItemSet)
    {
        // needs an SfxItemSet, use the one from the 1st object
        SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
        CPPUNIT_ASSERT(pDrawDoc);
        mpEmptySfxItemSet.reset( new SfxItemSet(pDrawDoc->GetItemPool()) );
        CPPUNIT_ASSERT(mpEmptySfxItemSet);
    }

    return *mpEmptySfxItemSet;
}

const SfxItemSet& SdDialogsTest::getEmptyFillStyleSfxItemSet()
{
    if (!mpEmptyFillStyleSfxItemSet)
    {
        SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
        CPPUNIT_ASSERT(pDrawDoc);
        mpEmptyFillStyleSfxItemSet.reset( new SfxItemSet(pDrawDoc->GetItemPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{}) );
        CPPUNIT_ASSERT(mpEmptyFillStyleSfxItemSet);
        mpEmptyFillStyleSfxItemSet->Put(XFillStyleItem(drawing::FillStyle_NONE));
    }

    return *mpEmptyFillStyleSfxItemSet;
}

void SdDialogsTest::registerKnownDialogsByID(mapType& rKnownDialogs)
{
    // fill map of known dialogs
    rKnownDialogs["modules/simpress/ui/publishingdialog.ui"] = 0;
    rKnownDialogs["modules/sdraw/ui/breakdialog.ui"] = 1;
    rKnownDialogs["modules/sdraw/ui/copydlg.ui"] = 2;
    rKnownDialogs["modules/simpress/ui/customslideshows.ui"] = 3;
    rKnownDialogs["modules/sdraw/ui/drawchardialog.ui"] = 4;
    rKnownDialogs["modules/sdraw/ui/drawpagedialog.ui"] = 5;
    rKnownDialogs["modules/simpress/ui/dlgfield.ui"] = 6;
    rKnownDialogs["modules/sdraw/ui/dlgsnap.ui"] = 7;
    rKnownDialogs["modules/sdraw/ui/insertlayer.ui"] = 8;
    rKnownDialogs["modules/sdraw/ui/insertslidesdialog.ui"] = 9;
    rKnownDialogs["modules/sdraw/ui/crossfadedialog.ui"] = 10;
    rKnownDialogs["modules/sdraw/ui/bulletsandnumbering.ui"] = 11;
    rKnownDialogs["modules/sdraw/ui/drawparadialog.ui"] = 12;
    rKnownDialogs["modules/simpress/ui/presentationdialog.ui"] = 13;
    rKnownDialogs["modules/simpress/ui/remotedialog.ui"] = 14;
    rKnownDialogs["modules/sdraw/ui/drawprtldialog.ui"] = 15;
    rKnownDialogs["modules/simpress/ui/slidedesigndialog.ui"] = 16;
    rKnownDialogs["modules/simpress/ui/templatedialog.ui"] = 17;
    rKnownDialogs["modules/simpress/ui/interactiondialog.ui"] = 18;
    rKnownDialogs["modules/sdraw/ui/vectorize.ui"] = 19;
    rKnownDialogs["modules/simpress/ui/photoalbum.ui"] = 20;
    rKnownDialogs["modules/simpress/ui/masterlayoutdlg.ui"] = 21;
    rKnownDialogs["modules/simpress/ui/headerfooterdialog.ui"] = 22;
}

VclPtr<VclAbstractDialog> SdDialogsTest::createDialogByID(sal_uInt32 nID)
{
    VclPtr<VclAbstractDialog> pRetval;

    if (getSdAbstractDialogFactory())
    {
        switch (nID)
        {
        case 0:
        {
            // CreateSdPublishingDlg(vcl::Window* pWindow, DocumentType eDocType) override;
            // this dialog does not need much, not even a SdDrawDocument. OTOH
            // it is more a 'wizard' in that it has prev/next buttons and implicitly
            // multiple pages. To make use of that it is necessary that the implementation
            // supports the 'Screenshot interface'
            pRetval = getSdAbstractDialogFactory()->CreateSdPublishingDlg(
                Application::GetDefDialogParent(),
                DocumentType::Impress);
            break;
        }
        case 1:
        {
            // CreateBreakDlg(vcl::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount) override;
            // this dialog requires pDrawView. It does not show much when
            // there is no object selected that can be broken up. For better
            // results it might be necessary to add/select an object that
            // delivers a good metafile (which is the base for breaking)
            pRetval = getSdAbstractDialogFactory()->CreateBreakDlg(
                Application::GetDefDialogParent(),
                getDrawView(),
                getDocShell(),
                0,
                0);
            break;
        }
        case 2:
        {
            // needs an SfxItemSet, use the one from the 1st object
            pRetval = getSdAbstractDialogFactory()->CreateCopyDlg(
                getViewShell()->GetActiveWindow(),
                getSfxItemSetFromSdrObject(),
                getDrawView());
            break;
        }
        case 3:
        {
            // CreateSdCustomShowDlg(SdDrawDocument& rDrawDoc) = 0;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);

            pRetval = getSdAbstractDialogFactory()->CreateSdCustomShowDlg(
                getViewShell()->GetActiveWindow(),
                *pDrawDoc);
            break;
        }
        case 4:
        {
            // CreateSdTabCharDialog(const SfxItemSet* pAttr, SfxObjectShell* pDocShell) override;
            // needs an SfxItemSet, use an empty constructed one
            // needs a 'SfxObjectShell* pDocShell', crashes without
            pRetval = getSdAbstractDialogFactory()->CreateSdTabCharDialog(
                getViewShell()->GetActiveWindow(),
                &getEmptySfxItemSet(),
                getDocShell());
            break;
        }
        case 5:
        {
            // CreateSdTabPageDialog(const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage = true) override;
            // needs a special SfxItemSet with merged content from page and other stuff, crashes without that (2nd page)
            // needs a 'SfxObjectShell* pDocShell', crashes without. Also sufficient: FillStyleItemSet with XFILL_NONE set
            pRetval = getSdAbstractDialogFactory()->CreateSdTabPageDialog(
                getViewShell()->GetActiveWindow(),
                &getEmptyFillStyleSfxItemSet(),
                getDocShell(),
                true);
            break;
        }
        case 6:
        {
            // CreateSdModifyFieldDlg(vcl::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdModifyFieldDlg(
                Application::GetDefDialogParent(),
                nullptr,
                getEmptySfxItemSet());
            break;
        }
        case 7:
        {
            // CreateSdSnapLineDlg(const SfxItemSet& rInAttrs, ::sd::View* pView) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aNewAttr(pDrawDoc->GetItemPool(), svl::Items<ATTR_SNAPLINE_START, ATTR_SNAPLINE_END>{});
            aNewAttr.Put(SfxInt32Item(ATTR_SNAPLINE_X, 0));
            aNewAttr.Put(SfxInt32Item(ATTR_SNAPLINE_Y, 0));
            pRetval = getSdAbstractDialogFactory()->CreateSdSnapLineDlg(
                getViewShell()->GetActiveWindow(),
                aNewAttr,
                getDrawView());
            break;
        }
        case 8:
        {
            // CreateSdInsertLayerDlg(const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aNewAttr(pDrawDoc->GetItemPool(), svl::Items<ATTR_LAYER_START, ATTR_LAYER_END>{});
            const OUString aLayerName = SdResId(STR_LAYER); // + OUString::number(2);
            aNewAttr.Put(makeSdAttrLayerName(aLayerName));
            aNewAttr.Put(makeSdAttrLayerTitle());
            aNewAttr.Put(makeSdAttrLayerDesc());
            aNewAttr.Put(makeSdAttrLayerVisible());
            aNewAttr.Put(makeSdAttrLayerPrintable());
            aNewAttr.Put(makeSdAttrLayerLocked());
            aNewAttr.Put(makeSdAttrLayerThisPage());
            pRetval = getSdAbstractDialogFactory()->CreateSdInsertLayerDlg(
                getViewShell()->GetActiveWindow(),
                aNewAttr,
                true, // alternative: false
                SdResId(STR_INSERTLAYER) /* alternative: STR_MODIFYLAYER */);
            break;
        }
        case 9:
        {
            // CreateSdInsertPagesObjsDlg(const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            const OUString aFileName("foo");
            pRetval = getSdAbstractDialogFactory()->CreateSdInsertPagesObjsDlg(
                getViewShell()->GetActiveWindow(),
                pDrawDoc,
                nullptr,
                aFileName);
            break;
        }
        case 10:
        {
            // CreateMorphDlg(vcl::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SdPage* pSdPage = pDrawDoc->GetSdPage(0, PageKind::Standard);
            CPPUNIT_ASSERT(pSdPage);
            SdrObject* pSdrObj = pSdPage->GetObj(0);
            // using one SdrObject is okay, none crashes
            CPPUNIT_ASSERT(pSdrObj);
            pRetval = getSdAbstractDialogFactory()->CreateMorphDlg(
                Application::GetDefDialogParent(),
                pSdrObj,
                pSdrObj);
            break;
        }
        case 11:
        {
            // CreateSdOutlineBulletTabDlg(const SfxItemSet* pAttr, ::sd::View* pView = nullptr) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdOutlineBulletTabDlg(
                getViewShell()->GetActiveWindow(),
                &getEmptySfxItemSet(),
                getDrawView());
            break;
        }
        case 12:
        {
            // CreateSdParagraphTabDlg(const SfxItemSet* pAttr) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdParagraphTabDlg(
                getViewShell()->GetActiveWindow(),
                &getEmptySfxItemSet());
            break;
        }
        case 13:
        {
            // CreateSdStartPresentationDlg(vcl::Window* pWindow, const SfxItemSet& rInAttrs, const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList) override;
            const std::vector<OUString> aPageNames;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aDlgSet(pDrawDoc->GetItemPool(), svl::Items<ATTR_PRESENT_START, ATTR_PRESENT_END>{});
            ::sd::PresentationSettings& rPresentationSettings = pDrawDoc->getPresentationSettings();
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_ALL, rPresentationSettings.mbAll));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_CUSTOMSHOW, rPresentationSettings.mbCustomShow));
            aDlgSet.Put(SfxStringItem(ATTR_PRESENT_DIANAME, OUString()));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_ENDLESS, rPresentationSettings.mbEndless));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_MANUEL, rPresentationSettings.mbManual));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_MOUSE, rPresentationSettings.mbMouseVisible));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_PEN, rPresentationSettings.mbMouseAsPen));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_ANIMATION_ALLOWED, rPresentationSettings.mbAnimationAllowed));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_CHANGE_PAGE, !rPresentationSettings.mbLockedPages));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_ALWAYS_ON_TOP, rPresentationSettings.mbAlwaysOnTop));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_FULLSCREEN, rPresentationSettings.mbFullScreen));
            aDlgSet.Put(SfxUInt32Item(ATTR_PRESENT_PAUSE_TIMEOUT, rPresentationSettings.mnPauseTimeout));
            aDlgSet.Put(SfxBoolItem(ATTR_PRESENT_SHOW_PAUSELOGO, rPresentationSettings.mbShowPauseLogo));
            //SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
            aDlgSet.Put(SfxInt32Item(ATTR_PRESENT_DISPLAY, 0 /*pOptions->GetDisplay()*/));
            pRetval = getSdAbstractDialogFactory()->CreateSdStartPresentationDlg(
                Application::GetDefDialogParent(),
                aDlgSet,
                aPageNames,
                nullptr);
            break;
        }
        case 14:
        {
            // CreateRemoteDialog(vcl::Window* pWindow) override; // ad for RemoteDialog
            pRetval = getSdAbstractDialogFactory()->CreateRemoteDialog(
                Application::GetDefDialogParent());
            break;
        }
        case 15:
        {
            // CreateSdPresLayoutTemplateDlg(SfxObjectShell* pDocSh, vcl::Window* pParent, const SdResId& DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool) override;
            // use STR_PSEUDOSHEET_TITLE configuration, see futempl.cxx for more possible configurations
            // may be nicer on the long run to take a configuration whch represents a selected SdrObject
            SfxStyleSheetBasePool* pStyleSheetPool = getDocShell()->GetStyleSheetPool();
            CPPUNIT_ASSERT(pStyleSheetPool);
            SfxStyleSheetBase* pStyleSheet = pStyleSheetPool->First();
            CPPUNIT_ASSERT(pStyleSheet);
            pRetval = getSdAbstractDialogFactory()->CreateSdPresLayoutTemplateDlg(
                getDocShell(),
                Application::GetDefDialogParent(),
                false,
                *pStyleSheet,
                PO_TITLE,
                pStyleSheetPool);
            break;
        }
        case 16:
        {
            // CreateSdPresLayoutDlg(::sd::DrawDocShell* pDocShell, vcl::Window* pWindow, const SfxItemSet& rInAttrs) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdPresLayoutDlg(
                getDocShell(),
                getEmptySfxItemSet());
            break;
        }
        case 17:
        {
            // CreateSdTabTemplateDlg(const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView) override;
            // pretty similar to CreateSdPresLayoutTemplateDlg, see above
            // may be nicer on the long run to take a configuration whch represents a selected SdrObject
            SfxStyleSheetBasePool* pStyleSheetPool = getDocShell()->GetStyleSheetPool();
            CPPUNIT_ASSERT(pStyleSheetPool);
            SfxStyleSheetBase* pStyleSheet = pStyleSheetPool->First();
            CPPUNIT_ASSERT(pStyleSheet);
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            pRetval = getSdAbstractDialogFactory()->CreateSdTabTemplateDlg(
                getViewShell()->GetActiveWindow(),
                getDocShell(),
                *pStyleSheet,
                pDrawDoc,
                getDrawView());
            break;
        }
        case 18:
        {
            // CreatSdActionDialog(const SfxItemSet* pAttr, ::sd::View* pView) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aSet(pDrawDoc->GetItemPool(), svl::Items<ATTR_ANIMATION_START, ATTR_ACTION_END>{});
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_ACTIVE, false));
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_EFFECT, sal_uInt16(presentation::AnimationEffect_NONE)));
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_TEXTEFFECT, sal_uInt16(presentation::AnimationEffect_NONE)));
            aSet.InvalidateItem(ATTR_ANIMATION_SPEED);
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_FADEOUT, false));
            aSet.Put(SvxColorItem(RGB_Color(COL_LIGHTGRAY), ATTR_ANIMATION_COLOR));
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_INVISIBLE, false));
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_SOUNDON, false));
            aSet.InvalidateItem(ATTR_ANIMATION_SOUNDFILE);
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_PLAYFULL, false));
            aSet.Put(SfxAllEnumItem(ATTR_ACTION, sal_uInt16(presentation::ClickAction_NONE)));
            aSet.InvalidateItem(ATTR_ACTION_FILENAME);
            aSet.Put(SfxAllEnumItem(ATTR_ACTION_EFFECT, sal_uInt16(presentation::AnimationEffect_NONE)));
            aSet.InvalidateItem(ATTR_ACTION_EFFECTSPEED);
            aSet.Put(SfxBoolItem(ATTR_ACTION_SOUNDON, false));
            aSet.Put(SfxBoolItem(ATTR_ACTION_PLAYFULL, false));
            pRetval = getSdAbstractDialogFactory()->CreatSdActionDialog(
                getViewShell()->GetActiveWindow(),
                &aSet,
                getDrawView());
            break;
        }
        case 19:
        {
            // CreateSdVectorizeDlg(vcl::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell) override;
            // works well with empty Bitmap, but my be nicer with setting one
            Bitmap aEmptyBitmap;
            pRetval = getSdAbstractDialogFactory()->CreateSdVectorizeDlg(
                Application::GetDefDialogParent(),
                aEmptyBitmap,
                getDocShell());
            break;
        }
        case 20:
        {
            // CreateSdPhotoAlbumDialog(vcl::Window* pWindow, SdDrawDocument* pDoc) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            pRetval = getSdAbstractDialogFactory()->CreateSdPhotoAlbumDialog(
                Application::GetDefDialogParent(),
                pDrawDoc);
            break;
        }
        case 21:
        {
            // CreateMasterLayoutDialog(vcl::Window* pParent, SdDrawDocument* pDoc, SdPage*) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SdPage* pSdPage = pDrawDoc->GetSdPage(0, PageKind::Standard);
            CPPUNIT_ASSERT(pSdPage);
            pRetval = getSdAbstractDialogFactory()->CreateMasterLayoutDialog(
                Application::GetDefDialogParent(),
                pDrawDoc,
                pSdPage);
            break;
        }
        case 22:
        {
            // CreateHeaderFooterDialog(sd::ViewShell* pViewShell, vcl::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage) override;
            // This is a hard case, for two reasons:
            // - It uses BaseClass TabPage which has a very sparse interface,
            //   need to add 'Screenshot interface' there and implement
            // - The concrete dialog has two TabPages which use the *same*
            //   .ui file, so extended markup will be needed to differ these two
            //   cases
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SdPage* pSdPage = pDrawDoc->GetSdPage(0, PageKind::Standard);
            CPPUNIT_ASSERT(pSdPage);
            pRetval = getSdAbstractDialogFactory()->CreateHeaderFooterDialog(
                getViewShell(),
                Application::GetDefDialogParent(),
                pDrawDoc,
                pSdPage);
            break;
        }
        default:
            break;
        }
    }

    return pRetval;
}

void SdDialogsTest::openAnyDialog()
{
    /// example how to process an input file containing the UXMLDescriptions of the dialogs
    /// to dump
    if (true)
    {
        processDialogBatchFile("sd/qa/unit/data/dialogs-test.txt");
    }

    /// example how to dump all known dialogs
    if (false)
    {
        processAllKnownDialogs();
    }

    /// example how to dump exactly one known dialog
    if (false)
    {
        // example for SfxTabDialog: 5 -> "modules/sdraw/ui/drawpagedialog.ui"
        // example for TabDialog: 22 -> "modules/simpress/ui/headerfooterdialog.ui"
        // example for self-adapted wizard: 0 -> "modules/simpress/ui/publishingdialog.ui"
        ScopedVclPtr<VclAbstractDialog> pDlg(createDialogByID(5));

        if (pDlg)
        {
            // known dialog, dump screenshot to path
            dumpDialogToPath(*pDlg);
        }
        else
        {
            // unknown dialog, should not happen in this basic loop.
            // You have probably forgotten to add a case and
            // implementastion to createDialogByID, please do this
        }
    }

    /// example how to dump a dialog using fallback functionality
    if (false)
    {
        // unknown dialog, try fallback to generic created
        // VclBuilder-generated instance. Keep in mind that Dialogs
        // using this mechanism will probably not be layouted well
        // since the setup/initialization part is missing. Thus,
        // only use for fallback when only the UI file is available.
        //
        // Take any example here, it's only for demonstration - using
        // even a known one to demonstrate the fallback possibility
        const OString aUIXMLDescription("modules/sdraw/ui/breakdialog.ui");

        dumpDialogToPath(aUIXMLDescription);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdDialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
