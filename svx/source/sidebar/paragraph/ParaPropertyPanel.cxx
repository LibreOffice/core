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
#include "ParaPropertyPanel.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/weldutils.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/unitconv.hxx>
#include <sal/log.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <utility>

using namespace css;
using namespace css::uno;

namespace svx::sidebar {
#define DEFAULT_VALUE          0

#define MAX_DURCH               31680 // tdf#68335: 1584 pt for UX interoperability with Word

#define MAX_SW                  1709400
#define MAX_SC_SD               116220200
#define NEGA_MAXVALUE          -10000000

std::unique_ptr<PanelLayout> ParaPropertyPanel::Create (
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to ParaPropertyPanel::Create"_ustr, nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(u"no XFrame given to ParaPropertyPanel::Create"_ustr, nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException(u"no SfxBindings given to ParaPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<ParaPropertyPanel>(pParent, rxFrame, pBindings, rxSidebar);
}

void ParaPropertyPanel::HandleContextChange (
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application::Calc, Context::DrawText):
        case CombinedEnumContext(Application::WriterVariants, Context::DrawText):
            mxTBxVertAlign->show();
            mxTBxBackColor->hide();
            mxTBxNumBullet->hide();
            ReSize();
            break;

        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
        case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
        case CombinedEnumContext(Application::DrawImpress, Context::Table):
            mxTBxVertAlign->show();
            mxTBxBackColor->hide();
            mxTBxNumBullet->hide();
            ReSize();
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Default):
        case CombinedEnumContext(Application::WriterVariants, Context::Text):
            mxTBxVertAlign->hide();
            mxTBxBackColor->show();
            mxTBxNumBullet->show();
            ReSize();
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Table):
            mxTBxVertAlign->show();
            mxTBxBackColor->show();
            mxTBxNumBullet->show();
            ReSize();
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
            mxTBxVertAlign->hide();
            mxTBxBackColor->hide();
            mxTBxNumBullet->hide();
            ReSize();
            break;

        case CombinedEnumContext(Application::Calc, Context::EditCell):
        case CombinedEnumContext(Application::Calc, Context::Cell):
        case CombinedEnumContext(Application::Calc, Context::Pivot):
        case CombinedEnumContext(Application::Calc, Context::Sparkline):
        case CombinedEnumContext(Application::DrawImpress, Context::Text):
        case CombinedEnumContext(Application::DrawImpress, Context::OutlineText):
            break;

        default:
            break;
    }
}

void ParaPropertyPanel::ReSize()
{
    if (mxSidebar.is())
        mxSidebar->requestLayout();
}

void ParaPropertyPanel::InitToolBoxIndent()
{
    Link<weld::MetricSpinButton&,void> aLink = LINK( this, ParaPropertyPanel, ModifyIndentHdl_Impl );
    mxLeftIndent->connect_value_changed( aLink );
    mxRightIndent->connect_value_changed( aLink );
    mxFLineIndent->connect_value_changed( aLink );

    m_eLRSpaceUnit = maLRSpaceControl.GetCoreMetric();
}

void ParaPropertyPanel::InitToolBoxSpacing()
{
    Link<weld::MetricSpinButton&,void> aLink = LINK( this, ParaPropertyPanel, ULSpaceHdl_Impl );
    mxTopDist->connect_value_changed(aLink);
    mxBottomDist->connect_value_changed( aLink );

    m_eULSpaceUnit = maULSpaceControl.GetCoreMetric();
}

void ParaPropertyPanel::initial()
{
    limitMetricWidths();

    //toolbox
    InitToolBoxIndent();
    InitToolBoxSpacing();
}

// for Paragraph Indent
IMPL_LINK_NOARG( ParaPropertyPanel, ModifyIndentHdl_Impl, weld::MetricSpinButton&, void)
{
    SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
    aMargin.SetTextLeft(mxLeftIndent->GetCoreValue(m_eLRSpaceUnit));
    aMargin.SetRight(mxRightIndent->GetCoreValue(m_eLRSpaceUnit));
    aMargin.SetTextFirstLineOffset(static_cast<short>(mxFLineIndent->GetCoreValue(m_eLRSpaceUnit)));

    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_PARA_LRSPACE, SfxCallMode::RECORD, { &aMargin });
}


// for Paragraph Spacing
IMPL_LINK_NOARG( ParaPropertyPanel, ULSpaceHdl_Impl, weld::MetricSpinButton&, void)
{
    SvxULSpaceItem aMargin( SID_ATTR_PARA_ULSPACE );
    aMargin.SetUpper( static_cast<sal_uInt16>(mxTopDist->GetCoreValue(m_eULSpaceUnit)));
    aMargin.SetLower( static_cast<sal_uInt16>(mxBottomDist->GetCoreValue(m_eULSpaceUnit)));

    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_PARA_ULSPACE, SfxCallMode::RECORD, { &aMargin });
}

