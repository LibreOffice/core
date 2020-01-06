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

#include <memory>
#include <osl/diagnose.h>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <svx/sidebar/LinePropertyPanelBase.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/weldutils.hxx>
#include <svx/xtable.hxx>
#include <svx/xdash.hxx>
#include <svx/drawitem.hxx>
#include <svx/svxitems.hrc>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
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
#include <bitmaps.hlst>

using namespace css;
using namespace css::uno;

const char SELECTWIDTH[] = "SelectWidth";

namespace svx { namespace sidebar {

LinePropertyPanelBase::LinePropertyPanelBase(
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame)
:   PanelLayout(pParent, "LinePropertyPanel", "svx/ui/sidebarline.ui", rxFrame, true),
    mxTBColor(m_xBuilder->weld_toolbar("color")),
    mxColorDispatch(new ToolbarUnoDispatcher(*mxTBColor, rxFrame)),
    mxFTWidth(m_xBuilder->weld_label("widthlabel")),
    mxTBWidth(m_xBuilder->weld_toolbar("width")),
    mxLBStyle(new SvxLineLB(m_xBuilder->weld_combo_box("linestyle"))),
    mxFTTransparency(m_xBuilder->weld_label("translabel")),
    mxMFTransparent(m_xBuilder->weld_metric_spin_button("linetransparency", FieldUnit::PERCENT)),
    mxLBStart(new SvxLineEndLB(m_xBuilder->weld_combo_box("beginarrowstyle"))),
    mxLBEnd(new SvxLineEndLB(m_xBuilder->weld_combo_box("endarrowstyle"))),
    mxFTEdgeStyle(m_xBuilder->weld_label("cornerlabel")),
    mxLBEdgeStyle(m_xBuilder->weld_combo_box("edgestyle")),
    mxFTCapStyle(m_xBuilder->weld_label("caplabel")),
    mxLBCapStyle(m_xBuilder->weld_combo_box("linecapstyle")),
    mxGridLineProps(m_xBuilder->weld_widget("lineproperties")),
    mxBoxArrowProps(m_xBuilder->weld_widget("arrowproperties")),
    mxLineWidthPopup(new LineWidthPopup(mxTBWidth.get(), *this)),
    mpStyleItem(),
    mpDashItem(),
    mnTrans(0),
    meMapUnit(MapUnit::MapMM),
    mnWidthCoreValue(0),
    mpStartItem(),
    mpEndItem(),
    maIMGNone(BMP_NONE_ICON),
    mbWidthValuable(true),
    mbArrowSupported(true)
{
    Initialize();
}

LinePropertyPanelBase::~LinePropertyPanelBase()
{
    disposeOnce();
}

void LinePropertyPanelBase::dispose()
{
    mxLineWidthPopup.reset();
    mxFTWidth.reset();
    mxTBWidth.reset();
    mxTBColor.reset();
    mxLBStyle.reset();
    mxFTTransparency.reset();
    mxMFTransparent.reset();
    mxLBStart.reset();
    mxLBEnd.reset();
    mxFTEdgeStyle.reset();
    mxLBEdgeStyle.reset();
    mxFTCapStyle.reset();
    mxLBCapStyle.reset();
    mxGridLineProps.reset();
    mxBoxArrowProps.reset();

    PanelLayout::dispose();
}

void LinePropertyPanelBase::Initialize()
{
    mxTBWidth->set_item_popover(SELECTWIDTH, mxLineWidthPopup->getTopLevel());

    maIMGWidthIcon[0] = BMP_WIDTH1_ICON;
    maIMGWidthIcon[1] = BMP_WIDTH2_ICON;
    maIMGWidthIcon[2] = BMP_WIDTH3_ICON;
    maIMGWidthIcon[3] = BMP_WIDTH4_ICON;
    maIMGWidthIcon[4] = BMP_WIDTH5_ICON;
    maIMGWidthIcon[5] = BMP_WIDTH6_ICON;
    maIMGWidthIcon[6] = BMP_WIDTH7_ICON;
    maIMGWidthIcon[7] = BMP_WIDTH8_ICON;

    FillLineStyleList();
    SelectLineStyle();
    mxLBStyle->connect_changed( LINK( this, LinePropertyPanelBase, ChangeLineStyleHdl ) );

    mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[0]);
    mxTBWidth->connect_clicked(LINK(this, LinePropertyPanelBase, ToolboxWidthSelectHdl));

