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

#include "tp_Scale.hxx"

#include <ResId.hxx>
#include <strings.hrc>
#include <chartview/ChartSfxItemIds.hxx>
#include <AxisHelper.hxx>

#include <svx/svxids.hrc>
#include <osl/diagnose.h>
#include <sfx2/dialoghelper.hxx>
#include <svx/chrtitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/numformat.hxx>
#include <vcl/formatter.hxx>
#include <vcl/weld.hxx>
#include <svl/zformat.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

namespace chart
{

namespace
{

void lcl_setValue(weld::FormattedSpinButton& rFmtField, double fValue)
{
    rFmtField.GetFormatter().SetValue(fValue);
}

}

ScaleTabPage::ScaleTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_Scale.ui"_ustr, u"tp_Scale"_ustr, &rInAttrs)
    , fMin(0.0)
    , fMax(0.0)
    , fStepMain(0.0)
    , nStepHelp(0)
    , fOrigin(0.0)
    , m_nTimeResolution(1)
    , m_nMainTimeUnit(1)
    , m_nHelpTimeUnit(1)
    , m_nAxisType(chart2::AxisType::REALNUMBER)
    , m_bAllowDateAxis(false)
    , pNumFormatter(nullptr)
    , m_bShowAxisOrigin(false)
    , m_xCbxReverse(m_xBuilder->weld_check_button(u"CBX_REVERSE"_ustr))
    , m_xCbxLogarithm(m_xBuilder->weld_check_button(u"CBX_LOGARITHM"_ustr))
    , m_xBxType(m_xBuilder->weld_widget(u"boxTYPE"_ustr))
    , m_xLB_AxisType(m_xBuilder->weld_combo_box(u"LB_AXIS_TYPE"_ustr))
    , m_xBxMinMax(m_xBuilder->weld_widget(u"gridMINMAX"_ustr))
    , m_xFmtFldMin(m_xBuilder->weld_formatted_spin_button(u"EDT_MIN"_ustr))
    , m_xCbxAutoMin(m_xBuilder->weld_check_button(u"CBX_AUTO_MIN"_ustr))
    , m_xFmtFldMax(m_xBuilder->weld_formatted_spin_button(u"EDT_MAX"_ustr))
    , m_xCbxAutoMax(m_xBuilder->weld_check_button(u"CBX_AUTO_MAX"_ustr))
    , m_xBxResolution(m_xBuilder->weld_widget(u"boxRESOLUTION"_ustr))
    , m_xLB_TimeResolution(m_xBuilder->weld_combo_box(u"LB_TIME_RESOLUTION"_ustr))
    , m_xCbx_AutoTimeResolution(m_xBuilder->weld_check_button(u"CBX_AUTO_TIME_RESOLUTION"_ustr))
    , m_xTxtMain(m_xBuilder->weld_label(u"TXT_STEP_MAIN"_ustr))
    , m_xFmtFldStepMain(m_xBuilder->weld_formatted_spin_button(u"EDT_STEP_MAIN"_ustr))
    , m_xMt_MainDateStep(m_xBuilder->weld_spin_button(u"MT_MAIN_DATE_STEP"_ustr))
    , m_xLB_MainTimeUnit(m_xBuilder->weld_combo_box(u"LB_MAIN_TIME_UNIT"_ustr))
    , m_xCbxAutoStepMain(m_xBuilder->weld_check_button(u"CBX_AUTO_STEP_MAIN"_ustr))
    , m_xTxtHelpCount(m_xBuilder->weld_label(u"TXT_STEP_HELP_COUNT"_ustr))
    , m_xTxtHelp(m_xBuilder->weld_label(u"TXT_STEP_HELP"_ustr))
    , m_xMtStepHelp(m_xBuilder->weld_spin_button(u"MT_STEPHELP"_ustr))
    , m_xLB_HelpTimeUnit(m_xBuilder->weld_combo_box(u"LB_HELP_TIME_UNIT"_ustr))
    , m_xCbxAutoStepHelp(m_xBuilder->weld_check_button(u"CBX_AUTO_STEP_HELP"_ustr))
    , m_xFmtFldOrigin(m_xBuilder->weld_formatted_spin_button(u"EDT_ORIGIN"_ustr))
    , m_xCbxAutoOrigin(m_xBuilder->weld_check_button(u"CBX_AUTO_ORIGIN"_ustr))
    , m_xBxOrigin(m_xBuilder->weld_widget(u"boxORIGIN"_ustr))
{
    m_xCbxAutoMin->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoMax->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoStepMain->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoStepHelp->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoOrigin->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbx_AutoTimeResolution->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));

    Formatter& rFmtFldMax = m_xFmtFldMax->GetFormatter();
    rFmtFldMax.ClearMinValue();
    rFmtFldMax.ClearMaxValue();
    Formatter& rFmtFldMin = m_xFmtFldMin->GetFormatter();
    rFmtFldMin.ClearMinValue();
    rFmtFldMin.ClearMaxValue();
    Formatter& rFmtFldStepMain = m_xFmtFldStepMain->GetFormatter();
    rFmtFldStepMain.ClearMinValue();
    rFmtFldStepMain.ClearMaxValue();
    Formatter& rFmtFldOrigin = m_xFmtFldOrigin->GetFormatter();
    rFmtFldOrigin.ClearMinValue();
    rFmtFldOrigin.ClearMaxValue();

    m_xLB_AxisType->connect_changed(LINK(this, ScaleTabPage, SelectAxisTypeHdl));

    HideAllControls();
}