// for Paragraph State change
void ParaPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    switch (nSID)
    {
    case SID_ATTR_METRIC:
        {
            m_eMetricUnit = GetCurrentUnit(eState,pState);
            if( m_eMetricUnit!=m_last_eMetricUnit )
            {
                mxLeftIndent->SetFieldUnit(m_eMetricUnit);
                mxRightIndent->SetFieldUnit(m_eMetricUnit);
                mxFLineIndent->SetFieldUnit(m_eMetricUnit);
                mxTopDist->SetFieldUnit(m_eMetricUnit);
                mxBottomDist->SetFieldUnit(m_eMetricUnit);

                limitMetricWidths();
            }
            m_last_eMetricUnit = m_eMetricUnit;
        }
        break;

    case SID_ATTR_PARA_LRSPACE:
        StateChangedIndentImpl( eState, pState );
        break;

    case SID_ATTR_PARA_ULSPACE:
        StateChangedULImpl( eState, pState );
        break;
    }
}

void ParaPropertyPanel::StateChangedIndentImpl( SfxItemState eState, const SfxPoolItem* pState )
{
    switch (maContext.GetCombinedContext_DI())
    {

    case CombinedEnumContext(Application::WriterVariants, Context::DrawText):
    case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
    case CombinedEnumContext(Application::Calc, Context::DrawText):
    case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
    case CombinedEnumContext(Application::DrawImpress, Context::Draw):
    case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
    case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
    case CombinedEnumContext(Application::DrawImpress, Context::Table):
        {
            mxLeftIndent->set_min( DEFAULT_VALUE, FieldUnit::NONE );
            mxRightIndent->set_min( DEFAULT_VALUE, FieldUnit::NONE );
            mxFLineIndent->set_min( DEFAULT_VALUE, FieldUnit::NONE );
        }
        break;
    case CombinedEnumContext(Application::WriterVariants, Context::Default):
    case CombinedEnumContext(Application::WriterVariants, Context::Text):
    case CombinedEnumContext(Application::WriterVariants, Context::Table):
        {
            mxLeftIndent->set_min( NEGA_MAXVALUE, FieldUnit::MM_100TH );
            mxRightIndent->set_min( NEGA_MAXVALUE, FieldUnit::MM_100TH );
            mxFLineIndent->set_min( NEGA_MAXVALUE, FieldUnit::MM_100TH );
        }
        break;
    }

    bool bDisabled = eState == SfxItemState::DISABLED;
    mxLeftIndent->set_sensitive(!bDisabled);
    mxRightIndent->set_sensitive(!bDisabled);
    mxFLineIndent->set_sensitive(!bDisabled);

    if (pState && eState >= SfxItemState::DEFAULT)
    {
        const SvxLRSpaceItem* pSpace = static_cast<const SvxLRSpaceItem*>(pState);
        maTxtLeft = pSpace->GetTextLeft();
        maTxtLeft = OutputDevice::LogicToLogic(maTxtLeft, m_eLRSpaceUnit, MapUnit::MapTwip);

        tools::Long aTxtRight = pSpace->GetRight();
        aTxtRight = OutputDevice::LogicToLogic(aTxtRight, m_eLRSpaceUnit, MapUnit::MapTwip);

        tools::Long aTxtFirstLineOfst = pSpace->GetTextFirstLineOffset();
        aTxtFirstLineOfst = OutputDevice::LogicToLogic( aTxtFirstLineOfst, m_eLRSpaceUnit, MapUnit::MapTwip );

        tools::Long nVal = o3tl::convert(maTxtLeft, o3tl::Length::twip, o3tl::Length::mm100);
        nVal = static_cast<tools::Long>(mxLeftIndent->normalize( nVal ));

        if ( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Text)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Default)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Table))
        {
            mxFLineIndent->set_min( nVal*-1, FieldUnit::MM_100TH );
        }

        tools::Long nrVal = o3tl::convert(aTxtRight, o3tl::Length::twip, o3tl::Length::mm100);
        nrVal = static_cast<tools::Long>(mxRightIndent->normalize( nrVal ));

        switch (maContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application::WriterVariants, Context::DrawText):
        case CombinedEnumContext(Application::WriterVariants, Context::Text):
        case CombinedEnumContext(Application::WriterVariants, Context::Default):
        case CombinedEnumContext(Application::WriterVariants, Context::Table):
        case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
            {
                mxLeftIndent->set_max( MAX_SW - nrVal, FieldUnit::MM_100TH );
                mxRightIndent->set_max( MAX_SW - nVal, FieldUnit::MM_100TH );
                mxFLineIndent->set_max( MAX_SW - nVal - nrVal, FieldUnit::MM_100TH );
            }
            break;
        case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::Table):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
            {
                mxLeftIndent->set_max( MAX_SC_SD - nrVal, FieldUnit::MM_100TH );
                mxRightIndent->set_max( MAX_SC_SD - nVal, FieldUnit::MM_100TH );
                mxFLineIndent->set_max( MAX_SC_SD - nVal - nrVal, FieldUnit::MM_100TH );
            }
        }

        mxLeftIndent->set_value( nVal, FieldUnit::MM_100TH );
        mxRightIndent->set_value( nrVal, FieldUnit::MM_100TH );

        tools::Long nfVal = o3tl::convert(aTxtFirstLineOfst, o3tl::Length::twip, o3tl::Length::mm100);
        nfVal = static_cast<tools::Long>(mxFLineIndent->normalize( nfVal ));
        mxFLineIndent->set_value( nfVal, FieldUnit::MM_100TH );
    }
    else if (eState != SfxItemState::DISABLED )
    {
        mxLeftIndent->set_text(u""_ustr);
        mxRightIndent->set_text(u""_ustr);
        mxFLineIndent->set_text(u""_ustr);
    }

    limitMetricWidths();
}