    FillLineEndList();
    SelectEndStyle(true);
    SelectEndStyle(false);
    mxLBStart->connect_changed( LINK( this, LinePropertyPanelBase, ChangeStartHdl ) );
    mxLBEnd->connect_changed( LINK( this, LinePropertyPanelBase, ChangeEndHdl ) );

    mxMFTransparent->connect_value_changed(LINK(this, LinePropertyPanelBase, ChangeTransparentHdl));

    mxLBEdgeStyle->connect_changed( LINK( this, LinePropertyPanelBase, ChangeEdgeStyleHdl ) );

    mxLBCapStyle->connect_changed( LINK( this, LinePropertyPanelBase, ChangeCapStyleHdl ) );
}

void LinePropertyPanelBase::DataChanged(const DataChangedEvent& /*rEvent*/)
{
}

void LinePropertyPanelBase::updateLineStyle(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem)
{
    if(bDisabled)
    {
        mxLBStyle->set_sensitive(false);
    }
    else
    {
        mxLBStyle->set_sensitive(true);
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
        mpStyleItem.reset();
    }

    SelectLineStyle();
}

void LinePropertyPanelBase::updateLineDash(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem)
{
    if(bDisabled)
    {
        mxLBStyle->set_sensitive(false);
    }
    else
    {
        mxLBStyle->set_sensitive(true);
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
        mxFTTransparency->set_sensitive(false);
        mxMFTransparent->set_sensitive(false);
    }
    else
    {
        mxFTTransparency->set_sensitive(true);
        mxMFTransparent->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineTransparenceItem* pItem = dynamic_cast<const XLineTransparenceItem*>(pState))
        {
            mnTrans = pItem->GetValue();
            mxMFTransparent->set_value(mnTrans, FieldUnit::PERCENT);
            return;
        }
    }

    mxMFTransparent->set_value(0, FieldUnit::PERCENT);//add
    mxMFTransparent->set_text(OUString());
}

void LinePropertyPanelBase::updateLineWidth(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxTBWidth->set_sensitive(false);
        mxFTWidth->set_sensitive(false);
    }
    else
    {
        mxTBWidth->set_sensitive(true);
        mxFTWidth->set_sensitive(true);
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
        mxLBStart->set_sensitive(false);
    }
    else
    {
        if (mbArrowSupported)
            mxLBStart->set_sensitive(true);
    }

    if(bSetOrDefault && pItem)
    {
        mpStartItem.reset(static_cast<XLineStartItem*>(pItem->Clone()));
        SelectEndStyle(true);
        return;
    }

    mpStartItem.reset();
    SelectEndStyle(true);
}

void LinePropertyPanelBase::updateLineEnd(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pItem)
{
    if(bDisabled)
    {
        mxLBEnd->set_sensitive(false);
    }
    else
    {
        if (mbArrowSupported)
            mxLBEnd->set_sensitive(true);
    }

    if(bSetOrDefault && pItem)
    {
        mpEndItem.reset(static_cast<XLineEndItem*>(pItem->Clone()));
        SelectEndStyle(false);
        return;
    }

    mpEndItem.reset();
    SelectEndStyle(false);
}

void LinePropertyPanelBase::updateLineJoint(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxLBEdgeStyle->set_sensitive(false);
        mxFTEdgeStyle->set_sensitive(false);
    }
    else
    {
        mxLBEdgeStyle->set_sensitive(true);
        mxFTEdgeStyle->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineJointItem* pItem = dynamic_cast<const XLineJointItem*>(pState))
        {
            sal_Int32 nEntryPos(0);

            switch(pItem->GetValue())
            {
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
                case drawing::LineJoint_MIDDLE:
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
                mxLBEdgeStyle->set_active(nEntryPos - 1);
                return;
            }
        }
    }

    mxLBEdgeStyle->set_active(-1);
}

