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

#ifndef SC_FILLDLG_HXX
#define SC_FILLDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include "global.hxx"

class ScDocument;

//----------------------------------------------------------------------------

#include "scui_def.hxx"
//============================================================================

class ScFillSeriesDlg : public ModalDialog
{
public:
            ScFillSeriesDlg( Window*        pParent,
                             ScDocument&    rDocument,
                             FillDir        eFillDir,
                             FillCmd        eFillCmd,
                             FillDateCmd    eFillDateCmd,
                             const OUString& aStartStr,
                             double         fStep,
                             double         fMax,
                             sal_uInt16     nPossDir );
            ~ScFillSeriesDlg();

    FillDir     GetFillDir() const          { return theFillDir; }
    FillCmd     GetFillCmd() const          { return theFillCmd; }
    FillDateCmd GetFillDateCmd() const      { return theFillDateCmd; }
    double      GetStart() const            { return fStartVal; }
    double      GetStep() const             { return fIncrement; }
    double      GetMax() const              { return fEndVal; }

    OUString GetStartStr() const       { return m_pEdStartVal->GetText(); }

    void SetEdStartValEnabled(bool bFlag = false);

private:
    FixedText* m_pFtStartVal;
    Edit* m_pEdStartVal;
    const OUString aStartStrVal;

    FixedText* m_pFtEndVal;
    Edit* m_pEdEndVal;

    FixedText* m_pFtIncrement;
    Edit* m_pEdIncrement;
    RadioButton* m_pBtnDown;
    RadioButton* m_pBtnRight;
    RadioButton* m_pBtnUp;
    RadioButton* m_pBtnLeft;

    RadioButton* m_pBtnArithmetic;
    RadioButton* m_pBtnGeometric;
    RadioButton* m_pBtnDate;
    RadioButton* m_pBtnAutoFill;

    FixedText* m_pFtTimeUnit;
    RadioButton* m_pBtnDay;
    RadioButton* m_pBtnDayOfWeek;
    RadioButton* m_pBtnMonth;
    RadioButton* m_pBtnYear;

    OKButton* m_pBtnOk;

    const OUString aErrMsgInvalidVal;

    //----------------------------------------------------------

    ScDocument& rDoc;
    FillDir     theFillDir;
    FillCmd     theFillCmd;
    FillDateCmd theFillDateCmd;
    double      fStartVal;
    double      fIncrement;
    double      fEndVal;

    bool        bStartValFlag;

#ifdef _FILLDLG_CXX
private:
    void Init( sal_uInt16 nPossDir );
    bool CheckStartVal();
    bool CheckIncrementVal();
    bool CheckEndVal();

    DECL_LINK( OKHdl, void * );
    DECL_LINK( DisableHdl, Button * );
#endif
};



#endif // SC_FILLDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
