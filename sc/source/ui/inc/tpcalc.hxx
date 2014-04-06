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

#ifndef SC_TPCALC_HXX
#define SC_TPCALC_HXX


#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <svtools/stdctrl.hxx>
#include "editfield.hxx"

class ScDocOptions;

class ScTpCalcOptions : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual bool        FillItemSet     ( SfxItemSet& rCoreSet ) SAL_OVERRIDE;
    virtual void        Reset           ( const SfxItemSet& rCoreSet ) SAL_OVERRIDE;
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL ) SAL_OVERRIDE;

private:
                ScTpCalcOptions( Window*            pParent,
                                 const SfxItemSet&  rCoreSet );
                virtual ~ScTpCalcOptions();

private:
    CheckBox*       m_pBtnIterate;
    FixedText*      m_pFtSteps;
    NumericField*   m_pEdSteps;
    FixedText*      m_pFtEps;
    ScDoubleField*  m_pEdEps;

    RadioButton*    m_pBtnDateStd;
    RadioButton*    m_pBtnDateSc10;
    RadioButton*    m_pBtnDate1904;

    CheckBox*       m_pBtnCase;
    CheckBox*       m_pBtnCalc;
    CheckBox*       m_pBtnMatch;
    CheckBox*       m_pBtnRegex;
    CheckBox*       m_pBtnLookUp;
    CheckBox*       m_pBtnGeneralPrec;

    FixedText*      m_pFtPrec;
    NumericField*   m_pEdPrec;

    ScDocOptions*   pOldOptions;
    ScDocOptions*   pLocalOptions;
    sal_uInt16          nWhichCalc;

private:
    void            Init();


    // Handler:
    DECL_LINK( RadioClickHdl, RadioButton* );
    DECL_LINK( CheckClickHdl, CheckBox* );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