void ParaPropertyPanel::StateChangedULImpl( SfxItemState eState, const SfxPoolItem* pState )
{
    mxTopDist->set_max( mxTopDist->normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );
    mxBottomDist->set_max( mxBottomDist->normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );

    bool bDisabled = eState == SfxItemState::DISABLED;
    mxTopDist->set_sensitive(!bDisabled);
    mxBottomDist->set_sensitive(!bDisabled);

    if( pState && eState >= SfxItemState::DEFAULT )
    {
        const SvxULSpaceItem* pOldItem = static_cast<const SvxULSpaceItem*>(pState);

        maUpper = pOldItem->GetUpper();
        maUpper = OutputDevice::LogicToLogic(maUpper, m_eULSpaceUnit, MapUnit::MapTwip);

        maLower = pOldItem->GetLower();
        maLower = OutputDevice::LogicToLogic(maLower, m_eULSpaceUnit, MapUnit::MapTwip);

        sal_Int64 nVal = o3tl::convert(maUpper, o3tl::Length::twip, o3tl::Length::mm100);
        nVal = mxTopDist->normalize( nVal );
        mxTopDist->set_value( nVal, FieldUnit::MM_100TH );

        nVal = o3tl::convert(maLower, o3tl::Length::twip, o3tl::Length::mm100);
        nVal = mxBottomDist->normalize( nVal );
        mxBottomDist->set_value( nVal, FieldUnit::MM_100TH );
    }
    else if (eState != SfxItemState::DISABLED )
    {
        mxTopDist->set_text(u""_ustr);
        mxBottomDist->set_text(u""_ustr);
    }
    limitMetricWidths();
}

FieldUnit ParaPropertyPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
    FieldUnit eUnit = FieldUnit::NONE;

    if ( pState && eState >= SfxItemState::DEFAULT )
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = nullptr;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )  //the object shell is not always available during reload
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                eUnit = pModule->GetFieldUnit();
            }
            else
            {
                SAL_WARN("svx.sidebar", "GetModuleFieldUnit(): no module found");
            }
        }
    }

    return eUnit;
}

