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

#include <sfx2/tabdlg.hxx>
#include "editfield.hxx"

class ScDocOptions;

class ScTpCalcOptions : public SfxTabPage
{
public:
    ScTpCalcOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&  rCoreSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet);
    virtual ~ScTpCalcOptions() override;
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
    std::unique_ptr<ScDocOptions> pOldOptions;
    std::unique_ptr<ScDocOptions> pLocalOptions;
    sal_uInt16                    nWhichCalc;

    std::unique_ptr<weld::CheckButton> m_xBtnIterate;
    std::unique_ptr<weld::Label> m_xFtSteps;
    std::unique_ptr<weld::SpinButton> m_xEdSteps;
    std::unique_ptr<weld::Label> m_xFtEps;
    std::unique_ptr<ScDoubleField> m_xEdEps;

    std::unique_ptr<weld::RadioButton> m_xBtnDateStd;
    std::unique_ptr<weld::RadioButton> m_xBtnDateSc10;
    std::unique_ptr<weld::RadioButton> m_xBtnDate1904;

    std::unique_ptr<weld::CheckButton> m_xBtnCase;
    std::unique_ptr<weld::CheckButton> m_xBtnCalc;
    std::unique_ptr<weld::CheckButton> m_xBtnMatch;
    std::unique_ptr<weld::RadioButton> m_xBtnWildcards;
    std::unique_ptr<weld::RadioButton> m_xBtnRegex;
    std::unique_ptr<weld::RadioButton> m_xBtnLiteral;
    std::unique_ptr<weld::CheckButton> m_xBtnLookUp;
    std::unique_ptr<weld::CheckButton> m_xBtnGeneralPrec;

    std::unique_ptr<weld::Label> m_xFtPrec;
    std::unique_ptr<weld::SpinButton> m_xEdPrec;

    std::unique_ptr<weld::CheckButton> m_xBtnThread;

private:
    void            Init();

    // Handler:
    DECL_LINK( RadioClickHdl, weld::Button&, void );
    DECL_LINK( CheckClickHdl, weld::ToggleButton&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
