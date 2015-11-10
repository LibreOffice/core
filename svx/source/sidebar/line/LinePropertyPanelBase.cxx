/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <svx/sidebar/LinePropertyPanelBase.hxx>
#include <LinePropertyPanel.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xtable.hxx>
#include <svx/xdash.hxx>
#include <svx/drawitem.hxx>
#include <svx/svxitems.hrc>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnwtit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include "svx/sidebar/PopupContainer.hxx"
#include "svx/sidebar/PopupControl.hxx"
#include "LineWidthControl.hxx"
#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;
using sfx2::sidebar::Theme;

const char UNO_SELECTWIDTH[] = ".uno:SelectWidth";

namespace
{

void FillLineEndListBox(ListBox& rListBoxStart, ListBox& rListBoxEnd, const XLineEndList& rList, const Bitmap& rBitmapZero)
{
    const sal_uInt32 nCount(rList.Count());
    const OUString sNone(SVX_RESSTR(RID_SVXSTR_NONE));

    rListBoxStart.SetUpdateMode(false);
    rListBoxEnd.SetUpdateMode(false);

    rListBoxStart.Clear();
    rListBoxEnd.Clear();

    for(sal_uInt32 i(0); i < nCount; i++)
    {
        XLineEndEntry* pEntry = rList.GetLineEnd(i);
        const Bitmap aBitmap = const_cast< XLineEndList& >(rList).GetUiBitmap(i);

        if(!aBitmap.IsEmpty())
        {
            Bitmap aCopyStart(aBitmap);
            Bitmap aCopyEnd(aBitmap);

            const Size aBmpSize(aCopyStart.GetSizePixel());
            const Rectangle aCropRectStart(Point(), Size(aBmpSize.Width() / 2, aBmpSize.Height()));
            const Rectangle aCropRectEnd(Point(aBmpSize.Width() / 2, 0), Size(aBmpSize.Width() / 2, aBmpSize.Height()));

            aCopyStart.Crop(aCropRectStart);
            rListBoxStart.InsertEntry(
                pEntry->GetName(),
                Image(aCopyStart));

            aCopyEnd.Crop(aCropRectEnd);
            rListBoxEnd.InsertEntry(
                pEntry->GetName(),
                Image(aCopyEnd));
        }
        else
        {
            rListBoxStart.InsertEntry(pEntry->GetName());
            rListBoxEnd.InsertEntry(pEntry->GetName());
        }
    }

    // add 'none' entries
    if (!rBitmapZero.IsEmpty())
    {
        const Image aImg = rListBoxStart.GetEntryImage(0);
        const Size aImgSize = aImg.GetSizePixel();

        // take solid line bitmap and crop it to the size of
        // line cap entries
        Bitmap aCopyZero( rBitmapZero );
        const Rectangle aCropZero( Point(), aImgSize );
        aCopyZero.Crop( aCropZero );

        // make it 1st item in list
        rListBoxStart.InsertEntry( sNone, Image(aCopyZero), 0);
        rListBoxEnd.InsertEntry( sNone, Image(aCopyZero), 0);
    }
    else
    {
       rListBoxStart.InsertEntry(sNone);
       rListBoxEnd.InsertEntry(sNone);
    }

    rListBoxStart.SetUpdateMode(true);
    rListBoxEnd.SetUpdateMode(true);
}

void FillLineStyleListBox(ListBox& rListBox, const XDashList& rList)
{
    const sal_uInt32 nCount(rList.Count());
    rListBox.SetUpdateMode(false);

    rListBox.Clear();

    // entry for 'none'
    rListBox.InsertEntry(rList.GetStringForUiNoLine());

    // entry for solid line
    rListBox.InsertEntry(rList.GetStringForUiSolidLine(),
            Image( rList.GetBitmapForUISolidLine()));

    for(sal_uInt32 i(0); i < nCount; i++)
    {
        XDashEntry* pEntry = rList.GetDash(i);
        const Bitmap aBitmap = const_cast< XDashList& >(rList).GetUiBitmap(i);

        if(!aBitmap.IsEmpty())
        {
            rListBox.InsertEntry(pEntry->GetName(), Image(aBitmap));
        }
        else
        {
            rListBox.InsertEntry(pEntry->GetName());
        }
    }

    rListBox.SetUpdateMode(true);
}

} // end of anonymous namespace

