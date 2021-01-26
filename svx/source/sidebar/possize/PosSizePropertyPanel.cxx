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

#include <sal/config.h>

#include <algorithm>

#include "PosSizePropertyPanel.hxx"
#include <sal/log.hxx>
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/weldutils.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/dialmgr.hxx>
#include <svx/rectenum.hxx>
#include <svx/sdangitm.hxx>
#include <unotools/viewoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/fieldvalues.hxx>
#include <svl/intitem.hxx>
#include <svx/strings.hrc>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/transfrmhelper.hxx>
#include <boost/property_tree/ptree.hpp>

#include <svtools/unitconv.hxx>

using namespace css;
using namespace css::uno;

constexpr OUStringLiteral USERITEM_NAME = u"FitItem";

namespace svx::sidebar {

PosSizePropertyPanel::PosSizePropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
:   PanelLayout(pParent, "PosSizePropertyPanel", "svx/ui/sidebarpossize.ui", rxFrame),
    mxFtPosX(m_xBuilder->weld_label("horizontallabel")),
    mxMtrPosX(m_xBuilder->weld_metric_spin_button("horizontalpos", FieldUnit::CM)),
    mxFtPosY(m_xBuilder->weld_label("verticallabel")),
    mxMtrPosY(m_xBuilder->weld_metric_spin_button("verticalpos", FieldUnit::CM)),
    mxFtWidth(m_xBuilder->weld_label("widthlabel")),
    mxMtrWidth(m_xBuilder->weld_metric_spin_button("selectwidth", FieldUnit::CM)),
    mxFtHeight(m_xBuilder->weld_label("heightlabel")),
    mxMtrHeight(m_xBuilder->weld_metric_spin_button("selectheight", FieldUnit::CM)),
    mxCbxScale(m_xBuilder->weld_check_button("ratio")),
    mxFtAngle(m_xBuilder->weld_label("rotationlabel")),
    mxMtrAngle(m_xBuilder->weld_metric_spin_button("rotation", FieldUnit::DEGREE)),
    mxCtrlDial(new DialControl),
    mxDial(new weld::CustomWeld(*m_xBuilder, "orientationcontrol", *mxCtrlDial)),
    mxFtFlip(m_xBuilder->weld_label("fliplabel")),
    mxFlipTbx(m_xBuilder->weld_toolbar("selectrotationtype")),
    mxFlipDispatch(new ToolbarUnoDispatcher(*mxFlipTbx, *m_xBuilder, rxFrame)),
    mxArrangeTbx(m_xBuilder->weld_toolbar("arrangetoolbar")),
    mxArrangeDispatch(new ToolbarUnoDispatcher(*mxArrangeTbx, *m_xBuilder, rxFrame)),
    mxArrangeTbx2(m_xBuilder->weld_toolbar("arrangetoolbar2")),
    mxArrangeDispatch2(new ToolbarUnoDispatcher(*mxArrangeTbx2, *m_xBuilder, rxFrame)),
    mxAlignTbx(m_xBuilder->weld_toolbar("aligntoolbar")),
    mxAlignDispatch(new ToolbarUnoDispatcher(*mxAlignTbx, *m_xBuilder, rxFrame)),
    mxAlignTbx2(m_xBuilder->weld_toolbar("aligntoolbar2")),
    mxAlignDispatch2(new ToolbarUnoDispatcher(*mxAlignTbx2, *m_xBuilder, rxFrame)),
    mxBtnEditChart(m_xBuilder->weld_button("btnEditChart")),
    maRect(),
    mpView(nullptr),
    mlOldWidth(1),
    mlOldHeight(1),
    mlRotX(0),
    mlRotY(0),
    maUIScale(),
    mePoolUnit(),
    meDlgUnit(FieldUnit::INCH), // #i124409# init with fallback default
    maTransfPosXControl(SID_ATTR_TRANSFORM_POS_X, *pBindings, *this),
    maTransfPosYControl(SID_ATTR_TRANSFORM_POS_Y, *pBindings, *this),
    maTransfWidthControl(SID_ATTR_TRANSFORM_WIDTH, *pBindings, *this),
    maTransfHeightControl(SID_ATTR_TRANSFORM_HEIGHT, *pBindings, *this),
    maSvxAngleControl( SID_ATTR_TRANSFORM_ANGLE, *pBindings, *this),
    maRotXControl(SID_ATTR_TRANSFORM_ROT_X, *pBindings, *this),
    maRotYControl(SID_ATTR_TRANSFORM_ROT_Y, *pBindings, *this),
    maProPosControl(SID_ATTR_TRANSFORM_PROTECT_POS, *pBindings, *this),
    maProSizeControl(SID_ATTR_TRANSFORM_PROTECT_SIZE, *pBindings, *this),
    maAutoWidthControl(SID_ATTR_TRANSFORM_AUTOWIDTH, *pBindings, *this),
    maAutoHeightControl(SID_ATTR_TRANSFORM_AUTOHEIGHT, *pBindings, *this),
    m_aMetricCtl(SID_ATTR_METRIC, *pBindings, *this),
    maContext(),
    mpBindings(pBindings),
    mbSizeProtected(false),
    mbPositionProtected(false),
    mbAutoWidth(false),
    mbAutoHeight(false),
    mbAdjustEnabled(false),
    mxSidebar(rxSidebar)
{
    Initialize();

    // A guesstimate of the longest label in the various sidebar panes to use
    // to get this pane's contents to align with them, for lack of a better
    // solution
    auto nWidth = mxFtWidth->get_preferred_size().Width();
    OUString sLabel = mxFtWidth->get_label();
    mxFtWidth->set_label(SvxResId(RID_SVXSTR_TRANSPARENCY));
    nWidth = std::max(nWidth, mxFtWidth->get_preferred_size().Width());;
    mxFtWidth->set_label(sLabel);
    mxFtWidth->set_size_request(nWidth, -1);

    mpBindings->Update( SID_ATTR_METRIC );
    mpBindings->Update( SID_ATTR_TRANSFORM_WIDTH );
    mpBindings->Update( SID_ATTR_TRANSFORM_HEIGHT );
    mpBindings->Update( SID_ATTR_TRANSFORM_PROTECT_SIZE );
}

PosSizePropertyPanel::~PosSizePropertyPanel()
{
    disposeOnce();
}

void PosSizePropertyPanel::dispose()
{
    mxFtPosX.reset();
    mxMtrPosX.reset();
    mxFtPosY.reset();
    mxMtrPosY.reset();
    mxFtWidth.reset();
    mxMtrWidth.reset();
    mxFtHeight.reset();
    mxMtrHeight.reset();
    mxCbxScale.reset();
    mxFtAngle.reset();
    mxMtrAngle.reset();
    mxDial.reset();
    mxCtrlDial.reset();
    mxFtFlip.reset();
    mxFlipDispatch.reset();
    mxFlipTbx.reset();
    mxAlignDispatch.reset();
    mxAlignDispatch2.reset();
    mxAlignTbx.reset();
    mxAlignTbx2.reset();
    mxArrangeDispatch.reset();
    mxArrangeDispatch2.reset();
    mxArrangeTbx.reset();
    mxArrangeTbx2.reset();
    mxBtnEditChart.reset();

    maTransfPosXControl.dispose();
    maTransfPosYControl.dispose();
    maTransfWidthControl.dispose();
    maTransfHeightControl.dispose();

    maSvxAngleControl.dispose();
    maRotXControl.dispose();
    maRotYControl.dispose();
    maProPosControl.dispose();
    maProSizeControl.dispose();
    maAutoWidthControl.dispose();
    maAutoHeightControl.dispose();
    m_aMetricCtl.dispose();

    PanelLayout::dispose();
}


namespace
{
    bool hasText(const SdrView& rSdrView)
    {
        const SdrMarkList& rMarkList = rSdrView.GetMarkedObjectList();

        if(1 == rMarkList.GetMarkCount())
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrObjKind eKind(pObj->GetObjIdentifier());

            if((pObj->GetObjInventor() == SdrInventor::Default) && (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind))
            {
                const SdrTextObj* pSdrTextObj = dynamic_cast< const SdrTextObj* >(pObj);

                if(pSdrTextObj && pSdrTextObj->HasText())
                {
                    return true;
                }
            }
        }

