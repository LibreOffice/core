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

#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif

#include "dpgroupdlg.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

namespace {

/** Date part flags in order of the list box entries. */
static const sal_Int32 spnDateParts[] =
{
    css::sheet::DataPilotFieldGroupBy::SECONDS,
    css::sheet::DataPilotFieldGroupBy::MINUTES,
    css::sheet::DataPilotFieldGroupBy::HOURS,
    css::sheet::DataPilotFieldGroupBy::DAYS,
    css::sheet::DataPilotFieldGroupBy::MONTHS,
    css::sheet::DataPilotFieldGroupBy::QUARTERS,
    css::sheet::DataPilotFieldGroupBy::YEARS
};

static const sal_uInt16 nDatePartResIds[] =
{
    STR_DPFIELD_GROUP_BY_SECONDS,
    STR_DPFIELD_GROUP_BY_MINUTES,
    STR_DPFIELD_GROUP_BY_HOURS,
    STR_DPFIELD_GROUP_BY_DAYS,
    STR_DPFIELD_GROUP_BY_MONTHS,
    STR_DPFIELD_GROUP_BY_QUARTERS,
    STR_DPFIELD_GROUP_BY_YEARS
};

} // namespace

ScDPGroupEditHelper::ScDPGroupEditHelper( RadioButton* pRbAuto, RadioButton* pRbMan, Edit* pEdValue ) :
    mpRbAuto( pRbAuto ),
    mpRbMan( pRbMan ),
    mpEdValue( pEdValue )
{
    mpRbAuto->SetClickHdl( LINK( this, ScDPGroupEditHelper, ClickHdl ) );
    mpRbMan->SetClickHdl( LINK( this, ScDPGroupEditHelper, ClickHdl ) );
}

bool ScDPGroupEditHelper::IsAuto() const
{
    return mpRbAuto->IsChecked();
}

double ScDPGroupEditHelper::GetValue() const
{
    double fValue;
    if( !ImplGetValue( fValue ) )
        fValue = 0.0;
    return fValue;
}

void ScDPGroupEditHelper::SetValue( bool bAuto, double fValue )
{
    if( bAuto )
    {
        mpRbAuto->Check();
        ClickHdl( mpRbAuto );
    }
    else
    {
        mpRbMan->Check();
        ClickHdl( mpRbMan );
    }
    ImplSetValue( fValue );
}

IMPL_LINK_TYPED( ScDPGroupEditHelper, ClickHdl, Button*, pButton, void )
{
    if( pButton == mpRbAuto )
    {
        // disable edit field on clicking "automatic" radio button
        mpEdValue->Disable();
    }
    else if( pButton == mpRbMan )
    {
        // enable and set focus to edit field on clicking "manual" radio button
        mpEdValue->Enable();
        mpEdValue->GrabFocus();
    }
}

ScDPNumGroupEditHelper::ScDPNumGroupEditHelper(
        RadioButton* pRbAuto, RadioButton* pRbMan, ScDoubleField* pEdValue ) :
    ScDPGroupEditHelper( pRbAuto, pRbMan, pEdValue ),
    mpEdValue( pEdValue )
{
}

bool ScDPNumGroupEditHelper::ImplGetValue( double& rfValue ) const
{
    return mpEdValue->GetValue( rfValue );
}

void ScDPNumGroupEditHelper::ImplSetValue( double fValue )
{
    mpEdValue->SetValue( fValue );
}

ScDPDateGroupEditHelper::ScDPDateGroupEditHelper(
        RadioButton* pRbAuto, RadioButton* pRbMan, DateField* pEdValue, const Date& rNullDate ) :
    ScDPGroupEditHelper( pRbAuto, pRbMan, pEdValue ),
    mpEdValue( pEdValue ),
    maNullDate( rNullDate )
{
}

bool ScDPDateGroupEditHelper::ImplGetValue( double& rfValue ) const
{
    rfValue = mpEdValue->GetDate() - maNullDate;
    return true;
}

void ScDPDateGroupEditHelper::ImplSetValue( double fValue )
{
    Date aDate( maNullDate );
    aDate += static_cast< sal_Int32 >( fValue );
    mpEdValue->SetDate( aDate );
}