ScaleTabPage::~ScaleTabPage()
{
}

void ScaleTabPage::EnableControls()
{
    bool bValueAxis = m_nAxisType == chart2::AxisType::REALNUMBER
                   || m_nAxisType == chart2::AxisType::PERCENT
                   || m_nAxisType == chart2::AxisType::DATE;
    bool bDateAxis = m_nAxisType == chart2::AxisType::DATE;

    m_xBxType->set_visible(m_bAllowDateAxis);

    m_xCbxLogarithm->set_visible( bValueAxis && !bDateAxis );

    m_xBxMinMax->set_visible(bValueAxis);

    m_xTxtMain->set_visible( bValueAxis );
    m_xCbxAutoStepMain->set_visible( bValueAxis );

    m_xTxtHelpCount->set_visible( bValueAxis && !bDateAxis );
    m_xTxtHelp->set_visible( bDateAxis );
    m_xMtStepHelp->set_visible( bValueAxis );
    m_xCbxAutoStepHelp->set_visible( bValueAxis );

    m_xBxOrigin->set_visible( m_bShowAxisOrigin && bValueAxis );
    m_xBxResolution->set_visible( bDateAxis );

    bool bWasDateAxis = m_xMt_MainDateStep->get_visible();
    if( bWasDateAxis != bDateAxis )
    {
        //transport value from one to other control
        if( bWasDateAxis )
            lcl_setValue( *m_xFmtFldStepMain, m_xMt_MainDateStep->get_value() );
        else
            m_xMt_MainDateStep->set_value(m_xFmtFldStepMain->GetFormatter().GetValue());
    }

    m_xFmtFldStepMain->set_visible( bValueAxis && !bDateAxis );
    m_xMt_MainDateStep->set_visible( bDateAxis );

    m_xLB_MainTimeUnit->set_visible( bDateAxis );
    m_xLB_HelpTimeUnit->set_visible( bDateAxis );

    EnableValueHdl(*m_xCbxAutoMin);
    EnableValueHdl(*m_xCbxAutoMax);
    EnableValueHdl(*m_xCbxAutoStepMain);
    EnableValueHdl(*m_xCbxAutoStepHelp);
    EnableValueHdl(*m_xCbxAutoOrigin);
    EnableValueHdl(*m_xCbx_AutoTimeResolution);
}