        return false;
    }
} // end of anonymous namespace


void PosSizePropertyPanel::Initialize()
{
    //Position : Horizontal / Vertical
    mxMtrPosX->connect_value_changed( LINK( this, PosSizePropertyPanel, ChangePosXHdl ) );
    mxMtrPosY->connect_value_changed( LINK( this, PosSizePropertyPanel, ChangePosYHdl ) );

    //Size : Width / Height
    mxMtrWidth->connect_value_changed( LINK( this, PosSizePropertyPanel, ChangeWidthHdl ) );
    mxMtrHeight->connect_value_changed( LINK( this, PosSizePropertyPanel, ChangeHeightHdl ) );

    //Size : Keep ratio
    mxCbxScale->connect_toggled( LINK( this, PosSizePropertyPanel, ClickAutoHdl ) );

    //rotation control
    mxCtrlDial->SetLinkedField(mxMtrAngle.get());
    mxCtrlDial->SetModifyHdl(LINK( this, PosSizePropertyPanel, RotationHdl));

    //use same logic as DialControl_Impl::SetSize
    weld::DrawingArea* pDrawingArea = mxCtrlDial->GetDrawingArea();
    int nDim = (std::min<int>(pDrawingArea->get_approximate_digit_width() * 6,
                              pDrawingArea->get_text_height() * 3) - 1) | 1;
    Size aSize(nDim, nDim);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    mxCtrlDial->Init(aSize);

    mxBtnEditChart->connect_clicked( LINK( this, PosSizePropertyPanel, ClickChartEditHdl ) );

    SfxViewShell* pCurSh = SfxViewShell::Current();
    if ( pCurSh )
        mpView = pCurSh->GetDrawView();
    else
        mpView = nullptr;

    if ( mpView != nullptr )
    {
        maUIScale = mpView->GetModel()->GetUIScale();
        mbAdjustEnabled = hasText(*mpView);
    }

    mePoolUnit = maTransfWidthControl.GetCoreMetric();
}