void LinePropertyPanelBase::updateLineCap(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxLBCapStyle->set_sensitive(false);
        mxFTCapStyle->set_sensitive(false);
    }
    else
    {
        mxLBCapStyle->set_sensitive(true);
        mxLBCapStyle->set_sensitive(true);
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
                mxLBCapStyle->set_active(nEntryPos - 1);
                return;
            }
        }
    }

    mxLBCapStyle->set_active(-1);
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeLineStyleHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos(mxLBStyle->get_active());

    if (nPos != -1 && mxLBStyle->get_value_changed_from_saved())
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
        else if (mxLineStyleList.is() && mxLineStyleList->Count() > static_cast<long>(nPos - 2))
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

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeStartHdl, weld::ComboBox&, void)
{
    sal_Int32  nPos = mxLBStart->get_active();
    if (nPos != -1 && mxLBStart->get_value_changed_from_saved())
    {
        std::unique_ptr<XLineStartItem> pItem;
        if( nPos == 0 )
            pItem.reset(new XLineStartItem());
        else if( mxLineEndList.is() && mxLineEndList->Count() > static_cast<long>( nPos - 1 ) )
            pItem.reset(new XLineStartItem( mxLBStart->get_active_text(),mxLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
        setLineStartStyle(pItem.get());
    }
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeEndHdl, weld::ComboBox&, void)
{
    sal_Int32  nPos = mxLBEnd->get_active();
    if (nPos != -1 && mxLBEnd->get_value_changed_from_saved())
    {
        std::unique_ptr<XLineEndItem> pItem;
        if( nPos == 0 )
            pItem.reset(new XLineEndItem());
        else if( mxLineEndList.is() && mxLineEndList->Count() > static_cast<long>( nPos - 1 ) )
            pItem.reset(new XLineEndItem( mxLBEnd->get_active_text(), mxLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
        setLineEndStyle(pItem.get());
    }
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeEdgeStyleHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos(mxLBEdgeStyle->get_active());

    if (nPos != -1 && mxLBEdgeStyle->get_value_changed_from_saved())
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

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeCapStyleHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos(mxLBCapStyle->get_active());

    if (nPos != -1 && mxLBCapStyle->get_value_changed_from_saved())
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

IMPL_LINK_NOARG(LinePropertyPanelBase, ToolboxWidthSelectHdl, const OString&, void)
{
    mxTBWidth->set_menu_item_active(SELECTWIDTH, !mxTBWidth->get_menu_item_active(SELECTWIDTH));
}

IMPL_LINK_NOARG( LinePropertyPanelBase, ChangeTransparentHdl, weld::MetricSpinButton&, void )
{
    sal_uInt16 nVal = static_cast<sal_uInt16>(mxMFTransparent->get_value(FieldUnit::PERCENT));
    XLineTransparenceItem aItem( nVal );

    setLineTransparency(aItem);
}

void LinePropertyPanelBase::SetWidthIcon(int n)
{
    if (n == 0)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGNone);
    else
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[n-1]);
}

void LinePropertyPanelBase::SetWidthIcon()
{
    if(!mbWidthValuable)
    {
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGNone);
        return;
    }

    long nVal = LogicToLogic(mnWidthCoreValue * 10, meMapUnit, MapUnit::MapPoint);

    if(nVal <= 6)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[0]);
    else if (nVal <= 9)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[1]);
    else if (nVal <= 12)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[2]);
    else if (nVal <= 19)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[3]);
    else if (nVal <= 26)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[4]);
    else if (nVal <= 37)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[5]);
    else if (nVal <= 52)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[6]);
    else
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[7]);

}

void LinePropertyPanelBase::SetWidth(long nWidth)
{
    mnWidthCoreValue = nWidth;
    mbWidthValuable = true;
    mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void  LinePropertyPanelBase::FillLineEndList()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh && pSh->GetItem( SID_LINEEND_LIST ) )
    {
        mxLBStart->set_sensitive(true);
        mxLineEndList = pSh->GetItem( SID_LINEEND_LIST )->GetLineEndList();

        if (mxLineEndList.is())
        {
            mxLBStart->Fill(mxLineEndList, true);
            mxLBEnd->Fill(mxLineEndList, false);
        }

        mxLBStart->set_active(0);
        mxLBEnd->set_active(0);
    }
    else
    {
        mxLBStart->set_sensitive(false);
        mxLBEnd->set_sensitive(false);
    }
}