IMPL_LINK( ScaleTabPage, EnableValueHdl, weld::Toggleable&, rCbx, void )
{
    bool bEnable = !rCbx.get_active() && rCbx.get_sensitive();
    if (&rCbx == m_xCbxAutoMin.get())
    {
        m_xFmtFldMin->set_sensitive( bEnable );
    }
    else if (&rCbx == m_xCbxAutoMax.get())
    {
        m_xFmtFldMax->set_sensitive( bEnable );
    }
    else if (&rCbx == m_xCbxAutoStepMain.get())
    {
        m_xFmtFldStepMain->set_sensitive( bEnable );
        m_xMt_MainDateStep->set_sensitive( bEnable );
        m_xLB_MainTimeUnit->set_sensitive( bEnable );
    }
    else if (&rCbx == m_xCbxAutoStepHelp.get())
    {
        m_xMtStepHelp->set_sensitive( bEnable );
        m_xLB_HelpTimeUnit->set_sensitive( bEnable );
    }
    else if (&rCbx == m_xCbx_AutoTimeResolution.get())
    {
        m_xLB_TimeResolution->set_sensitive( bEnable );
    }
    else if (&rCbx == m_xCbxAutoOrigin.get())
    {
        m_xFmtFldOrigin->set_sensitive( bEnable );
    }
}

namespace {

enum AxisTypeListBoxEntry
{
    TYPE_AUTO=0,
    TYPE_TEXT=1,
    TYPE_DATE=2
};

}

IMPL_LINK_NOARG(ScaleTabPage, SelectAxisTypeHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos = m_xLB_AxisType->get_active();
    if( nPos==TYPE_DATE )
        m_nAxisType = chart2::AxisType::DATE;
    else
        m_nAxisType = chart2::AxisType::CATEGORY;
    if( m_nAxisType == chart2::AxisType::DATE )
        m_xCbxLogarithm->set_active(false);
    EnableControls();
    SetNumFormat();
}

std::unique_ptr<SfxTabPage> ScaleTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<ScaleTabPage>(pPage, pController, *rOutAttrs);
}

bool ScaleTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    OSL_PRECOND( pNumFormatter, "No NumberFormatter available" );

    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXISTYPE, m_nAxisType));
    if(m_bAllowDateAxis)
        rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_DATEAXIS, m_xLB_AxisType->get_active()==TYPE_AUTO));

    bool bAutoScale = false;
    if( m_nAxisType==chart2::AxisType::CATEGORY )
        bAutoScale = true;//reset scaling for category charts

    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MIN      ,bAutoScale || m_xCbxAutoMin->get_active()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MAX      ,bAutoScale || m_xCbxAutoMax->get_active()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP,bAutoScale || m_xCbxAutoStepHelp->get_active()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN   ,bAutoScale || m_xCbxAutoOrigin->get_active()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM     ,m_xCbxLogarithm->get_active()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_REVERSE       ,m_xCbxReverse->get_active()));
    rOutAttrs->Put(SvxDoubleItem(fMax     , SCHATTR_AXIS_MAX));
    rOutAttrs->Put(SvxDoubleItem(fMin     , SCHATTR_AXIS_MIN));
    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_STEP_HELP, nStepHelp));
    rOutAttrs->Put(SvxDoubleItem(fOrigin  , SCHATTR_AXIS_ORIGIN));

    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN,bAutoScale || m_xCbxAutoStepMain->get_active()));
    rOutAttrs->Put(SvxDoubleItem(fStepMain,SCHATTR_AXIS_STEP_MAIN));

    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_TIME_RESOLUTION,bAutoScale || m_xCbx_AutoTimeResolution->get_active()));
    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_TIME_RESOLUTION,m_nTimeResolution));

    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_MAIN_TIME_UNIT,m_nMainTimeUnit));
    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_HELP_TIME_UNIT,m_nHelpTimeUnit));

    return true;
}