ParaPropertyPanel::ParaPropertyPanel(weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    css::uno::Reference<css::ui::XSidebar> xSidebar)
    : PanelLayout(pParent, u"ParaPropertyPanel"_ustr, u"svx/ui/sidebarparagraph.ui"_ustr),
      //Alignment
      mxTBxHorzAlign(m_xBuilder->weld_toolbar(u"horizontalalignment"_ustr)),
      mxHorzAlignDispatch(new ToolbarUnoDispatcher(*mxTBxHorzAlign, *m_xBuilder, rxFrame)),
      mxTBxVertAlign(m_xBuilder->weld_toolbar(u"verticalalignment"_ustr)),
      mxVertAlignDispatch(new ToolbarUnoDispatcher(*mxTBxVertAlign, *m_xBuilder, rxFrame)),
      //NumBullet&Backcolor
      mxTBxNumBullet(m_xBuilder->weld_toolbar(u"numberbullet"_ustr)),
      mxNumBulletDispatch(new ToolbarUnoDispatcher(*mxTBxNumBullet, *m_xBuilder, rxFrame)),
      mxTBxBackColor(m_xBuilder->weld_toolbar(u"backgroundcolor"_ustr)),
      mxBackColorDispatch(new ToolbarUnoDispatcher(*mxTBxBackColor, *m_xBuilder, rxFrame)),
      mxTBxWriteDirection(m_xBuilder->weld_toolbar(u"writedirection"_ustr)),
      mxWriteDirectionDispatch(new ToolbarUnoDispatcher(*mxTBxWriteDirection, *m_xBuilder, rxFrame)),
      mxTBxParaSpacing(m_xBuilder->weld_toolbar(u"paraspacing"_ustr)),
      mxParaSpacingDispatch(new ToolbarUnoDispatcher(*mxTBxParaSpacing, *m_xBuilder, rxFrame)),
      mxTBxLineSpacing(m_xBuilder->weld_toolbar(u"linespacing"_ustr)),
      mxLineSpacingDispatch(new ToolbarUnoDispatcher(*mxTBxLineSpacing, *m_xBuilder, rxFrame)),
      mxTBxIndent(m_xBuilder->weld_toolbar(u"indent"_ustr)),
      mxIndentDispatch(new ToolbarUnoDispatcher(*mxTBxIndent, *m_xBuilder, rxFrame)),
      //Paragraph spacing
      mxTopDist(m_xBuilder->weld_metric_spin_button(u"aboveparaspacing"_ustr, FieldUnit::CM)),
      mxBottomDist(m_xBuilder->weld_metric_spin_button(u"belowparaspacing"_ustr, FieldUnit::CM)),
      mxLeftIndent(m_xBuilder->weld_metric_spin_button(u"beforetextindent"_ustr, FieldUnit::CM)),
      mxRightIndent(m_xBuilder->weld_metric_spin_button(u"aftertextindent"_ustr, FieldUnit::CM)),
      mxFLineIndent(m_xBuilder->weld_metric_spin_button(u"firstlineindent"_ustr, FieldUnit::CM)),
      maTxtLeft (0),
      maUpper (0),
      maLower (0),
      m_eMetricUnit(FieldUnit::NONE),
      m_last_eMetricUnit(FieldUnit::NONE),
      m_eLRSpaceUnit(),
      m_eULSpaceUnit(),
      maLRSpaceControl (SID_ATTR_PARA_LRSPACE,*pBindings,*this),
      maULSpaceControl (SID_ATTR_PARA_ULSPACE, *pBindings,*this),
      m_aMetricCtl (SID_ATTR_METRIC, *pBindings,*this),
      mpBindings(pBindings),
      mxSidebar(std::move(xSidebar))
{
    // tdf#130197 We want to give this toolbar a width as if it had 5 entries
    // (the parent grid has homogeneous width set so both columns will have the
    // same width).  This ParaPropertyPanel is a default panel in writer, so
    // subsequent panels, e.g. the TableEditPanel panel can have up to 5
    // entries in each of its column and remain in alignment with this panel
    padWidthForSidebar(*mxTBxIndent, rxFrame);

    initial();
    m_aMetricCtl.RequestUpdate();
}

void ParaPropertyPanel::limitMetricWidths()
{
    limitWidthForSidebar(*mxTopDist);
    limitWidthForSidebar(*mxBottomDist);
    limitWidthForSidebar(*mxLeftIndent);
    limitWidthForSidebar(*mxRightIndent);
    limitWidthForSidebar(*mxFLineIndent);
}

ParaPropertyPanel::~ParaPropertyPanel()
{
    mxHorzAlignDispatch.reset();
    mxTBxHorzAlign.reset();

    mxVertAlignDispatch.reset();
    mxTBxVertAlign.reset();

    mxNumBulletDispatch.reset();
    mxTBxNumBullet.reset();

    mxBackColorDispatch.reset();
    mxTBxBackColor.reset();

    mxWriteDirectionDispatch.reset();
    mxTBxWriteDirection.reset();

    mxParaSpacingDispatch.reset();
    mxTBxParaSpacing.reset();

    mxLineSpacingDispatch.reset();
    mxTBxLineSpacing.reset();

    mxIndentDispatch.reset();
    mxTBxIndent.reset();

    mxTopDist.reset();
    mxBottomDist.reset();
    mxLeftIndent.reset();
    mxRightIndent.reset();
    mxFLineIndent.reset();

    maLRSpaceControl.dispose();
    maULSpaceControl.dispose();
    m_aMetricCtl.dispose();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
