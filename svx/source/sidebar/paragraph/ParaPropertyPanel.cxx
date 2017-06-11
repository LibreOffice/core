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
#include <svx/strings.hrc>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>
#include "svx/dialmgr.hxx"
#include <sfx2/objsh.hxx>
#include <svtools/unitconv.hxx>

using namespace css;
using namespace css::uno;

namespace svx {namespace sidebar {
#define DEFAULT_VALUE          0

#define MAX_DURCH             5670

#define MAX_SW                  1709400
#define MAX_SC_SD               116220200
#define NEGA_MAXVALUE          -10000000

VclPtr<vcl::Window> ParaPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ParaPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ParaPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to ParaPropertyPanel::Create", nullptr, 2);

    return VclPtr<ParaPropertyPanel>::Create(
                pParent,
                rxFrame,
                pBindings,
                rxSidebar);
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
            mpTBxVertAlign->Show();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Hide();
            ReSize();
            break;

        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
        case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
        case CombinedEnumContext(Application::DrawImpress, Context::Table):
            mpTBxVertAlign->Show();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Show();
            ReSize();
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Default):
        case CombinedEnumContext(Application::WriterVariants, Context::Text):
            mpTBxVertAlign->Hide();
            mpTBxBackColor->Show();
            mpTBxNumBullet->Show();
            ReSize();
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Table):
            mpTBxVertAlign->Show();
            mpTBxBackColor->Show();
            mpTBxNumBullet->Show();
            ReSize();
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
            mpTBxVertAlign->Hide();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Hide();
            ReSize();
            break;

        case CombinedEnumContext(Application::Calc, Context::EditCell):
        case CombinedEnumContext(Application::Calc, Context::Cell):
        case CombinedEnumContext(Application::Calc, Context::Pivot):
        case CombinedEnumContext(Application::DrawImpress, Context::Text):
        case CombinedEnumContext(Application::DrawImpress, Context::OutlineText):
            break;

        default:
            break;
    }

    mpTBxOutline->Show( maContext.GetApplication_DI() == vcl::EnumContext::Application::DrawImpress );
}

void ParaPropertyPanel::DataChanged (const DataChangedEvent&) {}

void ParaPropertyPanel::ReSize()
{
    if (mxSidebar.is())
        mxSidebar->requestLayout();
}

void ParaPropertyPanel::InitToolBoxIndent()
{
    Link<Edit&,void> aLink = LINK( this, ParaPropertyPanel, ModifyIndentHdl_Impl );
    mpLeftIndent->SetModifyHdl( aLink );
    mpRightIndent->SetModifyHdl( aLink );
    mpFLineIndent->SetModifyHdl( aLink );

    m_eLRSpaceUnit = maLRSpaceControl.GetCoreMetric();
    m_eLRSpaceUnit = maLRSpaceControl.GetCoreMetric();
}

void ParaPropertyPanel::InitToolBoxSpacing()
{
    Link<Edit&,void> aLink = LINK( this, ParaPropertyPanel, ULSpaceHdl_Impl );
    mpTopDist->SetModifyHdl(aLink);
    mpBottomDist->SetModifyHdl( aLink );

    m_eULSpaceUnit = maULSpaceControl.GetCoreMetric();
}

void ParaPropertyPanel::initial()
{
    //toolbox
    InitToolBoxIndent();
    InitToolBoxSpacing();
}

// for Paragraph Indent
IMPL_LINK_NOARG( ParaPropertyPanel, ModifyIndentHdl_Impl, Edit&, void)
{
    SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
    aMargin.SetTextLeft( GetCoreValue( *mpLeftIndent, m_eLRSpaceUnit ) );
    aMargin.SetRight( GetCoreValue( *mpRightIndent, m_eLRSpaceUnit ) );
    aMargin.SetTextFirstLineOfst( (short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ) );

    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_PARA_LRSPACE, SfxCallMode::RECORD, { &aMargin });
}


// for Paragraph Spacing
IMPL_LINK_NOARG( ParaPropertyPanel, ULSpaceHdl_Impl, Edit&, void)
{
    SvxULSpaceItem aMargin( SID_ATTR_PARA_ULSPACE );
    aMargin.SetUpper( (sal_uInt16)GetCoreValue( *mpTopDist, m_eULSpaceUnit ) );
    aMargin.SetLower( (sal_uInt16)GetCoreValue( *mpBottomDist, m_eULSpaceUnit ) );

    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_PARA_ULSPACE, SfxCallMode::RECORD, { &aMargin });
}