void ScaleTabPage::Reset(const SfxItemSet* rInAttrs)
{
    OSL_PRECOND( pNumFormatter, "No NumberFormatter available" );
    if(!pNumFormatter)
        return;

    if (const SfxBoolItem* pDateAxisItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_ALLOW_DATEAXIS))
        m_bAllowDateAxis = pDateAxisItem->GetValue();
    m_nAxisType=chart2::AxisType::REALNUMBER;
    if (const SfxInt32Item* pAxisTypeItem = rInAttrs->GetItemIfSet(SCHATTR_AXISTYPE))
        m_nAxisType = static_cast<int>(pAxisTypeItem->GetValue());
    if( m_nAxisType==chart2::AxisType::DATE && !m_bAllowDateAxis )
        m_nAxisType=chart2::AxisType::CATEGORY;
    if( m_bAllowDateAxis )
    {
        bool bAutoDateAxis = false;
        if (const SfxBoolItem* pDateAxisItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_AUTO_DATEAXIS))
            bAutoDateAxis = pDateAxisItem->GetValue();

        sal_uInt16 nPos = 0;
        if( m_nAxisType==chart2::AxisType::DATE )
            nPos=TYPE_DATE;
        else if( bAutoDateAxis )
            nPos=TYPE_AUTO;
        else
            nPos=TYPE_TEXT;
        m_xLB_AxisType->set_active( nPos );
    }

    m_xCbxAutoMin->set_active(true);
    m_xCbxAutoMax->set_active(true);
    m_xCbxAutoStepMain->set_active(true);
    m_xCbxAutoStepHelp->set_active(true);
    m_xCbxAutoOrigin->set_active(true);
    m_xCbx_AutoTimeResolution->set_active(true);

    if (const SfxBoolItem* pAutoMinItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_AUTO_MIN))
        m_xCbxAutoMin->set_active(pAutoMinItem->GetValue());

    if (const SvxDoubleItem* pAxisMinItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_MIN))
    {
        fMin = pAxisMinItem->GetValue();
        lcl_setValue( *m_xFmtFldMin, fMin );
        m_xFmtFldMin->save_value();
    }

    if (const SfxBoolItem* pAutoMaxItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_AUTO_MAX))
        m_xCbxAutoMax->set_active(pAutoMaxItem->GetValue());

    if (const SvxDoubleItem* pAxisMaxItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_MAX))
    {
        fMax = pAxisMaxItem->GetValue();
        lcl_setValue( *m_xFmtFldMax, fMax );
        m_xFmtFldMax->save_value();
    }

    if (const SfxBoolItem* pAutoStepMainItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_AUTO_STEP_MAIN))
        m_xCbxAutoStepMain->set_active(pAutoStepMainItem->GetValue());

    if (const SvxDoubleItem* pStepMainItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_STEP_MAIN))
    {
        fStepMain = pStepMainItem->GetValue();
        lcl_setValue( *m_xFmtFldStepMain, fStepMain );
        m_xFmtFldStepMain->save_value();
        m_xMt_MainDateStep->set_value( static_cast<sal_Int32>(fStepMain) );
        m_xMt_MainDateStep->save_value();
    }
    if (const SfxBoolItem* pAutoStepHelpItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_AUTO_STEP_HELP))
        m_xCbxAutoStepHelp->set_active(pAutoStepHelpItem->GetValue());
    if (const SfxBoolItem* pLogItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_LOGARITHM))
        m_xCbxLogarithm->set_active(pLogItem->GetValue());
    if (const SfxBoolItem* pReverseItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_REVERSE))
        m_xCbxReverse->set_active(pReverseItem->GetValue());
    if (const SfxInt32Item* pStepHelpItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_STEP_HELP))
    {
        nStepHelp = pStepHelpItem->GetValue();
        m_xMtStepHelp->set_value( nStepHelp );
        m_xMtStepHelp->save_value();
    }
    if (const SfxBoolItem* pOriginItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_AUTO_ORIGIN))
        m_xCbxAutoOrigin->set_active(pOriginItem->GetValue());
    if (const SvxDoubleItem* pOriginItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_ORIGIN))
    {
        fOrigin = pOriginItem->GetValue();
        lcl_setValue( *m_xFmtFldOrigin, fOrigin );
        m_xFmtFldOrigin->save_value();
    }

    if (const SfxBoolItem* pAutoTimeResItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_AUTO_TIME_RESOLUTION))
        m_xCbx_AutoTimeResolution->set_active(pAutoTimeResItem->GetValue());
    if (const SfxInt32Item* pTimeResItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_TIME_RESOLUTION))
    {
        m_nTimeResolution = pTimeResItem->GetValue();
        m_xLB_TimeResolution->set_active( m_nTimeResolution );
    }

    if (const SfxInt32Item* pMainTimeUnitItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_MAIN_TIME_UNIT))
    {
        m_nMainTimeUnit = pMainTimeUnitItem->GetValue();
        m_xLB_MainTimeUnit->set_active( m_nMainTimeUnit );
    }
    if (const SfxInt32Item* pHelpTimeUnitItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_HELP_TIME_UNIT))
    {
        m_nHelpTimeUnit = pHelpTimeUnitItem->GetValue();
        m_xLB_HelpTimeUnit->set_active( m_nHelpTimeUnit );
    }

    EnableControls();
    SetNumFormat();
}

