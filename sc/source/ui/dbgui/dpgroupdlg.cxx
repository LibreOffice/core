/*************************************************************************
 *
 *  $RCSfile: dpgroupdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:36:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#ifndef SC_DPGROUPDLG_HXX
#include "dpgroupdlg.hxx"
#endif
#ifndef SC_DPGROUPDLG_HRC
#include "dpgroupdlg.hrc"
#endif

#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef SC_SCRESID_HXX
#include "scresid.hxx"
#endif
#ifndef SC_SC_HRC
#include "sc.hrc"
#endif

// ============================================================================

namespace {

/** Date part flags in order of the list box entries. */
static const sal_Int32 spnDateParts[] =
{
    SC_DP_DATE_SECONDS,
    SC_DP_DATE_MINUTES,
    SC_DP_DATE_HOURS,
    SC_DP_DATE_DAYS,
    SC_DP_DATE_MONTHS,
    SC_DP_DATE_QUARTERS,
    SC_DP_DATE_YEARS
};

} // namespace

// ============================================================================

ScDPGroupEditHelper::ScDPGroupEditHelper( RadioButton& rRbAuto, RadioButton& rRbMan, Edit& rEdValue ) :
    mrRbAuto( rRbAuto ),
    mrRbMan( rRbMan ),
    mrEdValue( rEdValue )
{
    mrRbAuto.SetClickHdl( LINK( this, ScDPGroupEditHelper, ClickHdl ) );
    mrRbMan.SetClickHdl( LINK( this, ScDPGroupEditHelper, ClickHdl ) );
}

bool ScDPGroupEditHelper::IsAuto() const
{
    return mrRbAuto.IsChecked();
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
        mrRbAuto.Check();
        ClickHdl( &mrRbAuto );
    }
    else
    {
        mrRbMan.Check();
        ClickHdl( &mrRbMan );
    }
    ImplSetValue( fValue );
}

IMPL_LINK( ScDPGroupEditHelper, ClickHdl, RadioButton*, pButton )
{
    if( pButton == &mrRbAuto )
    {
        // disable edit field on clicking "automatic" radio button
        mrEdValue.Disable();
    }
    else if( pButton == &mrRbMan )
    {
        // enable and set focus to edit field on clicking "manual" radio button
        mrEdValue.Enable();
        mrEdValue.GrabFocus();
    }
    return 0;
}

// ----------------------------------------------------------------------------

ScDPNumGroupEditHelper::ScDPNumGroupEditHelper(
        RadioButton& rRbAuto, RadioButton& rRbMan, ScDoubleField& rEdValue ) :
    ScDPGroupEditHelper( rRbAuto, rRbMan, rEdValue ),
    mrEdValue( rEdValue )
{
}

bool ScDPNumGroupEditHelper::ImplGetValue( double& rfValue ) const
{
    return mrEdValue.GetValue( rfValue );
}

void ScDPNumGroupEditHelper::ImplSetValue( double fValue )
{
    mrEdValue.SetValue( fValue );
}

// ----------------------------------------------------------------------------

ScDPDateGroupEditHelper::ScDPDateGroupEditHelper(
        RadioButton& rRbAuto, RadioButton& rRbMan, DateField& rEdValue, const Date& rNullDate ) :
    ScDPGroupEditHelper( rRbAuto, rRbMan, rEdValue ),
    mrEdValue( rEdValue ),
    maNullDate( rNullDate )
{
}

bool ScDPDateGroupEditHelper::ImplGetValue( double& rfValue ) const
{
    rfValue = mrEdValue.GetDate() - maNullDate;
    return true;
}

void ScDPDateGroupEditHelper::ImplSetValue( double fValue )
{
    Date aDate( maNullDate );
    aDate += static_cast< sal_Int32 >( fValue );
    mrEdValue.SetDate( aDate );
}

// ============================================================================
// ============================================================================

