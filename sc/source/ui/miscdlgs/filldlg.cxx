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
#include "globstr.hrc"
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
    : ModalDialog(pParent, "FillSeriesDialog", "modules/scalc/ui/filldlg.ui"),
        aStartStrVal    ( aStartStr),
        aErrMsgInvalidVal( SC_RESSTR(STR_VALERR) ),
        rDoc            ( rDocument ),
        theFillDir      ( eFillDir ),
        theFillCmd      ( eFillCmd ),
        theFillDateCmd  ( eFillDateCmd ),
        fIncrement      ( fStep ),
        fEndVal         ( fMax )
{
    get(m_pFtStartVal, "startL");
    get(m_pEdStartVal, "startValue");
    get(m_pFtEndVal, "endL");
    get(m_pEdEndVal, "endValue");
    get(m_pFtIncrement, "incrementL");
    get(m_pEdIncrement, "increment");
    get(m_pBtnDown, "down");
    get(m_pBtnRight, "right");
    get(m_pBtnUp, "up");
    get(m_pBtnLeft, "left");
    get(m_pBtnArithmetic, "linear");
    get(m_pBtnGeometric, "growth");
    get(m_pBtnDate, "date");
    get(m_pBtnAutoFill, "autofill");
    get(m_pFtTimeUnit, "tuL");
    get(m_pBtnDay, "day");
    get(m_pBtnDayOfWeek, "week");
    get(m_pBtnMonth, "month");
    get(m_pBtnYear, "year");
    get(m_pBtnOk, "ok");

    Init( nPossDir );
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
       m_pFtStartVal->Enable();
       m_pEdStartVal->Enable();
    }
    else
    {
       m_pFtStartVal->Disable();
       m_pEdStartVal->Disable();
    }
}

//----------------------------------------------------------------------------

void ScFillSeriesDlg::Init( sal_uInt16 nPossDir )
{
    m_pBtnOk->SetClickHdl         ( LINK( this, ScFillSeriesDlg, OKHdl ) );
    m_pBtnArithmetic->SetClickHdl ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    m_pBtnGeometric->SetClickHdl  ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    m_pBtnDate->SetClickHdl       ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    m_pBtnAutoFill->SetClickHdl   ( LINK( this, ScFillSeriesDlg, DisableHdl ) );

    if( nPossDir == FDS_OPT_NONE )
    {
        m_pBtnLeft->Disable();
        m_pBtnRight->Disable();
        m_pBtnDown->Disable();
        m_pBtnUp->Disable();
    }

    if( nPossDir == FDS_OPT_HORZ )
    {
        m_pBtnDown->Disable();
        m_pBtnUp->Disable();
    }

    if( nPossDir == FDS_OPT_VERT )
    {
        m_pBtnLeft->Disable();
        m_pBtnRight->Disable();
    }

    switch ( theFillDir )
    {
        case FILL_TO_LEFT:      m_pBtnLeft->Check();   break;
        case FILL_TO_RIGHT:     m_pBtnRight->Check();  break;
        case FILL_TO_BOTTOM:    m_pBtnDown->Check();   break;
        case FILL_TO_TOP:       m_pBtnUp->Check();     break;
        default:
        break;
    }

    switch ( theFillCmd )
    {
        case FILL_LINEAR:
            m_pBtnArithmetic->Check();
            DisableHdl( m_pBtnArithmetic );
            break;
        case FILL_GROWTH:
            m_pBtnGeometric->Check();
            DisableHdl( m_pBtnGeometric );
            break;
        case FILL_DATE:
            m_pBtnDate->Check();
            DisableHdl( m_pBtnDate );
            break;
        case FILL_AUTO:
            m_pBtnAutoFill->Check();
            DisableHdl( m_pBtnAutoFill );
            break;
        default:
        break;
    }

    switch ( theFillDateCmd )
    {
        case FILL_DAY:          m_pBtnDay->Check();        break;
        case FILL_WEEKDAY:      m_pBtnDayOfWeek->Check();  break;
        case FILL_MONTH:        m_pBtnMonth->Check();      break;
        case FILL_YEAR:         m_pBtnYear->Check();       break;
        default:
        break;
    }

    fStartVal = MAXDOUBLE;

    m_pEdStartVal->SetText( aStartStrVal);

    OUString aIncrTxt;
    rDoc.GetFormatTable()->GetInputLineString( fIncrement, 0, aIncrTxt );
    m_pEdIncrement->SetText( aIncrTxt );

    OUString aEndTxt;
    if ( fEndVal != MAXDOUBLE )
        rDoc.GetFormatTable()->GetInputLineString( fEndVal, 0, aEndTxt );
    m_pEdEndVal->SetText( aEndTxt );

    bStartValFlag = false;
}


