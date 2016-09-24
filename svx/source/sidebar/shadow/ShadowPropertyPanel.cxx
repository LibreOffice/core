/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <ShadowPropertyPanel.hxx>
#include <comphelper/string.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xtable.hxx>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/xattr.hxx>
#include <svx/svddef.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdshtitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>
#include <svx/sdshcitm.hxx>

using namespace css;
using namespace css::uno;

namespace {

sal_uInt32 ParseText(OUString const & sTmp)
{
    if (sTmp.isEmpty())
        return 0;
    sal_Unicode nChar = sTmp[0];
    if( nChar == '-' )
    {
        if (sTmp.getLength() < 2)
            return 0;
        nChar = sTmp[1];
    }

    if( (nChar < '0') || (nChar > '9') )
        return 0;

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    rtl_math_ConversionStatus eStatus;
    double fTmp = rtl::math::stringToDouble( sTmp, cSep, 0, &eStatus);
    if (eStatus != rtl_math_ConversionStatus_Ok)
        return 0;

    return fTmp;
}

}

namespace svx { namespace sidebar {

ShadowPropertyPanel::ShadowPropertyPanel(
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "ShadowPropertyPanel", "svx/ui/sidebarshadow.ui", rxFrame),
    maShadowController(SID_ATTR_FILL_SHADOW, *pBindings, *this),
    maShadowTransController(SID_ATTR_SHADOW_TRANSPARENCE, *pBindings, *this),
    maShadowColorController(SID_ATTR_SHADOW_COLOR, *pBindings, *this),
    maShadowXDistanceController(SID_ATTR_SHADOW_XDISTANCE, *pBindings, *this),
    maShadowYDistanceController(SID_ATTR_SHADOW_YDISTANCE, *pBindings, *this),
    mpBindings(pBindings),
    nX(0),
    nY(0),
    nXY(0)
{
    get(mpShowShadow,"SHOW_SHADOW");
    get(mpFTAngle,"angle");
    get(mpShadowAngle,"LB_ANGLE");
    get(mpFTDistance,"distance");
    get(mpShadowDistance,"LB_DISTANCE");
    get(mpFTTransparency,"transparency_label");
    get(mpShadowTransSlider,"transparency_slider");
    get(mpShadowTransMetric,"FIELD_TRANSPARENCY");
    get(mpFTColor,"color");
    get(mpLBShadowColor,"LB_SHADOW_COLOR");

    Initialize();
}

ShadowPropertyPanel::~ShadowPropertyPanel()
{
    disposeOnce();
}

void ShadowPropertyPanel::dispose()
{
    mpShowShadow.clear();
    mpFTAngle.clear();
    mpShadowAngle.clear();
    mpFTDistance.clear();
    mpShadowDistance.clear();
    mpFTTransparency.clear();
    mpShadowTransSlider.clear();
    mpShadowTransMetric.clear();
    mpFTColor.clear();
    mpLBShadowColor.clear();

    maShadowController.dispose();
    maShadowTransController.dispose();
    maShadowColorController.dispose();
    maShadowXDistanceController.dispose();
    maShadowYDistanceController.dispose();
    PanelLayout::dispose();
}

void ShadowPropertyPanel::Initialize()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();

    const SvxColorListItem* pColorListItem = static_cast<const SvxColorListItem*>(pSh ? pSh->GetItem(SID_COLOR_TABLE) : nullptr);
    if (pColorListItem)
    {
        mpLBShadowColor->Fill(pColorListItem->GetColorList());
        mpShowShadow->SetState( TRISTATE_FALSE );
        mpShowShadow->SetClickHdl( LINK(this, ShadowPropertyPanel, ClickShadowHdl ) );
        mpShadowTransMetric->SetModifyHdl( LINK(this, ShadowPropertyPanel, ModifyShadowTransMetricHdl) );
        mpLBShadowColor->SetSelectHdl( LINK( this, ShadowPropertyPanel, ModifyShadowColorHdl ) );
        mpShadowAngle->SetModifyHdl( LINK(this, ShadowPropertyPanel, ModifyShadowDistanceHdl) );
        mpShadowDistance->SetModifyHdl( LINK(this, ShadowPropertyPanel, ModifyShadowDistanceHdl) );
        mpShadowTransSlider->SetRange(Range(0,100));
        mpShadowTransSlider->SetUpdateMode(true);
        mpShadowTransSlider->SetSlideHdl( LINK(this, ShadowPropertyPanel, ModifyShadowTransSliderHdl) );
        InsertDistanceValues();
        InsertAngleValues();
    }
}

IMPL_LINK_NOARG_TYPED(ShadowPropertyPanel, ClickShadowHdl, Button*, void)
{
    if( mpShowShadow->GetState() == TRISTATE_FALSE )
    {
        SdrOnOffItem aItem(makeSdrShadowItem(false));
        GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_SHADOW,
                SfxCallMode::RECORD, { &aItem });
    }
    else
    {
        SdrOnOffItem aItem(makeSdrShadowItem(true));
        GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_SHADOW,
                SfxCallMode::RECORD, { &aItem });
    }
}

