/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    FixedLine       aHSeparatorFL;
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