namespace svx { namespace sidebar {

LinePropertyPanelBase::LinePropertyPanelBase(
    vcl::Window* pParent,
    const uno::Reference<frame::XFrame>& rxFrame)
:   PanelLayout(pParent, "LinePropertyPanel", "svx/ui/sidebarline.ui", rxFrame),
    mpStyleItem(),
    mpDashItem(),
    mnTrans(0),
    meMapUnit(SFX_MAPUNIT_MM),
    mnWidthCoreValue(0),
    mpStartItem(),
    mpEndItem(),
    maLineWidthPopup(this, ::boost::bind(&LinePropertyPanelBase::CreateLineWidthPopupControl, this, _1)),
    maIMGNone(SVX_RES(IMG_NONE_ICON)),
    mpIMGWidthIcon(),
    mxFrame(rxFrame),
    mbWidthValuable(true),
    mbArrowSupported(true)
{
    get(mpFTWidth, "widthlabel");
    get(mpTBWidth, "width");
    get(mpTBColor, "color");
    get(mpLBStyle, "linestyle");
    get(mpFTTransparency, "translabel");
    get(mpMFTransparent, "linetransparency");
    get(mpLBStart, "beginarrowstyle");
    get(mpLBEnd, "endarrowstyle");
    get(mpFTEdgeStyle, "cornerlabel");
    get(mpLBEdgeStyle, "edgestyle");
    get(mpFTCapStyle, "caplabel");
    get(mpLBCapStyle, "linecapstyle");
    get(mpGridLineProps, "lineproperties");
    get(mpBoxArrowProps, "arrowproperties");

    Initialize();
}

LinePropertyPanelBase::~LinePropertyPanelBase()
{
    disposeOnce();
}

void LinePropertyPanelBase::dispose()
{
    mpFTWidth.clear();
    mpTBWidth.clear();
    mpTBColor.clear();
    mpLBStyle.clear();
    mpFTTransparency.clear();
    mpMFTransparent.clear();
    mpLBStart.clear();
    mpLBEnd.clear();
    mpFTEdgeStyle.clear();
    mpLBEdgeStyle.clear();
    mpFTCapStyle.clear();
    mpLBCapStyle.clear();
    mpGridLineProps.clear();
    mpBoxArrowProps.clear();

    PanelLayout::dispose();
}

void LinePropertyPanelBase::Initialize()
{
    mpIMGWidthIcon.reset(new Image[8]);
    mpIMGWidthIcon[0] = Image(SVX_RES(IMG_WIDTH1_ICON));
    mpIMGWidthIcon[1] = Image(SVX_RES(IMG_WIDTH2_ICON));
    mpIMGWidthIcon[2] = Image(SVX_RES(IMG_WIDTH3_ICON));
    mpIMGWidthIcon[3] = Image(SVX_RES(IMG_WIDTH4_ICON));
    mpIMGWidthIcon[4] = Image(SVX_RES(IMG_WIDTH5_ICON));
    mpIMGWidthIcon[5] = Image(SVX_RES(IMG_WIDTH6_ICON));
    mpIMGWidthIcon[6] = Image(SVX_RES(IMG_WIDTH7_ICON));
    mpIMGWidthIcon[7] = Image(SVX_RES(IMG_WIDTH8_ICON));

    FillLineStyleList();
    SelectLineStyle();
    mpLBStyle->SetSelectHdl( LINK( this, LinePropertyPanelBase, ChangeLineStyleHdl ) );
    mpLBStyle->SetAccessibleName("Style");
    mpLBStyle->AdaptDropDownLineCountToMaximum();

    const sal_uInt16 nIdWidth = mpTBWidth->GetItemId(UNO_SELECTWIDTH);
    mpTBWidth->SetItemImage(nIdWidth, mpIMGWidthIcon[0]);
    mpTBWidth->SetItemBits( nIdWidth, mpTBWidth->GetItemBits( nIdWidth ) | ToolBoxItemBits::DROPDOWNONLY );
    Link<ToolBox *, void> aLink2 = LINK(this, LinePropertyPanelBase, ToolboxWidthSelectHdl);
    mpTBWidth->SetDropdownClickHdl ( aLink2 );
    mpTBWidth->SetSelectHdl ( aLink2 );

    FillLineEndList();
    SelectEndStyle(true);
    SelectEndStyle(false);
    mpLBStart->SetSelectHdl( LINK( this, LinePropertyPanelBase, ChangeStartHdl ) );
    mpLBStart->SetAccessibleName("Beginning Style"); //wj acc
    mpLBStart->AdaptDropDownLineCountToMaximum();
    mpLBEnd->SetSelectHdl( LINK( this, LinePropertyPanelBase, ChangeEndHdl ) );
    mpLBEnd->SetAccessibleName("Ending Style");  //wj acc
    mpLBEnd->AdaptDropDownLineCountToMaximum();

    mpMFTransparent->SetModifyHdl(LINK(this, LinePropertyPanelBase, ChangeTransparentHdl));
    mpMFTransparent->SetAccessibleName("Transparency");  //wj acc

    mpTBWidth->SetAccessibleRelationLabeledBy(mpFTWidth);
    mpMFTransparent->SetAccessibleRelationLabeledBy(mpFTTransparency);
    mpLBEnd->SetAccessibleRelationLabeledBy(mpLBEnd);

    mpLBEdgeStyle->SetSelectHdl( LINK( this, LinePropertyPanelBase, ChangeEdgeStyleHdl ) );
    mpLBEdgeStyle->SetAccessibleName("Corner Style");

    mpLBCapStyle->SetSelectHdl( LINK( this, LinePropertyPanelBase, ChangeCapStyleHdl ) );
    mpLBCapStyle->SetAccessibleName("Cap Style");
}

void LinePropertyPanelBase::DataChanged(const DataChangedEvent& /*rEvent*/)
{
}

void LinePropertyPanelBase::updateLineStyle(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem)
{
    if(bDisabled)
    {
        mpLBStyle->Disable();
    }
    else
    {
        mpLBStyle->Enable();
    }

    if(bSetOrDefault)
    {
        if(pItem)
        {
            mpStyleItem.reset(static_cast<XLineStyleItem*>(pItem->Clone()));
        }
    }
    else
    {
        mpStyleItem.reset(nullptr);
    }

    SelectLineStyle();
}

void LinePropertyPanelBase::updateLineDash(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem)
{
    if(bDisabled)
    {
        mpLBStyle->Disable();
    }
    else
    {
        mpLBStyle->Enable();
    }

    if(bSetOrDefault)
    {
        if(pItem)
        {
            mpDashItem.reset(static_cast<XLineDashItem*>(pItem->Clone()));
        }
    }
    else
    {
        mpDashItem.reset();
    }

    SelectLineStyle();
}

void LinePropertyPanelBase::updateLineTransparence(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mpFTTransparency->Disable();
        mpMFTransparent->Disable();
    }
    else
    {
        mpFTTransparency->Enable();
        mpMFTransparent->Enable();
    }

