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

using namespace svx;

#define DEFAULT_VALUE          0
#define MAX_DURCH             5670
#define MAX_SW                  1709400
#define MAX_SC_SD               116220200
#define NEGA_MAXVALUE          -10000000

// ParaULSpacingWindow

ParaULSpacingWindow::ParaULSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame> const & xFrame)
    : VclVBox(pParent)
    , m_eUnit(MapUnit::MapTwip)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(),
                                  "svx/ui/paraulspacing.ui",
                                  "ParaULSpacingWindow",
                                  xFrame));

    get(m_pAboveSpacing, "aboveparaspacing");
    get(m_pBelowSpacing, "belowparaspacing");
    get(m_pAboveContainer, "above");
    get(m_pBelowContainer, "below");

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
    m_pAboveContainer.clear();
    m_pBelowContainer.clear();

    disposeBuilder();
    VclVBox::dispose();
}

void ParaULSpacingWindow::SetUnit(FieldUnit eUnit)
{
    SetFieldUnit(*m_pAboveSpacing, eUnit);
    SetFieldUnit(*m_pBelowSpacing, eUnit);

    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    sal_uInt16 nWhich = rPool.GetWhich(SID_ATTR_PARA_ULSPACE);
    m_eUnit = rPool.GetMetric(nWhich);

    m_pAboveSpacing->SetMax(m_pAboveSpacing->Normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));
    m_pBelowSpacing->SetMax(m_pBelowSpacing->Normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));
}

void ParaULSpacingWindow::SetValue(const SvxULSpaceItem* pItem)
{
    sal_Int64 nVal = pItem->GetUpper();
    nVal = m_pAboveSpacing->Normalize(nVal);
    m_pAboveSpacing->SetValue(nVal, FUNIT_100TH_MM);

    nVal = pItem->GetLower();
    nVal = m_pBelowSpacing->Normalize(nVal);
    m_pBelowSpacing->SetValue(nVal, FUNIT_100TH_MM);
}

IMPL_LINK_NOARG(ParaULSpacingWindow, ModifySpacingHdl, Edit&, void)
{
    SfxDispatcher* pDisp = SfxViewFrame::Current()->GetBindings().GetDispatcher();
    if(pDisp)
    {
        SvxULSpaceItem aMargin(SID_ATTR_PARA_ULSPACE);
        aMargin.SetUpper(static_cast<sal_uInt16>(GetCoreValue(*m_pAboveSpacing, m_eUnit)));
        aMargin.SetLower(static_cast<sal_uInt16>(GetCoreValue(*m_pBelowSpacing, m_eUnit)));
        pDisp->ExecuteList(SID_ATTR_PARA_ULSPACE, SfxCallMode::RECORD, {&aMargin});
    }
}

// ParaAboveSpacingWindow

ParaAboveSpacingWindow::ParaAboveSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame> const & xFrame)
    : ParaULSpacingWindow(pParent, xFrame)
{
    m_pAboveContainer->Show();
    m_pBelowContainer->Hide();
}

// ParaBelowSpacingWindow

ParaBelowSpacingWindow::ParaBelowSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame> const & xFrame)
    : ParaULSpacingWindow(pParent, xFrame)
{
    m_pAboveContainer->Hide();
    m_pBelowContainer->Show();
}

// ParaLRSpacingWindow

ParaLRSpacingWindow::ParaLRSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame> const & xFrame)
    : VclVBox(pParent)
    , m_eUnit(MapUnit::MapTwip)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(),
                                  "svx/ui/paralrspacing.ui",
                                  "ParaLRSpacingWindow",
                                  xFrame));

    get(m_pBeforeSpacing, "beforetextindent");
    get(m_pAfterSpacing, "aftertextindent");
    get(m_pFLSpacing, "firstlineindent");
    get(m_pBeforeContainer, "before");
    get(m_pAfterContainer, "after");
    get(m_pFirstLineContainer, "firstline");

    Link<Edit&,void> aLink = LINK(this, ParaLRSpacingWindow, ModifySpacingHdl);
    m_pBeforeSpacing->SetModifyHdl(aLink);
    m_pAfterSpacing->SetModifyHdl(aLink);
    m_pFLSpacing->SetModifyHdl(aLink);
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
    m_pBeforeContainer.clear();
    m_pAfterContainer.clear();
    m_pFirstLineContainer.clear();

    disposeBuilder();
    VclVBox::dispose();
}

void ParaLRSpacingWindow::SetContext(const vcl::EnumContext& eContext)
{
    m_aContext = eContext;
}

