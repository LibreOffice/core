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

#ifndef INCLUDED_SC_SOURCE_UI_INC_FILLDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FILLDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include "global.hxx"

class ScDocument;

#include "scui_def.hxx"

class ScFillSeriesDlg : public ModalDialog
{
public:
            ScFillSeriesDlg( vcl::Window*        pParent,
                             ScDocument&    rDocument,
                             FillDir        eFillDir,
                             FillCmd        eFillCmd,
                             FillDateCmd    eFillDateCmd,
                             const OUString& aStartStr,
                             double         fStep,
                             double         fMax,
                             sal_uInt16     nPossDir );
            virtual ~ScFillSeriesDlg();
    virtual void dispose() override;

    FillDir     GetFillDir() const          { return theFillDir; }
    FillCmd     GetFillCmd() const          { return theFillCmd; }
    FillDateCmd GetFillDateCmd() const      { return theFillDateCmd; }
    double      GetStart() const            { return fStartVal; }
    double      GetStep() const             { return fIncrement; }
    double      GetMax() const              { return fEndVal; }

    OUString GetStartStr() const       { return m_pEdStartVal->GetText(); }

    void SetEdStartValEnabled(bool bFlag);

private:
    VclPtr<FixedText> m_pFtStartVal;
    VclPtr<Edit> m_pEdStartVal;
    const OUString aStartStrVal;

    VclPtr<FixedText> m_pFtEndVal;
    VclPtr<Edit> m_pEdEndVal;

    VclPtr<FixedText> m_pFtIncrement;
    VclPtr<Edit> m_pEdIncrement;
    VclPtr<RadioButton> m_pBtnDown;
    VclPtr<RadioButton> m_pBtnRight;
    VclPtr<RadioButton> m_pBtnUp;
    VclPtr<RadioButton> m_pBtnLeft;

    VclPtr<RadioButton> m_pBtnArithmetic;
    VclPtr<RadioButton> m_pBtnGeometric;
    VclPtr<RadioButton> m_pBtnDate;
    VclPtr<RadioButton> m_pBtnAutoFill;

    VclPtr<FixedText> m_pFtTimeUnit;
    VclPtr<RadioButton> m_pBtnDay;
    VclPtr<RadioButton> m_pBtnDayOfWeek;
    VclPtr<RadioButton> m_pBtnMonth;
    VclPtr<RadioButton> m_pBtnYear;

    VclPtr<OKButton> m_pBtnOk;

    const OUString aErrMsgInvalidVal;

    ScDocument& rDoc;
    FillDir     theFillDir;
    FillCmd     theFillCmd;
    FillDateCmd theFillDateCmd;
    double      fStartVal;
    double      fIncrement;
    double      fEndVal;

    bool        bStartValFlag;

    void Init( sal_uInt16 nPossDir );
    bool CheckStartVal();
    bool CheckIncrementVal();
    bool CheckEndVal();

    DECL_LINK_TYPED( OKHdl, Button*, void );
    DECL_LINK_TYPED( DisableHdl, Button*, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_FILLDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
