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

#include "ParaSpacingWindow.hxx"
#include <editeng/editids.hrc>
#include <sfx2/dispatch.hxx>
#include <svl/itempool.hxx>
#include <svl/intitem.hxx>
#include <vcl/builderfactory.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>

using namespace svx;

#define DEFAULT_VALUE          0
#define MAX_DURCH             5670
#define MAX_SW                  1709400
#define MAX_SC_SD               116220200
#define NEGA_MAXVALUE          -10000000

VCL_BUILDER_FACTORY(ParaLRSpacingWindow);

// ParaULSpacingWindow

ParaULSpacingWindow::ParaULSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame>& xFrame)
    : VclVBox(pParent)
    , m_pULSpaceControl(nullptr)
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(),
                                  "svx/ui/paraulspacing.ui",
                                  "ParaULSpacingWindow",
                                  xFrame);

    get(m_pAboveSpacing, "aboveparaspacing");
    get(m_pBelowSpacing, "belowparaspacing");

    Link<Edit&,void> aLink = LINK(this, ParaULSpacingWindow, ModifySpacingHdl);
    m_pAboveSpacing->SetModifyHdl(aLink);
    m_pBelowSpacing->SetModifyHdl(aLink);
}

ParaULSpacingWindow::~ParaULSpacingWindow()
{
    disposeOnce();
}

void ParaULSpacingWindow::dispose()
{
    m_pAboveSpacing.clear();
    m_pBelowSpacing.clear();
    if(m_pULSpaceControl)
        m_pULSpaceControl->dispose();

    disposeBuilder();
    VclVBox::dispose();
}

void ParaULSpacingWindow::Initialize()
{
    const SfxPoolItem* pItem;
    SfxViewFrame* pView = SfxViewFrame::Current();

    if(pView)
    {
        pView->GetBindings().GetDispatcher()->QueryState(SID_ATTR_METRIC, pItem);
        const SfxUInt16Item* pMetricItem = static_cast<const SfxUInt16Item*>(pItem);

        if(pMetricItem)
        {
            SetUnit((FieldUnit)pMetricItem->GetValue());

            m_pAboveSpacing->SetMax(m_pAboveSpacing->Normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));
            m_pBelowSpacing->SetMax(m_pBelowSpacing->Normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));

            if(!m_pULSpaceControl)
            {
                SfxBindings& rBindings = pView->GetBindings();
                m_pULSpaceControl = new ::sfx2::sidebar::ControllerItem(SID_ATTR_PARA_ULSPACE,
                                                                    rBindings,
                                                                    *this);
            }
        }
    }
}

void ParaULSpacingWindow::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool /*bIsEnabled*/)
{
    if(nSID == SID_ATTR_PARA_ULSPACE && pState && eState >= SfxItemState::DEFAULT)
    {
        const SvxULSpaceItem* pItem = static_cast<const SvxULSpaceItem*>(pState);
        this->SetValue(pItem);
    }
}

void ParaULSpacingWindow::SetUnit(FieldUnit eUnit)
{
    SetFieldUnit(*m_pAboveSpacing, eUnit);
    SetFieldUnit(*m_pBelowSpacing, eUnit);

    m_eUnit = GetCoreMetric();
}

void ParaULSpacingWindow::SetValue(const SvxULSpaceItem* pItem)
{
    m_pAboveSpacing->SetMax(m_pAboveSpacing->Normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));
    m_pBelowSpacing->SetMax(m_pBelowSpacing->Normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));

    long maUpper = pItem->GetUpper();

    maUpper = OutputDevice::LogicToLogic(maUpper, (MapUnit)m_eUnit, MAP_100TH_MM);
    maUpper = OutputDevice::LogicToLogic(maUpper, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP));

    long maLower = pItem->GetLower();
    maLower = OutputDevice::LogicToLogic(maLower, (MapUnit)m_eUnit, MAP_100TH_MM);
    maLower = OutputDevice::LogicToLogic(maLower, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP));

    sal_Int64 nVal = OutputDevice::LogicToLogic(maUpper, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM);
    nVal = m_pAboveSpacing->Normalize(nVal);
    m_pAboveSpacing->SetValue(nVal, FUNIT_100TH_MM);

    nVal = OutputDevice::LogicToLogic(maLower, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM);
    nVal = m_pBelowSpacing->Normalize(nVal);
    m_pBelowSpacing->SetValue(nVal, FUNIT_100TH_MM);
}

SfxMapUnit ParaULSpacingWindow::GetCoreMetric() const
{
    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    sal_uInt16 nWhich = rPool.GetWhich(SID_ATTR_PARA_ULSPACE);
    return rPool.GetMetric(nWhich);
}

