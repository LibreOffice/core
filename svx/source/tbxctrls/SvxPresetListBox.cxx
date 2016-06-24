#include <svx/SvxPresetListBox.hxx>
#include <svx/xtable.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/image.hxx>

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

sal_uInt32 SvxPresetListBox::getEntryEdgeLength()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    return rStyleSettings.GetListBoxPreviewDefaultPixelSize().Height() + 1;
}

sal_uInt32 SvxPresetListBox::getColumnCount()
{
    return 2;
}

void SvxPresetListBox::setColumnCount( sal_uInt32 nPos )
{
}

void SvxPresetListBox::FillPresetListBox(const XPropertyList &pList, sal_uInt32 nStartIndex)
{
    const sal_uInt32 nCount = pList.Count();

    for(sal_uInt32 nIndex(0); nIndex < nCount;nIndex++, nStartIndex++)
    {
        const XPropertyEntry* pEntry = pList.Get( nIndex );

        if(pEntry)
        {
            BitmapEx aBmp(pEntry->GetUiBitmap());
            aBmp.SetSizePixel(Size(80,40), BmpScaleFlag::BestQuality);
            InsertItem(nStartIndex, Image(aBmp), pEntry->GetName());
        }
    }
}