    if(bSetOrDefault)
    {
        if (const XLineTransparenceItem* pItem = dynamic_cast<const XLineTransparenceItem*>(pState))
        {
            mnTrans = pItem->GetValue();
            mpMFTransparent->SetValue(mnTrans);
            return;
        }
    }

    mpMFTransparent->SetValue(0);//add
    mpMFTransparent->SetText(OUString());
}

void LinePropertyPanelBase::updateLineWidth(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mpTBWidth->Disable();
        mpFTWidth->Disable();
    }
    else
    {
        mpTBWidth->Enable();
        mpFTWidth->Enable();
    }

    if(bSetOrDefault)
    {
        if (const XLineWidthItem* pItem = dynamic_cast<const XLineWidthItem*>(pState))
        {
            mnWidthCoreValue = pItem->GetValue();
            mbWidthValuable = true;
            SetWidthIcon();
            return;
        }
    }

    mbWidthValuable = false;
    SetWidthIcon();
}

void LinePropertyPanelBase::updateLineStart(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pItem)
{
    if(bDisabled)
    {
        mpLBStart->Disable();
    }
    else
    {
        if (mbArrowSupported)
            mpLBStart->Enable();
    }

    if(bSetOrDefault)
    {
        if(pItem)
        {
            mpStartItem.reset(static_cast<XLineStartItem*>(pItem->Clone()));
            SelectEndStyle(true);
            return;
        }
    }

    mpStartItem.reset(nullptr);
    SelectEndStyle(true);
}

