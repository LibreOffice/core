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

#include <svl/zforlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <scresid.hxx>
#include <document.hxx>
#include <strings.hrc>
#include <filldlg.hxx>
#include <scui_def.hxx>

// class ScFillSeriesDlg

ScFillSeriesDlg::ScFillSeriesDlg( weld::Window*       pParent,
                                  ScDocument&   rDocument,
                                  FillDir       eFillDir,
                                  FillCmd       eFillCmd,
                                  FillDateCmd   eFillDateCmd,
                                  const OUString& aStartStr,
                                  double        fStep,
                                  double        fMax,
                                  sal_uInt16        nPossDir )
    : GenericDialogController(pParent, "modules/scalc/ui/filldlg.ui", "FillSeriesDialog")
    , aStartStrVal(aStartStr)
    , aErrMsgInvalidVal(ScResId(SCSTR_VALERR))
    , rDoc(rDocument)
    , theFillDir(eFillDir)
    , theFillCmd(eFillCmd)
    , theFillDateCmd(eFillDateCmd)
    , fIncrement(fStep)
    , fEndVal(fMax)
    , m_xFtStartVal(m_xBuilder->weld_label("startL"))
    , m_xEdStartVal(m_xBuilder->weld_entry("startValue"))
    , m_xFtEndVal(m_xBuilder->weld_label("endL"))
    , m_xEdEndVal(m_xBuilder->weld_entry("endValue"))
    , m_xFtIncrement(m_xBuilder->weld_label("incrementL"))
    , m_xEdIncrement(m_xBuilder->weld_entry("increment"))
    , m_xBtnDown(m_xBuilder->weld_radio_button("down"))
    , m_xBtnRight(m_xBuilder->weld_radio_button("right"))
    , m_xBtnUp(m_xBuilder->weld_radio_button("up"))
    , m_xBtnLeft(m_xBuilder->weld_radio_button("left"))
    , m_xBtnArithmetic(m_xBuilder->weld_radio_button("linear"))
    , m_xBtnGeometric(m_xBuilder->weld_radio_button("growth"))
    , m_xBtnDate(m_xBuilder->weld_radio_button("date"))
    , m_xBtnAutoFill(m_xBuilder->weld_radio_button("autofill"))
    , m_xFtTimeUnit(m_xBuilder->weld_label("tuL"))
    , m_xBtnDay(m_xBuilder->weld_radio_button("day"))
    , m_xBtnDayOfWeek(m_xBuilder->weld_radio_button("week"))
    , m_xBtnMonth(m_xBuilder->weld_radio_button("month"))
    , m_xBtnYear(m_xBuilder->weld_radio_button("year"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
{
    Init(nPossDir);
}

ScFillSeriesDlg::~ScFillSeriesDlg()
{
}

void ScFillSeriesDlg::SetEdStartValEnabled(bool bFlag)
{
    if(bFlag)
    {
       m_xFtStartVal->set_sensitive(true);
       m_xEdStartVal->set_sensitive(true);
    }
    else
    {
       m_xFtStartVal->set_sensitive(false);
       m_xEdStartVal->set_sensitive(false);
    }
}

void ScFillSeriesDlg::Init( sal_uInt16 nPossDir )
{
    m_xBtnOk->connect_clicked         ( LINK( this, ScFillSeriesDlg, OKHdl ) );
    m_xBtnArithmetic->connect_toggled ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    m_xBtnGeometric->connect_toggled  ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    m_xBtnDate->connect_toggled       ( LINK( this, ScFillSeriesDlg, DisableHdl ) );
    m_xBtnAutoFill->connect_toggled   ( LINK( this, ScFillSeriesDlg, DisableHdl ) );

    if( nPossDir == FDS_OPT_NONE )
    {
        m_xBtnLeft->set_sensitive(false);
        m_xBtnRight->set_sensitive(false);
        m_xBtnDown->set_sensitive(false);
        m_xBtnUp->set_sensitive(false);
    }

    if( nPossDir == FDS_OPT_HORZ )
    {
        m_xBtnDown->set_sensitive(false);
        m_xBtnUp->set_sensitive(false);
    }

    if( nPossDir == FDS_OPT_VERT )
    {
        m_xBtnLeft->set_sensitive(false);
        m_xBtnRight->set_sensitive(false);
    }

    switch ( theFillDir )
    {
        case FILL_TO_LEFT:      m_xBtnLeft->set_active(true);   break;
        case FILL_TO_RIGHT:     m_xBtnRight->set_active(true);  break;
        case FILL_TO_BOTTOM:    m_xBtnDown->set_active(true);   break;
        case FILL_TO_TOP:       m_xBtnUp->set_active(true);     break;
        default:
        break;
    }

    switch ( theFillCmd )
    {
        case FILL_LINEAR:
            m_xBtnArithmetic->set_active(true);
            DisableHdl(*m_xBtnArithmetic);
            break;
        case FILL_GROWTH:
            m_xBtnGeometric->set_active(true);
            DisableHdl(*m_xBtnGeometric );
            break;
        case FILL_DATE:
            m_xBtnDate->set_active(true);
            DisableHdl(*m_xBtnDate);
            break;
        case FILL_AUTO:
            m_xBtnAutoFill->set_active(true);
            DisableHdl(*m_xBtnAutoFill);
            break;
        default:
        break;
    }

    switch ( theFillDateCmd )
    {
        case FILL_DAY:          m_xBtnDay->set_active(true);        break;
        case FILL_WEEKDAY:      m_xBtnDayOfWeek->set_active(true);  break;
        case FILL_MONTH:        m_xBtnMonth->set_active(true);      break;
        case FILL_YEAR:         m_xBtnYear->set_active(true);       break;
        default:
        break;
    }

    fStartVal = MAXDOUBLE;

    m_xEdStartVal->set_text( aStartStrVal);

    OUString aIncrTxt;
    rDoc.GetFormatTable()->GetInputLineString( fIncrement, 0, aIncrTxt );
    m_xEdIncrement->set_text( aIncrTxt );

    OUString aEndTxt;
    if ( fEndVal != MAXDOUBLE )
        rDoc.GetFormatTable()->GetInputLineString( fEndVal, 0, aEndTxt );
    m_xEdEndVal->set_text( aEndTxt );
}

bool ScFillSeriesDlg::CheckStartVal()
{
    bool bValOk = false;
    OUString aStr = m_xEdStartVal->get_text();

    if ( aStr.isEmpty() || m_xBtnAutoFill->get_active())
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

bool ScFillSeriesDlg::CheckIncrementVal()
{
    sal_uInt32 nKey = 0;
    OUString aStr = m_xEdIncrement->get_text();

    return rDoc.GetFormatTable()->IsNumberFormat( aStr, nKey, fIncrement );
}

bool ScFillSeriesDlg::CheckEndVal()
{
    bool bValOk = false;
    OUString aStr = m_xEdEndVal->get_text();

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

// Handler:
IMPL_LINK(ScFillSeriesDlg, DisableHdl, weld::ToggleButton&, rBtn, void)
{
    if (&rBtn == m_xBtnDate.get())
    {
        m_xBtnDay->set_sensitive(true);
        m_xBtnDayOfWeek->set_sensitive(true);
        m_xBtnMonth->set_sensitive(true);
        m_xBtnYear->set_sensitive(true);
        m_xFtTimeUnit->set_sensitive(true);
    }
    else
    {
        m_xBtnDay->set_sensitive(false);
        m_xBtnDayOfWeek->set_sensitive(false);
        m_xBtnMonth->set_sensitive(false);
        m_xBtnYear->set_sensitive(false);
        m_xFtTimeUnit->set_sensitive(false);
    }

    if (&rBtn != m_xBtnAutoFill.get())
    {
        m_xFtIncrement->set_sensitive(true);
        m_xEdIncrement->set_sensitive(true);
        m_xFtEndVal->set_sensitive(true);
        m_xEdEndVal->set_sensitive(true);
    }
    else
    {
        m_xFtIncrement->set_sensitive(false);
        m_xEdIncrement->set_sensitive(false);
        m_xFtEndVal->set_sensitive(false);
        m_xEdEndVal->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(ScFillSeriesDlg, OKHdl, weld::Button&, void)
{
    if ( m_xBtnLeft->get_active() )             theFillDir = FILL_TO_LEFT;
    else if ( m_xBtnRight->get_active() )       theFillDir = FILL_TO_RIGHT;
    else if ( m_xBtnDown->get_active() )        theFillDir = FILL_TO_BOTTOM;
    else if ( m_xBtnUp->get_active() )          theFillDir = FILL_TO_TOP;

    if ( m_xBtnArithmetic->get_active() )       theFillCmd = FILL_LINEAR;
    else if ( m_xBtnGeometric->get_active() )   theFillCmd = FILL_GROWTH;
    else if ( m_xBtnDate->get_active() )        theFillCmd = FILL_DATE;
    else if ( m_xBtnAutoFill->get_active() )    theFillCmd = FILL_AUTO;

    if ( m_xBtnDay->get_active() )              theFillDateCmd = FILL_DAY;
    else if ( m_xBtnDayOfWeek->get_active() )   theFillDateCmd = FILL_WEEKDAY;
    else if ( m_xBtnMonth->get_active() )       theFillDateCmd = FILL_MONTH;
    else if ( m_xBtnYear->get_active() )        theFillDateCmd = FILL_YEAR;

    bool  bAllOk = true;
    weld::Entry* pEdWrong = nullptr;
    if ( !CheckStartVal() )
    {
        bAllOk = false;
        pEdWrong = m_xEdStartVal.get();
    }
    else if ( !CheckIncrementVal() )
    {
        bAllOk = false;
        pEdWrong = m_xEdIncrement.get();
    }
    else if ( !CheckEndVal() )
    {
        bAllOk = false;
        pEdWrong = m_xEdEndVal.get();
    }
    if ( bAllOk )
        m_xDialog->response(RET_OK);
    else
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Warning,
                    VclButtonsType::Ok, aErrMsgInvalidVal));
        xBox->run();
        pEdWrong->grab_focus();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
