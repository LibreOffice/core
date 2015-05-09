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
#include <LinePropertyPanel.hxx>
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

void FillLineEndListBox(ListBox& rListBoxStart, ListBox& rListBoxEnd, const XLineEndList& rList)
{
    const sal_uInt32 nCount(rList.Count());
    const OUString sNone(SVX_RESSTR(RID_SVXSTR_NONE));

    rListBoxStart.SetUpdateMode(false);
    rListBoxEnd.SetUpdateMode(false);

    rListBoxStart.Clear();
    rListBoxEnd.Clear();

    // add 'none' entries
    rListBoxStart.InsertEntry(sNone);
    rListBoxEnd.InsertEntry(sNone);

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

LinePropertyPanel::LinePropertyPanel(
    vcl::Window* pParent,
    const uno::Reference<frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "LinePropertyPanel", "svx/ui/sidebarline.ui", rxFrame),
    maStyleControl(SID_ATTR_LINE_STYLE, *pBindings, *this),
    maDashControl (SID_ATTR_LINE_DASH, *pBindings, *this),
    maWidthControl(SID_ATTR_LINE_WIDTH, *pBindings, *this),
    maStartControl(SID_ATTR_LINE_START, *pBindings, *this),
    maEndControl(SID_ATTR_LINE_END, *pBindings, *this),
    maLineEndListControl(SID_LINEEND_LIST, *pBindings, *this),
    maLineStyleListControl(SID_DASH_LIST, *pBindings, *this),
    maTransControl(SID_ATTR_LINE_TRANSPARENCE, *pBindings, *this),
    maEdgeStyle(SID_ATTR_LINE_JOINT, *pBindings, *this),
    maCapStyle(SID_ATTR_LINE_CAP, *pBindings, *this),
    mpStyleItem(),
    mpDashItem(),
    mnTrans(0),
    meMapUnit(SFX_MAPUNIT_MM),
    mnWidthCoreValue(0),
    mpStartItem(),
    mpEndItem(),
    maLineWidthPopup(this, ::boost::bind(&LinePropertyPanel::CreateLineWidthPopupControl, this, _1)),
    maIMGNone(SVX_RES(IMG_NONE_ICON)),
    mpIMGWidthIcon(),
    mxFrame(rxFrame),
    mpBindings(pBindings),
    mbWidthValuable(true)
{
    get(mpFTWidth, "widthlabel");
    get(mpTBWidth, "width");
    get(mpFTStyle, "stylelabel");
    get(mpLBStyle, "linestyle");
    get(mpFTTransparency, "translabel");
    get(mpMFTransparent, "linetransparency");
    get(mpFTArrow, "arrowlabel");
    get(mpLBStart, "beginarrowstyle");
    get(mpLBEnd, "endarrowstyle");
    get(mpFTEdgeStyle, "cornerlabel");
    get(mpLBEdgeStyle, "edgestyle");
    get(mpFTCapStyle, "caplabel");
    get(mpLBCapStyle, "linecapstyle");

    Initialize();
}

LinePropertyPanel::~LinePropertyPanel()
{
    disposeOnce();
}

void LinePropertyPanel::dispose()
{
    mpFTWidth.clear();
    mpTBWidth.clear();
    mpFTStyle.clear();
    mpLBStyle.clear();
    mpFTTransparency.clear();
    mpMFTransparent.clear();
    mpFTArrow.clear();
    mpLBStart.clear();
    mpLBEnd.clear();
    mpFTEdgeStyle.clear();
    mpLBEdgeStyle.clear();
    mpFTCapStyle.clear();
    mpLBCapStyle.clear();

    maStyleControl.dispose();
    maDashControl.dispose();
    maWidthControl.dispose();
    maStartControl.dispose();
    maEndControl.dispose();
    maLineEndListControl.dispose();
    maLineStyleListControl.dispose();
    maTransControl.dispose();
    maEdgeStyle.dispose();
    maCapStyle.dispose();

    PanelLayout::dispose();
}

void LinePropertyPanel::Initialize()
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

    meMapUnit = maWidthControl.GetCoreMetric();

    FillLineStyleList();
    SelectLineStyle();
    Link<> aLink = LINK( this, LinePropertyPanel, ChangeLineStyleHdl );
    mpLBStyle->SetSelectHdl( aLink );
    mpLBStyle->SetAccessibleName(OUString( "Style"));
    mpLBStyle->AdaptDropDownLineCountToMaximum();

    const sal_uInt16 nIdWidth = mpTBWidth->GetItemId(UNO_SELECTWIDTH);
    mpTBWidth->SetItemImage(nIdWidth, mpIMGWidthIcon[0]);
    mpTBWidth->SetItemBits( nIdWidth, mpTBWidth->GetItemBits( nIdWidth ) | ToolBoxItemBits::DROPDOWNONLY );
    Link<ToolBox *, void> aLink2 = LINK(this, LinePropertyPanel, ToolboxWidthSelectHdl);
    mpTBWidth->SetDropdownClickHdl ( aLink2 );
    mpTBWidth->SetSelectHdl ( aLink2 );

    FillLineEndList();
    SelectEndStyle(true);
    SelectEndStyle(false);
    aLink = LINK( this, LinePropertyPanel, ChangeStartHdl );
    mpLBStart->SetSelectHdl( aLink );
    mpLBStart->SetAccessibleName(OUString("Beginning Style")); //wj acc
    mpLBStart->AdaptDropDownLineCountToMaximum();
    aLink = LINK( this, LinePropertyPanel, ChangeEndHdl );
    mpLBEnd->SetSelectHdl( aLink );
    mpLBEnd->SetAccessibleName(OUString("Ending Style"));  //wj acc
    mpLBEnd->AdaptDropDownLineCountToMaximum();

    aLink = LINK(this, LinePropertyPanel, ChangeTransparentHdl);
    mpMFTransparent->SetModifyHdl(aLink);
    mpMFTransparent->SetAccessibleName(OUString("Transparency"));  //wj acc

    mpTBWidth->SetAccessibleRelationLabeledBy(mpFTWidth);
    mpLBStyle->SetAccessibleRelationLabeledBy(mpFTStyle);
    mpMFTransparent->SetAccessibleRelationLabeledBy(mpFTTransparency);
    mpLBStart->SetAccessibleRelationLabeledBy(mpFTArrow);
    mpLBEnd->SetAccessibleRelationLabeledBy(mpLBEnd);

    aLink = LINK( this, LinePropertyPanel, ChangeEdgeStyleHdl );
    mpLBEdgeStyle->SetSelectHdl( aLink );
    mpLBEdgeStyle->SetAccessibleName(OUString("Corner Style"));

    aLink = LINK( this, LinePropertyPanel, ChangeCapStyleHdl );
    mpLBCapStyle->SetSelectHdl( aLink );
    mpLBCapStyle->SetAccessibleName(OUString("Cap Style"));
}