DeactivateRC ScaleTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( !pNumFormatter )
    {
        OSL_FAIL( "No NumberFormatter available" );
        return DeactivateRC::LeavePage;
    }

    bool bDateAxis = m_nAxisType == chart2::AxisType::DATE;

    sal_uInt32 nMinMaxOriginFmt = m_xFmtFldMax->GetFormatter().GetFormatKey();
    if (pNumFormatter->GetType(nMinMaxOriginFmt) == SvNumFormatType::TEXT)
        nMinMaxOriginFmt = 0;
    // numberformat_text cause numbers to fail being numbers...  Shouldn't happen, but can.
    sal_uInt32 nStepFmt = m_xFmtFldStepMain->GetFormatter().GetFormatKey();
    if (pNumFormatter->GetType(nStepFmt) == SvNumFormatType::TEXT)
        nStepFmt = 0;

    weld::Widget* pControl = nullptr;
    TranslateId pErrStrId;
    double fDummy;

    fMax = m_xFmtFldMax->GetFormatter().GetValue();
    fMin = m_xFmtFldMin->GetFormatter().GetValue();
    fOrigin = m_xFmtFldOrigin->GetFormatter().GetValue();
    fStepMain = bDateAxis ? m_xMt_MainDateStep->get_value() : m_xFmtFldStepMain->GetFormatter().GetValue();
    nStepHelp = m_xMtStepHelp->get_value();
    m_nTimeResolution = m_xLB_TimeResolution->get_active();
    m_nMainTimeUnit = m_xLB_MainTimeUnit->get_active();
    m_nHelpTimeUnit = m_xLB_HelpTimeUnit->get_active();

    if( m_nAxisType != chart2::AxisType::REALNUMBER )
        m_xCbxLogarithm->hide();

    //check which entries need user action

    if ( m_xCbxLogarithm->get_active() &&
            ( ( !m_xCbxAutoMin->get_active() && fMin <= 0.0 )
             || ( !m_xCbxAutoMax->get_active() && fMax <= 0.0 ) ) )
    {
        pControl = m_xFmtFldMin.get();
        pErrStrId = STR_BAD_LOGARITHM;
    }
    // check for entries that cannot be parsed for the current number format
    else if ( m_xFmtFldMin->get_value_changed_from_saved()
              && !m_xCbxAutoMin->get_active()
              && !pNumFormatter->IsNumberFormat( m_xFmtFldMin->get_text(), nMinMaxOriginFmt, fDummy))
    {
        pControl = m_xFmtFldMin.get();
        pErrStrId = STR_INVALID_NUMBER;
    }
    else if ( m_xFmtFldMax->get_value_changed_from_saved()
              && !m_xCbxAutoMax->get_active()
              && !pNumFormatter->IsNumberFormat( m_xFmtFldMax->get_text(), nMinMaxOriginFmt, fDummy))
    {
        pControl = m_xFmtFldMax.get();
        pErrStrId = STR_INVALID_NUMBER;
    }
    else if ( !bDateAxis && m_xFmtFldStepMain->get_value_changed_from_saved()
              && !m_xCbxAutoStepMain->get_active()
              && !pNumFormatter->IsNumberFormat( m_xFmtFldStepMain->get_text(), nStepFmt, fDummy))
    {
        pControl = m_xFmtFldStepMain.get();
        pErrStrId = STR_INVALID_NUMBER;
    }
    else if (m_xFmtFldOrigin->get_value_changed_from_saved() && !m_xCbxAutoOrigin->get_active() &&
             !pNumFormatter->IsNumberFormat( m_xFmtFldOrigin->get_text(), nMinMaxOriginFmt, fDummy))
    {
        pControl = m_xFmtFldOrigin.get();
        pErrStrId = STR_INVALID_NUMBER;
    }
    else if (!m_xCbxAutoStepMain->get_active() && fStepMain <= 0.0)
    {
        pControl = m_xFmtFldStepMain.get();
        pErrStrId = STR_STEP_GT_ZERO;
    }
    else if (!m_xCbxAutoMax->get_active() && !m_xCbxAutoMin->get_active() &&
             fMin >= fMax)
    {
        pControl = m_xFmtFldMin.get();
        pErrStrId = STR_MIN_GREATER_MAX;
    }
    else if( bDateAxis )
    {
        if( !m_xCbxAutoStepMain->get_active() && !m_xCbxAutoStepHelp->get_active() )
        {
            if( m_nHelpTimeUnit > m_nMainTimeUnit )
            {
                pControl = m_xLB_MainTimeUnit.get();
                pErrStrId = STR_INVALID_INTERVALS;
            }
            else if( m_nHelpTimeUnit == m_nMainTimeUnit && nStepHelp > fStepMain )
            {
                pControl = m_xLB_MainTimeUnit.get();
                pErrStrId = STR_INVALID_INTERVALS;
            }
        }
        if( !pErrStrId && !m_xCbx_AutoTimeResolution->get_active() )
        {
            if( (!m_xCbxAutoStepMain->get_active() && m_nTimeResolution > m_nMainTimeUnit )
                ||
                (!m_xCbxAutoStepHelp->get_active() && m_nTimeResolution > m_nHelpTimeUnit )
                )
            {
                pControl = m_xLB_TimeResolution.get();
                pErrStrId = STR_INVALID_TIME_UNIT;
            }
        }
    }

    if( ShowWarning( pErrStrId, pControl ) )
        return DeactivateRC::KeepPage;

    if( pItemSet )
        FillItemSet( pItemSet );

    return DeactivateRC::LeavePage;
}