void LinePropertyPanelBase::updateLineEnd(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pItem)
{
    if(bDisabled)
    {
        mpLBEnd->Disable();
    }
    else
    {
        if (mbArrowSupported)
            mpLBEnd->Enable();
    }

    if(bSetOrDefault)
    {
        if(pItem)
        {
            mpEndItem.reset(static_cast<XLineEndItem*>(pItem->Clone()));
            SelectEndStyle(false);
            return;
        }
    }

    mpEndItem.reset(nullptr);
    SelectEndStyle(false);
}

void LinePropertyPanelBase::updateLineJoint(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mpLBEdgeStyle->Disable();
        mpFTEdgeStyle->Disable();
    }
    else
    {
        mpLBEdgeStyle->Enable();
        mpFTEdgeStyle->Enable();
    }

    if(bSetOrDefault)
    {
        if (const XLineJointItem* pItem = dynamic_cast<const XLineJointItem*>(pState))
        {
            sal_Int32 nEntryPos(0);

            switch(pItem->GetValue())
            {
                case drawing::LineJoint_MIDDLE:
                case drawing::LineJoint_ROUND:
                {
                    nEntryPos = 1;
                    break;
                }
                case drawing::LineJoint_NONE:
                {
                    nEntryPos = 2;
                    break;
                }
                case drawing::LineJoint_MITER:
                {
                    nEntryPos = 3;
                    break;
                }
                case drawing::LineJoint_BEVEL:
                {
                    nEntryPos = 4;
                    break;
                }

                default:
                break;
            }

            if(nEntryPos)
            {
                mpLBEdgeStyle->SelectEntryPos(nEntryPos - 1);
                return;
            }
        }
    }

    mpLBEdgeStyle->SetNoSelection();
}

void LinePropertyPanelBase::updateLineCap(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mpLBCapStyle->Disable();
        mpFTCapStyle->Disable();
    }
    else
    {
        mpLBCapStyle->Enable();
        mpLBCapStyle->Enable();
    }

    if(bSetOrDefault)
    {
        if (const XLineCapItem* pItem = dynamic_cast<const XLineCapItem*>(pState))
        {
            sal_Int32 nEntryPos(0);

            switch(pItem->GetValue())
            {
                case drawing::LineCap_BUTT:
                {
                    nEntryPos = 1;
                    break;
                }
                case drawing::LineCap_ROUND:
                {
                    nEntryPos = 2;
                    break;
                }
                case drawing::LineCap_SQUARE:
                {
                    nEntryPos = 3;
                    break;
                }

                default:
                break;
            }

            if(nEntryPos)
            {
                mpLBCapStyle->SelectEntryPos(nEntryPos - 1);
                return;
            }
        }
    }

    mpLBCapStyle->SetNoSelection();
}