// for Paragraph State change
void ParaPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool)
{
    switch (nSID)
    {
    case SID_ATTR_METRIC:
        {
            m_eMetricUnit = GetCurrentUnit(eState,pState);
            if( m_eMetricUnit!=m_last_eMetricUnit )
            {
                SetFieldUnit( *mpLeftIndent, m_eMetricUnit );
                SetFieldUnit( *mpRightIndent, m_eMetricUnit );
                SetFieldUnit( *mpFLineIndent, m_eMetricUnit );
                SetFieldUnit( *mpTopDist, m_eMetricUnit );
                SetFieldUnit( *mpBottomDist, m_eMetricUnit );
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
            mpLeftIndent->SetMin( DEFAULT_VALUE );
            mpRightIndent->SetMin( DEFAULT_VALUE );
            mpFLineIndent->SetMin( DEFAULT_VALUE );
        }
        break;
    case CombinedEnumContext(Application::WriterVariants, Context::Default):
    case CombinedEnumContext(Application::WriterVariants, Context::Text):
    case CombinedEnumContext(Application::WriterVariants, Context::Table):
        {
            mpLeftIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            mpRightIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            mpFLineIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
        }
        break;
    }

    if( pState && eState >= SfxItemState::DEFAULT )
    {
        const SvxLRSpaceItem* pSpace = static_cast<const SvxLRSpaceItem*>(pState);
        maTxtLeft = pSpace->GetTextLeft();
        maTxtLeft = OutputDevice::LogicToLogic( maTxtLeft, m_eLRSpaceUnit, MapUnit::Map100thMM );
        maTxtLeft = OutputDevice::LogicToLogic( maTxtLeft, MapUnit::Map100thMM, MapUnit::MapTwip );

        long aTxtRight = pSpace->GetRight();
        aTxtRight = OutputDevice::LogicToLogic( aTxtRight, m_eLRSpaceUnit, MapUnit::Map100thMM );
        aTxtRight = OutputDevice::LogicToLogic( aTxtRight, MapUnit::Map100thMM, MapUnit::MapTwip );

        long aTxtFirstLineOfst = pSpace->GetTextFirstLineOfst();
        aTxtFirstLineOfst = OutputDevice::LogicToLogic( aTxtFirstLineOfst, m_eLRSpaceUnit, MapUnit::Map100thMM );
        aTxtFirstLineOfst = OutputDevice::LogicToLogic( aTxtFirstLineOfst, MapUnit::Map100thMM, MapUnit::MapTwip );

        long nVal = OutputDevice::LogicToLogic( maTxtLeft, MapUnit::MapTwip, MapUnit::Map100thMM );
        nVal = (long)mpLeftIndent->Normalize( nVal );

        if ( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Text)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Default)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Table))
        {
            mpFLineIndent->SetMin( nVal*(-1), FUNIT_100TH_MM );
        }

        long nrVal = OutputDevice::LogicToLogic( aTxtRight, MapUnit::MapTwip, MapUnit::Map100thMM );
        nrVal = (long)mpRightIndent->Normalize( nrVal );

        switch (maContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application::WriterVariants, Context::DrawText):
        case CombinedEnumContext(Application::WriterVariants, Context::Text):
        case CombinedEnumContext(Application::WriterVariants, Context::Default):
        case CombinedEnumContext(Application::WriterVariants, Context::Table):
        case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
            {
                mpLeftIndent->SetMax( MAX_SW - nrVal, FUNIT_100TH_MM );
                mpRightIndent->SetMax( MAX_SW - nVal, FUNIT_100TH_MM );
                mpFLineIndent->SetMax( MAX_SW - nVal - nrVal, FUNIT_100TH_MM );
            }
            break;
        case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::Table):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
            {
                mpLeftIndent->SetMax( MAX_SC_SD - nrVal, FUNIT_100TH_MM );
                mpRightIndent->SetMax( MAX_SC_SD - nVal, FUNIT_100TH_MM );
                mpFLineIndent->SetMax( MAX_SC_SD - nVal - nrVal, FUNIT_100TH_MM );
            }
        }

        mpLeftIndent->SetValue( nVal, FUNIT_100TH_MM );
        mpRightIndent->SetValue( nrVal, FUNIT_100TH_MM );

        long nfVal = OutputDevice::LogicToLogic( aTxtFirstLineOfst, MapUnit::MapTwip, MapUnit::Map100thMM );
        nfVal = (long)mpFLineIndent->Normalize( nfVal );
        mpFLineIndent->SetValue( nfVal, FUNIT_100TH_MM );
    }
    else if( eState == SfxItemState::DISABLED )
    {
        mpLeftIndent-> Disable();
        mpRightIndent->Disable();
        mpFLineIndent->Disable();
    }
    else
    {
        mpLeftIndent->SetEmptyFieldValue();
        mpRightIndent->SetEmptyFieldValue();
        mpFLineIndent->SetEmptyFieldValue();
    }
}