ScDPNumGroupDlg::ScDPNumGroupDlg( Window* pParent, const ScDPNumGroupInfo& rInfo ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DPNUMGROUP ) ),
    maFlStart       ( this, ScResId( FL_START ) ),
    maRbAutoStart   ( this, ScResId( RB_AUTOSTART ) ),
    maRbManStart    ( this, ScResId( RB_MANSTART ) ),
    maEdStart       ( this, ScResId( ED_START ) ),
    maFlEnd         ( this, ScResId( FL_END ) ),
    maRbAutoEnd     ( this, ScResId( RB_AUTOEND ) ),
    maRbManEnd      ( this, ScResId( RB_MANEND ) ),
    maEdEnd         ( this, ScResId( ED_END ) ),
    maFlBy          ( this, ScResId( FL_BY ) ),
    maEdBy          ( this, ScResId( ED_BY ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) ),
    maStartHelper   ( maRbAutoStart, maRbManStart, maEdStart ),
    maEndHelper     ( maRbAutoEnd, maRbManEnd, maEdEnd )
{
    FreeResource();

    maStartHelper.SetValue( rInfo.AutoStart, rInfo.Start );
    maEndHelper.SetValue( rInfo.AutoEnd, rInfo.End );
    maEdBy.SetValue( (rInfo.Step <= 0.0) ? 1.0 : rInfo.Step );

    /*  Set the initial focus, currently it is somewhere after calling all the radio
        button click handlers. Now the first enabled editable control is focused. */
    if( maEdStart.IsEnabled() )
        maEdStart.GrabFocus();
    else if( maEdEnd.IsEnabled() )
        maEdEnd.GrabFocus();
    else
        maEdBy.GrabFocus();
}

ScDPNumGroupInfo ScDPNumGroupDlg::GetGroupInfo() const
{
    ScDPNumGroupInfo aInfo;
    aInfo.Enable = sal_True;
    aInfo.DateValues = sal_False;
    aInfo.AutoStart = maStartHelper.IsAuto();
    aInfo.AutoEnd = maEndHelper.IsAuto();

    // get values and silently auto-correct them, if they are not valid
    // TODO: error messages in OK event?
    aInfo.Start = maStartHelper.GetValue();
    aInfo.End = maEndHelper.GetValue();
    if( !maEdBy.GetValue( aInfo.Step ) || (aInfo.Step <= 0.0) )
        aInfo.Step = 1.0;
    if( aInfo.End <= aInfo.Start )
        aInfo.End = aInfo.Start + aInfo.Step;

    return aInfo;
}

// ============================================================================

ScDPDateGroupDlg::ScDPDateGroupDlg( Window* pParent,
        const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart, const Date& rNullDate ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DPDATEGROUP ) ),
    maFlStart       ( this, ScResId( FL_START ) ),
    maRbAutoStart   ( this, ScResId( RB_AUTOSTART ) ),
    maRbManStart    ( this, ScResId( RB_MANSTART ) ),
    maEdStart       ( this, ScResId( ED_START ) ),
    maFlEnd         ( this, ScResId( FL_END ) ),
    maRbAutoEnd     ( this, ScResId( RB_AUTOEND ) ),
    maRbManEnd      ( this, ScResId( RB_MANEND ) ),
    maEdEnd         ( this, ScResId( ED_END ) ),
    maFlBy          ( this, ScResId( FL_BY ) ),
    maRbNumDays     ( this, ScResId( RB_NUMDAYS ) ),
    maRbUnits       ( this, ScResId( RB_UNITS ) ),
    maEdNumDays     ( this, ScResId( ED_NUMDAYS ) ),
    maLbUnits       ( this, ScResId( LB_UNITS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) ),
    maStartHelper   ( maRbAutoStart, maRbManStart, maEdStart, rNullDate ),
    maEndHelper     ( maRbAutoEnd, maRbManEnd, maEdEnd, rNullDate )
{
    maLbUnits.SetHelpId( HID_SC_DPDATEGROUP_LB );
    ResStringArray aArr( ScResId( STR_UNITS ) );
    for( USHORT nIdx = 0, nCount = aArr.Count(); nIdx < nCount; ++nIdx )
        maLbUnits.InsertEntry( aArr.GetString( nIdx ) );

    FreeResource();

    maEdStart.SetShowDateCentury( TRUE );
    maEdEnd.SetShowDateCentury( TRUE );

    maStartHelper.SetValue( rInfo.AutoStart, rInfo.Start );
    maEndHelper.SetValue( rInfo.AutoEnd, rInfo.End );

    if( nDatePart == 0 )
        nDatePart = SC_DP_DATE_MONTHS;
    for( ULONG nIdx = 0, nCount = maLbUnits.GetEntryCount(); nIdx < nCount; ++nIdx )
        maLbUnits.CheckEntryPos( static_cast< USHORT >( nIdx ), (nDatePart & spnDateParts[ nIdx ]) != 0 );

    if( rInfo.DateValues )
    {
        maRbNumDays.Check();
        ClickHdl( &maRbNumDays );

        double fNumDays = rInfo.Step;
        if( fNumDays < 1.0 )
            fNumDays = 1.0;
        else if( fNumDays > 32767.0 )
            fNumDays = 32767.0;
        maEdNumDays.SetValue( static_cast< long >( fNumDays ) );
    }
    else
    {
        maRbUnits.Check();
        ClickHdl( &maRbUnits );
    }

    /*  Set the initial focus, currently it is somewhere after calling all the radio
        button click handlers. Now the first enabled editable control is focused. */
    if( maEdStart.IsEnabled() )
        maEdStart.GrabFocus();
    else if( maEdEnd.IsEnabled() )
        maEdEnd.GrabFocus();
    else if( maEdNumDays.IsEnabled() )
        maEdNumDays.GrabFocus();
    else if( maLbUnits.IsEnabled() )
        maLbUnits.GrabFocus();

    maRbNumDays.SetClickHdl( LINK( this, ScDPDateGroupDlg, ClickHdl ) );
    maRbUnits.SetClickHdl( LINK( this, ScDPDateGroupDlg, ClickHdl ) );
    maLbUnits.SetCheckButtonHdl( LINK( this, ScDPDateGroupDlg, CheckHdl ) );
}