IMPL_LINK_NOARG_TYPED(LinePropertyPanelBase, ChangeLineStyleHdl, ListBox&, void)
{
    const sal_Int32 nPos(mpLBStyle->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos && mpLBStyle->IsValueChangedFromSaved())
    {
        if(0 == nPos)
        {
            // drawing::LineStyle_NONE
            const XLineStyleItem aItem(drawing::LineStyle_NONE);

            setLineStyle(aItem);
        }
        else if(1 == nPos)
        {
            // drawing::LineStyle_SOLID
            const XLineStyleItem aItem(drawing::LineStyle_SOLID);

            setLineStyle(aItem);
        }
        else if (mxLineStyleList.is() && mxLineStyleList->Count() > (long)(nPos - 2))
        {
            // drawing::LineStyle_DASH
            const XLineStyleItem aItemA(drawing::LineStyle_DASH);
            const XDashEntry* pDashEntry = mxLineStyleList->GetDash(nPos - 2);
            OSL_ENSURE(pDashEntry, "OOps, got empty XDash from XDashList (!)");
            const XLineDashItem aItemB(
                pDashEntry ? pDashEntry->GetName() : OUString(),
                pDashEntry ? pDashEntry->GetDash() : XDash());

            setLineStyle(aItemA);
            setLineDash(aItemB);
        }
    }

    ActivateControls();
}

IMPL_LINK_NOARG_TYPED(LinePropertyPanelBase, ChangeStartHdl, ListBox&, void)
{
    sal_Int32  nPos = mpLBStart->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND && mpLBStart->IsValueChangedFromSaved() )
    {
        std::unique_ptr<XLineStartItem> pItem;
        if( nPos == 0 )
            pItem.reset(new XLineStartItem());
        else if( mxLineEndList.is() && mxLineEndList->Count() > (long) ( nPos - 1 ) )
            pItem.reset(new XLineStartItem( mpLBStart->GetSelectEntry(),mxLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
        setLineStartStyle(pItem.get());
    }
}

IMPL_LINK_NOARG_TYPED(LinePropertyPanelBase, ChangeEndHdl, ListBox&, void)
{
    sal_Int32  nPos = mpLBEnd->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND && mpLBEnd->IsValueChangedFromSaved() )
    {
        std::unique_ptr<XLineEndItem> pItem;
        if( nPos == 0 )
            pItem.reset(new XLineEndItem());
        else if( mxLineEndList.is() && mxLineEndList->Count() > (long) ( nPos - 1 ) )
            pItem.reset(new XLineEndItem( mpLBEnd->GetSelectEntry(), mxLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
        setLineEndStyle(pItem.get());
    }
}

IMPL_LINK_NOARG_TYPED(LinePropertyPanelBase, ChangeEdgeStyleHdl, ListBox&, void)
{
    const sal_Int32 nPos(mpLBEdgeStyle->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos && mpLBEdgeStyle->IsValueChangedFromSaved())
    {
        std::unique_ptr<XLineJointItem> pItem;

        switch(nPos)
        {
            case 0: // rounded
            {
                pItem.reset(new XLineJointItem(drawing::LineJoint_ROUND));
                break;
            }
            case 1: // none
            {
                pItem.reset(new XLineJointItem(drawing::LineJoint_NONE));
                break;
            }
            case 2: // mitered
            {
                pItem.reset(new XLineJointItem(drawing::LineJoint_MITER));
                break;
            }
            case 3: // beveled
            {
                pItem.reset(new XLineJointItem(drawing::LineJoint_BEVEL));
                break;
            }
        }

        setLineJoint(pItem.get());
    }
}

IMPL_LINK_NOARG_TYPED(LinePropertyPanelBase, ChangeCapStyleHdl, ListBox&, void)
{
    const sal_Int32 nPos(mpLBCapStyle->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos && mpLBCapStyle->IsValueChangedFromSaved())
    {
        std::unique_ptr<XLineCapItem> pItem;

        switch(nPos)
        {
            case 0: // flat
            {
                pItem.reset(new XLineCapItem(drawing::LineCap_BUTT));
                break;
            }
            case 1: // round
            {
                pItem.reset(new XLineCapItem(drawing::LineCap_ROUND));
                break;
            }
            case 2: // square
            {
                pItem.reset(new XLineCapItem(drawing::LineCap_SQUARE));
                break;
            }
        }

        setLineCap(pItem.get());
    }
}

IMPL_LINK_TYPED(LinePropertyPanelBase, ToolboxWidthSelectHdl,ToolBox*, pToolBox, void)
{
    if (pToolBox->GetItemCommand(pToolBox->GetCurItemId()) == UNO_SELECTWIDTH)
    {
        maLineWidthPopup.SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
        maLineWidthPopup.Show(*pToolBox);
    }
}

IMPL_LINK_NOARG_TYPED( LinePropertyPanelBase, ChangeTransparentHdl, Edit&, void )
{
    sal_uInt16 nVal = (sal_uInt16)mpMFTransparent->GetValue();
    XLineTransparenceItem aItem( nVal );

    setLineTransparency(aItem);
}

VclPtr<PopupControl> LinePropertyPanelBase::CreateLineWidthPopupControl (PopupContainer* pParent)
{
    return VclPtrInstance<LineWidthControl>(pParent, *this);
}

void LinePropertyPanelBase::EndLineWidthPopupMode()
{
    maLineWidthPopup.Hide();
}

void LinePropertyPanelBase::SetWidthIcon(int n)
{
    const sal_uInt16 nIdWidth = mpTBWidth->GetItemId(UNO_SELECTWIDTH);
    if (n == 0)
        mpTBWidth->SetItemImage( nIdWidth, maIMGNone);
    else
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[n-1]);
}

void LinePropertyPanelBase::SetWidthIcon()
{
    if(!mbWidthValuable)
    {
        const sal_uInt16 nIdWidth = mpTBWidth->GetItemId(UNO_SELECTWIDTH);
        mpTBWidth->SetItemImage(nIdWidth, maIMGNone);
        return;
    }

    long nVal = LogicToLogic(mnWidthCoreValue * 10,(MapUnit)meMapUnit , MAP_POINT);
    const sal_uInt16 nIdWidth = mpTBWidth->GetItemId(UNO_SELECTWIDTH);

    if(nVal <= 6)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[0]);
    else if(nVal > 6 && nVal <= 9)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[1]);
    else if(nVal > 9 && nVal <= 12)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[2]);
    else if(nVal > 12 && nVal <= 19)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[3]);
    else if(nVal > 19 && nVal <= 26)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[4]);
    else if(nVal > 26 && nVal <= 37)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[5]);
    else if(nVal > 37 && nVal <=52)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[6]);
    else if(nVal > 52)
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[7]);

}