ScDPNumGroupDlg::ScDPNumGroupDlg( vcl::Window* pParent, const ScDPNumGroupInfo& rInfo ) :
    ModalDialog     ( pParent, "PivotTableGroupByNumber", "modules/scalc/ui/groupbynumber.ui" ),
    mpRbAutoStart   ( get<RadioButton>("auto_start") ),
    mpRbManStart    ( get<RadioButton>("manual_start") ),
    mpEdStart       ( get<ScDoubleField> ("edit_start") ),
    mpRbAutoEnd     ( get<RadioButton> ( "auto_end" ) ),
    mpRbManEnd      ( get<RadioButton> ("manual_end") ),
    mpEdEnd         ( get<ScDoubleField>( "edit_end") ),
    mpEdBy          ( get<ScDoubleField> ("edit_by") ),
    maStartHelper   ( mpRbAutoStart, mpRbManStart, mpEdStart ),
    maEndHelper     ( mpRbAutoEnd, mpRbManEnd, mpEdEnd )
{

    maStartHelper.SetValue( rInfo.mbAutoStart, rInfo.mfStart );
    maEndHelper.SetValue( rInfo.mbAutoEnd, rInfo.mfEnd );
    mpEdBy->SetValue( (rInfo.mfStep <= 0.0) ? 1.0 : rInfo.mfStep );

    /*  Set the initial focus, currently it is somewhere after calling all the radio
        button click handlers. Now the first enabled editable control is focused. */
    if( mpEdStart->IsEnabled() )
        mpEdStart->GrabFocus();
    else if( mpEdEnd->IsEnabled() )
        mpEdEnd->GrabFocus();
    else
        mpEdBy->GrabFocus();
}

ScDPNumGroupDlg::~ScDPNumGroupDlg()
{
    disposeOnce();
}

void ScDPNumGroupDlg::dispose()
{
    mpRbAutoStart.clear();
    mpRbManStart.clear();
    mpEdStart.clear();
    mpRbAutoEnd.clear();
    mpRbManEnd.clear();
    mpEdEnd.clear();
    mpEdBy.clear();
    ModalDialog::dispose();
}


ScDPNumGroupInfo ScDPNumGroupDlg::GetGroupInfo() const
{
    ScDPNumGroupInfo aInfo;
    aInfo.mbEnable = true;
    aInfo.mbDateValues = false;
    aInfo.mbAutoStart = maStartHelper.IsAuto();
    aInfo.mbAutoEnd = maEndHelper.IsAuto();

    // get values and silently auto-correct them, if they are not valid
    // TODO: error messages in OK event?
    aInfo.mfStart = maStartHelper.GetValue();
    aInfo.mfEnd = maEndHelper.GetValue();
    if( !mpEdBy->GetValue( aInfo.mfStep ) || (aInfo.mfStep <= 0.0) )
        aInfo.mfStep = 1.0;
    if( aInfo.mfEnd <= aInfo.mfStart )
        aInfo.mfEnd = aInfo.mfStart + aInfo.mfStep;

    return aInfo;
}

ScDPDateGroupDlg::ScDPDateGroupDlg( vcl::Window* pParent,
        const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart, const Date& rNullDate ) :
    ModalDialog( pParent, "PivotTableGroupByDate", "modules/scalc/ui/groupbydate.ui" ),
    mpRbAutoStart   ( get<RadioButton>("auto_start") ),
    mpRbManStart    ( get<RadioButton>("manual_start") ),
    mpEdStart       ( get<DateField>("start_date") ),
    mpRbAutoEnd     ( get<RadioButton>("auto_end") ),
    mpRbManEnd      ( get<RadioButton>("manual_end") ),
    mpEdEnd         ( get<DateField>("end_date") ),
    mpRbNumDays     ( get<RadioButton>("days") ),
    mpRbUnits       ( get<RadioButton>("intervals") ),
    mpEdNumDays     ( get<NumericField>("days_value") ),
    mpLbUnits       ( get<SvxCheckListBox>("interval_list") ),
    mpBtnOk         ( get<OKButton>("ok") ),
    maStartHelper   ( mpRbAutoStart, mpRbManStart, mpEdStart, rNullDate ),
    maEndHelper     ( mpRbAutoEnd, mpRbManEnd, mpEdEnd, rNullDate )
{
    static const size_t nCount = sizeof( nDatePartResIds ) / sizeof( nDatePartResIds[0] );
    for( size_t nIdx = 0 ; nIdx < nCount; ++nIdx )
        mpLbUnits->InsertEntry( ScGlobal::GetRscString( nDatePartResIds[nIdx] ) );

    mpEdStart->SetShowDateCentury( true );
    mpEdEnd->SetShowDateCentury( true );

    maStartHelper.SetValue( rInfo.mbAutoStart, rInfo.mfStart );
    maEndHelper.SetValue( rInfo.mbAutoEnd, rInfo.mfEnd );

    if( nDatePart == 0 )
        nDatePart = css::sheet::DataPilotFieldGroupBy::MONTHS;
    for( size_t nIdx = 0; nIdx < nCount; ++nIdx )
        mpLbUnits->CheckEntryPos( static_cast< sal_uInt16 >( nIdx ), (nDatePart & spnDateParts[ nIdx ]) != 0 );

    if( rInfo.mbDateValues )
    {
        mpRbNumDays->Check();
        ClickHdl( mpRbNumDays );

        double fNumDays = rInfo.mfStep;
        if( fNumDays < 1.0 )
            fNumDays = 1.0;
        else if( fNumDays > 32767.0 )
            fNumDays = 32767.0;
        mpEdNumDays->SetValue( static_cast< long >( fNumDays ) );
    }
    else
    {
        mpRbUnits->Check();
        ClickHdl( mpRbUnits );
    }

    /*  Set the initial focus, currently it is somewhere after calling all the radio
        button click handlers. Now the first enabled editable control is focused. */
    if( mpEdStart->IsEnabled() )
        mpEdStart->GrabFocus();
    else if( mpEdEnd->IsEnabled() )
        mpEdEnd->GrabFocus();
    else if( mpEdNumDays->IsEnabled() )
        mpEdNumDays->GrabFocus();
    else if( mpLbUnits->IsEnabled() )
        mpLbUnits->GrabFocus();

    mpRbNumDays->SetClickHdl( LINK( this, ScDPDateGroupDlg, ClickHdl ) );
    mpRbUnits->SetClickHdl( LINK( this, ScDPDateGroupDlg, ClickHdl ) );
    mpLbUnits->SetCheckButtonHdl( LINK( this, ScDPDateGroupDlg, CheckHdl ) );
}