IMPL_LINK_NOARG_TYPED(ParaULSpacingWindow, ModifySpacingHdl, Edit&, void)
{
    SfxDispatcher* pDisp = SfxViewFrame::Current()->GetBindings().GetDispatcher();
    if(pDisp)
    {
        SvxULSpaceItem aMargin(SID_ATTR_PARA_ULSPACE);
        aMargin.SetUpper((sal_uInt16)GetCoreValue(*m_pAboveSpacing, m_eUnit));
        aMargin.SetLower((sal_uInt16)GetCoreValue(*m_pBelowSpacing, m_eUnit));
        pDisp->ExecuteList(SID_ATTR_PARA_ULSPACE, SfxCallMode::RECORD, {&aMargin});
    }
}

// ParaLRSpacingWindow

ParaLRSpacingWindow::ParaLRSpacingWindow(vcl::Window* pParent)
    : VclVBox(pParent)
{
    css::uno::Reference<css::frame::XFrame> xFrame;
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(),
                                  "svx/ui/paralrspacing.ui",
                                  "ParaLRSpacingWindow",
                                  xFrame);

    get(m_pBeforeSpacing, "beforetextindent");
    get(m_pAfterSpacing, "aftertextindent");
    get(m_pFLSpacing, "firstlineindent");

    Link<Edit&,void> aLink = LINK(this, ParaLRSpacingWindow, ModifySpacingHdl);
    m_pBeforeSpacing->SetModifyHdl(aLink);
    m_pAfterSpacing->SetModifyHdl(aLink);
    m_pFLSpacing->SetModifyHdl(aLink);

    m_aLateInitTimer.SetTimeout(200);
    m_aLateInitTimer.SetTimeoutHdl(LINK(this, ParaLRSpacingWindow, LateInitCallback));
    m_aLateInitTimer.Start();
}

ParaLRSpacingWindow::~ParaLRSpacingWindow()
{
    disposeOnce();
}

void ParaLRSpacingWindow::dispose()
{
    m_pBeforeSpacing.clear();
    m_pAfterSpacing.clear();
    m_pFLSpacing.clear();
    m_pLRSpaceControl->dispose();

    disposeBuilder();
    VclVBox::dispose();
}

IMPL_LINK_NOARG_TYPED(ParaLRSpacingWindow, LateInitCallback, Timer *, void)
{
    const SfxPoolItem* pItem;
    SfxViewFrame* pView = SfxViewFrame::Current();

    if(pView)
    {
        pView->GetBindings().GetDispatcher()->QueryState(SID_ATTR_METRIC, pItem);
        const SfxUInt16Item* pMetricItem = static_cast<const SfxUInt16Item*>(pItem);

        if(pMetricItem)
        {
            this->SetUnit((FieldUnit)pMetricItem->GetValue());
            SfxBindings& rBindings = pView->GetBindings();
            m_pLRSpaceControl = new ::sfx2::sidebar::ControllerItem(SID_ATTR_PARA_LRSPACE,
                                                                    rBindings,
                                                                    *this);
        }

        css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
            css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        css::uno::Reference<css::frame::XFrame> xFrame = pView->GetFrame().GetFrameInterface();
        if(xMultiplexer.is())
            xMultiplexer->addContextChangeEventListener(this, xFrame->getController());
    }
}

void SAL_CALL ParaLRSpacingWindow::notifyContextChangeEvent (
    const ui::ContextChangeEventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    maContext = ::sfx2::sidebar::EnumContext(
        ::sfx2::sidebar::EnumContext::GetApplicationEnum(rEvent.ApplicationName),
        ::sfx2::sidebar::EnumContext::GetContextEnum(rEvent.ContextName));
}