void LinePropertyPanel::SetupIcons()
{
    if(Theme::GetBoolean(Theme::Bool_UseSymphonyIcons))
    {
        // todo
    }
    else
    {
        // todo
    }
}

VclPtr<vcl::Window> LinePropertyPanel::Create (
    vcl::Window* pParent,
    const uno::Reference<frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to LinePropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to LinePropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to LinePropertyPanel::Create", NULL, 2);

    return VclPtr<LinePropertyPanel>::Create(pParent, rxFrame, pBindings);
}

void LinePropertyPanel::DataChanged(const DataChangedEvent& /*rEvent*/)
{
    SetupIcons();
}

void LinePropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool /*bIsEnabled*/)
{
    const bool bDisabled(SfxItemState::DISABLED == eState);

    switch(nSID)
    {
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_STYLE:
        {
            if(bDisabled)
            {
                mpFTStyle->Disable();
                mpLBStyle->Disable();
            }
            else
            {
                mpFTStyle->Enable();
                mpLBStyle->Enable();
            }

            if(eState  >= SfxItemState::DEFAULT)
            {
                if(nSID == SID_ATTR_LINE_STYLE)
                {
                    const XLineStyleItem* pItem = dynamic_cast< const XLineStyleItem* >(pState);

                    if(pItem)
                    {
                        mpStyleItem.reset(pState ? static_cast<XLineStyleItem*>(pItem->Clone()) : 0);
                    }
                }
                else // if(nSID == SID_ATTR_LINE_DASH)
                {
                    const XLineDashItem* pItem = dynamic_cast< const XLineDashItem* >(pState);

                    if(pItem)
                    {
                        mpDashItem.reset(pState ? static_cast<XLineDashItem*>(pItem->Clone()) : 0);
                    }
                }
            }
            else
            {
                if(nSID == SID_ATTR_LINE_STYLE)
                {
                    mpStyleItem.reset(0);
                }
                else
                {
                    mpDashItem.reset(0);
                }
            }

            SelectLineStyle();
            break;
        }
        case SID_ATTR_LINE_TRANSPARENCE:
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

            if(eState >= SfxItemState::DEFAULT)
            {
                const XLineTransparenceItem* pItem = dynamic_cast< const XLineTransparenceItem* >(pState);

                if(pItem)
                {
                    mnTrans = pItem->GetValue();
                    mpMFTransparent->SetValue(mnTrans);
                    break;
                }
            }

            mpMFTransparent->SetValue(0);//add
            mpMFTransparent->SetText(OUString());
            break;
        }
        case SID_ATTR_LINE_WIDTH:
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

            if(eState >= SfxItemState::DEFAULT)
            {
                const XLineWidthItem* pItem = dynamic_cast< const XLineWidthItem* >(pState);

                if(pItem)
                {
                    mnWidthCoreValue = pItem->GetValue();
                    mbWidthValuable = true;
                    SetWidthIcon();
                    break;
                }
            }

            mbWidthValuable = false;
            SetWidthIcon();
            break;
        }
        case SID_ATTR_LINE_START:
        {
            if(bDisabled)
            {
                mpFTArrow->Disable();
                mpLBStart->Disable();
            }
            else
            {
                mpFTArrow->Enable();
                mpLBStart->Enable();
            }

            if(eState >= SfxItemState::DEFAULT)
            {
                const XLineStartItem* pItem = dynamic_cast< const XLineStartItem* >(pState);

                if(pItem)
                {
                    mpStartItem.reset(static_cast<XLineStartItem*>(pItem->Clone()));
                    SelectEndStyle(true);
                    break;
                }
            }

            mpStartItem.reset(0);
            SelectEndStyle(true);
            break;
        }
        case SID_ATTR_LINE_END:
        {
            if(bDisabled)
            {
                mpFTArrow->Disable();
                mpLBEnd->Disable();
            }
            else
            {
                mpFTArrow->Enable();
                mpLBEnd->Enable();
            }

            if(eState >= SfxItemState::DEFAULT)
            {
                const XLineEndItem* pItem = dynamic_cast< const XLineEndItem* >(pState);

                if(pItem)
                {
                    mpEndItem.reset(static_cast<XLineEndItem*>(pItem->Clone()));
                    SelectEndStyle(false);
                    break;
                }
            }

            mpEndItem.reset(0);
            SelectEndStyle(false);
            break;
        }
        case SID_LINEEND_LIST:
        {
            FillLineEndList();
            SelectEndStyle(true);
            SelectEndStyle(false);
            break;
        }
        case SID_DASH_LIST:
        {
            FillLineStyleList();
            SelectLineStyle();
            break;
        }
        case SID_ATTR_LINE_JOINT:
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

            if(eState >= SfxItemState::DEFAULT)
            {
                const XLineJointItem* pItem = dynamic_cast< const XLineJointItem* >(pState);

                if(pItem)
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
                        break;
                    }
                }
            }

            mpLBEdgeStyle->SetNoSelection();
            break;
        }
        case SID_ATTR_LINE_CAP:
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

            if(eState >= SfxItemState::DEFAULT)
            {
                const XLineCapItem* pItem = dynamic_cast< const XLineCapItem* >(pState);

                if(pItem)
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
                        break;
                    }
                }
            }

            mpLBCapStyle->SetNoSelection();
            break;
        }
    }
}