IMPL_LINK_NOARG_TYPED(ShadowPropertyPanel, ModifyShadowColorHdl, ListBox&, void)
{
    XColorItem aItem(makeSdrShadowColorItem(mpLBShadowColor->GetSelectEntryColor()));
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_COLOR,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG_TYPED(ShadowPropertyPanel, ModifyShadowTransMetricHdl, Edit&, void)
{
    sal_uInt16 nVal = mpShadowTransMetric->GetValue();
    SetTransparencyValue(nVal);
    SdrPercentItem aItem( makeSdrShadowTransparenceItem(nVal) );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_TRANSPARENCE,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG_TYPED(ShadowPropertyPanel, ModifyShadowTransSliderHdl, Slider*, void)
{
    sal_uInt16 nVal = mpShadowTransSlider->GetThumbPos();
    SetTransparencyValue(nVal);
    SdrPercentItem aItem( makeSdrShadowTransparenceItem(nVal) );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_TRANSPARENCE,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG_TYPED(ShadowPropertyPanel, ModifyShadowDistanceHdl, Edit&, void)
{
    OUString sAngle = mpShadowAngle->GetText();
    nXY = mpShadowDistance->GetValue(FUNIT_100TH_MM);
    switch(ParseText(sAngle))
    {
        case 0: nX = nXY; nY = 0;             break;
        case 45: nX = nXY; nY = -nXY;         break;
        case 90: nX = 0; nY = - nXY;          break;
        case 135: nX = nY = -nXY;             break;
        case 180: nX = -nXY; nY = 0;          break;
        case 225: nX = -nXY; nY = nXY;        break;
        case 270: nX = 0; nY = nXY;           break;
        case 315: nX = nY = nXY;              break;
    }
    SdrMetricItem aXItem(makeSdrShadowXDistItem(nX));
    SdrMetricItem aYItem(makeSdrShadowYDistItem(nY));
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_XDISTANCE,
            SfxCallMode::RECORD, { &aXItem });
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_YDISTANCE,
            SfxCallMode::RECORD, { &aYItem });
}

void ShadowPropertyPanel::UpdateControls()
{
    if(mpShowShadow->GetState() == TRISTATE_FALSE)
    {
        mpShadowDistance->Disable();
        mpLBShadowColor->Disable();
        mpShadowAngle->Disable();
        mpFTAngle->Disable();
        mpFTDistance->Disable();
        mpFTTransparency->Disable();
        mpFTColor->Disable();
        mpShadowTransSlider->Disable();
        mpShadowTransMetric->Disable();
        return;
    }
    else
    {
        mpShadowDistance->Enable();
        mpLBShadowColor->Enable();
        mpShadowAngle->Enable();
        mpFTAngle->Enable();
        mpFTDistance->Enable();
        mpFTTransparency->Enable();
        mpFTColor->Enable();
        mpShadowTransSlider->Enable();
        mpShadowTransMetric->Enable();
    }

    if(nX > 0 && nY == 0) { mpShadowAngle->SelectEntryPos(0); nXY = nX; }
    else if( nX > 0 && nY < 0 ) { mpShadowAngle->SelectEntryPos(1); nXY = nX; }
    else if( nX == 0 && nY < 0 ) { mpShadowAngle->SelectEntryPos(2); nXY = -nY; }
    else if( nX < 0 && nY < 0 ) { mpShadowAngle->SelectEntryPos(3); nXY = -nY; }
    else if( nX < 0 && nY == 0 ) { mpShadowAngle->SelectEntryPos(4); nXY = -nX; }
    else if( nX < 0 && nY > 0 ) { mpShadowAngle->SelectEntryPos(5); nXY = nY; }
    else if( nX == 0 && nY > 0 ) { mpShadowAngle->SelectEntryPos(6); nXY = nY; }
    else if( nX > 0 && nY > 0 ) { mpShadowAngle->SelectEntryPos(7); nXY = nX; }
    else { nXY = 0; }
    mpShadowDistance->SetValue(nXY, FUNIT_100TH_MM);
}

void ShadowPropertyPanel::SetTransparencyValue(long nVal)
{
    mpShadowTransSlider->SetThumbPos(nVal);
    mpShadowTransMetric->SetValue(nVal);
}

void ShadowPropertyPanel::DataChanged(const DataChangedEvent& /*rEvent*/)
{
}

void ShadowPropertyPanel::InsertDistanceValues()
{
    for(sal_uInt16 i = 0; i <= 20 ; i++)
        mpShadowDistance->InsertValue(i*2,FUNIT_POINT);
}

void ShadowPropertyPanel::InsertAngleValues()
{
    mpShadowAngle->InsertValue(0, FUNIT_CUSTOM);
    mpShadowAngle->InsertValue(45, FUNIT_CUSTOM);
    mpShadowAngle->InsertValue(90, FUNIT_CUSTOM);
    mpShadowAngle->InsertValue(135, FUNIT_CUSTOM);
    mpShadowAngle->InsertValue(180, FUNIT_CUSTOM);
    mpShadowAngle->InsertValue(225,FUNIT_CUSTOM);
    mpShadowAngle->InsertValue(270, FUNIT_CUSTOM);
    mpShadowAngle->InsertValue(315,FUNIT_CUSTOM);
}

void ShadowPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool /*bIsEnabled*/)
{
    switch(nSID)
    {
        case SID_ATTR_FILL_SHADOW:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrOnOffItem* pItem = dynamic_cast< const SdrOnOffItem* >(pState);
                if(pItem)
                {
                    if(pItem->GetValue())
                        mpShowShadow->SetState( TRISTATE_TRUE );
                    else
                        mpShowShadow->SetState( TRISTATE_FALSE );
                }
                else
                    mpShowShadow.reset();
            }
        }
        break;

        case SID_ATTR_SHADOW_TRANSPARENCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrPercentItem* pTransparencyItem = dynamic_cast< const SdrPercentItem* >(pState);
                if(pTransparencyItem)
                {
                    const sal_uInt16 nVal = pTransparencyItem->GetValue();
                    SetTransparencyValue(nVal);
                }
                else
                    SetTransparencyValue(0);
            }
        }
        break;

        case SID_ATTR_SHADOW_COLOR:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const XColorItem* pColorItem = dynamic_cast< const XColorItem* >(pState);
                if(pColorItem)
                {
                   mpLBShadowColor->SelectEntry(pColorItem->GetColorValue());
                }
                else
                {
                }
            }
        }
        break;
        case SID_ATTR_SHADOW_XDISTANCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pXDistItem = dynamic_cast< const SdrMetricItem* >(pState);
                if(pXDistItem)
                {
                    nX = pXDistItem->GetValue();
                }
            }
        }
        break;
        case SID_ATTR_SHADOW_YDISTANCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pYDistItem = dynamic_cast< const SdrMetricItem* >(pState);
                if(pYDistItem)
                {
                    nY = pYDistItem->GetValue();
                }
            }
        }
        break;
    }
    UpdateControls();
}

VclPtr<vcl::Window> ShadowPropertyPanel::Create (
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if(pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ShadowPropertyPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw lang::IllegalArgumentException("no XFrame given to ShadowPropertyPanel::Create", nullptr, 1);
    if(pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to ShadowPropertyPanel::Create", nullptr, 2);

    return VclPtr<ShadowPropertyPanel>::Create(pParent, rxFrame, pBindings);
}
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
