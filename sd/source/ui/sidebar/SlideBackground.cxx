#include <com/sun/star/animations/XAnimationNode.hpp>
#include "SlideBackground.hxx"
#include "TransitionPreset.hxx"
#include "sdresid.hxx"
#include "ViewShellBase.hxx"
#include "DrawDocShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "drawdoc.hxx"
#include "filedlg.hxx"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "DocumentHelper.hxx"
#include "MasterPagesSelector.hxx"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "sdresid.hxx"
#include <svtools/controldims.hrc>
#include <svx/gallery.hxx>
#include <svx/drawitem.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <tools/resary.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include "app.hrc"
#include "SlideBackground.hrc"
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/svxids.hrc>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xattr.hxx>
#include <svx/xflhtit.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;


namespace sd { namespace sidebar {

SlideBackground::SlideBackground(
    Window * pParent,
    ViewShellBase& rBase,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings
     ) :
        PanelLayout( pParent, "SlideBackground", "modules/simpress/ui/sidebarslidebackground.ui", rxFrame ),
        mrBase( rBase ),
        maPaperController(SID_ATTR_PAGE_SIZE, *pBindings, *this),
        maBckColorController(SID_PAGEBACKGROUND_COLOR, *pBindings, *this),
        maBckGradientController(SID_PAGEBACKGROUND_GRADIENT, *pBindings, *this),
        maBckHatchController(SID_PAGEBACKGROUND_HATCH, *pBindings, *this),
        maBckBitmapController(SID_PAGEBACKGROUND_BITMAP, *pBindings, *this),
        maBckFillStyleController(SID_PAGEBACKGROUND_FILLSTYLE, *pBindings, *this),
        maBckImageController(SID_SELECT_BACKGROUND, *pBindings, *this),
        maDspBckController(SID_DISPLAY_MASTER_BACKGROUND, *pBindings, *this),
        maDspObjController(SID_DISPLAY_MASTER_OBJECTS, *pBindings, *this),
        mpFillStyleItem(),
        mpColorItem(),
        mpGradientItem(),
        mpHatchItem(),
        mpBitmapItem(),
        mpBindings(pBindings)
{
    get(mpPaperSizeBox,"paperformat");
    get(mpPaperOrientation, "orientation");
    get(mpMasterSlide, "masterslide");
    get(mpFillAttr, "fillattr1");
    get(mpFillGrad, "fillattr2");
    get(mpFillStyle, "fillstyle");
    get(mpFillLB, "fillattr");
    get(mpDspMasterBackground, "displaymasterbackground");
    get(mpDspMasterObjects, "displaymasterobjects");
    Initialize();
}

SlideBackground::~SlideBackground()
{
    disposeOnce();
}

void SlideBackground::Initialize()
{
    SdResId aResourceId(RID_SVXSTRARY_PAPERSIZE_DRAW);
    ResStringArray aPaperformat(aResourceId);
    sal_uInt32 nCnt = aPaperformat.Count();
    for(sal_uInt32 i = 0; i < nCnt ; i++)
    {
        OUString aStr = aPaperformat.GetString(i);
        mpPaperSizeBox->InsertEntry(aStr);
    }
    mpPaperSizeBox->SetSelectHdl(LINK(this,SlideBackground,PaperSizeModifyHdl));
    mpPaperOrientation->SetSelectHdl(LINK(this,SlideBackground,PaperSizeModifyHdl));
    ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
    SdDrawDocument* pDoc = pDocSh->GetDoc();
    sal_uInt16 nCount = pDoc->GetMasterPageCount();
    for( sal_uInt16 nLayout = 0; nLayout < nCount; nLayout++ )
    {
        SdPage* pMaster = static_cast<SdPage*>(pDoc->GetMasterPage(nLayout));
        if( pMaster->GetPageKind() == PK_STANDARD)
        {
            OUString aLayoutName(pMaster->GetLayoutName());
            aLayoutName = aLayoutName.copy(0,aLayoutName.indexOf(SD_LT_SEPARATOR));
            mpMasterSlide->InsertEntry(aLayoutName);
        }
    }

    mpMasterSlide->SetSelectHdl(LINK(this, SlideBackground, AssignMasterPage));
    mpFillStyle->SetSelectHdl(LINK(this, SlideBackground, FillStyleModifyHdl));
    mpFillLB->SetSelectHdl(LINK(this, SlideBackground, FillColorHdl));
    mpFillGrad->SetSelectHdl(LINK(this, SlideBackground, FillColorHdl));
    mpFillAttr->SetSelectHdl(LINK(this, SlideBackground, FillBackgroundHdl));
    SfxObjectShell* pSh = SfxObjectShell::Current();
    ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
    DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
    SdPage* mpPage = pDrawViewShell->getCurrentPage();
    OUString aLayoutName( mpPage->GetLayoutName() );
    aLayoutName = aLayoutName.copy(0,aLayoutName.indexOf(SD_LT_SEPARATOR));
    mpMasterSlide->SelectEntry(aLayoutName);
    mpFillStyle->SelectEntryPos(0);
    mpDspMasterBackground->SetClickHdl(LINK(this, SlideBackground, DspBackground));
    mpDspMasterObjects->SetClickHdl(LINK(this,SlideBackground, DspObjects));
    Update();
}

void SlideBackground::Update()
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(eXFS)
    {
        case drawing::FillStyle_NONE:
        {
            mpFillLB->Hide();
            mpFillAttr->Hide();
            mpFillGrad->Hide();
        }
        break;
        case drawing::FillStyle_SOLID:
        {
            mpFillAttr->Hide();
            mpFillGrad->Hide();
            mpFillLB->Show();
            mpFillLB->Clear();
            const SvxColorListItem aItem( *static_cast<const SvxColorListItem*>(pSh->GetItem(SID_COLOR_TABLE)));
            mpFillLB->Fill(aItem.GetColorList());
        }
        break;
        case drawing::FillStyle_GRADIENT:
        {
            mpFillLB->Show();
            const SvxColorListItem aItem(*static_cast<const SvxColorListItem*>(pSh->GetItem(SID_COLOR_TABLE)));
            mpFillAttr->Hide();
            mpFillGrad->Show();
            mpFillLB->Clear();
            mpFillGrad->Clear();
            mpFillLB->Fill(aItem.GetColorList());
            mpFillGrad->Fill(aItem.GetColorList());
        }
        break;
        case drawing::FillStyle_HATCH:
        {
            mpFillLB->Hide();
            const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
            mpFillAttr->Show();
            mpFillAttr->Clear();
            mpFillAttr->Fill(aItem.GetHatchList());
            mpFillGrad->Hide();
        }
        break;
        case drawing::FillStyle_BITMAP:
        {
            mpFillLB->Hide();
            const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
            mpFillAttr->Show();
            mpFillAttr->Clear();
            mpFillAttr->Fill(aItem.GetBitmapList());
            mpFillGrad->Hide();
        }
        break;
    }
}

void SlideBackground::dispose()
{
    mpPaperSizeBox.clear();
    mpPaperOrientation.clear();
    mpMasterSlide.clear();
    mpFillAttr.clear();
    mpFillGrad.clear();
    mpFillStyle.clear();
    mpFillLB.clear();
    mpDspMasterBackground.clear();
    mpDspMasterObjects.clear();

    maPaperController.dispose();
    maBckColorController.dispose();
    maBckGradientController.dispose();
    maBckHatchController.dispose();
    maBckBitmapController.dispose();
    maBckFillStyleController.dispose();
    maBckImageController.dispose();
    maDspBckController.dispose();
    maDspObjController.dispose();
}

void SlideBackground::DataChanged (const DataChangedEvent& /*rEvent*/)
{

}
void SlideBackground::NotifyItemUpdate(
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;
    const bool bDisabled(SfxItemState::DISABLED == eState);

    switch(nSID)
    {
/*
        case SID_PAGEBACKGROUND_COLOR:
        {
            if(bDisabled)
            {
                mpFillStyle->Disable();
                mpFillLB->Disable();
            }
            else
            {
                mpFillStyle->Enable();
                mpFillLB->Enable();
            }
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(1);
                mpColorItem.reset(pState ? static_cast< XFillColorItem* >(pState->Clone()) : 0);
                Update();
            }
        }
        break;

        case SID_PAGEBACKGROUND_HATCH:
        {
            if(bDisabled)
            {
                mpFillStyle->Disable();
                mpFillAttr->Disable();
            }
            else
            {
                mpFillStyle->Enable();
                mpFillAttr->Enable();
            }
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(3);
                mpHatchItem.reset(pState ? static_cast < XFillHatchItem* >(pState->Clone()) : 0);
                Update();
            }
        }
        break;

        case SID_PAGEBACKGROUND_GRADIENT:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(2);
                mpGradientItem.reset(pState ? static_cast< XFillGradientItem* >(pState->Clone()) : 0);
                Update();
            }
        }
        break;
        case SID_PAGEBACKGROUND_BITMAP:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(4);
                mpBitmapItem.reset(pState ? static_cast< XFillBitmapItem* >(pState->Clone()) : 0);
                Update();
            }
        }
        break;
        case SID_PAGEBACKGROUND_FILLSTYLE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const XFillStyleItem* aFillStyleItem = dynamic_cast< const XFillStyleItem* >(pState);
                css::drawing::FillStyle eXFS = aFillStyleItem->GetValue();
                switch(eXFS)
                {
                    case drawing::FillStyle_NONE:
                        mpFillStyle->SelectEntryPos(0);
                        break;
                    case drawing::FillStyle_SOLID:
                        mpFillStyle->SelectEntryPos(1);
                        break;
                    case drawing::FillStyle_GRADIENT:
                        mpFillStyle->SelectEntryPos(2);
                        break;
                    case drawing::FillStyle_HATCH:
                        mpFillStyle->SelectEntryPos(3);
                        break;
                    case drawing::FillStyle_BITMAP:
                        mpFillStyle->SelectEntryPos(4);
                        break;
                }
                Update();
            }
        }
        break;
*/
        case SID_DISPLAY_MASTER_BACKGROUND:
        {
            const SfxBoolItem* aBoolItem = dynamic_cast< const SfxBoolItem* >(pState);
            if(aBoolItem->GetValue())
                mpDspMasterBackground->SetState(TRISTATE_TRUE);
            else
                mpDspMasterBackground->SetState(TRISTATE_FALSE);
        }
        break;
        case SID_DISPLAY_MASTER_OBJECTS:
        {
            const SfxBoolItem* aBoolItem = dynamic_cast< const SfxBoolItem* >(pState);
            if(aBoolItem->GetValue())
                mpDspMasterObjects->SetState(TRISTATE_TRUE);
            else
                mpDspMasterObjects->SetState(TRISTATE_FALSE);
        }
        break;

        case SID_SELECT_BACKGROUND:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(4);
                Update();
            }
        }
        break;
        default:
        {
            SfxObjectShell* pSh = SfxObjectShell::Current();
            ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
            DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
            SdPage* mpPage = pDrawViewShell->getCurrentPage();
            SdrPageProperties& rP = mpPage->getSdrPageProperties();
            const drawing::FillStyle eXFS = static_cast< const XFillStyleItem& >(rP.GetItemSet().Get(XATTR_FILLSTYLE)).GetValue();
            switch(eXFS)
            {
                case drawing::FillStyle_NONE:
                    mpFillStyle->SelectEntryPos(0);
                    break;
                case drawing::FillStyle_SOLID:
                    mpFillStyle->SelectEntryPos(1);
                    break;
                case drawing::FillStyle_GRADIENT:
                    mpFillStyle->SelectEntryPos(2);
                    break;
                case drawing::FillStyle_HATCH:
                    mpFillStyle->SelectEntryPos(3);
                    break;
                case drawing::FillStyle_BITMAP:
                    mpFillStyle->SelectEntryPos(4);
                    break;
            }
            Update();
        }
    }
}
IMPL_LINK_NOARG(SlideBackground, FillStyleModifyHdl)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    const XFillStyleItem aXFillStyleItem(eXFS);
    Update();
    GetBindings()->GetDispatcher()->Execute(SID_PAGEBACKGROUND_FILLSTYLE, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
    return 0;
}
IMPL_LINK_NOARG(SlideBackground, PaperSizeModifyHdl)
{
    sal_uInt32 nPos = mpPaperSizeBox->GetSelectEntryPos();
    Paper ePaper = (Paper)reinterpret_cast<sal_uLong>(mpPaperSizeBox->GetEntryData(nPos));
    Size  aSize(SvxPaperInfo::GetPaperSize(ePaper));
    if(mpPaperOrientation->GetSelectEntryPos() == 0)
        Swap(aSize);
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD,&aSizeItem,0L);
    return 0;
}
IMPL_LINK_NOARG(SlideBackground, FillColorHdl)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    switch(eXFS)
    {
        case drawing::FillStyle_SOLID:
        {
            XFillColorItem aItem(OUString(), mpFillLB->GetSelectEntryColor());
            GetBindings()->GetDispatcher()->Execute(SID_PAGEBACKGROUND_COLOR, SfxCallMode::RECORD, &aItem, 0L);
        }
        break;
        case drawing::FillStyle_GRADIENT:
        {
            const XFillStyleItem aFillStyleItem(drawing::FillStyle_GRADIENT);
            XGradient aGradient;
            aGradient.SetStartColor(mpFillLB->GetSelectEntryColor());
            aGradient.SetEndColor(mpFillGrad->GetSelectEntryColor());
            XFillGradientItem aItem(mpFillStyle->GetSelectEntry(),aGradient);
            GetBindings()->GetDispatcher()->Execute(SID_PAGEBACKGROUND_GRADIENT, SfxCallMode::RECORD, &aFillStyleItem ,&aItem, 0l);
        }
        break;
        default:
        break;
    }
    return 0;
}
IMPL_LINK_NOARG(SlideBackground, FillBackgroundHdl)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(eXFS)
    {

        case drawing::FillStyle_HATCH:
        {
            //XFillBackgroundItem aBackgroundItem(true);
            const XFillStyleItem aFillStyleItem(drawing::FillStyle_HATCH);
            const SvxHatchListItem aHatchListItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
            sal_uInt16 nPos = mpFillAttr->GetSelectEntryPos();
            XHatch aHatch = aHatchListItem.GetHatchList()->GetHatch(nPos)->GetHatch();
            XFillHatchItem aItem(mpFillStyle->GetSelectEntry(), aHatch);
            GetBindings()->GetDispatcher()->Execute(SID_PAGEBACKGROUND_HATCH, SfxCallMode::RECORD, &aItem, &aFillStyleItem, 0L);
        }
        break;

        case drawing::FillStyle_BITMAP:
        {
            const XFillStyleItem aFillStyleItem(drawing::FillStyle_BITMAP);
            SvxBitmapListItem aBitmapListItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
            sal_Int16 nPos = mpFillAttr->GetSelectEntryPos();
            GraphicObject aBitmap = aBitmapListItem.GetBitmapList()->GetBitmap(nPos)->GetGraphicObject();
            XFillBitmapItem aItem(mpFillStyle->GetSelectEntry(), aBitmap);
            GetBindings()->GetDispatcher()->Execute(SID_PAGEBACKGROUND_BITMAP, SfxCallMode::RECORD,&aFillStyleItem, &aItem, 0L);
        }
        break;

    }
    return 0;
}
IMPL_LINK_NOARG(SlideBackground, AssignMasterPage)
{
    ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
    SdDrawDocument* mpDoc = pDocSh->GetDoc();
    sal_uInt16 nSelectedPage = SDRPAGE_NOTFOUND;
    for( sal_uInt16 nPage = 0; nPage < mpDoc->GetSdPageCount(PK_STANDARD); nPage++ )
    {
        if(mpDoc->GetSdPage(nPage,PK_STANDARD)->IsSelected())
        {
            nSelectedPage = nPage;
            break;
        }
    }
    OUString aLayoutName(mpMasterSlide->GetSelectEntry());
    mpDoc->SetMasterPage(nSelectedPage, aLayoutName, mpDoc, false, false);
    return 0;
}

IMPL_LINK_NOARG(SlideBackground, DspBackground)
{
    bool IsChecked = (mpDspMasterBackground->IsChecked() ? true : false);
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_BACKGROUND, IsChecked);
    GetBindings()->GetDispatcher()->Execute(SID_DISPLAY_MASTER_BACKGROUND, SfxCallMode::RECORD, &aBoolItem, 0l);
    return 0;
}

IMPL_LINK_NOARG(SlideBackground, DspObjects)
{
    bool IsChecked = (mpDspMasterObjects->IsChecked() ? true : false);
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_OBJECTS,IsChecked);
    GetBindings()->GetDispatcher()->Execute(SID_DISPLAY_MASTER_OBJECTS, SfxCallMode::RECORD, &aBoolItem, &aBoolItem, 0l);
    return 0;
}

}}
