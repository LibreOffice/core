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

#include "ResId.hxx"
#include "Strings.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "AxisHelper.hxx"

#include <svx/svxids.hrc>
#include <rtl/math.hxx>
#include <svx/chrtitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/msgbox.hxx>
#include <svl/zformat.hxx>
#include <svtools/controldims.hrc>

#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

namespace chart
{

namespace
{

void lcl_setValue( FormattedField& rFmtField, double fValue )
{
    rFmtField.SetValue( fValue );
    rFmtField.SetDefaultValue( fValue );
}

}

ScaleTabPage::ScaleTabPage(vcl::Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow
               , "tp_Scale"
               , "modules/schart/ui/tp_Scale.ui"
               , &rInAttrs),
    fMin(0.0),
    fMax(0.0),
    fStepMain(0.0),
    nStepHelp(0),
    fOrigin(0.0),
    m_nTimeResolution(1),
    m_nMainTimeUnit(1),
    m_nHelpTimeUnit(1),
    m_nAxisType(chart2::AxisType::REALNUMBER),
    m_bAllowDateAxis(false),
    pNumFormatter(nullptr),
    m_bShowAxisOrigin(false)
{
    get(m_pCbxReverse, "CBX_REVERSE");
    get(m_pCbxLogarithm, "CBX_LOGARITHM");
    get(m_pLB_AxisType, "LB_AXIS_TYPE");
    get(m_pBxType,"boxTYPE");

    get(m_pBxMinMax, "gridMINMAX");
    get(m_pFmtFldMin, "EDT_MIN");
    get(m_pCbxAutoMin, "CBX_AUTO_MIN");
    get(m_pFmtFldMax, "EDT_MAX");
    get(m_pCbxAutoMax, "CBX_AUTO_MAX");

    get(m_pBxResolution, "boxRESOLUTION");
    get(m_pLB_TimeResolution, "LB_TIME_RESOLUTION");
    get(m_pCbx_AutoTimeResolution, "CBX_AUTO_TIME_RESOLUTION");

    get(m_pTxtMain, "TXT_STEP_MAIN");
    get(m_pFmtFldStepMain, "EDT_STEP_MAIN");
    get(m_pMt_MainDateStep, "MT_MAIN_DATE_STEP");
    get(m_pLB_MainTimeUnit, "LB_MAIN_TIME_UNIT");
    get(m_pCbxAutoStepMain, "CBX_AUTO_STEP_MAIN");

    get(m_pMtStepHelp, "MT_STEPHELP");
    get(m_pLB_HelpTimeUnit, "LB_HELP_TIME_UNIT");
    get(m_pCbxAutoStepHelp, "CBX_AUTO_STEP_HELP");
    get(m_pTxtHelpCount,"TXT_STEP_HELP_COUNT");
    get(m_pTxtHelp,"TXT_STEP_HELP");

    get(m_pBxOrigin,"boxORIGIN");
    get(m_pFmtFldOrigin, "EDT_ORIGIN");
    get(m_pCbxAutoOrigin, "CBX_AUTO_ORIGIN");

    m_pCbxAutoMin->SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    m_pCbxAutoMax->SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    m_pCbxAutoStepMain->SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    m_pCbxAutoStepHelp->SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    m_pCbxAutoOrigin->SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));
    m_pCbx_AutoTimeResolution->SetClickHdl(LINK(this, ScaleTabPage, EnableValueHdl));

    m_pLB_AxisType->SetDropDownLineCount(3);
    m_pLB_AxisType->SetSelectHdl(LINK(this, ScaleTabPage, SelectAxisTypeHdl));

    m_pLB_TimeResolution->SetDropDownLineCount(3);
    m_pLB_MainTimeUnit->SetDropDownLineCount(3);
    m_pLB_HelpTimeUnit->SetDropDownLineCount(3);

    m_pFmtFldMin->SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));
    m_pFmtFldMax->SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));
    m_pFmtFldStepMain->SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));
    m_pFmtFldOrigin->SetModifyHdl(LINK(this, ScaleTabPage, FmtFieldModifiedHdl));

    HideAllControls();
}

ScaleTabPage::~ScaleTabPage()
{
    disposeOnce();
}