void ParaLRSpacingWindow::SetValue(SfxItemState eState, const SfxPoolItem* pState)
{
    switch(m_aContext.GetCombinedContext_DI())
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
            m_pBeforeSpacing->SetMin(DEFAULT_VALUE);
            m_pAfterSpacing->SetMin(DEFAULT_VALUE);
            m_pFLSpacing->SetMin(DEFAULT_VALUE);
        }
        break;
    case CombinedEnumContext(Application::WriterVariants, Context::Default):
    case CombinedEnumContext(Application::WriterVariants, Context::Text):
    case CombinedEnumContext(Application::WriterVariants, Context::Table):
        {
            m_pBeforeSpacing->SetMin(NEGA_MAXVALUE, FUNIT_100TH_MM);
            m_pAfterSpacing->SetMin(NEGA_MAXVALUE, FUNIT_100TH_MM);
            m_pFLSpacing->SetMin(NEGA_MAXVALUE, FUNIT_100TH_MM);
        }
        break;
    }

    if(pState && eState >= SfxItemState::DEFAULT)
    {
        m_pBeforeSpacing-> Enable();
        m_pAfterSpacing->Enable();
        m_pFLSpacing->Enable();

        const SvxLRSpaceItem* pSpace = static_cast<const SvxLRSpaceItem*>(pState);
        long aTxtLeft = pSpace->GetTextLeft();
        long aTxtRight = pSpace->GetRight();
        long aTxtFirstLineOfst = pSpace->GetTextFirstLineOfst();

        aTxtLeft = static_cast<long>(m_pBeforeSpacing->Normalize(aTxtLeft));

        if(m_aContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Text)
             && m_aContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Default)
             && m_aContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Table))
        {
            m_pFLSpacing->SetMin(aTxtLeft*-1, FUNIT_100TH_MM);
        }

        aTxtRight = static_cast<long>(m_pAfterSpacing->Normalize(aTxtRight));

        switch(m_aContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application::WriterVariants, Context::DrawText):
        case CombinedEnumContext(Application::WriterVariants, Context::Text):
        case CombinedEnumContext(Application::WriterVariants, Context::Default):
        case CombinedEnumContext(Application::WriterVariants, Context::Table):
        case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
            {
                m_pBeforeSpacing->SetMax(MAX_SW - aTxtRight, FUNIT_100TH_MM);
                m_pAfterSpacing->SetMax(MAX_SW - aTxtLeft, FUNIT_100TH_MM);
                m_pFLSpacing->SetMax(MAX_SW - aTxtLeft - aTxtRight, FUNIT_100TH_MM);
            }
            break;
        case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::Table):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
            {
                m_pBeforeSpacing->SetMax(MAX_SC_SD - aTxtRight, FUNIT_100TH_MM);
                m_pAfterSpacing->SetMax(MAX_SC_SD - aTxtLeft, FUNIT_100TH_MM);
                m_pFLSpacing->SetMax(MAX_SC_SD - aTxtLeft - aTxtRight, FUNIT_100TH_MM);
            }
        }

        m_pBeforeSpacing->SetValue(aTxtLeft, FUNIT_100TH_MM);
        m_pAfterSpacing->SetValue(aTxtRight, FUNIT_100TH_MM);

        aTxtFirstLineOfst = static_cast<long>(m_pFLSpacing->Normalize(aTxtFirstLineOfst));
        m_pFLSpacing->SetValue(aTxtFirstLineOfst, FUNIT_100TH_MM);
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

    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    sal_uInt16 nWhich = rPool.GetWhich(SID_ATTR_PARA_LRSPACE);
    m_eUnit = rPool.GetMetric(nWhich);
}

IMPL_LINK_NOARG(ParaLRSpacingWindow, ModifySpacingHdl, Edit&, void)
{
    SfxDispatcher* pDisp = SfxViewFrame::Current()->GetBindings().GetDispatcher();
    if(pDisp)
    {
        SvxLRSpaceItem aMargin(SID_ATTR_PARA_LRSPACE);
        aMargin.SetTextLeft(GetCoreValue(*m_pBeforeSpacing, m_eUnit));
        aMargin.SetRight(GetCoreValue(*m_pAfterSpacing, m_eUnit));
        aMargin.SetTextFirstLineOfst(static_cast<short>(GetCoreValue(*m_pFLSpacing, m_eUnit)));

        pDisp->ExecuteList(SID_ATTR_PARA_LRSPACE, SfxCallMode::RECORD, {&aMargin});
    }
}

// ParaLeftSpacingWindow

ParaLeftSpacingWindow::ParaLeftSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame> const & xFrame)
    : ParaLRSpacingWindow(pParent, xFrame)
{
    m_pBeforeContainer->Show();
    m_pAfterContainer->Hide();
    m_pFirstLineContainer->Hide();
}

// ParaRightSpacingWindow

ParaRightSpacingWindow::ParaRightSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame> const & xFrame)
    : ParaLRSpacingWindow(pParent, xFrame)
{
    m_pBeforeContainer->Hide();
    m_pAfterContainer->Show();
    m_pFirstLineContainer->Hide();
}

// ParaFirstLineSpacingWindow

ParaFirstLineSpacingWindow::ParaFirstLineSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame> const & xFrame)
    : ParaLRSpacingWindow(pParent, xFrame)
{
    m_pBeforeContainer->Hide();
    m_pAfterContainer->Hide();
    m_pFirstLineContainer->Show();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
