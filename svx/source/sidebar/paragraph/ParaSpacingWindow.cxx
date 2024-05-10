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
#include <editeng/lrspitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>

using namespace svx;

#define DEFAULT_VALUE          0
#define MAX_DURCH               31680 // tdf#68335: 1584 pt for UX interoperability with Word
#define MAX_SW                  1709400
#define MAX_SC_SD               116220200
#define NEGA_MAXVALUE          -10000000

// ParaULSpacingWindow

ParaULSpacingWindow::ParaULSpacingWindow(vcl::Window* pParent)
    : InterimItemWindow(pParent, u"svx/ui/paraulspacing.ui"_ustr, u"ParaULSpacingWindow"_ustr)
    , m_eUnit(MapUnit::MapTwip)
{
    m_xAboveSpacing.emplace(m_xBuilder->weld_metric_spin_button(u"aboveparaspacing"_ustr, FieldUnit::CM));
    m_xBelowSpacing.emplace(m_xBuilder->weld_metric_spin_button(u"belowparaspacing"_ustr, FieldUnit::CM));
    m_xAboveContainer = m_xBuilder->weld_container(u"above"_ustr);
    m_xBelowContainer = m_xBuilder->weld_container(u"below"_ustr);

    Link<weld::MetricSpinButton&,void> aLink = LINK(this, ParaULSpacingWindow, ModifySpacingHdl);
    m_xAboveSpacing->connect_value_changed(aLink);
    m_xBelowSpacing->connect_value_changed(aLink);

    /// set the initial values of max width
    m_xAboveSpacing->set_max(m_xAboveSpacing->normalize(MAX_DURCH), FieldUnit::CM);
    m_xBelowSpacing->set_max(m_xBelowSpacing->normalize(MAX_DURCH), FieldUnit::CM);
}

ParaULSpacingWindow::~ParaULSpacingWindow()
{
    disposeOnce();
}

void ParaULSpacingWindow::dispose()
{
    m_xAboveSpacing.reset();
    m_xBelowSpacing.reset();
    m_xAboveContainer.reset();
    m_xBelowContainer.reset();

    InterimItemWindow::dispose();
}

void ParaULSpacingWindow::SetUnit(FieldUnit eUnit)
{
    m_xAboveSpacing->SetFieldUnit(eUnit);
    m_xBelowSpacing->SetFieldUnit(eUnit);

    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    m_eUnit = rPool.GetMetric(SID_ATTR_PARA_ULSPACE);

    m_xAboveSpacing->set_max(m_xAboveSpacing->normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));
    m_xBelowSpacing->set_max(m_xBelowSpacing->normalize(MAX_DURCH), MapToFieldUnit(m_eUnit));
}

void ParaULSpacingWindow::SetValue(const SvxULSpaceItem* pItem)
{
    sal_Int64 nVal = pItem->GetUpper();
    nVal = m_xAboveSpacing->normalize(nVal);
    m_xAboveSpacing->set_value(nVal, FieldUnit::MM_100TH);

    nVal = pItem->GetLower();
    nVal = m_xBelowSpacing->normalize(nVal);
    m_xBelowSpacing->set_value(nVal, FieldUnit::MM_100TH);
}

IMPL_LINK_NOARG(ParaULSpacingWindow, ModifySpacingHdl, weld::MetricSpinButton&, void)
{
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if (!pFrame)
        return;
    SfxDispatcher* pDisp = pFrame->GetBindings().GetDispatcher();
    if(pDisp)
    {
        SvxULSpaceItem aMargin(SID_ATTR_PARA_ULSPACE);
        aMargin.SetUpper(m_xAboveSpacing->GetCoreValue(m_eUnit));
        aMargin.SetLower(m_xBelowSpacing->GetCoreValue(m_eUnit));
        pDisp->ExecuteList(SID_ATTR_PARA_ULSPACE, SfxCallMode::RECORD, {&aMargin});
    }
}

// ParaAboveSpacingWindow
ParaAboveSpacingWindow::ParaAboveSpacingWindow(vcl::Window* pParent)
    : ParaULSpacingWindow(pParent)
{
    InitControlBase(&m_xAboveSpacing->get_widget());

    m_xAboveContainer->show();
    m_xBelowContainer->hide();

    SetSizePixel(get_preferred_size());
}