IMPL_LINK_NOARG(LinePropertyPanel, ChangeLineStyleHdl)
{
    const sal_Int32 nPos(mpLBStyle->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos && mpLBStyle->IsValueChangedFromSaved())
    {
        if(0 == nPos)
        {
            // drawing::LineStyle_NONE
            const XLineStyleItem aItem(drawing::LineStyle_NONE);

            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SfxCallMode::RECORD, &aItem, 0L);
        }
        else if(1 == nPos)
        {
            // drawing::LineStyle_SOLID
            const XLineStyleItem aItem(drawing::LineStyle_SOLID);

            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SfxCallMode::RECORD, &aItem, 0L);
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

            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SfxCallMode::RECORD, &aItemA, 0L);
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_DASH, SfxCallMode::RECORD, &aItemB, 0L);
        }
    }

    return 0;
}

IMPL_LINK_NOARG(LinePropertyPanel, ChangeStartHdl)
{
    sal_Int32  nPos = mpLBStart->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND && mpLBStart->IsValueChangedFromSaved() )
    {
        std::unique_ptr<XLineStartItem> pItem;
        if( nPos == 0 )
            pItem.reset(new XLineStartItem());
        else if( mxLineEndList.is() && mxLineEndList->Count() > (long) ( nPos - 1 ) )
            pItem.reset(new XLineStartItem( mpLBStart->GetSelectEntry(),mxLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINEEND_STYLE, SfxCallMode::RECORD, pItem.get(),  0L);
    }
    return 0;
}