void ScaleTabPage::dispose()
{
    m_pCbxReverse.clear();
    m_pCbxLogarithm.clear();
    m_pBxType.clear();
    m_pLB_AxisType.clear();
    m_pBxMinMax.clear();
    m_pFmtFldMin.clear();
    m_pCbxAutoMin.clear();
    m_pFmtFldMax.clear();
    m_pCbxAutoMax.clear();
    m_pBxResolution.clear();
    m_pLB_TimeResolution.clear();
    m_pCbx_AutoTimeResolution.clear();
    m_pTxtMain.clear();
    m_pFmtFldStepMain.clear();
    m_pMt_MainDateStep.clear();
    m_pLB_MainTimeUnit.clear();
    m_pCbxAutoStepMain.clear();
    m_pTxtHelpCount.clear();
    m_pTxtHelp.clear();
    m_pMtStepHelp.clear();
    m_pLB_HelpTimeUnit.clear();
    m_pCbxAutoStepHelp.clear();
    m_pFmtFldOrigin.clear();
    m_pCbxAutoOrigin.clear();
    m_pBxOrigin.clear();
    SfxTabPage::dispose();
}

IMPL_STATIC_LINK_TYPED(
    ScaleTabPage, FmtFieldModifiedHdl, Edit&, rEdit, void )
{
    FormattedField& rFmtField = static_cast<FormattedField&>(rEdit);
    rFmtField.SetDefaultValue( rFmtField.GetValue() );
}

void ScaleTabPage::StateChanged( StateChangedType nType )
{
    TabPage::StateChanged( nType );
}

void ScaleTabPage::EnableControls()
{
    bool bValueAxis = chart2::AxisType::REALNUMBER == m_nAxisType
                   || chart2::AxisType::PERCENT == m_nAxisType
                   || chart2::AxisType::DATE == m_nAxisType;
    bool bDateAxis = chart2::AxisType::DATE == m_nAxisType;

    m_pBxType->Show(m_bAllowDateAxis);

    m_pCbxLogarithm->Show( bValueAxis && !bDateAxis );

    m_pBxMinMax->Show(bValueAxis);

    m_pTxtMain->Show( bValueAxis );
    m_pCbxAutoStepMain->Show( bValueAxis );

    m_pTxtHelpCount->Show( bValueAxis && !bDateAxis );
    m_pTxtHelp->Show( bDateAxis );
    m_pMtStepHelp->Show( bValueAxis );
    m_pCbxAutoStepHelp->Show( bValueAxis );

    m_pBxOrigin->Show( m_bShowAxisOrigin && bValueAxis );
    m_pBxResolution->Show( bDateAxis );

    bool bWasDateAxis = m_pMt_MainDateStep->IsVisible();
    if( bWasDateAxis != bDateAxis )
    {
        //transport value from one to other control
        if( bWasDateAxis )
            lcl_setValue( *m_pFmtFldStepMain, m_pMt_MainDateStep->GetValue() );
        else
            m_pMt_MainDateStep->SetValue( static_cast<sal_Int32>(m_pFmtFldStepMain->GetValue()) );
    }

    m_pFmtFldStepMain->Show( bValueAxis && !bDateAxis );
    m_pMt_MainDateStep->Show( bDateAxis );

    m_pLB_MainTimeUnit->Show( bDateAxis );
    m_pLB_HelpTimeUnit->Show( bDateAxis );

    EnableValueHdl(m_pCbxAutoMin);
    EnableValueHdl(m_pCbxAutoMax);
    EnableValueHdl(m_pCbxAutoStepMain);
    EnableValueHdl(m_pCbxAutoStepHelp);
    EnableValueHdl(m_pCbxAutoOrigin);
    EnableValueHdl(m_pCbx_AutoTimeResolution);
}

IMPL_LINK_TYPED( ScaleTabPage, EnableValueHdl, Button *, pButton, void )
{
    CheckBox * pCbx = static_cast<CheckBox*>(pButton);
    bool bEnable = pCbx && !pCbx->IsChecked() && pCbx->IsEnabled();
    if (pCbx == m_pCbxAutoMin)
    {
        m_pFmtFldMin->Enable( bEnable );
    }
    else if (pCbx == m_pCbxAutoMax)
    {
        m_pFmtFldMax->Enable( bEnable );
    }
    else if (pCbx == m_pCbxAutoStepMain)
    {
        m_pFmtFldStepMain->Enable( bEnable );
        m_pMt_MainDateStep->Enable( bEnable );
        m_pLB_MainTimeUnit->Enable( bEnable );
    }
    else if (pCbx == m_pCbxAutoStepHelp)
    {
        m_pMtStepHelp->Enable( bEnable );
        m_pLB_HelpTimeUnit->Enable( bEnable );
    }
    else if (pCbx == m_pCbx_AutoTimeResolution)
    {
        m_pLB_TimeResolution->Enable( bEnable );
    }
    else if (pCbx == m_pCbxAutoOrigin)
    {
        m_pFmtFldOrigin->Enable( bEnable );
    }
}