void LinePropertyPanelBase::SetWidth(long nWidth)
{
    mnWidthCoreValue = nWidth;
    mbWidthValuable = true;
}

void  LinePropertyPanelBase::FillLineEndList()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh && pSh->GetItem( SID_LINEEND_LIST ) )
    {
        mpLBStart->Enable();
        SvxLineEndListItem aItem( *static_cast<const SvxLineEndListItem*>(pSh->GetItem( SID_LINEEND_LIST ) ) );
        mxLineEndList = aItem.GetLineEndList();

        if (mxLineEndList.is())
        {
            Bitmap aZeroBitmap;

            if (mxLineStyleList.is())
                aZeroBitmap = mxLineStyleList->GetBitmapForUISolidLine();

            FillLineEndListBox(*mpLBStart, *mpLBEnd, *mxLineEndList, aZeroBitmap);
        }

        mpLBStart->SelectEntryPos(0);
        mpLBEnd->SelectEntryPos(0);
    }
    else
    {
        mpLBStart->Disable();
        mpLBEnd->Disable();
    }
}

void  LinePropertyPanelBase::FillLineStyleList()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh && pSh->GetItem( SID_DASH_LIST ) )
    {
        mpLBStyle->Enable();
        SvxDashListItem aItem( *static_cast<const SvxDashListItem*>(pSh->GetItem( SID_DASH_LIST ) ) );
        mxLineStyleList = aItem.GetDashList();

        if (mxLineStyleList.is())
        {
            FillLineStyleListBox(*mpLBStyle, *mxLineStyleList);
        }

        mpLBStyle->SelectEntryPos(0);
    }
    else
    {
        mpLBStyle->Disable();
    }
}

