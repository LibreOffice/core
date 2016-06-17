/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
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
#include "../source/ui/inc/DrawDocShell.hxx"

#include <osl/process.h>
#include <osl/thread.h>

#include "sdabstdlg.hxx"
#include <vcl/pngwrite.hxx>
#include "unomodel.hxx"
#include "ViewShell.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#include <app.hrc>
#include "stlsheet.hxx"
#include "sdattr.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "sdattr.hxx"
#include "prltempl.hrc"

#include <comphelper/servicehelper.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <comphelper/processfactory.hxx>
#include <unotest/macros_test.hxx>

//#include "DrawController.hxx"
//#include "ViewShellBase.hxx"

using namespace ::com::sun::star;

/// Test opening a dialog in sd
class SdDialogsTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    /// Document and ComponentContext
    uno::Reference<uno::XComponentContext>  mxComponentContext;
    uno::Reference<lang::XComponent>        mxComponent;

    /// initially created SdAbstractDialogFactory and pointer to document
    SdAbstractDialogFactory*                mpFact;
    SdXImpressDocument*                     mpImpressDocument;

    /// on-demand created instances required for varoius dialogs to open
    ::sd::DrawDocShell*                     mpDocShell;
    ::sd::ViewShell*                        mpViewShell;
    ::sd::DrawView*                         mpDrawView;
    const SfxItemSet*                       mpSfxItemSetFromSdrObject;
    const SfxItemSet*                       mpEmptySfxItemSet;
    SfxItemSet*                             mpEmptyFillStyleSfxItemSet;

    /// helpers
    SdAbstractDialogFactory* getSdAbstractDialogFactory();
    SdXImpressDocument* getSdXImpressDocument();
    ::sd::DrawDocShell* getDocShell();
    ::sd::ViewShell* getViewShell();
    ::sd::DrawView* getDrawView();
    const SfxItemSet& getSfxItemSetFromSdrObject();
    const SfxItemSet& getEmptySfxItemSet();
    const SfxItemSet& getEmptyFillStyleSfxItemSet();

    /// central methods: dialog creation and dumping to target directory (path)
    VclAbstractDialog* createDialogByID(sal_uInt32 nID);
    void dumpDialogToPath(VclAbstractDialog& rDlg, const OUString& rPath);

public:
    SdDialogsTest();
    ~SdDialogsTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(SdDialogsTest);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

SdDialogsTest::SdDialogsTest()
:   mxComponentContext(),
    mxComponent(),
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

SdDialogsTest::~SdDialogsTest()
{
}

void SdDialogsTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
    mpFact = SdAbstractDialogFactory::Create();
    mxComponent = loadFromDesktop("private:factory/simpress", "com.sun.star.presentation.PresentationDocument");
    CPPUNIT_ASSERT(mxComponent.is());
    mpImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(mpImpressDocument);
}

void SdDialogsTest::tearDown()
{
    if (mpEmptySfxItemSet)
    {
        delete mpEmptySfxItemSet;
    }

    if (mpEmptyFillStyleSfxItemSet)
    {
        delete mpEmptyFillStyleSfxItemSet;
    }

    if (mxComponent.is())
    {
        mxComponent->dispose();
    }

    test::BootstrapFixture::tearDown();
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
        SdPage* pSdPage = pDrawDoc->GetSdPage(0, PK_STANDARD);
        CPPUNIT_ASSERT(pSdPage);
        SdrObject* pSdrObj = pSdPage->GetObj(0);
        CPPUNIT_ASSERT(pSdrObj);
        mpSfxItemSetFromSdrObject = &pSdrObj->GetMergedItemSet();
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
        mpEmptySfxItemSet = new SfxItemSet(pDrawDoc->GetItemPool());
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
        mpEmptyFillStyleSfxItemSet = new SfxItemSet(pDrawDoc->GetItemPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);
        CPPUNIT_ASSERT(mpEmptyFillStyleSfxItemSet);
        mpEmptyFillStyleSfxItemSet->Put(XFillStyleItem(drawing::FillStyle_NONE));
    }

    return *mpEmptyFillStyleSfxItemSet;
}

