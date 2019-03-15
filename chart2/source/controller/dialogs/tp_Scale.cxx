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
#include <svx/chrtitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/weld.hxx>
#include <svl/zformat.hxx>

#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

namespace chart
{

namespace
{

void lcl_setValue(weld::FormattedSpinButton& rFmtField, double fValue)
{
    rFmtField.set_value(fValue);
}

}

ScaleTabPage::ScaleTabPage(TabPageParent pWindow,const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow, "modules/schart/ui/tp_Scale.ui", "tp_Scale", &rInAttrs)
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
    , m_xCbxReverse(m_xBuilder->weld_check_button("CBX_REVERSE"))
    , m_xCbxLogarithm(m_xBuilder->weld_check_button("CBX_LOGARITHM"))
    , m_xBxType(m_xBuilder->weld_widget("boxTYPE"))
    , m_xLB_AxisType(m_xBuilder->weld_combo_box("LB_AXIS_TYPE"))
    , m_xBxMinMax(m_xBuilder->weld_widget("gridMINMAX"))
    , m_xFmtFldMin(m_xBuilder->weld_formatted_spin_button("EDT_MIN"))
    , m_xCbxAutoMin(m_xBuilder->weld_check_button("CBX_AUTO_MIN"))
    , m_xFmtFldMax(m_xBuilder->weld_formatted_spin_button("EDT_MAX"))
    , m_xCbxAutoMax(m_xBuilder->weld_check_button("CBX_AUTO_MAX"))
    , m_xBxResolution(m_xBuilder->weld_widget("boxRESOLUTION"))
    , m_xLB_TimeResolution(m_xBuilder->weld_combo_box("LB_TIME_RESOLUTION"))
    , m_xCbx_AutoTimeResolution(m_xBuilder->weld_check_button("CBX_AUTO_TIME_RESOLUTION"))
    , m_xTxtMain(m_xBuilder->weld_label("TXT_STEP_MAIN"))
    , m_xFmtFldStepMain(m_xBuilder->weld_formatted_spin_button("EDT_STEP_MAIN"))
    , m_xMt_MainDateStep(m_xBuilder->weld_spin_button("MT_MAIN_DATE_STEP"))
    , m_xLB_MainTimeUnit(m_xBuilder->weld_combo_box("LB_MAIN_TIME_UNIT"))
    , m_xCbxAutoStepMain(m_xBuilder->weld_check_button("CBX_AUTO_STEP_MAIN"))
    , m_xTxtHelpCount(m_xBuilder->weld_label("TXT_STEP_HELP_COUNT"))
    , m_xTxtHelp(m_xBuilder->weld_label("TXT_STEP_HELP"))
    , m_xMtStepHelp(m_xBuilder->weld_spin_button("MT_STEPHELP"))
    , m_xLB_HelpTimeUnit(m_xBuilder->weld_combo_box("LB_HELP_TIME_UNIT"))
    , m_xCbxAutoStepHelp(m_xBuilder->weld_check_button("CBX_AUTO_STEP_HELP"))
    , m_xFmtFldOrigin(m_xBuilder->weld_formatted_spin_button("EDT_ORIGIN"))
    , m_xCbxAutoOrigin(m_xBuilder->weld_check_button("CBX_AUTO_ORIGIN"))
    , m_xBxOrigin(m_xBuilder->weld_widget("boxORIGIN"))
{
    m_xCbxAutoMin->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoMax->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoStepMain->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoStepHelp->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbxAutoOrigin->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));
    m_xCbx_AutoTimeResolution->connect_toggled(LINK(this, ScaleTabPage, EnableValueHdl));

    const double nMin = static_cast<double>(SAL_MIN_INT64);
    const double nMax = static_cast<double>(SAL_MAX_INT64);
    m_xFmtFldMin->set_range(nMin, nMax);
    m_xFmtFldMax->set_range(nMin, nMax);
    m_xFmtFldStepMain->set_range(nMin, nMax);
    m_xFmtFldOrigin->set_range(nMin, nMax);

    m_xLB_AxisType->connect_changed(LINK(this, ScaleTabPage, SelectAxisTypeHdl));

    HideAllControls();
}