// ParaBelowSpacingWindow
ParaBelowSpacingWindow::ParaBelowSpacingWindow(vcl::Window* pParent)
    : ParaULSpacingWindow(pParent)
{
    InitControlBase(&m_xBelowSpacing->get_widget());

    m_xAboveContainer->hide();
    m_xBelowContainer->show();

    SetSizePixel(get_preferred_size());
}

// ParaLRSpacingWindow
ParaLRSpacingWindow::ParaLRSpacingWindow(vcl::Window* pParent)
    : InterimItemWindow(pParent, u"svx/ui/paralrspacing.ui"_ustr, u"ParaLRSpacingWindow"_ustr)
    , m_eUnit(MapUnit::MapTwip)
{
    m_xBeforeSpacing.emplace(m_xBuilder->weld_metric_spin_button(u"beforetextindent"_ustr, FieldUnit::CM));
    m_xAfterSpacing.emplace(m_xBuilder->weld_metric_spin_button(u"aftertextindent"_ustr, FieldUnit::CM));
    m_xFLSpacing.emplace(m_xBuilder->weld_metric_spin_button(u"firstlineindent"_ustr, FieldUnit::CM));
    m_xBeforeContainer = m_xBuilder->weld_container(u"before"_ustr);
    m_xAfterContainer = m_xBuilder->weld_container(u"after"_ustr);
    m_xFirstLineContainer = m_xBuilder->weld_container(u"firstline"_ustr);

    Link<weld::MetricSpinButton&,void> aLink = LINK(this, ParaLRSpacingWindow, ModifySpacingHdl);
    m_xBeforeSpacing->connect_value_changed(aLink);
    m_xAfterSpacing->connect_value_changed(aLink);
    m_xFLSpacing->connect_value_changed(aLink);

    /// set the initial values of max width
    m_xBeforeSpacing->set_min(NEGA_MAXVALUE, FieldUnit::MM_100TH);
    m_xAfterSpacing->set_min(NEGA_MAXVALUE, FieldUnit::MM_100TH);
    m_xFLSpacing->set_min(NEGA_MAXVALUE, FieldUnit::MM_100TH);
}

ParaLRSpacingWindow::~ParaLRSpacingWindow()
{
    disposeOnce();
}

void ParaLRSpacingWindow::dispose()
{
    m_xBeforeSpacing.reset();
    m_xAfterSpacing.reset();
    m_xFLSpacing.reset();
    m_xBeforeContainer.reset();
    m_xAfterContainer.reset();
    m_xFirstLineContainer.reset();

    InterimItemWindow::dispose();
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
            m_xBeforeSpacing->set_min(DEFAULT_VALUE, FieldUnit::NONE);
            m_xAfterSpacing->set_min(DEFAULT_VALUE, FieldUnit::NONE);
            m_xFLSpacing->set_min(DEFAULT_VALUE, FieldUnit::NONE);
        }
        break;
    case CombinedEnumContext(Application::WriterVariants, Context::Default):
    case CombinedEnumContext(Application::WriterVariants, Context::Text):
    case CombinedEnumContext(Application::WriterVariants, Context::Table):
        {
            m_xBeforeSpacing->set_min(NEGA_MAXVALUE, FieldUnit::MM_100TH);
            m_xAfterSpacing->set_min(NEGA_MAXVALUE, FieldUnit::MM_100TH);
            m_xFLSpacing->set_min(NEGA_MAXVALUE, FieldUnit::MM_100TH);
        }
        break;
    }

    if(pState && eState >= SfxItemState::DEFAULT)
    {
        m_xBeforeSpacing->set_sensitive(true);
        m_xAfterSpacing->set_sensitive(true);
        m_xFLSpacing->set_sensitive(true);

        const SvxLRSpaceItem* pSpace = static_cast<const SvxLRSpaceItem*>(pState);
        tools::Long aTxtLeft = pSpace->GetTextLeft();
        tools::Long aTxtRight = pSpace->GetRight();
        tools::Long aTxtFirstLineOfst = pSpace->GetTextFirstLineOffset();

        aTxtLeft = m_xBeforeSpacing->normalize(aTxtLeft);

        if(m_aContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Text)
             && m_aContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Default)
             && m_aContext.GetCombinedContext_DI() != CombinedEnumContext(Application::WriterVariants, Context::Table))
        {
            m_xFLSpacing->set_min(aTxtLeft*-1, FieldUnit::MM_100TH);
        }

        aTxtRight = m_xAfterSpacing->normalize(aTxtRight);

        switch(m_aContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application::WriterVariants, Context::DrawText):
        case CombinedEnumContext(Application::WriterVariants, Context::Text):
        case CombinedEnumContext(Application::WriterVariants, Context::Default):
        case CombinedEnumContext(Application::WriterVariants, Context::Table):
        case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
            {
                m_xBeforeSpacing->set_max(MAX_SW - aTxtRight, FieldUnit::MM_100TH);
                m_xAfterSpacing->set_max(MAX_SW - aTxtLeft, FieldUnit::MM_100TH);
                m_xFLSpacing->set_max(MAX_SW - aTxtLeft - aTxtRight, FieldUnit::MM_100TH);
            }
            break;
        case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::Table):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
            {
                m_xBeforeSpacing->set_max(MAX_SC_SD - aTxtRight, FieldUnit::MM_100TH);
                m_xAfterSpacing->set_max(MAX_SC_SD - aTxtLeft, FieldUnit::MM_100TH);
                m_xFLSpacing->set_max(MAX_SC_SD - aTxtLeft - aTxtRight, FieldUnit::MM_100TH);
            }
        }

        m_xBeforeSpacing->set_value(aTxtLeft, FieldUnit::MM_100TH);
        m_xAfterSpacing->set_value(aTxtRight, FieldUnit::MM_100TH);

        aTxtFirstLineOfst = m_xFLSpacing->normalize(aTxtFirstLineOfst);
        m_xFLSpacing->set_value(aTxtFirstLineOfst, FieldUnit::MM_100TH);
    }
    else if(eState == SfxItemState::DISABLED)
    {
        m_xBeforeSpacing->set_sensitive(false);
        m_xAfterSpacing->set_sensitive(false);
        m_xFLSpacing->set_sensitive(false);
    }
    else
    {
        m_xBeforeSpacing->set_text(u""_ustr);
        m_xAfterSpacing->set_text(u""_ustr);
        m_xFLSpacing->set_text(u""_ustr);
    }
}

