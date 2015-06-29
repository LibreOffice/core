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
#include <svx/xattr.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;


namespace sd { namespace sidebar {

SlideBackground::SlideBackground(
    Window * pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings
     ) :
        PanelLayout( pParent, "SlideBackground", "modules/simpress/ui/sidebarslidebackground.ui", rxFrame ),
        maPaperController(SID_ATTR_PAGE_SIZE, *pBindings, *this),
        mpBindings(pBindings)
{
    get(mpPaperSizeBox,"paperformat");
    get(mpPaperOrientation, "orientation");
    get(mpMasterSlide, "masterslide");
    get(mpFillAttr, "fillattr1");
    get(mpFillGrad, "fillattr2");
    get(mpFillStyle, "fillstyle");
    get(mpFillLB, "fillattr");
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
    const SvxGradientListItem aGradientItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));
    const SvxColorListItem aColorItem(*static_cast<const SvxColorListItem*>(pSh->GetItem(SID_COLOR_TABLE)));
    mpFillGrad->Clear();
    mpFillGrad->Clear();
    mpFillLB->Clear();
    mpFillGrad->Fill(aColorItem.GetColorList());
    mpFillAttr->Fill(aGradientItem.GetGradientList());
    mpFillLB->Fill(aColorItem.GetColorList());
}

void SlideBackground::dispose()
{
    mpPaperSizeBox.clear();
    mpPaperOrientation.clear();
    mpMasterSlide.clear();
    mpBspWin.clear();
    mpFillAttr.clear();
    mpFillGrad.clear();
    mpFillStyle.clear();
    mpFillLB.clear();
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
        case SID_PAGESETUP:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                //const XFill
            }
        }
        break;
    }
}
IMPL_LINK_NOARG(SlideBackground, FillStyleModifyHdl)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    const XFillStyleItem aXFillStyleItem(eXFS);
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(eXFS)
    {
        case drawing::FillStyle_NONE:
        {
            mpFillLB->Hide();
            mpFillAttr->Hide();
            mpFillGrad->Hide();
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aXFillStyleItem, 0l);
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
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
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
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
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
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
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
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aXFillStyleItem, 0l);
        }
        break;
    }
    return 0;
}
IMPL_LINK_NOARG(SlideBackground, PaperSizeModifyHdl)
{
    sal_uInt32 nPos = mpPaperSizeBox->GetSelectEntryPos();
    Paper ePaper = (Paper)reinterpret_cast<sal_uLong>(mpPaperSizeBox->GetEntryData(nPos));
    Size  aSize(SvxPaperInfo::GetPaperSize(ePaper, MAP_100TH_MM));
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
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aItem, 0L);
        }
        break;
        case drawing::FillStyle_GRADIENT:
        {
            XGradient aGradient;
            aGradient.SetStartColor(mpFillLB->GetSelectEntryColor());
            aGradient.SetEndColor(mpFillGrad->GetSelectEntryColor());
            XFillGradientItem aItem(mpFillStyle->GetSelectEntry(),aGradient);
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aItem, 0l);
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
    switch(eXFS)
    {
/*
        case drawing::FillStyle_HATCH:
        {
            XFillHatchItem aItem(OUString(), mpFillAttr->GetSelectEntry());
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aItem, 0L);
        }
        break;
        case drawing::FillStyle_BITMAP:
        {
            XFillBitmapItem aItem(OUString(), mpFillAttr->GetSelectEntry());
            GetBindings()->GetDispatcher()->Execute(SID_PAGESETUP, SfxCallMode::RECORD, &aItem, 0L);
        }
        break;
*/
    }
    return 0;
}
IMPL_LINK_NOARG(SlideBackground, AssignMasterPage)
{
    return 0;
}

}}
