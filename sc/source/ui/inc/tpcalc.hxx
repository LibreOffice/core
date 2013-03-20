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
#if 1


#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <svtools/stdctrl.hxx>
#include "editfield.hxx"

//===================================================================

class ScDocOptions;

class ScTpCalcOptions : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
                ScTpCalcOptions( Window*            pParent,
                                 const SfxItemSet&  rCoreSet );
                ~ScTpCalcOptions();

private:
    FixedLine       aGbZRefs;
    CheckBox        aBtnIterate;
    FixedText       aFtSteps;
    NumericField    aEdSteps;
    FixedText       aFtEps;
    ScDoubleField   aEdEps;

    FixedLine       aSeparatorFL;
    FixedLine       aGbDate;
    RadioButton     aBtnDateStd;
    RadioButton     aBtnDateSc10;
    RadioButton     aBtnDate1904;

    CheckBox        aBtnCase;
    CheckBox        aBtnCalc;
    CheckBox        aBtnMatch;
    CheckBox        aBtnRegex;
    CheckBox        aBtnLookUp;
    CheckBox        aBtnGeneralPrec;

    FixedText       aFtPrec;
    NumericField    aEdPrec;

    ScDocOptions*   pOldOptions;
    ScDocOptions*   pLocalOptions;
    sal_uInt16          nWhichCalc;

#ifdef _TPCALC_CXX
private:
    void            Init();

    //------------------------------------
    // Handler:
    DECL_LINK( RadioClickHdl, RadioButton* );
    DECL_LINK( CheckClickHdl, CheckBox* );

#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
