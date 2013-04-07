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


#undef SC_DLLIMPLEMENTATION



//----------------------------------------------------------------------------

#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <limits.h>

#include "scresid.hxx"
#include "document.hxx"
#include "miscdlgs.hrc"

#define _FILLDLG_CXX
#include "filldlg.hxx"
#undef _FILLDLG_CXX



//============================================================================
// class ScFillSeriesDlg

//----------------------------------------------------------------------------

ScFillSeriesDlg::ScFillSeriesDlg( Window*       pParent,
                                  ScDocument&   rDocument,
                                  FillDir       eFillDir,
                                  FillCmd       eFillCmd,
                                  FillDateCmd   eFillDateCmd,
                                  const OUString& aStartStr,
                                  double        fStep,
                                  double        fMax,
                                  sal_uInt16        nPossDir )

    :   ModalDialog     ( pParent, ScResId( RID_SCDLG_FILLSERIES ) ),

        aFtStartVal     ( this, ScResId( FT_START_VALUE ) ),
        aEdStartVal     ( this, ScResId( ED_START_VALUES ) ),
        aStartStrVal    ( aStartStr),

        aFtEndVal       ( this, ScResId( FT_END_VALUE ) ),
        aEdEndVal       ( this, ScResId( ED_END_VALUES ) ),

        aFtIncrement    ( this, ScResId( FT_INCREMENT ) ),
        aEdIncrement    ( this, ScResId( ED_INCREMENT ) ),

        aFlDirection    ( this, ScResId( FL_DIRECTION ) ),
        aBtnDown        ( this, ScResId( BTN_BOTTOM ) ),
        aBtnRight       ( this, ScResId( BTN_RIGHT ) ),
        aBtnUp          ( this, ScResId( BTN_TOP ) ),
        aBtnLeft        ( this, ScResId( BTN_LEFT ) ),
        aFlSep1         ( this, ScResId( FL_SEP1 ) ),
        aFlType         ( this, ScResId( FL_TYPE ) ),
        aBtnArithmetic  ( this, ScResId( BTN_ARITHMETIC ) ),
        aBtnGeometric   ( this, ScResId( BTN_GEOMETRIC ) ),
        aBtnDate        ( this, ScResId( BTN_DATE ) ),
        aBtnAutoFill    ( this, ScResId( BTN_AUTOFILL ) ),
        aFlSep2         ( this, ScResId( FL_SEP2 ) ),
        aFlTimeUnit     ( this, ScResId( FL_TIME_UNIT ) ),
        aBtnDay         ( this, ScResId( BTN_DAY ) ),
        aBtnDayOfWeek   ( this, ScResId( BTN_DAY_OF_WEEK ) ),
        aBtnMonth       ( this, ScResId( BTN_MONTH ) ),
        aBtnYear        ( this, ScResId( BTN_YEAR ) ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aErrMsgInvalidVal( SC_RESSTR(STR_VALERR) ),
        rDoc            ( rDocument ),
        theFillDir      ( eFillDir ),
        theFillCmd      ( eFillCmd ),
        theFillDateCmd  ( eFillDateCmd ),
        fIncrement      ( fStep ),
        fEndVal         ( fMax )
{
    Init( nPossDir );
    FreeResource();
}


//----------------------------------------------------------------------------

ScFillSeriesDlg::~ScFillSeriesDlg()
{
}

//----------------------------------------------------------------------------

void ScFillSeriesDlg::SetEdStartValEnabled(bool bFlag)
{
    bStartValFlag=bFlag;
    if(bFlag)
    {
       aFtStartVal.Enable();
       aEdStartVal.Enable();
    }
    else
    {
       aFtStartVal.Disable();
       aEdStartVal.Disable();
    }
}

//----------------------------------------------------------------------------

void ScFillSeriesDlg::Init( sal_uInt16 nPossDir )
{
    aBtnOk.SetClickHdl         ( LINK( this, ScFillSeriesDlg, OKHdl ) );
    aBtnArithmetic.SetClickHdl ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    aBtnGeometric.SetClickHdl  ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    aBtnDate.SetClickHdl       ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    aBtnAutoFill.SetClickHdl   ( LINK( this, ScFillSeriesDlg, DisableHdl ) );

    if( nPossDir == FDS_OPT_NONE )
    {
        aBtnLeft.Disable();
        aBtnRight.Disable();
        aBtnDown.Disable();
        aBtnUp.Disable();
    }

    if( nPossDir == FDS_OPT_HORZ )
    {
        aBtnDown.Disable();
        aBtnUp.Disable();
    }

    if( nPossDir == FDS_OPT_VERT )
    {
        aBtnLeft.Disable();
        aBtnRight.Disable();
    }

    switch ( theFillDir )
    {
        case FILL_TO_LEFT:      aBtnLeft.Check();   break;
        case FILL_TO_RIGHT:     aBtnRight.Check();  break;
        case FILL_TO_BOTTOM:    aBtnDown.Check();   break;
        case FILL_TO_TOP:       aBtnUp.Check();     break;
        default:
        break;
    }

    switch ( theFillCmd )
    {
        case FILL_LINEAR:
            aBtnArithmetic.Check();
            DisableHdl( &aBtnArithmetic );
            break;
        case FILL_GROWTH:
            aBtnGeometric.Check();
            DisableHdl( &aBtnGeometric );
            break;
        case FILL_DATE:
            aBtnDate.Check();
            DisableHdl( &aBtnDate );
            break;
        case FILL_AUTO:
            aBtnAutoFill.Check();
            DisableHdl( &aBtnAutoFill );
            break;
        default:
        break;
    }

    switch ( theFillDateCmd )
    {
        case FILL_DAY:          aBtnDay.Check();        break;
        case FILL_WEEKDAY:      aBtnDayOfWeek.Check();  break;
        case FILL_MONTH:        aBtnMonth.Check();      break;
        case FILL_YEAR:         aBtnYear.Check();       break;
        default:
        break;
    }

    fStartVal = MAXDOUBLE;

    aEdStartVal.SetText( aStartStrVal);

    String aIncrTxt;
    rDoc.GetFormatTable()->GetInputLineString( fIncrement, 0, aIncrTxt );
    aEdIncrement.SetText( aIncrTxt );

    String aEndTxt;
    if ( fEndVal != MAXDOUBLE )
        rDoc.GetFormatTable()->GetInputLineString( fEndVal, 0, aEndTxt );
    aEdEndVal.SetText( aEndTxt );

    bStartValFlag = false;

    aFlSep1.SetStyle( aFlSep1.GetStyle() | WB_VERT );
    aFlSep2.SetStyle( aFlSep2.GetStyle() | WB_VERT );
}


//----------------------------------------------------------------------------

bool ScFillSeriesDlg::CheckStartVal()
{
    bool bValOk = false;
    OUString aStr = aEdStartVal.GetText();

    if ( aStr.isEmpty() || aBtnAutoFill.IsChecked())
    {
        fStartVal = MAXDOUBLE;
        bValOk = true;
    }
    else
    {
        sal_uInt32 nKey = 0;
        bValOk = rDoc.GetFormatTable()->IsNumberFormat( aStr, nKey, fStartVal );
    }
    return bValOk;
}


//----------------------------------------------------------------------------

bool ScFillSeriesDlg::CheckIncrementVal()
{
    sal_uInt32 nKey = 0;
    OUString aStr = aEdIncrement.GetText();

    return rDoc.GetFormatTable()->IsNumberFormat( aStr, nKey, fIncrement );
}


//----------------------------------------------------------------------------

bool ScFillSeriesDlg::CheckEndVal()
{
    bool bValOk = false;
    OUString aStr = aEdEndVal.GetText();

    if (aStr.isEmpty())
    {
        fEndVal = (fIncrement < 0) ? -MAXDOUBLE : MAXDOUBLE;
        bValOk  = true;
    }
    else
    {
        sal_uInt32 nKey = 0;
        bValOk = rDoc.GetFormatTable()->IsNumberFormat( aStr, nKey, fEndVal );
    }
    return bValOk;
}


//----------------------------------------------------------------------------
// Handler:
//----------------------------------------------------------------------------

IMPL_LINK( ScFillSeriesDlg, DisableHdl, Button *, pBtn )
{
    if ( pBtn == &aBtnDate )
    {
        aBtnDay.Enable();
        aBtnDayOfWeek.Enable();
        aBtnMonth.Enable();
        aBtnYear.Enable();
        aFlTimeUnit.Enable();
    }
    else
    {
        aBtnDay.Disable();
        aBtnDayOfWeek.Disable();
        aBtnMonth.Disable();
        aBtnYear.Disable();
        aFlTimeUnit.Disable();
    }

    if ( pBtn != &aBtnAutoFill )
    {
        aFtIncrement.Enable();
        aEdIncrement.Enable();
        aFtEndVal.Enable();
        aEdEndVal.Enable();
    }
    else
    {
        aFtIncrement.Disable();
        aEdIncrement.Disable();
        aFtEndVal.Disable();
        aEdEndVal.Disable();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScFillSeriesDlg, OKHdl)
{
    if ( aBtnLeft.IsChecked() )             theFillDir = FILL_TO_LEFT;
    else if ( aBtnRight.IsChecked() )       theFillDir = FILL_TO_RIGHT;
    else if ( aBtnDown.IsChecked() )        theFillDir = FILL_TO_BOTTOM;
    else if ( aBtnUp.IsChecked() )          theFillDir = FILL_TO_TOP;

    if ( aBtnArithmetic.IsChecked() )       theFillCmd = FILL_LINEAR;
    else if ( aBtnGeometric.IsChecked() )   theFillCmd = FILL_GROWTH;
    else if ( aBtnDate.IsChecked() )        theFillCmd = FILL_DATE;
    else if ( aBtnAutoFill.IsChecked() )    theFillCmd = FILL_AUTO;

    if ( aBtnDay.IsChecked() )              theFillDateCmd = FILL_DAY;
    else if ( aBtnDayOfWeek.IsChecked() )   theFillDateCmd = FILL_WEEKDAY;
    else if ( aBtnMonth.IsChecked() )       theFillDateCmd = FILL_MONTH;
    else if ( aBtnYear.IsChecked() )        theFillDateCmd = FILL_YEAR;

    sal_Bool  bAllOk = true;
    Edit* pEdWrong = NULL;
    if ( !CheckStartVal() )
    {
        bAllOk = false;
        pEdWrong = &aEdStartVal;
    }
    else if ( !CheckIncrementVal() )
    {
        bAllOk = false;
        pEdWrong = &aEdIncrement;
    }
    else if ( !CheckEndVal() )
    {
        bAllOk = false;
        pEdWrong = &aEdEndVal;
    }
    if ( bAllOk )
        EndDialog( RET_OK );
    else
    {
        ErrorBox( this,
                  WinBits( WB_OK | WB_DEF_OK ),
                  aErrMsgInvalidVal
                ).Execute();
        pEdWrong->GrabFocus();
    }

    return 0;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