ScDPDateGroupDlg::~ScDPDateGroupDlg()
{
    disposeOnce();
}

void ScDPDateGroupDlg::dispose()
{
    mpRbAutoStart.clear();
    mpRbManStart.clear();
    mpEdStart.clear();
    mpRbAutoEnd.clear();
    mpRbManEnd.clear();
    mpEdEnd.clear();
    mpRbNumDays.clear();
    mpRbUnits.clear();
    mpEdNumDays.clear();
    mpLbUnits.clear();
    mpBtnOk.clear();
    ModalDialog::dispose();
}

ScDPNumGroupInfo ScDPDateGroupDlg::GetGroupInfo() const
{
    ScDPNumGroupInfo aInfo;
    aInfo.mbEnable = true;
    aInfo.mbDateValues = mpRbNumDays->IsChecked();
    aInfo.mbAutoStart = maStartHelper.IsAuto();
    aInfo.mbAutoEnd = maEndHelper.IsAuto();

    // get values and silently auto-correct them, if they are not valid
    // TODO: error messages in OK event?
    aInfo.mfStart = maStartHelper.GetValue();
    aInfo.mfEnd = maEndHelper.GetValue();
    sal_Int64 nNumDays = mpEdNumDays->GetValue();
    aInfo.mfStep = static_cast<double>( aInfo.mbDateValues ? nNumDays : 0L );
    if( aInfo.mfEnd <= aInfo.mfStart )
        aInfo.mfEnd = aInfo.mfStart + nNumDays;

    return aInfo;
}

sal_Int32 ScDPDateGroupDlg::GetDatePart() const
{
    // return DAYS for special "number of days" mode
    if( mpRbNumDays->IsChecked() )
        return css::sheet::DataPilotFieldGroupBy::DAYS;

    // return listbox contents for "units" mode
    sal_Int32 nDatePart = 0;
    for( sal_uLong nIdx = 0, nCount = mpLbUnits->GetEntryCount(); nIdx < nCount; ++nIdx )
        if( mpLbUnits->IsChecked( static_cast< sal_uInt16 >( nIdx ) ) )
            nDatePart |= spnDateParts[ nIdx ];
    return nDatePart;
}

IMPL_LINK_TYPED( ScDPDateGroupDlg, ClickHdl, Button*, pButton, void )
{
    if( pButton == mpRbNumDays )
    {
        mpLbUnits->Disable();
        // enable and set focus to edit field on clicking "num of days" radio button
        mpEdNumDays->Enable();
        mpEdNumDays->GrabFocus();
        mpBtnOk->Enable();
    }
    else if( pButton == mpRbUnits )
    {
        mpEdNumDays->Disable();
        // enable and set focus to listbox on clicking "units" radio button
        mpLbUnits->Enable();
        mpLbUnits->GrabFocus();
        // disable OK button if no date part selected
        CheckHdl( mpLbUnits );
    }
}

IMPL_LINK_TYPED( ScDPDateGroupDlg, CheckHdl, SvTreeListBox*, pListBox, void )
{
    // enable/disable OK button on modifying check list box
    if( pListBox == mpLbUnits )
        mpBtnOk->Enable( mpLbUnits->GetCheckedEntryCount() > 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