ScaleTabPage::~ScaleTabPage()
{
    disposeOnce();
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
            m_xMt_MainDateStep->set_value(m_xFmtFldStepMain->get_value());
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

IMPL_LINK( ScaleTabPage, EnableValueHdl, weld::ToggleButton&, rCbx, void )
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

enum AxisTypeListBoxEntry
{
    TYPE_AUTO=0,
    TYPE_TEXT=1,
    TYPE_DATE=2
};

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

VclPtr<SfxTabPage> ScaleTabPage::Create(TabPageParent pParent, const SfxItemSet* rOutAttrs)
{
    return VclPtr<ScaleTabPage>::Create(pParent, *rOutAttrs);
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

    const SfxPoolItem *pPoolItem = nullptr;
    if (rInAttrs->GetItemState(SCHATTR_AXIS_ALLOW_DATEAXIS, true, &pPoolItem) == SfxItemState::SET)
        m_bAllowDateAxis = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
    m_nAxisType=chart2::AxisType::REALNUMBER;
    if (rInAttrs->GetItemState(SCHATTR_AXISTYPE, true, &pPoolItem) == SfxItemState::SET)
        m_nAxisType = static_cast<int>(static_cast<const SfxInt32Item*>(pPoolItem)->GetValue());
    if( m_nAxisType==chart2::AxisType::DATE && !m_bAllowDateAxis )
        m_nAxisType=chart2::AxisType::CATEGORY;
    if( m_bAllowDateAxis )
    {
        bool bAutoDateAxis = false;
        if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_DATEAXIS, true, &pPoolItem) == SfxItemState::SET)
            bAutoDateAxis = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();

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

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_MIN,true,&pPoolItem) == SfxItemState::SET)
        m_xCbxAutoMin->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());

    if (rInAttrs->GetItemState(SCHATTR_AXIS_MIN,true, &pPoolItem) == SfxItemState::SET)
    {
        fMin = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_xFmtFldMin, fMin );
        m_xFmtFldMin->save_value();
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_MAX,true, &pPoolItem) == SfxItemState::SET)
        m_xCbxAutoMax->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());

    if (rInAttrs->GetItemState(SCHATTR_AXIS_MAX,true, &pPoolItem) == SfxItemState::SET)
    {
        fMax = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_xFmtFldMax, fMax );
        m_xFmtFldMax->save_value();
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_STEP_MAIN,true, &pPoolItem) == SfxItemState::SET)
        m_xCbxAutoStepMain->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());

    if (rInAttrs->GetItemState(SCHATTR_AXIS_STEP_MAIN,true, &pPoolItem) == SfxItemState::SET)
    {
        fStepMain = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_xFmtFldStepMain, fStepMain );
        m_xFmtFldStepMain->save_value();
        m_xMt_MainDateStep->set_value( static_cast<sal_Int32>(fStepMain) );
        m_xMt_MainDateStep->save_value();
    }
    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_STEP_HELP,true, &pPoolItem) == SfxItemState::SET)
        m_xCbxAutoStepHelp->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_LOGARITHM,true, &pPoolItem) == SfxItemState::SET)
        m_xCbxLogarithm->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_REVERSE,true, &pPoolItem) == SfxItemState::SET)
        m_xCbxReverse->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_STEP_HELP,true, &pPoolItem) == SfxItemState::SET)
    {
        nStepHelp = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_xMtStepHelp->set_value( nStepHelp );
        m_xMtStepHelp->save_value();
    }
    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_ORIGIN,true, &pPoolItem) == SfxItemState::SET)
        m_xCbxAutoOrigin->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_ORIGIN,true, &pPoolItem) == SfxItemState::SET)
    {
        fOrigin = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_xFmtFldOrigin, fOrigin );
        m_xFmtFldOrigin->save_value();
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_TIME_RESOLUTION,true, &pPoolItem) == SfxItemState::SET)
        m_xCbx_AutoTimeResolution->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_TIME_RESOLUTION,true, &pPoolItem) == SfxItemState::SET)
    {
        m_nTimeResolution = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_xLB_TimeResolution->set_active( m_nTimeResolution );
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_MAIN_TIME_UNIT,true, &pPoolItem) == SfxItemState::SET)
    {
        m_nMainTimeUnit = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_xLB_MainTimeUnit->set_active( m_nMainTimeUnit );
    }
    if (rInAttrs->GetItemState(SCHATTR_AXIS_HELP_TIME_UNIT,true, &pPoolItem) == SfxItemState::SET)
    {
        m_nHelpTimeUnit = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
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

    sal_uInt32 nMinMaxOriginFmt = m_xFmtFldMax->get_format_key();
    if (pNumFormatter->GetType(nMinMaxOriginFmt) == SvNumFormatType::TEXT)
        nMinMaxOriginFmt = 0;
    // numberformat_text cause numbers to fail being numbers...  Shouldn't happen, but can.
    sal_uInt32 nStepFmt = m_xFmtFldStepMain->get_format_key();
    if (pNumFormatter->GetType(nStepFmt) == SvNumFormatType::TEXT)
        nStepFmt = 0;

    weld::Widget* pControl = nullptr;
    const char* pErrStrId = nullptr;
    double fDummy;

    fMax = m_xFmtFldMax->get_value();
    fMin = m_xFmtFldMin->get_value();
    fOrigin = m_xFmtFldOrigin->get_value();
    fStepMain = bDateAxis ? m_xMt_MainDateStep->get_value() : m_xFmtFldStepMain->get_value();
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
    m_xFmtFldMax->set_formatter( pNumFormatter );
    m_xFmtFldMin->set_formatter( pNumFormatter );
    m_xFmtFldStepMain->set_formatter( pNumFormatter );
    m_xFmtFldOrigin->set_formatter( pNumFormatter );
    SetNumFormat();
}