//----------------------------------------------------------------------------

bool ScFillSeriesDlg::CheckStartVal()
{
    bool bValOk = false;
    OUString aStr = m_pEdStartVal->GetText();

    if ( aStr.isEmpty() || m_pBtnAutoFill->IsChecked())
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
    OUString aStr = m_pEdIncrement->GetText();

    return rDoc.GetFormatTable()->IsNumberFormat( aStr, nKey, fIncrement );
}


//----------------------------------------------------------------------------

bool ScFillSeriesDlg::CheckEndVal()
{
    bool bValOk = false;
    OUString aStr = m_pEdEndVal->GetText();

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
    if ( pBtn == m_pBtnDate )
    {
        m_pBtnDay->Enable();
        m_pBtnDayOfWeek->Enable();
        m_pBtnMonth->Enable();
        m_pBtnYear->Enable();
        m_pFtTimeUnit->Enable();
    }
    else
    {
        m_pBtnDay->Disable();
        m_pBtnDayOfWeek->Disable();
        m_pBtnMonth->Disable();
        m_pBtnYear->Disable();
        m_pFtTimeUnit->Disable();
    }

    if ( pBtn != m_pBtnAutoFill )
    {
        m_pFtIncrement->Enable();
        m_pEdIncrement->Enable();
        m_pFtEndVal->Enable();
        m_pEdEndVal->Enable();
    }
    else
    {
        m_pFtIncrement->Disable();
        m_pEdIncrement->Disable();
        m_pFtEndVal->Disable();
        m_pEdEndVal->Disable();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScFillSeriesDlg, OKHdl)
{
    if ( m_pBtnLeft->IsChecked() )             theFillDir = FILL_TO_LEFT;
    else if ( m_pBtnRight->IsChecked() )       theFillDir = FILL_TO_RIGHT;
    else if ( m_pBtnDown->IsChecked() )        theFillDir = FILL_TO_BOTTOM;
    else if ( m_pBtnUp->IsChecked() )          theFillDir = FILL_TO_TOP;

    if ( m_pBtnArithmetic->IsChecked() )       theFillCmd = FILL_LINEAR;
    else if ( m_pBtnGeometric->IsChecked() )   theFillCmd = FILL_GROWTH;
    else if ( m_pBtnDate->IsChecked() )        theFillCmd = FILL_DATE;
    else if ( m_pBtnAutoFill->IsChecked() )    theFillCmd = FILL_AUTO;

    if ( m_pBtnDay->IsChecked() )              theFillDateCmd = FILL_DAY;
    else if ( m_pBtnDayOfWeek->IsChecked() )   theFillDateCmd = FILL_WEEKDAY;
    else if ( m_pBtnMonth->IsChecked() )       theFillDateCmd = FILL_MONTH;
    else if ( m_pBtnYear->IsChecked() )        theFillDateCmd = FILL_YEAR;

    sal_Bool  bAllOk = true;
    Edit* pEdWrong = NULL;
    if ( !CheckStartVal() )
    {
        bAllOk = false;
        pEdWrong = m_pEdStartVal;
    }
    else if ( !CheckIncrementVal() )
    {
        bAllOk = false;
        pEdWrong = m_pEdIncrement;
    }
    else if ( !CheckEndVal() )
    {
        bAllOk = false;
        pEdWrong = m_pEdEndVal;
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