VclPtr<vcl::Window> PosSizePropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to PosSizePropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to PosSizePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to PosSizePropertyPanel::Create", nullptr, 2);

    return VclPtr<PosSizePropertyPanel>::Create(
                        pParent,
                        rxFrame,
                        pBindings,
                        rxSidebar);
}


void PosSizePropertyPanel::DataChanged(
    const DataChangedEvent& /*rEvent*/)
{
}

void PosSizePropertyPanel::HandleContextChange(
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;

    bool bShowPosition = false;
    bool bShowAngle = false;
    bool bShowFlip = false;
    bool bShowEditChart = false;
    bool bShowArrangeTbx2 = false;

    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application::WriterVariants, Context::Draw):
            bShowAngle = true;
            bShowFlip = true;
            bShowArrangeTbx2 = true;
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Graphic):
            bShowFlip = true;
            bShowAngle = true; // RotGrfFlyFrame: Writer FlyFrames for Graphics now support angle
            break;

        case CombinedEnumContext(Application::Calc, Context::Draw):
        case CombinedEnumContext(Application::Calc, Context::DrawLine):
        case CombinedEnumContext(Application::Calc, Context::Graphic):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::DrawLine):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
            bShowPosition = true;
            bShowAngle = true;
            bShowFlip = true;
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::OLE):
            bShowEditChart = true;
            break;

        case CombinedEnumContext(Application::Calc, Context::OLE):
        case CombinedEnumContext(Application::DrawImpress, Context::OLE):
            bShowPosition = true;
            bShowEditChart = true;
            break;

        case CombinedEnumContext(Application::Calc, Context::Chart):
        case CombinedEnumContext(Application::Calc, Context::Form):
        case CombinedEnumContext(Application::Calc, Context::Media):
        case CombinedEnumContext(Application::Calc, Context::MultiObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Media):
        case CombinedEnumContext(Application::DrawImpress, Context::Form):
        case CombinedEnumContext(Application::DrawImpress, Context::ThreeDObject):
        case CombinedEnumContext(Application::DrawImpress, Context::MultiObject):
            bShowPosition = true;
            break;
    }

    // Position
    mxFtPosX->set_visible(bShowPosition);
    mxMtrPosX->set_visible(bShowPosition);
    mxFtPosY->set_visible(bShowPosition);
    mxMtrPosY->set_visible(bShowPosition);

    // Rotation
    mxFtAngle->set_visible(bShowAngle);
    mxMtrAngle->set_visible(bShowAngle);
    mxCtrlDial->set_visible(bShowAngle);

    // Flip
    mxFtFlip->set_visible(bShowFlip);
    mxFlipTbx->set_visible(bShowFlip);

    // Edit Chart
    mxBtnEditChart->set_visible(bShowEditChart);

    // Arrange tool bar 2
    mxArrangeTbx2->set_visible(bShowArrangeTbx2);

    if (mxSidebar.is())
        mxSidebar->requestLayout();
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ChangeWidthHdl, weld::MetricSpinButton&, void )
{
    if( mxCbxScale->get_active() &&
        mxCbxScale->get_sensitive() )
    {
        tools::Long nHeight = static_cast<tools::Long>( (static_cast<double>(mlOldHeight) * static_cast<double>(mxMtrWidth->get_value(FieldUnit::NONE))) / static_cast<double>(mlOldWidth) );
        if( nHeight <= mxMtrHeight->get_max( FieldUnit::NONE ) )
        {
            mxMtrHeight->set_value( nHeight, FieldUnit::NONE );
        }
        else
        {
            nHeight = static_cast<tools::Long>(mxMtrHeight->get_max( FieldUnit::NONE ));
            mxMtrHeight->set_value(nHeight, FieldUnit::NONE);
            const tools::Long nWidth = static_cast<tools::Long>( (static_cast<double>(mlOldWidth) * static_cast<double>(nHeight)) / static_cast<double>(mlOldHeight) );
            mxMtrWidth->set_value( nWidth, FieldUnit::NONE );
        }
    }
    executeSize();
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ChangeHeightHdl, weld::MetricSpinButton&, void )
{
    if( mxCbxScale->get_active() &&
        mxCbxScale->get_sensitive() )
    {
        tools::Long nWidth = static_cast<tools::Long>( (static_cast<double>(mlOldWidth) * static_cast<double>(mxMtrHeight->get_value(FieldUnit::NONE))) / static_cast<double>(mlOldHeight) );
        if( nWidth <= mxMtrWidth->get_max( FieldUnit::NONE ) )
        {
            mxMtrWidth->set_value( nWidth, FieldUnit::NONE );
        }
        else
        {
            nWidth = static_cast<tools::Long>(mxMtrWidth->get_max( FieldUnit::NONE ));
            mxMtrWidth->set_value( nWidth, FieldUnit::NONE );
            const tools::Long nHeight = static_cast<tools::Long>( (static_cast<double>(mlOldHeight) * static_cast<double>(nWidth)) / static_cast<double>(mlOldWidth) );
            mxMtrHeight->set_value( nHeight, FieldUnit::NONE );
        }
    }
    executeSize();
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ChangePosXHdl, weld::MetricSpinButton&, void )
{
    if ( mxMtrPosX->get_value_changed_from_saved())
    {
        tools::Long lX = GetCoreValue( *mxMtrPosX, mePoolUnit );

        Fraction aUIScale = mpView->GetModel()->GetUIScale();
        lX = tools::Long( lX * aUIScale );

        SfxInt32Item aPosXItem( SID_ATTR_TRANSFORM_POS_X,static_cast<sal_uInt32>(lX));

        GetBindings()->GetDispatcher()->ExecuteList(
            SID_ATTR_TRANSFORM, SfxCallMode::RECORD, { &aPosXItem });
    }
}