IMPL_LINK_NOARG(LinePropertyPanel, ChangeEndHdl)
{
    sal_Int32  nPos = mpLBEnd->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND && mpLBEnd->IsValueChangedFromSaved() )
    {
        std::unique_ptr<XLineEndItem> pItem;
        if( nPos == 0 )
            pItem.reset(new XLineEndItem());
        else if( mxLineEndList.is() && mxLineEndList->Count() > (long) ( nPos - 1 ) )
            pItem.reset(new XLineEndItem( mpLBEnd->GetSelectEntry(), mxLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINEEND_STYLE, SfxCallMode::RECORD, pItem.get(),  0L);
    }
    return 0;
}

IMPL_LINK_NOARG(LinePropertyPanel, ChangeEdgeStyleHdl)
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

        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_JOINT, SfxCallMode::RECORD, pItem.get(),  0L);
    }
    return 0;
}

IMPL_LINK_NOARG(LinePropertyPanel, ChangeCapStyleHdl)
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

        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_CAP, SfxCallMode::RECORD, pItem.get(),  0L);
    }
    return 0;
}

IMPL_LINK_TYPED(LinePropertyPanel, ToolboxWidthSelectHdl,ToolBox*, pToolBox, void)
{
    if (pToolBox->GetItemCommand(pToolBox->GetCurItemId()) == UNO_SELECTWIDTH)
    {
        maLineWidthPopup.SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
        maLineWidthPopup.Show(*pToolBox);
    }
}

IMPL_LINK_NOARG( LinePropertyPanel, ChangeTransparentHdl )
{
    sal_uInt16 nVal = (sal_uInt16)mpMFTransparent->GetValue();
    XLineTransparenceItem aItem( nVal );

    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SfxCallMode::RECORD, &aItem, 0L);
    return 0L;
}

VclPtr<PopupControl> LinePropertyPanel::CreateLineWidthPopupControl (PopupContainer* pParent)
{
    return VclPtrInstance<LineWidthControl>(pParent, *this);
}

void LinePropertyPanel::EndLineWidthPopupMode()
{
    maLineWidthPopup.Hide();
}

void LinePropertyPanel::SetWidthIcon(int n)
{
    const sal_uInt16 nIdWidth = mpTBWidth->GetItemId(UNO_SELECTWIDTH);
    if (n == 0)
        mpTBWidth->SetItemImage( nIdWidth, maIMGNone);
    else
        mpTBWidth->SetItemImage( nIdWidth, mpIMGWidthIcon[n-1]);
}

void LinePropertyPanel::SetWidthIcon()
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

void LinePropertyPanel::SetWidth(long nWidth)
{
    mnWidthCoreValue = nWidth;
    mbWidthValuable = true;
}

void  LinePropertyPanel::FillLineEndList()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh && pSh->GetItem( SID_LINEEND_LIST ) )
    {
        mpLBStart->Enable();
        SvxLineEndListItem aItem( *static_cast<const SvxLineEndListItem*>(pSh->GetItem( SID_LINEEND_LIST ) ) );
        mxLineEndList = aItem.GetLineEndList();

        if (mxLineEndList.is())
        {
            FillLineEndListBox(*mpLBStart, *mpLBEnd, *mxLineEndList);
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

void  LinePropertyPanel::FillLineStyleList()
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

void LinePropertyPanel::SelectLineStyle()
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
}

void LinePropertyPanel::SelectEndStyle(bool bStart)
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

}} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