void  LinePropertyPanelBase::FillLineStyleList()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh && pSh->GetItem( SID_DASH_LIST ) )
    {
        mxLBStyle->set_sensitive(true);
        mxLineStyleList = pSh->GetItem( SID_DASH_LIST )->GetDashList();

        if (mxLineStyleList.is())
        {
            mxLBStyle->Fill(mxLineStyleList);
        }

        mxLBStyle->set_active(0);
    }
    else
    {
        mxLBStyle->set_sensitive(false);
    }
}

void LinePropertyPanelBase::SelectLineStyle()
{
    if (!mpStyleItem || !mpDashItem)
    {
        mxLBStyle->set_active(-1);
        mxLBStyle->set_sensitive(false);
        return;
    }

    const drawing::LineStyle eXLS(mpStyleItem->GetValue());
    bool bSelected(false);

    switch(eXLS)
    {
        case drawing::LineStyle_NONE:
            break;
        case drawing::LineStyle_SOLID:
            mxLBStyle->set_active(1);
            bSelected = true;
            break;
        default:
            if(mxLineStyleList.is())
            {
                const XDash& rDash = mpDashItem->GetDashValue();
                for(long a(0);!bSelected &&  a < mxLineStyleList->Count(); a++)
                {
                    const XDashEntry* pEntry = mxLineStyleList->GetDash(a);
                    const XDash& rEntry = pEntry->GetDash();
                    if(rDash == rEntry)
                    {
                        mxLBStyle->set_active(a + 2);
                        bSelected = true;
                    }
                }
            }
            break;
    }

    if(!bSelected)
        mxLBStyle->set_active( 0 );

    ActivateControls();
}

void LinePropertyPanelBase::SelectEndStyle(bool bStart)
{
    bool bSelected(false);

    if(bStart)
    {
        if (!mpStartItem)
        {
            mxLBStart->set_active(-1);
            mxLBStart->set_sensitive(false);
            return;
        }

        if (mxLineEndList.is())
        {
            const basegfx::B2DPolyPolygon& rItemPolygon = mpStartItem->GetLineStartValue();
            for(long a(0);!bSelected &&  a < mxLineEndList->Count(); a++)
            {
                const XLineEndEntry* pEntry = mxLineEndList->GetLineEnd(a);
                const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();
                if(rItemPolygon == rEntryPolygon)
                {
                    mxLBStart->set_active(a + 1);
                    bSelected = true;
                }
            }
        }

        if(!bSelected)
        {
            mxLBStart->set_active( 0 );
        }
    }
    else
    {
        if (!mpEndItem)
        {
            mxLBEnd->set_active(-1);
            mxLBEnd->set_sensitive(false);
            return;
        }

        if (mxLineEndList.is())
        {
            const basegfx::B2DPolyPolygon& rItemPolygon = mpEndItem->GetLineEndValue();
            for(long a(0);!bSelected &&  a < mxLineEndList->Count(); a++)
            {
                const XLineEndEntry* pEntry = mxLineEndList->GetLineEnd(a);
                const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();
                if(rItemPolygon == rEntryPolygon)
                {
                    mxLBEnd->set_active(a + 1);
                    bSelected = true;
                }
            }
        }

        if(!bSelected)
        {
            mxLBEnd->set_active( 0 );
        }
    }
}

void LinePropertyPanelBase::ActivateControls()
{
    const sal_Int32 nPos(mxLBStyle->get_active());
    bool bLineStyle( nPos != 0 );

    mxGridLineProps->set_sensitive( bLineStyle );
    mxBoxArrowProps->set_sensitive( bLineStyle );
    mxLBStart->set_sensitive( bLineStyle && mbArrowSupported );
    mxLBEnd->set_sensitive( bLineStyle && mbArrowSupported );
}

void LinePropertyPanelBase::setMapUnit(MapUnit eMapUnit)
{
    meMapUnit = eMapUnit;
    mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void LinePropertyPanelBase::disableArrowHead()
{
    mbArrowSupported = false;
    ActivateControls();
}

}} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