IMPL_LINK_NOARG( PosSizePropertyPanel, ChangePosYHdl, weld::MetricSpinButton&, void )
{
    if ( mxMtrPosY->get_value_changed_from_saved() )
    {
        tools::Long lY = GetCoreValue( *mxMtrPosY, mePoolUnit );

        Fraction aUIScale = mpView->GetModel()->GetUIScale();
        lY = tools::Long( lY * aUIScale );

        SfxInt32Item aPosYItem( SID_ATTR_TRANSFORM_POS_Y,static_cast<sal_uInt32>(lY));

        GetBindings()->GetDispatcher()->ExecuteList(
            SID_ATTR_TRANSFORM, SfxCallMode::RECORD, { &aPosYItem });
    }
}

IMPL_LINK_NOARG( PosSizePropertyPanel, ClickAutoHdl, weld::ToggleButton&, void )
{
    if ( mxCbxScale->get_active() )
    {
        mlOldWidth  = std::max(GetCoreValue(*mxMtrWidth,  mePoolUnit), 1);
        mlOldHeight = std::max(GetCoreValue(*mxMtrHeight, mePoolUnit), 1);
    }

    // mxCbxScale must synchronized with that on Position and Size tabpage on Shape Properties dialog
    SvtViewOptions aPageOpt(EViewType::TabPage, "cui/ui/possizetabpage/PositionAndSize");
    aPageOpt.SetUserItem( USERITEM_NAME, css::uno::makeAny( OUString::number( int(mxCbxScale->get_active()) ) ) );
}

IMPL_LINK_NOARG( PosSizePropertyPanel, RotationHdl, DialControl&, void )
{
    Degree100 nTmp = mxCtrlDial->GetRotation();

    // #i123993# Need to take UIScale into account when executing rotations
    const double fUIScale(mpView && mpView->GetModel() ? double(mpView->GetModel()->GetUIScale()) : 1.0);
    SdrAngleItem aAngleItem( SID_ATTR_TRANSFORM_ANGLE, nTmp);
    SfxInt32Item aRotXItem( SID_ATTR_TRANSFORM_ROT_X, basegfx::fround(mlRotX * fUIScale));
    SfxInt32Item aRotYItem( SID_ATTR_TRANSFORM_ROT_Y, basegfx::fround(mlRotY * fUIScale));

    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
            SfxCallMode::RECORD, { &aAngleItem, &aRotXItem, &aRotYItem });
}

IMPL_STATIC_LINK_NOARG( PosSizePropertyPanel, ClickChartEditHdl, weld::Button&, void )
{
    SfxViewShell* pCurSh = SfxViewShell::Current();
    if ( pCurSh)
    {
        pCurSh->DoVerb( -1 );
    }
}

namespace
{
    void limitWidth(weld::MetricSpinButton& rMetricSpinButton)
    {
        // space is limited in the sidebar, so limit MetricSpinButtons to a width of 7 digits
        const int nMaxDigits = 7;

        weld::SpinButton& rSpinButton = rMetricSpinButton.get_widget();
        rSpinButton.set_width_chars(std::min(rSpinButton.get_width_chars(), nMaxDigits));
    }
}

void PosSizePropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    mxFtAngle->set_sensitive(true);
    mxMtrAngle->set_sensitive(true);
    mxDial->set_sensitive(true);
    mxFtFlip->set_sensitive(true);
    mxFlipTbx->set_sensitive(true);

    const SfxUInt32Item*    pWidthItem;
    const SfxUInt32Item*    pHeightItem;

    SfxViewShell* pCurSh = SfxViewShell::Current();
    if ( pCurSh )
        mpView = pCurSh->GetDrawView();
    else
        mpView = nullptr;

    if ( mpView == nullptr )
        return;

    mbAdjustEnabled = hasText(*mpView);

    // Pool unit and dialog unit may have changed, make sure that we
    // have the current values.
    mePoolUnit = maTransfWidthControl.GetCoreMetric();

    switch (nSID)
    {
        case SID_ATTR_TRANSFORM_WIDTH:
            if ( SfxItemState::DEFAULT == eState )
            {
                pWidthItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pWidthItem)
                {
                    tools::Long lOldWidth1 = tools::Long( pWidthItem->GetValue() / maUIScale );
                    SetFieldUnit( *mxMtrWidth, meDlgUnit, true );
                    SetMetricValue( *mxMtrWidth, lOldWidth1, mePoolUnit );
                    limitWidth(*mxMtrWidth);
                    mlOldWidth = lOldWidth1;
                    mxMtrWidth->save_value();
                    break;
                }
            }

            mxMtrWidth->set_text( "" );
            break;

        case SID_ATTR_TRANSFORM_HEIGHT:
            if ( SfxItemState::DEFAULT == eState )
            {
                pHeightItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pHeightItem)
                {
                    tools::Long nTmp = tools::Long( pHeightItem->GetValue() / maUIScale);
                    SetFieldUnit( *mxMtrHeight, meDlgUnit, true );
                    SetMetricValue( *mxMtrHeight, nTmp, mePoolUnit );
                    limitWidth(*mxMtrHeight);
                    mlOldHeight = nTmp;
                    mxMtrHeight->save_value();
                    break;
                }
            }

            mxMtrHeight->set_text( "");
            break;

        case SID_ATTR_TRANSFORM_POS_X:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    tools::Long nTmp = tools::Long(pItem->GetValue() / maUIScale);
                    SetFieldUnit( *mxMtrPosX, meDlgUnit, true );
                    SetMetricValue( *mxMtrPosX, nTmp, mePoolUnit );
                    limitWidth(*mxMtrPosX);
                    mxMtrPosX->save_value();
                    break;
                }
            }

            mxMtrPosX->set_text( "" );
            break;

        case SID_ATTR_TRANSFORM_POS_Y:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    tools::Long nTmp = tools::Long(pItem->GetValue() / maUIScale);
                    SetFieldUnit( *mxMtrPosY, meDlgUnit, true );
                    SetMetricValue( *mxMtrPosY, nTmp, mePoolUnit );
                    limitWidth(*mxMtrPosY);
                    mxMtrPosY->save_value();
                    break;
                }
            }

            mxMtrPosY->set_text( "" );
            break;

        case SID_ATTR_TRANSFORM_ROT_X:
            if (SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    mlRotX = pItem->GetValue();
                    mlRotX = tools::Long( mlRotX / maUIScale );
                }
            }
            break;

        case SID_ATTR_TRANSFORM_ROT_Y:
            if (SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    mlRotY = pItem->GetValue();
                    mlRotY = tools::Long( mlRotY / maUIScale );
                }
            }
            break;

        case SID_ATTR_TRANSFORM_PROTECT_POS:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    // record the state of position protect
                    mbPositionProtected = pItem->GetValue();
                    break;
                }
            }

            mbPositionProtected = false;
            break;

        case SID_ATTR_TRANSFORM_PROTECT_SIZE:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    // record the state of size protect
                    mbSizeProtected = pItem->GetValue();
                    break;
                }
            }

            mbSizeProtected = false;
            break;

        case SID_ATTR_TRANSFORM_AUTOWIDTH:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    mbAutoWidth = pItem->GetValue();
                }
            }
            break;

        case SID_ATTR_TRANSFORM_AUTOHEIGHT:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    mbAutoHeight = pItem->GetValue();
                }
            }
            break;

        case SID_ATTR_TRANSFORM_ANGLE:
            if (eState >= SfxItemState::DEFAULT)
            {
                const SdrAngleItem* pItem = dynamic_cast< const SdrAngleItem* >(pState);

                if(pItem)
                {
                    Degree100 nTmp = NormAngle36000(pItem->GetValue());

                    mxMtrAngle->set_value(nTmp.get(), FieldUnit::DEGREE);
                    mxCtrlDial->SetRotation(nTmp);

                    break;
                }
            }

            mxMtrAngle->set_text( "" );
            mxCtrlDial->SetRotation( 0_deg100 );
            break;

        case SID_ATTR_METRIC:
            MetricState( eState, pState );
            UpdateUIScale();
            break;

        default:
            break;
    }

    const sal_Int32 nCombinedContext(maContext.GetCombinedContext_DI());
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    switch (rMarkList.GetMarkCount())
    {
        case 0:
            break;

        case 1:
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrObjKind eKind(pObj->GetObjIdentifier());

            if(((nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::Draw)
               || nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::TextObject)
                 ) && OBJ_EDGE == eKind)
               || OBJ_CAPTION == eKind)
            {
                mxFtAngle->set_sensitive(false);
                mxMtrAngle->set_sensitive(false);
                mxDial->set_sensitive(false);
                mxFlipTbx->set_sensitive(false);
                mxFtFlip->set_sensitive(false);
            }
            break;
        }

        default:
        {
            sal_uInt16 nMarkObj = 0;
            bool isNoEdge = true;

            while(isNoEdge && rMarkList.GetMark(nMarkObj))
            {
                const SdrObject* pObj = rMarkList.GetMark(nMarkObj)->GetMarkedSdrObj();
                const SdrObjKind eKind(pObj->GetObjIdentifier());

                if(((nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::Draw)
                  || nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::TextObject)
                     ) && OBJ_EDGE == eKind)
                  || OBJ_CAPTION == eKind)
                {
                    isNoEdge = false;
                    break;
                }
                nMarkObj++;
            }

            if(!isNoEdge)
            {
                mxFtAngle->set_sensitive(false);
                mxMtrAngle->set_sensitive(false);
                mxDial->set_sensitive(false);
                mxFlipTbx->set_sensitive(false);
                mxFtFlip->set_sensitive(false);
            }
            break;
        }
    }

    if(nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::TextObject))
    {
        mxFlipTbx->set_sensitive(false);
        mxFtFlip->set_sensitive(false);
    }

    DisableControls();

    // mxCbxScale must synchronized with that on Position and Size tabpage on Shape Properties dialog
    SvtViewOptions aPageOpt(EViewType::TabPage, "cui/ui/possizetabpage/PositionAndSize");
    OUString  sUserData;
    css::uno::Any  aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
    OUString aTemp;
    if ( aUserItem >>= aTemp )
        sUserData = aTemp;
    mxCbxScale->set_active(static_cast<bool>(sUserData.toInt32()));
}