void ScaleTabPage::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    pNumFormatter = pFormatter;

    Formatter& rFmtFldMax = m_xFmtFldMax->GetFormatter();
    Formatter& rFmtFldMin = m_xFmtFldMin->GetFormatter();
    Formatter& rFmtFldStepMain = m_xFmtFldStepMain->GetFormatter();
    Formatter& rFmtFldOrigin = m_xFmtFldOrigin->GetFormatter();

    rFmtFldMax.SetFormatter( pNumFormatter );
    rFmtFldMin.SetFormatter( pNumFormatter );
    rFmtFldStepMain.SetFormatter( pNumFormatter );
    rFmtFldOrigin.SetFormatter( pNumFormatter );

    // #i6278# allow more decimal places than the output format.  As
    // the numbers shown in the edit fields are used for input, it makes more
    // sense to display the values in the input format rather than the output
    // format.
    rFmtFldMax.UseInputStringForFormatting();
    rFmtFldMin.UseInputStringForFormatting();
    rFmtFldStepMain.UseInputStringForFormatting();
    rFmtFldOrigin.UseInputStringForFormatting();

    SetNumFormat();
}

void ScaleTabPage::SetNumFormat()
{
    const SfxUInt32Item *pNumFormatItem = GetItemSet().GetItemIfSet( SID_ATTR_NUMBERFORMAT_VALUE );

    if( !pNumFormatItem )
        return;

    sal_uInt32 nFmt = pNumFormatItem->GetValue();

    Formatter& rFmtFldMax = m_xFmtFldMax->GetFormatter();
    rFmtFldMax.SetFormatKey(nFmt);
    Formatter& rFmtFldMin = m_xFmtFldMin->GetFormatter();
    rFmtFldMin.SetFormatKey(nFmt);
    Formatter& rFmtFldOrigin = m_xFmtFldOrigin->GetFormatter();
    rFmtFldOrigin.SetFormatKey(nFmt);

    if( pNumFormatter )
    {
        SvNumFormatType eType = pNumFormatter->GetType( nFmt );
        if( eType == SvNumFormatType::DATE )
        {
            // for intervals use standard format for dates (so you can enter a number of days)
            const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
            if( pFormat )
                nFmt = pNumFormatter->GetStandardIndex( pFormat->GetLanguage());
            else
                nFmt = pNumFormatter->GetStandardIndex();
        }
        else if( eType == SvNumFormatType::DATETIME )
        {
            // for intervals use time format for date times
            const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
            if( pFormat )
                nFmt = pNumFormatter->GetStandardFormat( SvNumFormatType::TIME, pFormat->GetLanguage() );
            else
                nFmt = pNumFormatter->GetStandardFormat( SvNumFormatType::TIME );

            // tdf#141625 give enough space to see full date+time
            int nWidestTime(m_xFmtFldMin->get_pixel_size(getWidestDateTime(Application::GetSettings().GetLocaleDataWrapper(), true)).Width());
            int nWidthChars = std::ceil(nWidestTime / m_xFmtFldMin->get_approximate_digit_width()) + 1;
            m_xFmtFldMin->set_width_chars(nWidthChars);
            m_xFmtFldMax->set_width_chars(nWidthChars);
        }

        if( m_nAxisType == chart2::AxisType::DATE && ( eType != SvNumFormatType::DATE && eType != SvNumFormatType::DATETIME) )
        {
            const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
            if( pFormat )
                nFmt = pNumFormatter->GetStandardFormat( SvNumFormatType::DATE, pFormat->GetLanguage() );
            else
                nFmt = pNumFormatter->GetStandardFormat( SvNumFormatType::DATE );

            rFmtFldMax.SetFormatKey(nFmt);
            rFmtFldMin.SetFormatKey(nFmt);
            rFmtFldOrigin.SetFormatKey(nFmt);
        }
    }

    m_xFmtFldStepMain->GetFormatter().SetFormatKey(nFmt);
}