enum AxisTypeListBoxEntry
{
    TYPE_AUTO=0,
    TYPE_TEXT=1,
    TYPE_DATE=2
};

IMPL_LINK_NOARG_TYPED(ScaleTabPage, SelectAxisTypeHdl, ListBox&, void)
{
    const sal_Int32 nPos = m_pLB_AxisType->GetSelectEntryPos();
    if( nPos==TYPE_DATE )
        m_nAxisType = chart2::AxisType::DATE;
    else
        m_nAxisType = chart2::AxisType::CATEGORY;
    if( chart2::AxisType::DATE == m_nAxisType )
        m_pCbxLogarithm->Check(false);
    EnableControls();
    SetNumFormat();
}

VclPtr<SfxTabPage> ScaleTabPage::Create(vcl::Window* pWindow,const SfxItemSet* rOutAttrs)
{
    return VclPtr<ScaleTabPage>::Create(pWindow, *rOutAttrs);
}

bool ScaleTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    OSL_PRECOND( pNumFormatter, "No NumberFormatter available" );

    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXISTYPE, m_nAxisType));
    if(m_bAllowDateAxis)
        rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_DATEAXIS, TYPE_AUTO==m_pLB_AxisType->GetSelectEntryPos()));

    bool bAutoScale = false;
    if( m_nAxisType==chart2::AxisType::CATEGORY )
        bAutoScale = true;//reset scaling for category charts

    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MIN      ,bAutoScale || m_pCbxAutoMin->IsChecked()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MAX      ,bAutoScale || m_pCbxAutoMax->IsChecked()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP,bAutoScale || m_pCbxAutoStepHelp->IsChecked()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN   ,bAutoScale || m_pCbxAutoOrigin->IsChecked()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM     ,m_pCbxLogarithm->IsChecked()));
    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_REVERSE       ,m_pCbxReverse->IsChecked()));
    rOutAttrs->Put(SvxDoubleItem(fMax     , SCHATTR_AXIS_MAX));
    rOutAttrs->Put(SvxDoubleItem(fMin     , SCHATTR_AXIS_MIN));
    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_STEP_HELP, nStepHelp));
    rOutAttrs->Put(SvxDoubleItem(fOrigin  , SCHATTR_AXIS_ORIGIN));

    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN,bAutoScale || m_pCbxAutoStepMain->IsChecked()));
    rOutAttrs->Put(SvxDoubleItem(fStepMain,SCHATTR_AXIS_STEP_MAIN));

    rOutAttrs->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_TIME_RESOLUTION,bAutoScale || m_pCbx_AutoTimeResolution->IsChecked()));
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
        m_bAllowDateAxis = (bool) static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
    m_nAxisType=chart2::AxisType::REALNUMBER;
    if (rInAttrs->GetItemState(SCHATTR_AXISTYPE, true, &pPoolItem) == SfxItemState::SET)
        m_nAxisType = (int) static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
    if( m_nAxisType==chart2::AxisType::DATE && !m_bAllowDateAxis )
        m_nAxisType=chart2::AxisType::CATEGORY;
    if( m_bAllowDateAxis )
    {
        bool bAutoDateAxis = false;
        if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_DATEAXIS, true, &pPoolItem) == SfxItemState::SET)
            bAutoDateAxis = (bool) static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();

        sal_uInt16 nPos = 0;
        if( m_nAxisType==chart2::AxisType::DATE )
            nPos=TYPE_DATE;
        else if( bAutoDateAxis )
            nPos=TYPE_AUTO;
        else
            nPos=TYPE_TEXT;
        m_pLB_AxisType->SelectEntryPos( nPos );
    }

    m_pCbxAutoMin->Check();
    m_pCbxAutoMax->Check();
    m_pCbxAutoStepMain->Check();
    m_pCbxAutoStepHelp->Check();
    m_pCbxAutoOrigin->Check();
    m_pCbx_AutoTimeResolution->Check();

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_MIN,true,&pPoolItem) == SfxItemState::SET)
        m_pCbxAutoMin->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());

    if (rInAttrs->GetItemState(SCHATTR_AXIS_MIN,true, &pPoolItem) == SfxItemState::SET)
    {
        fMin = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_pFmtFldMin, fMin );
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_MAX,true, &pPoolItem) == SfxItemState::SET)
        m_pCbxAutoMax->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());

    if (rInAttrs->GetItemState(SCHATTR_AXIS_MAX,true, &pPoolItem) == SfxItemState::SET)
    {
        fMax = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_pFmtFldMax, fMax );
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_STEP_MAIN,true, &pPoolItem) == SfxItemState::SET)
        m_pCbxAutoStepMain->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());

    if (rInAttrs->GetItemState(SCHATTR_AXIS_STEP_MAIN,true, &pPoolItem) == SfxItemState::SET)
    {
        fStepMain = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_pFmtFldStepMain, fStepMain );
        m_pMt_MainDateStep->SetValue( static_cast<sal_Int32>(fStepMain) );
    }
    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_STEP_HELP,true, &pPoolItem) == SfxItemState::SET)
        m_pCbxAutoStepHelp->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_LOGARITHM,true, &pPoolItem) == SfxItemState::SET)
        m_pCbxLogarithm->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_REVERSE,true, &pPoolItem) == SfxItemState::SET)
        m_pCbxReverse->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_STEP_HELP,true, &pPoolItem) == SfxItemState::SET)
    {
        nStepHelp = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pMtStepHelp->SetValue( nStepHelp );
    }
    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_ORIGIN,true, &pPoolItem) == SfxItemState::SET)
        m_pCbxAutoOrigin->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_ORIGIN,true, &pPoolItem) == SfxItemState::SET)
    {
        fOrigin = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
        lcl_setValue( *m_pFmtFldOrigin, fOrigin );
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_AUTO_TIME_RESOLUTION,true, &pPoolItem) == SfxItemState::SET)
        m_pCbx_AutoTimeResolution->Check(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    if (rInAttrs->GetItemState(SCHATTR_AXIS_TIME_RESOLUTION,true, &pPoolItem) == SfxItemState::SET)
    {
        m_nTimeResolution = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pLB_TimeResolution->SelectEntryPos( m_nTimeResolution );
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_MAIN_TIME_UNIT,true, &pPoolItem) == SfxItemState::SET)
    {
        m_nMainTimeUnit = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pLB_MainTimeUnit->SelectEntryPos( m_nMainTimeUnit );
    }
    if (rInAttrs->GetItemState(SCHATTR_AXIS_HELP_TIME_UNIT,true, &pPoolItem) == SfxItemState::SET)
    {
        m_nHelpTimeUnit = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pLB_HelpTimeUnit->SelectEntryPos( m_nHelpTimeUnit );
    }

    EnableControls();
    SetNumFormat();
}