void PosSizePropertyPanel::GetControlState(const sal_uInt16 nSID, boost::property_tree::ptree& rState)
{
    weld::MetricSpinButton* pControl = nullptr;
    switch (nSID)
    {
        case SID_ATTR_TRANSFORM_POS_X:
            pControl = mxMtrPosX.get();
            break;
        case SID_ATTR_TRANSFORM_POS_Y:
            pControl = mxMtrPosY.get();
            break;
        case SID_ATTR_TRANSFORM_WIDTH:
            pControl = mxMtrWidth.get();
            break;
        case SID_ATTR_TRANSFORM_HEIGHT:
            pControl = mxMtrHeight.get();
            break;
    }

    if (pControl && !pControl->get_text().isEmpty())
    {
        OUString sValue = Application::GetSettings().GetNeutralLocaleDataWrapper().
            getNum(pControl->get_value(pControl->get_unit()), pControl->get_digits(), false, false);
        rState.put(pControl->get_buildable_name().getStr(), sValue.toUtf8().getStr());
    }
}

void PosSizePropertyPanel::executeSize()
{
    if ( !mxMtrWidth->get_value_changed_from_saved() && !mxMtrHeight->get_value_changed_from_saved())
        return;

    Fraction aUIScale = mpView->GetModel()->GetUIScale();

    // get Width
    double nWidth = static_cast<double>(mxMtrWidth->get_value(FieldUnit::MM_100TH));
    tools::Long lWidth = tools::Long(nWidth * static_cast<double>(aUIScale));
    lWidth = OutputDevice::LogicToLogic( lWidth, MapUnit::Map100thMM, mePoolUnit );
    lWidth = static_cast<tools::Long>(mxMtrWidth->denormalize( lWidth ));

    // get Height
    double nHeight = static_cast<double>(mxMtrHeight->get_value(FieldUnit::MM_100TH));
    tools::Long lHeight = tools::Long(nHeight * static_cast<double>(aUIScale));
    lHeight = OutputDevice::LogicToLogic( lHeight, MapUnit::Map100thMM, mePoolUnit );
    lHeight = static_cast<tools::Long>(mxMtrHeight->denormalize( lHeight ));

    // put Width & Height to itemset
    SfxUInt32Item aWidthItem( SID_ATTR_TRANSFORM_WIDTH, static_cast<sal_uInt32>(lWidth));
    SfxUInt32Item aHeightItem( SID_ATTR_TRANSFORM_HEIGHT, static_cast<sal_uInt32>(lHeight));
    SfxUInt16Item aPointItem (SID_ATTR_TRANSFORM_SIZE_POINT, sal_uInt16(RectPoint::LT));
    const sal_Int32 nCombinedContext(maContext.GetCombinedContext_DI());

    if( nCombinedContext == CombinedEnumContext(Application::WriterVariants, Context::Graphic)
        || nCombinedContext == CombinedEnumContext(Application::WriterVariants, Context::OLE)
        )
    {
        GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
            SfxCallMode::RECORD, { &aWidthItem, &aHeightItem, &aPointItem });
    }
    else
    {
        if ( (mxMtrWidth->get_value_changed_from_saved()) && (mxMtrHeight->get_value_changed_from_saved()))
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
                SfxCallMode::RECORD, { &aWidthItem, &aHeightItem, &aPointItem });
        else if( mxMtrWidth->get_value_changed_from_saved())
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
                SfxCallMode::RECORD, { &aWidthItem, &aPointItem });
        else if ( mxMtrHeight->get_value_changed_from_saved())
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
                SfxCallMode::RECORD, { &aHeightItem, &aPointItem });
    }
}