void ParaPropertyPanel::StateChangedULImpl( SfxItemState eState, const SfxPoolItem* pState )
{
    mpTopDist->SetMax( mpTopDist->Normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );
    mpBottomDist->SetMax( mpBottomDist->Normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );

    if( pState && eState >= SfxItemState::DEFAULT )
    {
        const SvxULSpaceItem* pOldItem = static_cast<const SvxULSpaceItem*>(pState);

        maUpper = pOldItem->GetUpper();
        maUpper = OutputDevice::LogicToLogic( maUpper, m_eULSpaceUnit, MapUnit::Map100thMM );
        maUpper = OutputDevice::LogicToLogic( maUpper, MapUnit::Map100thMM, MapUnit::MapTwip );

        maLower = pOldItem->GetLower();
        maLower = OutputDevice::LogicToLogic( maLower, m_eULSpaceUnit, MapUnit::Map100thMM );
        maLower = OutputDevice::LogicToLogic( maLower, MapUnit::Map100thMM, MapUnit::MapTwip );

        sal_Int64 nVal = OutputDevice::LogicToLogic( maUpper, MapUnit::MapTwip, MapUnit::Map100thMM );
        nVal = mpTopDist->Normalize( nVal );
        mpTopDist->SetValue( nVal, FUNIT_100TH_MM );

        nVal = OutputDevice::LogicToLogic( maLower, MapUnit::MapTwip, MapUnit::Map100thMM );
        nVal = mpBottomDist->Normalize( nVal );
        mpBottomDist->SetValue( nVal, FUNIT_100TH_MM );
    }
    else if(eState == SfxItemState::DISABLED )
    {
        mpTopDist->Disable();
        mpBottomDist->Disable();
    }
    else
    {
        mpTopDist->SetEmptyFieldValue();
        mpBottomDist->SetEmptyFieldValue();
    }
}

FieldUnit ParaPropertyPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
    FieldUnit eUnit = FUNIT_NONE;

    if ( pState && eState >= SfxItemState::DEFAULT )
        eUnit = (FieldUnit) static_cast<const SfxUInt16Item*>(pState)->GetValue();
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
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    eUnit = (FieldUnit) static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            }
            else
            {
                SAL_WARN("svx.sidebar", "GetModuleFieldUnit(): no module found");
            }
        }
    }

    return eUnit;
}

ParaPropertyPanel::ParaPropertyPanel(vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
    : PanelLayout(pParent, "ParaPropertyPanel", "svx/ui/sidebarparagraph.ui", rxFrame),

      maTxtLeft (0),
      maUpper (0),
      maLower (0),
      m_eMetricUnit(FUNIT_NONE),
      m_last_eMetricUnit(FUNIT_NONE),
      m_eLRSpaceUnit(),
      m_eULSpaceUnit(),
      maLRSpaceControl (SID_ATTR_PARA_LRSPACE,*pBindings,*this),
      maULSpaceControl (SID_ATTR_PARA_ULSPACE, *pBindings,*this),
      m_aMetricCtl (SID_ATTR_METRIC, *pBindings,*this),
      maContext(),
      mpBindings(pBindings),
      mxSidebar(rxSidebar)
{
    //Alignment
    get(mpTBxVertAlign, "verticalalignment");
    //NumBullet&Backcolor
    get(mpTBxNumBullet, "numberbullet");
    get(mpTBxBackColor, "backgroundcolor");
    get(mpTBxOutline, "outline");
    //Paragraph spacing
    get(mpTopDist,      "aboveparaspacing");
    mpTopDist->set_width_request(mpTopDist->get_preferred_size().Width());
    get(mpBottomDist,   "belowparaspacing");
    mpBottomDist->set_width_request(mpBottomDist->get_preferred_size().Width());
    get(mpLeftIndent,   "beforetextindent");
    mpLeftIndent->set_width_request(mpLeftIndent->get_preferred_size().Width());
    get(mpRightIndent,  "aftertextindent");
    mpRightIndent->set_width_request(mpRightIndent->get_preferred_size().Width());
    get(mpFLineIndent,  "firstlineindent");
    mpFLineIndent->set_width_request(mpFLineIndent->get_preferred_size().Width());

    initial();
    m_aMetricCtl.RequestUpdate();
}

ParaPropertyPanel::~ParaPropertyPanel()
{
    disposeOnce();
}

void ParaPropertyPanel::dispose()
{
    mpTBxVertAlign.clear();
    mpTBxNumBullet.clear();
    mpTBxBackColor.clear();
    mpTBxOutline.clear();
    mpTopDist.clear();
    mpBottomDist.clear();
    mpLeftIndent.clear();
    mpRightIndent.clear();
    mpFLineIndent.clear();

    maLRSpaceControl.dispose();
    maULSpaceControl.dispose();
    m_aMetricCtl.dispose();

    PanelLayout::dispose();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