ScDPNumGroupInfo ScDPDateGroupDlg::GetGroupInfo() const
{
    ScDPNumGroupInfo aInfo;
    aInfo.Enable = sal_True;
    aInfo.DateValues = maRbNumDays.IsChecked();
    aInfo.AutoStart = maStartHelper.IsAuto();
    aInfo.AutoEnd = maEndHelper.IsAuto();

    // get values and silently auto-correct them, if they are not valid
    // TODO: error messages in OK event?
    aInfo.Start = maStartHelper.GetValue();
    aInfo.End = maEndHelper.GetValue();
    long nNumDays = maEdNumDays.GetValue();
    aInfo.Step = aInfo.DateValues ? nNumDays : 0L;
    if( aInfo.End <= aInfo.Start )
        aInfo.End = aInfo.Start + nNumDays;

    return aInfo;
}

sal_Int32 ScDPDateGroupDlg::GetDatePart() const
{
    // return DAYS for special "number of days" mode
    if( maRbNumDays.IsChecked() )
        return SC_DP_DATE_DAYS;

    // return listbox contents for "units" mode
    sal_Int32 nDatePart = 0;
    for( ULONG nIdx = 0, nCount = maLbUnits.GetEntryCount(); nIdx < nCount; ++nIdx )
        if( maLbUnits.IsChecked( static_cast< USHORT >( nIdx ) ) )
            nDatePart |= spnDateParts[ nIdx ];
    return nDatePart;
}

IMPL_LINK( ScDPDateGroupDlg, ClickHdl, RadioButton*, pButton )
{
    if( pButton == &maRbNumDays )
    {
        maLbUnits.Disable();
        // enable and set focus to edit field on clicking "num of days" radio button
        maEdNumDays.Enable();
        maEdNumDays.GrabFocus();
        maBtnOk.Enable();
    }
    else if( pButton == &maRbUnits )
    {
        maEdNumDays.Disable();
        // enable and set focus to listbox on clicking "units" radio button
        maLbUnits.Enable();
        maLbUnits.GrabFocus();
        // disable OK button if no date part selected
        CheckHdl( &maLbUnits );
    }
    return 0;
}

IMPL_LINK( ScDPDateGroupDlg, CheckHdl, SvxCheckListBox*, pListBox )
{
    // enable/disable OK button on modifying check list box
    if( pListBox == &maLbUnits )
        maBtnOk.Enable( maLbUnits.GetCheckedEntryCount() > 0 );
    return 0;
}

// ============================================================================