void ScaleTabPage::SetNumFormat()
{
    const SfxPoolItem *pPoolItem = nullptr;

    if( GetItemSet().GetItemState( SID_ATTR_NUMBERFORMAT_VALUE, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_uLong nFmt = static_cast<const SfxUInt32Item*>(pPoolItem)->GetValue();

        m_xFmtFldMax->set_format_key(nFmt);
        m_xFmtFldMin->set_format_key(nFmt);
        m_xFmtFldOrigin->set_format_key(nFmt);

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
            }

            if( m_nAxisType == chart2::AxisType::DATE && ( eType != SvNumFormatType::DATE && eType != SvNumFormatType::DATETIME) )
            {
                const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
                if( pFormat )
                    nFmt = pNumFormatter->GetStandardFormat( SvNumFormatType::DATE, pFormat->GetLanguage() );
                else
                    nFmt = pNumFormatter->GetStandardFormat( SvNumFormatType::DATE );

                m_xFmtFldMax->set_format_key(nFmt);
                m_xFmtFldMin->set_format_key(nFmt);
                m_xFmtFldOrigin->set_format_key(nFmt);
            }
        }

        m_xFmtFldStepMain->set_format_key(nFmt);
    }
}

void ScaleTabPage::ShowAxisOrigin( bool bShowOrigin )
{
    m_bShowAxisOrigin = bShowOrigin;
    if( !AxisHelper::isAxisPositioningEnabled() )
        m_bShowAxisOrigin = true;
}

bool ScaleTabPage::ShowWarning(const char* pResIdMessage, weld::Widget* pControl /* = nullptr */)
{
    if (pResIdMessage == nullptr)
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