void LinePropertyPanelBase::SelectLineStyle()
{
    if( !mpStyleItem.get() || !mpDashItem.get() )
    {
        mpLBStyle->SetNoSelection();
        mpLBStyle->Disable();
        return;
    }

    const drawing::LineStyle eXLS(mpStyleItem ? (drawing::LineStyle)mpStyleItem->GetValue() : drawing::LineStyle_NONE);
    bool bSelected(false);

    switch(eXLS)
    {
        case drawing::LineStyle_NONE:
            break;
        case drawing::LineStyle_SOLID:
            mpLBStyle->SelectEntryPos(1);
            bSelected = true;
            break;
        default:
            if(mpDashItem && mxLineStyleList.is())
            {
                const XDash& rDash = mpDashItem->GetDashValue();
                for(sal_Int32 a(0);!bSelected &&  a < mxLineStyleList->Count(); a++)
                {
                    XDashEntry* pEntry = mxLineStyleList->GetDash(a);
                    const XDash& rEntry = pEntry->GetDash();
                    if(rDash == rEntry)
                    {
                        mpLBStyle->SelectEntryPos(a + 2);
                        bSelected = true;
                    }
                }
            }
            break;
    }

    if(!bSelected)
        mpLBStyle->SelectEntryPos( 0 );

    ActivateControls();
}

void LinePropertyPanelBase::SelectEndStyle(bool bStart)
{
    bool bSelected(false);

    if(bStart)
    {
        if( !mpStartItem.get() )
        {
            mpLBStart->SetNoSelection();
            mpLBStart->Disable();
            return;
        }

        if (mpStartItem && mxLineEndList.is())
        {
            const basegfx::B2DPolyPolygon& rItemPolygon = mpStartItem->GetLineStartValue();
            for(sal_Int32 a(0);!bSelected &&  a < mxLineEndList->Count(); a++)
            {
                XLineEndEntry* pEntry = mxLineEndList->GetLineEnd(a);
                const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();
                if(rItemPolygon == rEntryPolygon)
                {
                    mpLBStart->SelectEntryPos(a + 1);
                    bSelected = true;
                }
            }
        }

        if(!bSelected)
        {
            mpLBStart->SelectEntryPos( 0 );
        }
    }
    else
    {
        if( !mpEndItem.get() )
        {
            mpLBEnd->SetNoSelection();
            mpLBEnd->Disable();
            return;
        }

        if (mpEndItem && mxLineEndList.is())
        {
            const basegfx::B2DPolyPolygon& rItemPolygon = mpEndItem->GetLineEndValue();
            for(sal_Int32 a(0);!bSelected &&  a < mxLineEndList->Count(); a++)
            {
                XLineEndEntry* pEntry = mxLineEndList->GetLineEnd(a);
                const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();
                if(rItemPolygon == rEntryPolygon)
                {
                    mpLBEnd->SelectEntryPos(a + 1);
                    bSelected = true;
                }
            }
        }

        if(!bSelected)
        {
            mpLBEnd->SelectEntryPos( 0 );
        }
    }
}

void LinePropertyPanelBase::ActivateControls()
{
    const sal_Int32 nPos(mpLBStyle->GetSelectEntryPos());
    bool bLineStyle( nPos != 0 );

    mpGridLineProps->Enable( bLineStyle );
    mpBoxArrowProps->Enable( bLineStyle );
    mpLBStart->Enable( bLineStyle );
    mpLBEnd->Enable( bLineStyle );
}

void LinePropertyPanelBase::setMapUnit(SfxMapUnit eMapUnit)
{
    meMapUnit = eMapUnit;
}

void LinePropertyPanelBase::disableArrowHead()
{
    mbArrowSupported = false;
    mpLBStart->Hide();
    mpLBEnd->Hide();
}

}} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