void ScaleTabPage::ShowAxisOrigin( bool bShowOrigin )
{
    m_bShowAxisOrigin = bShowOrigin;
    if( !AxisHelper::isAxisPositioningEnabled() )
        m_bShowAxisOrigin = true;
}

bool ScaleTabPage::ShowWarning(TranslateId pResIdMessage, weld::Widget* pControl /* = nullptr */)
{
    if (!pResIdMessage)
        return false;

    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                               VclMessageType::Warning, VclButtonsType::Ok,
                                               SchResId(pResIdMessage)));
    xWarn->run();
    if (pControl)
    {
        pControl->grab_focus();
        weld::Entry* pEdit = dynamic_cast<weld::Entry*>(pControl);
        if (pEdit)
            pEdit->select_region(0, -1);
    }
    return true;
}

void ScaleTabPage::HideAllControls()
{
    // We need to set these controls invisible when the class is instantiated
    // since some code in EnableControls() depends on that logic. The real
    // visibility of these controls depend on axis data type, and are
    // set in EnableControls().

    m_xBxType->hide();
    m_xCbxLogarithm->hide();
    m_xBxMinMax->hide();
    m_xTxtMain->hide();
    m_xFmtFldStepMain->hide();
    m_xMt_MainDateStep->hide();
    m_xLB_MainTimeUnit->hide();
    m_xCbxAutoStepMain->hide();
    m_xTxtHelpCount->hide();
    m_xTxtHelp->hide();
    m_xMtStepHelp->hide();
    m_xCbxAutoStepHelp->hide();
    m_xBxOrigin->hide();
    m_xBxResolution->hide();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