void ParaLRSpacingWindow::SetUnit(FieldUnit eUnit)
{
    m_xBeforeSpacing->SetFieldUnit(eUnit);
    m_xAfterSpacing->SetFieldUnit(eUnit);
    m_xFLSpacing->SetFieldUnit(eUnit);

    SfxItemPool &rPool = SfxGetpApp()->GetPool();
    m_eUnit = rPool.GetMetric(SID_ATTR_PARA_LRSPACE);
}

IMPL_LINK_NOARG(ParaLRSpacingWindow, ModifySpacingHdl, weld::MetricSpinButton&, void)
{
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if (!pFrame)
        return;
    SfxDispatcher* pDisp = pFrame->GetBindings().GetDispatcher();
    if(pDisp)
    {
        SvxLRSpaceItem aMargin(SID_ATTR_PARA_LRSPACE);
        aMargin.SetTextLeft(m_xBeforeSpacing->GetCoreValue(m_eUnit));
        aMargin.SetRight(m_xAfterSpacing->GetCoreValue(m_eUnit));
        aMargin.SetTextFirstLineOffset(m_xFLSpacing->GetCoreValue(m_eUnit));

        pDisp->ExecuteList(SID_ATTR_PARA_LRSPACE, SfxCallMode::RECORD, {&aMargin});
    }
}

// ParaLeftSpacingWindow
ParaLeftSpacingWindow::ParaLeftSpacingWindow(vcl::Window* pParent)
    : ParaLRSpacingWindow(pParent)
{
    InitControlBase(&m_xBeforeSpacing->get_widget());

    m_xBeforeContainer->show();
    m_xAfterContainer->hide();
    m_xFirstLineContainer->hide();

    SetSizePixel(get_preferred_size());
}

// ParaRightSpacingWindow
ParaRightSpacingWindow::ParaRightSpacingWindow(vcl::Window* pParent)
    : ParaLRSpacingWindow(pParent)
{
    InitControlBase(&m_xAfterSpacing->get_widget());

    m_xBeforeContainer->hide();
    m_xAfterContainer->show();
    m_xFirstLineContainer->hide();

    SetSizePixel(get_preferred_size());
}

// ParaFirstLineSpacingWindow
ParaFirstLineSpacingWindow::ParaFirstLineSpacingWindow(vcl::Window* pParent)
    : ParaLRSpacingWindow(pParent)
{
    InitControlBase(&m_xFLSpacing->get_widget());

    m_xBeforeContainer->hide();
    m_xAfterContainer->hide();
    m_xFirstLineContainer->show();

    SetSizePixel(get_preferred_size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