SfxTabPage::sfxpg ScaleTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( !pNumFormatter )
    {
        OSL_FAIL( "No NumberFormatter available" );
        return LEAVE_PAGE;
    }

    bool bDateAxis = chart2::AxisType::DATE == m_nAxisType;

    sal_uInt32 nMinMaxOriginFmt = m_pFmtFldMax->GetFormatKey();
    if ((pNumFormatter->GetType(nMinMaxOriginFmt) & ~css::util::NumberFormat::DEFINED) == css::util::NumberFormat::TEXT)
        nMinMaxOriginFmt = 0;
    // numberformat_text cause numbers to fail being numbers...  Shouldn't happen, but can.
    sal_uInt32 nStepFmt = m_pFmtFldStepMain->GetFormatKey();
    if ((pNumFormatter->GetType(nStepFmt) & ~css::util::NumberFormat::DEFINED) == css::util::NumberFormat::TEXT)
        nStepFmt = 0;

    Control* pControl = nullptr;
    sal_uInt16 nErrStrId = 0;
    double fDummy;

    fMax = m_pFmtFldMax->GetValue();
    fMin = m_pFmtFldMin->GetValue();
    fOrigin = m_pFmtFldOrigin->GetValue();
    fStepMain = bDateAxis ? m_pMt_MainDateStep->GetValue() : m_pFmtFldStepMain->GetValue();
    nStepHelp = static_cast< sal_Int32 >( m_pMtStepHelp->GetValue());
    m_nTimeResolution = m_pLB_TimeResolution->GetSelectEntryPos();
    m_nMainTimeUnit = m_pLB_MainTimeUnit->GetSelectEntryPos();
    m_nHelpTimeUnit = m_pLB_HelpTimeUnit->GetSelectEntryPos();

    if( chart2::AxisType::REALNUMBER != m_nAxisType )
        m_pCbxLogarithm->Show( false );

    //check which entries need user action

    if ( m_pCbxLogarithm->IsChecked() &&
            ( ( !m_pCbxAutoMin->IsChecked() && fMin <= 0.0 )
             || ( !m_pCbxAutoMax->IsChecked() && fMax <= 0.0 ) ) )
    {
        pControl = m_pFmtFldMin;
        nErrStrId = STR_BAD_LOGARITHM;
    }
    // check for entries that cannot be parsed for the current number format
    else if ( m_pFmtFldMin->IsModified()
              && !m_pCbxAutoMin->IsChecked()
              && !pNumFormatter->IsNumberFormat( m_pFmtFldMin->GetText(), nMinMaxOriginFmt, fDummy))
    {
        pControl = m_pFmtFldMin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if ( m_pFmtFldMax->IsModified()
              && !m_pCbxAutoMax->IsChecked()
              && !pNumFormatter->IsNumberFormat( m_pFmtFldMax->GetText(), nMinMaxOriginFmt, fDummy))
    {
        pControl = m_pFmtFldMax;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if ( !bDateAxis && m_pFmtFldStepMain->IsModified()
              && !m_pCbxAutoStepMain->IsChecked()
              && !pNumFormatter->IsNumberFormat( m_pFmtFldStepMain->GetText(), nStepFmt, fDummy))
    {
        pControl = m_pFmtFldStepMain;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (m_pFmtFldOrigin->IsModified() && !m_pCbxAutoOrigin->IsChecked() &&
             !pNumFormatter->IsNumberFormat( m_pFmtFldOrigin->GetText(), nMinMaxOriginFmt, fDummy))
    {
        pControl = m_pFmtFldOrigin;
        nErrStrId = STR_INVALID_NUMBER;
    }
    else if (!m_pCbxAutoStepMain->IsChecked() && fStepMain <= 0.0)
    {
        pControl = m_pFmtFldStepMain;
        nErrStrId = STR_STEP_GT_ZERO;
    }
    else if (!m_pCbxAutoMax->IsChecked() && !m_pCbxAutoMin->IsChecked() &&
             fMin >= fMax)
    {
        pControl = m_pFmtFldMin;
        nErrStrId = STR_MIN_GREATER_MAX;
    }
    else if( bDateAxis )
    {
        if( !m_pCbxAutoStepMain->IsChecked() && !m_pCbxAutoStepHelp->IsChecked() )
        {
            if( m_nHelpTimeUnit > m_nMainTimeUnit )
            {
                pControl = m_pLB_MainTimeUnit;
                nErrStrId = STR_INVALID_INTERVALS;
            }
            else if( m_nHelpTimeUnit == m_nMainTimeUnit && nStepHelp > fStepMain )
            {
                pControl = m_pLB_MainTimeUnit;
                nErrStrId = STR_INVALID_INTERVALS;
            }
        }
        if( !nErrStrId && !m_pCbx_AutoTimeResolution->IsChecked() )
        {
            if( (!m_pCbxAutoStepMain->IsChecked() && m_nTimeResolution > m_nMainTimeUnit )
                ||
                (!m_pCbxAutoStepHelp->IsChecked() && m_nTimeResolution > m_nHelpTimeUnit )
                )
            {
                pControl = m_pLB_TimeResolution;
                nErrStrId = STR_INVALID_TIME_UNIT;
            }
        }
    }

    if( ShowWarning( nErrStrId, pControl ) )
        return KEEP_PAGE;

    if( pItemSet )
        FillItemSet( pItemSet );

    return LEAVE_PAGE;
}

void ScaleTabPage::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    pNumFormatter = pFormatter;
    m_pFmtFldMax->SetFormatter( pNumFormatter );
    m_pFmtFldMin->SetFormatter( pNumFormatter );
    m_pFmtFldStepMain->SetFormatter( pNumFormatter );
    m_pFmtFldOrigin->SetFormatter( pNumFormatter );

    // #i6278# allow more decimal places than the output format.  As
    // the numbers shown in the edit fields are used for input, it makes more
    // sense to display the values in the input format rather than the output
    // format.
    m_pFmtFldMax->UseInputStringForFormatting();
    m_pFmtFldMin->UseInputStringForFormatting();
    m_pFmtFldStepMain->UseInputStringForFormatting();
    m_pFmtFldOrigin->UseInputStringForFormatting();

    SetNumFormat();
}

void ScaleTabPage::SetNumFormat()
{
    const SfxPoolItem *pPoolItem = nullptr;

    if( GetItemSet().GetItemState( SID_ATTR_NUMBERFORMAT_VALUE, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_uLong nFmt = (sal_uLong)static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();

        m_pFmtFldMax->SetFormatKey( nFmt );
        m_pFmtFldMin->SetFormatKey( nFmt );
        m_pFmtFldOrigin->SetFormatKey( nFmt );

        if( pNumFormatter )
        {
            short eType = pNumFormatter->GetType( nFmt );
            if( eType == css::util::NumberFormat::DATE )
            {
                // for intervals use standard format for dates (so you can enter a number of days)
                const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
                if( pFormat )
                    nFmt = pNumFormatter->GetStandardIndex( pFormat->GetLanguage());
                else
                    nFmt = pNumFormatter->GetStandardIndex();
            }
            else if( eType == css::util::NumberFormat::DATETIME )
            {
                // for intervals use time format for date times
                const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
                if( pFormat )
                    nFmt = pNumFormatter->GetStandardFormat( css::util::NumberFormat::TIME, pFormat->GetLanguage() );
                else
                    nFmt = pNumFormatter->GetStandardFormat( css::util::NumberFormat::TIME );
            }

            if( chart2::AxisType::DATE == m_nAxisType && ( eType != css::util::NumberFormat::DATE && eType != css::util::NumberFormat::DATETIME) )
            {
                const SvNumberformat* pFormat = pNumFormatter->GetEntry( nFmt );
                if( pFormat )
                    nFmt = pNumFormatter->GetStandardFormat( css::util::NumberFormat::DATE, pFormat->GetLanguage() );
                else
                    nFmt = pNumFormatter->GetStandardFormat( css::util::NumberFormat::DATE );

                m_pFmtFldMax->SetFormatKey( nFmt );
                m_pFmtFldMin->SetFormatKey( nFmt );
                m_pFmtFldOrigin->SetFormatKey( nFmt );
            }
        }

        m_pFmtFldStepMain->SetFormatKey( nFmt );
    }
}

void ScaleTabPage::ShowAxisOrigin( bool bShowOrigin )
{
    m_bShowAxisOrigin = bShowOrigin;
    if( !AxisHelper::isAxisPositioningEnabled() )
        m_bShowAxisOrigin = true;
}

bool ScaleTabPage::ShowWarning( sal_uInt16 nResIdMessage, Control* pControl /* = NULL */ )
{
    if( nResIdMessage == 0 )
        return false;

    ScopedVclPtrInstance<WarningBox>(this, WinBits( WB_OK ), SCH_RESSTR(nResIdMessage))->Execute();
    if( pControl )
    {
        pControl->GrabFocus();
        Edit* pEdit = dynamic_cast<Edit*>(pControl);
        if(pEdit)
            pEdit->SetSelection( Selection( 0, SELECTION_MAX ));
    }
    return true;
}

void ScaleTabPage::HideAllControls()
{
    // We need to set these controls invisible when the class is instantiated
    // since some code in EnableControls() depends on that logic. The real
    // visibility of these controls depend on axis data type, and are
    // set in EnableControls().

    m_pBxType->Hide();
    m_pCbxLogarithm->Hide();
    m_pBxMinMax->Hide();
    m_pTxtMain->Hide();
    m_pFmtFldStepMain->Hide();
    m_pMt_MainDateStep->Hide();
    m_pLB_MainTimeUnit->Hide();
    m_pCbxAutoStepMain->Hide();
    m_pTxtHelpCount->Hide();
    m_pTxtHelp->Hide();
    m_pMtStepHelp->Hide();
    m_pCbxAutoStepHelp->Hide();
    m_pBxOrigin->Hide();
    m_pBxResolution->Hide();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