void ParaLRSpacingWindow::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)nSID;
    (void)bIsEnabled;

    switch (maContext.GetCombinedContext_DI())
    {

    case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
    case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
    case CombinedEnumContext(Application_Calc, Context_DrawText):
    case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
    case CombinedEnumContext(Application_DrawImpress, Context_Draw):
    case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
    case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
    case CombinedEnumContext(Application_DrawImpress, Context_Table):
        {
            m_pBeforeSpacing->SetMin(DEFAULT_VALUE);
            m_pAfterSpacing->SetMin(DEFAULT_VALUE);
            m_pFLSpacing->SetMin(DEFAULT_VALUE);
        }
        break;
    case CombinedEnumContext(Application_WriterVariants, Context_Default):
    case CombinedEnumContext(Application_WriterVariants, Context_Text):
    case CombinedEnumContext(Application_WriterVariants, Context_Table):
        {
            m_pBeforeSpacing->SetMin(NEGA_MAXVALUE, FUNIT_100TH_MM);
            m_pAfterSpacing->SetMin(NEGA_MAXVALUE, FUNIT_100TH_MM);
            m_pFLSpacing->SetMin(NEGA_MAXVALUE, FUNIT_100TH_MM);
        }
        break;
    }

    if(pState && eState >= SfxItemState::DEFAULT)
    {
        const SvxLRSpaceItem* pSpace = static_cast<const SvxLRSpaceItem*>(pState);
        long maTxtLeft = pSpace->GetTextLeft();
        maTxtLeft = OutputDevice::LogicToLogic(maTxtLeft, (MapUnit)m_eUnit, MAP_100TH_MM);
        maTxtLeft = OutputDevice::LogicToLogic(maTxtLeft, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP));

        long aTxtRight = pSpace->GetRight();
        aTxtRight = OutputDevice::LogicToLogic(aTxtRight, (MapUnit)m_eUnit, MAP_100TH_MM);
        aTxtRight = OutputDevice::LogicToLogic(aTxtRight, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP));

        long aTxtFirstLineOfst = pSpace->GetTextFirstLineOfst();
        aTxtFirstLineOfst = OutputDevice::LogicToLogic(aTxtFirstLineOfst, (MapUnit)m_eUnit, MAP_100TH_MM);
        aTxtFirstLineOfst = OutputDevice::LogicToLogic(aTxtFirstLineOfst, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP));

        long nVal = OutputDevice::LogicToLogic(maTxtLeft, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM);
        nVal = (long)m_pBeforeSpacing->Normalize((long)nVal);

        if ( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Table))
        {
            m_pFLSpacing->SetMin(nVal*(-1), FUNIT_100TH_MM);
        }

        long nrVal = OutputDevice::LogicToLogic(aTxtRight, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM);
        nrVal = (long)m_pAfterSpacing->Normalize((long)nrVal);

        switch (maContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Text):
        case CombinedEnumContext(Application_WriterVariants, Context_Default):
        case CombinedEnumContext(Application_WriterVariants, Context_Table):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
            {
                m_pBeforeSpacing->SetMax(MAX_SW - nrVal, FUNIT_100TH_MM);
                m_pAfterSpacing->SetMax(MAX_SW - nVal, FUNIT_100TH_MM);
                m_pFLSpacing->SetMax(MAX_SW - nVal - nrVal, FUNIT_100TH_MM);
            }
            break;
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            {
                m_pBeforeSpacing->SetMax(MAX_SC_SD - nrVal, FUNIT_100TH_MM);
                m_pAfterSpacing->SetMax(MAX_SC_SD - nVal, FUNIT_100TH_MM);
                m_pFLSpacing->SetMax(MAX_SC_SD - nVal - nrVal, FUNIT_100TH_MM);
            }
        }

        m_pBeforeSpacing->SetValue(nVal, FUNIT_100TH_MM);
        m_pAfterSpacing->SetValue(nrVal, FUNIT_100TH_MM);

        long nfVal = OutputDevice::LogicToLogic(aTxtFirstLineOfst, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM);
        nfVal = (long)m_pFLSpacing->Normalize((long)nfVal);
        m_pFLSpacing->SetValue(nfVal, FUNIT_100TH_MM);
    }
    else if(eState == SfxItemState::DISABLED)
    {
        m_pBeforeSpacing-> Disable();
        m_pAfterSpacing->Disable();
        m_pFLSpacing->Disable();
    }
    else
    {
        m_pBeforeSpacing->SetEmptyFieldValue();
        m_pAfterSpacing->SetEmptyFieldValue();
        m_pFLSpacing->SetEmptyFieldValue();
    }
}

void ParaLRSpacingWindow::SetUnit(FieldUnit eUnit)
{
    SetFieldUnit(*m_pBeforeSpacing, eUnit);
    SetFieldUnit(*m_pAfterSpacing, eUnit);
    SetFieldUnit(*m_pFLSpacing, eUnit);

    m_eUnit = GetCoreMetric();
}

SfxMapUnit ParaLRSpacingWindow::GetCoreMetric() const
{
    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    sal_uInt16 nWhich = rPool.GetWhich(SID_ATTR_PARA_LRSPACE);
    return rPool.GetMetric(nWhich);
}

IMPL_LINK_NOARG_TYPED(ParaLRSpacingWindow, ModifySpacingHdl, Edit&, void)
{
    SfxDispatcher* pDisp = SfxViewFrame::Current()->GetBindings().GetDispatcher();
    if(pDisp)
    {
        SvxLRSpaceItem aMargin(SID_ATTR_PARA_LRSPACE);
        aMargin.SetTextLeft((const long)GetCoreValue(*m_pBeforeSpacing, m_eUnit));
        aMargin.SetRight((const long)GetCoreValue(*m_pAfterSpacing, m_eUnit));
        aMargin.SetTextFirstLineOfst((const short)GetCoreValue(*m_pFLSpacing, m_eUnit));

        pDisp->ExecuteList(SID_ATTR_PARA_LRSPACE, SfxCallMode::RECORD, {&aMargin});
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