void PosSizePropertyPanel::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    if (meDlgUnit != GetCurrentUnit(SfxItemState::DEFAULT, nullptr))
    {
        mpBindings->Update( SID_ATTR_METRIC );
    }

    PanelLayout::DumpAsPropertyTree(rJsonWriter);
}

void PosSizePropertyPanel::MetricState( SfxItemState eState, const SfxPoolItem* pState )
{
    bool bPosXBlank = false;
    bool bPosYBlank = false;
    bool bWidthBlank = false;
    bool bHeightBlank = false;

    // #i124409# use the given Item to get the correct UI unit and initialize it
    // and the Fields using it
    meDlgUnit = GetCurrentUnit(eState,pState);

    if (mxMtrPosX->get_text().isEmpty())
        bPosXBlank = true;
    SetFieldUnit( *mxMtrPosX, meDlgUnit, true );
    if(bPosXBlank)
        mxMtrPosX->set_text(OUString());

    if (mxMtrPosY->get_text().isEmpty())
        bPosYBlank = true;
    SetFieldUnit( *mxMtrPosY, meDlgUnit, true );
    if(bPosYBlank)
        mxMtrPosY->set_text(OUString());
    SetPosSizeMinMax();

    if (mxMtrWidth->get_text().isEmpty())
        bWidthBlank = true;
    SetFieldUnit( *mxMtrWidth, meDlgUnit, true );
    if(bWidthBlank)
        mxMtrWidth->set_text(OUString());

    if (mxMtrHeight->get_text().isEmpty())
        bHeightBlank = true;
    SetFieldUnit( *mxMtrHeight, meDlgUnit, true );
    if(bHeightBlank)
        mxMtrHeight->set_text(OUString());
}


FieldUnit PosSizePropertyPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
    FieldUnit eUnit = FieldUnit::NONE;

    if ( pState && eState >= SfxItemState::DEFAULT )
    {
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    }
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = nullptr;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
            }
            else
            {
                SAL_WARN("svx.sidebar", "GetModuleFieldUnit(): no module found");
            }
        }
    }

    return eUnit;
}


void PosSizePropertyPanel::DisableControls()
{
    if( mbPositionProtected )
    {
        // the position is protected("Position protect" option in modal dialog is checked),
        // disable all the Position controls in sidebar
        mxFtPosX->set_sensitive(false);
        mxMtrPosX->set_sensitive(false);
        mxFtPosY->set_sensitive(false);
        mxMtrPosY->set_sensitive(false);
        mxFtAngle->set_sensitive(false);
        mxMtrAngle->set_sensitive(false);
        mxDial->set_sensitive(false);
        mxFtFlip->set_sensitive(false);
        mxFlipTbx->set_sensitive(false);

        mxFtWidth->set_sensitive(false);
        mxMtrWidth->set_sensitive(false);
        mxFtHeight->set_sensitive(false);
        mxMtrHeight->set_sensitive(false);
        mxCbxScale->set_sensitive(false);
    }
    else
    {
        mxFtPosX->set_sensitive(true);
        mxMtrPosX->set_sensitive(true);
        mxFtPosY->set_sensitive(true);
        mxMtrPosY->set_sensitive(true);

        if( mbSizeProtected )
        {
            mxFtWidth->set_sensitive(false);
            mxMtrWidth->set_sensitive(false);
            mxFtHeight->set_sensitive(false);
            mxMtrHeight->set_sensitive(false);
            mxCbxScale->set_sensitive(false);
        }
        else
        {
            if( mbAdjustEnabled )
            {
                if( mbAutoWidth )
                {
                    mxFtWidth->set_sensitive(false);
                    mxMtrWidth->set_sensitive(false);
                    mxCbxScale->set_sensitive(false);
                }
                else
                {
                    mxFtWidth->set_sensitive(true);
                    mxMtrWidth->set_sensitive(true);
                }
                if( mbAutoHeight )
                {
                    mxFtHeight->set_sensitive(false);
                    mxMtrHeight->set_sensitive(false);
                    mxCbxScale->set_sensitive(false);
                }
                else
                {
                    mxFtHeight->set_sensitive(true);
                    mxMtrHeight->set_sensitive(true);
                }
                if( !mbAutoWidth && !mbAutoHeight )
                    mxCbxScale->set_sensitive(true);
            }
            else
            {
                mxFtWidth->set_sensitive(true);
                mxMtrWidth->set_sensitive(true);
                mxFtHeight->set_sensitive(true);
                mxMtrHeight->set_sensitive(true);
                mxCbxScale->set_sensitive(true);
            }
        }
    }
}