VclAbstractDialog* SdDialogsTest::createDialogByID(sal_uInt32 nID)
{
    VclAbstractDialog* pRetval = 0;

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
                DOCUMENT_TYPE_IMPRESS);
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
            // CreateCopyDlg(const SfxItemSet& rInAttrs, const rtl::Reference<XColorList> &pColTab, ::sd::View* pView) override;
            // works without XColorList (DropDowns will be empty), but
            // it's easy to access, so use it
            XColorListRef pColorList(XColorList::CreateStdColorList());

            // needs an SfxItemSet, use the one from the 1st object
            pRetval = getSdAbstractDialogFactory()->CreateCopyDlg(
                getSfxItemSetFromSdrObject(),
                pColorList,
                getDrawView());
            break;
        }
        case 3:
        {
            // CreateSdCustomShowDlg(SdDrawDocument& rDrawDoc) = 0;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);

            pRetval = getSdAbstractDialogFactory()->CreateSdCustomShowDlg(
                *pDrawDoc);
            break;
        }
        case 4:
        {
            // CreateSdTabCharDialog(const SfxItemSet* pAttr, SfxObjectShell* pDocShell) override;
            // needs an SfxItemSet, use an empty constructed one
            // needs a 'SfxObjectShell* pDocShell', crashes without
            pRetval = getSdAbstractDialogFactory()->CreateSdTabCharDialog(
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
                &getEmptyFillStyleSfxItemSet(),
                getDocShell(),
                true);
            break;
        }
        case 6:
        {
            // CreateAssistentDlg(bool bAutoPilot) override;
            // This is also a 'wizard' in that it has prev/next buttons and implicitly
            // multiple pages. To make use of that it is necessary that the implementation
            // supports the 'Screenshot interface', see AssistentDlg class
            pRetval = getSdAbstractDialogFactory()->CreateAssistentDlg(true);
            break;
        }
        case 7:
        {
            // CreateSdModifyFieldDlg(vcl::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdModifyFieldDlg(
                Application::GetDefDialogParent(),
                nullptr,
                getEmptySfxItemSet());
            break;
        }
        case 8:
        {
            // CreateSdSnapLineDlg(const SfxItemSet& rInAttrs, ::sd::View* pView) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aNewAttr(pDrawDoc->GetItemPool(), ATTR_SNAPLINE_START, ATTR_SNAPLINE_END);
            aNewAttr.Put(SfxInt32Item(ATTR_SNAPLINE_X, 0));
            aNewAttr.Put(SfxInt32Item(ATTR_SNAPLINE_Y, 0));
            pRetval = getSdAbstractDialogFactory()->CreateSdSnapLineDlg(
                aNewAttr,
                getDrawView());
            break;
        }
        case 9:
        {
            // CreateSdInsertLayerDlg(const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aNewAttr(pDrawDoc->GetItemPool(), ATTR_LAYER_START, ATTR_LAYER_END);
            const OUString aLayerName = SD_RESSTR(STR_LAYER); // + OUString::number(2);
            aNewAttr.Put(SdAttrLayerName(aLayerName));
            aNewAttr.Put(SdAttrLayerTitle());
            aNewAttr.Put(SdAttrLayerDesc());
            aNewAttr.Put(SdAttrLayerVisible());
            aNewAttr.Put(SdAttrLayerPrintable());
            aNewAttr.Put(SdAttrLayerLocked());
            aNewAttr.Put(SdAttrLayerThisPage());
            pRetval = getSdAbstractDialogFactory()->CreateSdInsertLayerDlg(
                aNewAttr,
                true, // alternative: false
                SD_RESSTR(STR_INSERTLAYER) /* alternative: STR_MODIFYLAYER */);
            break;
        }
        case 10:
        {
            // CreateSdInsertPagesObjsDlg(const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            const OUString aFileName("foo");
            pRetval = getSdAbstractDialogFactory()->CreateSdInsertPagesObjsDlg(
                pDrawDoc,
                nullptr,
                aFileName);
            break;
        }
        case 11:
        {
            // CreateMorphDlg(vcl::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SdPage* pSdPage = pDrawDoc->GetSdPage(0, PK_STANDARD);
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
        case 12:
        {
            // CreateSdOutlineBulletTabDlg(const SfxItemSet* pAttr, ::sd::View* pView = nullptr) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdOutlineBulletTabDlg(
                &getEmptySfxItemSet(),
                getDrawView());
            break;
        }
        case 13:
        {
            // CreateSdParagraphTabDlg(const SfxItemSet* pAttr) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdParagraphTabDlg(
                &getEmptySfxItemSet());
            break;
        }
        case 14:
        {
            // CreateSdStartPresentationDlg(vcl::Window* pWindow, const SfxItemSet& rInAttrs, const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList) override;
            const std::vector<OUString> aPageNames;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aDlgSet(pDrawDoc->GetItemPool(), ATTR_PRESENT_START, ATTR_PRESENT_END);
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
            //SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
            aDlgSet.Put(SfxInt32Item(ATTR_PRESENT_DISPLAY, 0 /*pOptions->GetDisplay()*/));
            pRetval = getSdAbstractDialogFactory()->CreateSdStartPresentationDlg(
                Application::GetDefDialogParent(),
                aDlgSet,
                aPageNames,
                nullptr);
            break;
        }
        case 15:
        {
            // CreateRemoteDialog(vcl::Window* pWindow) override; // ad for RemoteDialog
            pRetval = getSdAbstractDialogFactory()->CreateRemoteDialog(
                Application::GetDefDialogParent());
            break;
        }
        case 16:
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
                SdResId(TAB_PRES_LAYOUT_TEMPLATE),
                *pStyleSheet,
                PO_TITLE,
                pStyleSheetPool);
            break;
        }
        case 17:
        {
            // CreateSdPresLayoutDlg(::sd::DrawDocShell* pDocShell, vcl::Window* pWindow, const SfxItemSet& rInAttrs) override;
            pRetval = getSdAbstractDialogFactory()->CreateSdPresLayoutDlg(
                getDocShell(),
                Application::GetDefDialogParent(),
                getEmptySfxItemSet());
            break;
        }
        case 18:
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
                getDocShell(),
                *pStyleSheet,
                pDrawDoc,
                getDrawView());
            break;
        }
        case 19:
        {
            // CreatSdActionDialog(const SfxItemSet* pAttr, ::sd::View* pView) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SfxItemSet aSet(pDrawDoc->GetItemPool(), ATTR_ANIMATION_START, ATTR_ACTION_END);
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_ACTIVE, false));
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_EFFECT, presentation::AnimationEffect_NONE));
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_TEXTEFFECT, presentation::AnimationEffect_NONE));
            aSet.InvalidateItem(ATTR_ANIMATION_SPEED);
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_FADEOUT, false));
            aSet.Put(SvxColorItem(RGB_Color(COL_LIGHTGRAY), ATTR_ANIMATION_COLOR));
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_INVISIBLE, false));
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_SOUNDON, false));
            aSet.InvalidateItem(ATTR_ANIMATION_SOUNDFILE);
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_PLAYFULL, false));
            aSet.Put(SfxAllEnumItem(ATTR_ACTION, presentation::ClickAction_NONE));
            aSet.InvalidateItem(ATTR_ACTION_FILENAME);
            aSet.Put(SfxAllEnumItem(ATTR_ACTION_EFFECT, presentation::AnimationEffect_NONE));
            aSet.InvalidateItem(ATTR_ACTION_EFFECTSPEED);
            aSet.Put(SfxBoolItem(ATTR_ACTION_SOUNDON, false));
            aSet.Put(SfxBoolItem(ATTR_ACTION_PLAYFULL, false));
            pRetval = getSdAbstractDialogFactory()->CreatSdActionDialog(
                &aSet,
                getDrawView());
            break;
        }
        case 20:
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
        case 21:
        {
            // CreateSdPhotoAlbumDialog(vcl::Window* pWindow, SdDrawDocument* pDoc) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            pRetval = getSdAbstractDialogFactory()->CreateSdPhotoAlbumDialog(
                Application::GetDefDialogParent(),
                pDrawDoc);
            break;
        }
        case 22:
        {
            // CreateMasterLayoutDialog(vcl::Window* pParent, SdDrawDocument* pDoc, SdPage*) override;
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SdPage* pSdPage = pDrawDoc->GetSdPage(0, PK_STANDARD);
            CPPUNIT_ASSERT(pSdPage);
            pRetval = getSdAbstractDialogFactory()->CreateMasterLayoutDialog(
                Application::GetDefDialogParent(),
                pDrawDoc,
                pSdPage);
            break;
        }
        case 23:
        {
            // CreateHeaderFooterDialog(sd::ViewShell* pViewShell, vcl::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage) override;
            // This is a hard case, for two reasons:
            // - It uses BaseClass TabPage which hase a very spartse interface,
            //   need to add 'Screenshot interface' there and implement
            // - The concrete dialog has two TabPages which use the *same*
            //   .ui file, so extended markup will be needed to differ these two
            //   cases
            SdDrawDocument* pDrawDoc = getSdXImpressDocument()->GetDoc();
            CPPUNIT_ASSERT(pDrawDoc);
            SdPage* pSdPage = pDrawDoc->GetSdPage(0, PK_STANDARD);
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

void SdDialogsTest::dumpDialogToPath(VclAbstractDialog& rDlg, const OUString& rPath)
{
    // for dumping, a lossless format is needed. It may be seen if the created data
    // will be further modified/reduced, but for a input creating step it is
    // unavoidable to use a lossless format initially
    const std::vector<OString> aPageDescriptions(rDlg.getAllPageUIXMLDescriptions());

    if (aPageDescriptions.size())
    {
        for (sal_uInt32 a(0); a < aPageDescriptions.size(); a++)
        {
            if (rDlg.selectPageByUIXMLDescription(aPageDescriptions[a]))
            {
                const Bitmap aScreenshot(rDlg.createScreenshot());

                if (!aScreenshot.IsEmpty())
                {
                    SvFileStream aNew(rPath + OUString("_") + OUString::number(a) + OUString(".png"), StreamMode::WRITE | StreamMode::TRUNC);
                    vcl::PNGWriter aPNGWriter(aScreenshot);
                    aPNGWriter.Write(aNew);
                }
            }
            else
            {
                CPPUNIT_ASSERT(false);
            }
        }
    }
    else
    {
        const Bitmap aScreenshot(rDlg.createScreenshot());

        if (!aScreenshot.IsEmpty())
        {
            SvFileStream aNew(rPath + OUString(".png"), StreamMode::WRITE | StreamMode::TRUNC);
            vcl::PNGWriter aPNGWriter(aScreenshot);
            aPNGWriter.Write(aNew);
        }
    }
}

void SdDialogsTest::openAnyDialog()
{
// activate for debug using attach
//    while (true)
//    {
//        Sound::Beep();
//    }

    // current target for png's is defined here
    const OUString aTempTargetPath(
#ifdef _WIN32
        "c:\\test_dlgF_"
#else
        "~/test_dlgF_"
#endif
        );

    // example for SfxTabDialog: 5
    // example for TabDialog: 23
    // example for self-adapted wizard: 0
    static sal_uInt32 nStartValue(0);
    static sal_uInt32 nEndValue(24);

    // loop and dump all Dialogs from SD for now
    for (sal_uInt32 a(nStartValue); a < nEndValue; a++)
    {
        VclAbstractDialog* pDlg = createDialogByID(a);

        if (pDlg)
        {
            if (false)
            {
                const Application::DialogCancelMode aOriginalDialogCancelMode(Application::GetDialogCancelMode());
                Application::SetDialogCancelMode(Application::DIALOG_CANCEL_OFF);
                pDlg->Execute();
                Application::SetDialogCancelMode(aOriginalDialogCancelMode);
            }

            dumpDialogToPath(*pDlg, aTempTargetPath + OUString::number(a));
            delete pDlg;
        }
    }


}

CPPUNIT_TEST_SUITE_REGISTRATION(SdDialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
