/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_TPCALC_HXX
#define SC_TPCALC_HXX


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
    static  sal_uInt16*     GetRanges       ();
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
