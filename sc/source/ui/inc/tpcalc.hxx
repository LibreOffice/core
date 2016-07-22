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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPCALC_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPCALC_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <svtools/stdctrl.hxx>
#include "editfield.hxx"

class ScDocOptions;

class ScTpCalcOptions : public SfxTabPage
{
    friend class VclPtr<ScTpCalcOptions>;
public:
    static  VclPtr<SfxTabPage> Create          ( vcl::Window*               pParent,
                                          const SfxItemSet*     rCoreSet );
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
                ScTpCalcOptions( vcl::Window*            pParent,
                                 const SfxItemSet&  rCoreSet );
                virtual ~ScTpCalcOptions();
    virtual void dispose() override;

private:
    VclPtr<CheckBox>       m_pBtnIterate;
    VclPtr<FixedText>      m_pFtSteps;
    VclPtr<NumericField>   m_pEdSteps;
    VclPtr<FixedText>      m_pFtEps;
    VclPtr<ScDoubleField>  m_pEdEps;

    VclPtr<RadioButton>    m_pBtnDateStd;
    VclPtr<RadioButton>    m_pBtnDateSc10;
    VclPtr<RadioButton>    m_pBtnDate1904;

    VclPtr<CheckBox>       m_pBtnCase;
    VclPtr<CheckBox>       m_pBtnCalc;
    VclPtr<CheckBox>       m_pBtnMatch;
    VclPtr<RadioButton>    m_pBtnWildcards;
    VclPtr<RadioButton>    m_pBtnRegex;
    VclPtr<RadioButton>    m_pBtnLiteral;
    VclPtr<CheckBox>       m_pBtnLookUp;
    VclPtr<CheckBox>       m_pBtnGeneralPrec;

    VclPtr<FixedText>      m_pFtPrec;
    VclPtr<NumericField>   m_pEdPrec;

    ScDocOptions*          pOldOptions;
    ScDocOptions*          pLocalOptions;
    sal_uInt16             nWhichCalc;

private:
    void            Init();

    // Handler:
    DECL_LINK_TYPED( RadioClickHdl, Button*, void );
    DECL_LINK_TYPED( CheckClickHdl, Button*, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
