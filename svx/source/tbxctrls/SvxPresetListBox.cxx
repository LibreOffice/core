#include <svx/SvxPresetListBox.hxx>
#include <svx/xtable.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/image.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <vcl/menu.hxx>
#include <vcl/popupmenuwindow.hxx>

SvxPresetListBox::SvxPresetListBox(vcl::Window* pParent, WinBits nWinStyle)
    : ValueSet(pParent, nWinStyle)
{
    SetEdgeBlending(true);
    SetExtraSpacing(2);
}

VCL_BUILDER_DECL_FACTORY(SvxPresetListBox)
{
    WinBits nWinBits = WB_TABSTOP;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if(!sBorder.isEmpty())
        nWinBits |= WB_BORDER;
    nWinBits |= (WB_ITEMBORDER| WB_3DLOOK|WB_OWNERDRAWDECORATION|WB_DOUBLEBORDER);
    rRet = VclPtr<SvxPresetListBox>::Create(pParent, nWinBits);
}

void SvxPresetListBox::Resize()
{
    layoutToGivenHeight(GetOutputSizePixel().Height(), GetItemCount());
    ValueSet::Resize();
}

void SvxPresetListBox::Command( const CommandEvent& rEvent )
{
    switch(rEvent.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            const sal_uInt16 nIndex = GetSelectItemId();
            if(nIndex > 0)
            {
                Rectangle aBBox(GetItemRect(nIndex));
                Point aPos = aBBox.Center();
                ScopedVclPtrInstance<PopupMenu> pMenu(SVX_RES(RID_SVX_PRESET_MENU));
                FloatingWindow* pMenuWindow = dynamic_cast<FloatingWindow*>(pMenu->GetWindow());
                if(pMenuWindow != nullptr)
                {
                    pMenuWindow->SetPopupModeFlags(
                    pMenuWindow->GetPopupModeFlags() | FloatWinPopupFlags::NoMouseUpClose);
                }
                pMenu->Execute(this,Rectangle(aPos,Size(1,1)),PopupMenuFlags::ExecuteDown);
            }
        }
        break;
        default:
            ValueSet::Command( rEvent );
            break;
    }
}

void SvxPresetListBox::layoutToGivenHeight(sal_uInt32 nHeight, sal_uInt32 nEntryCount)
{
    SetStyle(GetStyle()|WB_VSCROLL);
    SetColCount(getColumnCount());
    SetLineCount(5);
}

sal_uInt32 SvxPresetListBox::getMaxRowCount()
{
    return 10;
}

void SvxPresetListBox::FillPresetListBox(XHatchList& pList, sal_uInt32 nStartIndex)
{
    const sal_uInt32 nCount = pList.Count();
    const Size aSize(80,40);
    BitmapEx aBitmap;
    for(sal_uInt32 nIndex(0); nIndex < nCount; nIndex++, nStartIndex++)
    {
        aBitmap = pList.GetBitmapForPreview(nIndex, aSize);
        InsertItem(nStartIndex, Image(aBitmap));
    }
}
