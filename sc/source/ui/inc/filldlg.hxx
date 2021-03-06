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

#pragma once

#include <vcl/weld.hxx>
#include <global.hxx>

class ScDocument;

class ScFillSeriesDlg : public weld::GenericDialogController
{
public:
    ScFillSeriesDlg( weld::Window*        pParent,
                     ScDocument&    rDocument,
                     FillDir        eFillDir,
                     FillCmd        eFillCmd,
                     FillDateCmd    eFillDateCmd,
                     const OUString& aStartStr,
                     double         fStep,
                     double         fMax,
                     SCSIZE       nSelectHeight,
                     SCSIZE       nSelectWidth,
                     sal_uInt16     nPossDir );
    virtual ~ScFillSeriesDlg() override;

    FillDir     GetFillDir() const          { return theFillDir; }
    FillCmd     GetFillCmd() const          { return theFillCmd; }
    FillDateCmd GetFillDateCmd() const      { return theFillDateCmd; }
    double      GetStart() const            { return fStartVal; }
    double      GetStep() const             { return fIncrement; }
    double      GetMax() const              { return fEndVal; }

    OUString GetStartStr() const       { return m_xEdStartVal->get_text(); }

    void SetEdStartValEnabled(bool bFlag);

private:
    const OUString aStartStrVal;
    const OUString aErrMsgInvalidVal;

    ScDocument& rDoc;
    FillDir     theFillDir;
    FillCmd     theFillCmd;
    FillDateCmd theFillDateCmd;
    double      fStartVal;
    double      fIncrement;
    double      fEndVal;
    const SCSIZE m_nSelectHeight;
    const SCSIZE m_nSelectWidth;

    std::unique_ptr<weld::Label> m_xFtStartVal;
    std::unique_ptr<weld::Entry> m_xEdStartVal;

    std::unique_ptr<weld::Label> m_xFtEndVal;
    std::unique_ptr<weld::Entry> m_xEdEndVal;

    std::unique_ptr<weld::Label> m_xFtIncrement;
    std::unique_ptr<weld::Entry> m_xEdIncrement;
    std::unique_ptr<weld::RadioButton> m_xBtnDown;
    std::unique_ptr<weld::RadioButton> m_xBtnRight;
    std::unique_ptr<weld::RadioButton> m_xBtnUp;
    std::unique_ptr<weld::RadioButton> m_xBtnLeft;

    std::unique_ptr<weld::RadioButton> m_xBtnArithmetic;
    std::unique_ptr<weld::RadioButton> m_xBtnGeometric;
    std::unique_ptr<weld::RadioButton> m_xBtnDate;
    std::unique_ptr<weld::RadioButton> m_xBtnAutoFill;

    std::unique_ptr<weld::Label> m_xFtTimeUnit;
    std::unique_ptr<weld::RadioButton> m_xBtnDay;
    std::unique_ptr<weld::RadioButton> m_xBtnDayOfWeek;
    std::unique_ptr<weld::RadioButton> m_xBtnMonth;
    std::unique_ptr<weld::RadioButton> m_xBtnYear;

    std::unique_ptr<weld::Button> m_xBtnOk;

    void Init( sal_uInt16 nPossDir );
    weld::Entry* CheckValues();

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(DisableHdl, weld::ToggleButton&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