void PosSizePropertyPanel::SetPosSizeMinMax()
{
    SdrPageView* pPV = mpView->GetSdrPageView();
    if (!pPV)
        return;
    tools::Rectangle aTmpRect(mpView->GetAllMarkedRect());
    pPV->LogicToPagePos(aTmpRect);
    maRect = vcl::unotools::b2DRectangleFromRectangle(aTmpRect);

    tools::Rectangle aTmpRect2(mpView->GetWorkArea());
    pPV->LogicToPagePos(aTmpRect2);
    maWorkArea = vcl::unotools::b2DRectangleFromRectangle(aTmpRect2);

    const Fraction aUIScale(mpView->GetModel()->GetUIScale());
    TransfrmHelper::ScaleRect( maWorkArea, aUIScale );
    TransfrmHelper::ScaleRect( maRect, aUIScale );

    const sal_uInt16 nDigits(mxMtrPosX->get_digits());
    TransfrmHelper::ConvertRect( maWorkArea, nDigits, mePoolUnit, meDlgUnit );
    TransfrmHelper::ConvertRect( maRect, nDigits, mePoolUnit, meDlgUnit );

    double fLeft(maWorkArea.getMinX());
    double fTop(maWorkArea.getMinY());
    double fRight(maWorkArea.getMaxX());
    double fBottom(maWorkArea.getMaxY());

    // seems that sidebar defaults to top left reference point
    // and there's no way to set it to something else
    fRight  -= maRect.getWidth();
    fBottom -= maRect.getHeight();

    const double fMaxLong(static_cast<double>(vcl::ConvertValue( LONG_MAX, 0, MapUnit::Map100thMM, meDlgUnit ) - 1));
    fLeft = std::clamp(fLeft, -fMaxLong, fMaxLong);
    fRight = std::clamp(fRight, -fMaxLong, fMaxLong);
    fTop = std::clamp(fTop, - fMaxLong, fMaxLong);
    fBottom = std::clamp(fBottom, -fMaxLong, fMaxLong);

    mxMtrPosX->set_range(basegfx::fround64(fLeft), basegfx::fround64(fRight), FieldUnit::NONE);
    limitWidth(*mxMtrPosX);
    mxMtrPosY->set_range(basegfx::fround64(fTop), basegfx::fround64(fBottom), FieldUnit::NONE);
    limitWidth(*mxMtrPosY);

    double fMaxWidth = maWorkArea.getWidth() - (maRect.getWidth() - fLeft);
    double fMaxHeight = maWorkArea.getHeight() - (maRect.getHeight() - fTop);
    mxMtrWidth->set_max(std::min<sal_Int64>(INT_MAX, basegfx::fround64(fMaxWidth*100)), FieldUnit::NONE);
    limitWidth(*mxMtrWidth);
    mxMtrHeight->set_max(std::min<sal_Int64>(INT_MAX, basegfx::fround64(fMaxHeight*100)), FieldUnit::NONE);
    limitWidth(*mxMtrHeight);
}

void PosSizePropertyPanel::UpdateUIScale()
{
    const Fraction aUIScale (mpView->GetModel()->GetUIScale());
    if (maUIScale == aUIScale)
        return;

    // UI scale has changed.

    // Remember the new UI scale.
    maUIScale = aUIScale;

    // The content of the position and size boxes is only updated when item changes are notified.
    // Request such notifications without changing the actual item values.
    GetBindings()->Invalidate(SID_ATTR_TRANSFORM_POS_X, true);
    GetBindings()->Invalidate(SID_ATTR_TRANSFORM_POS_Y, true);
    GetBindings()->Invalidate(SID_ATTR_TRANSFORM_WIDTH, true);
    GetBindings()->Invalidate(SID_ATTR_TRANSFORM_HEIGHT, true);
}


} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
