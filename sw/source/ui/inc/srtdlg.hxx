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


#ifndef _SRTDLG_HXX
#define _SRTDLG_HXX

#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#include <svx/stddlg.hxx>
#include <svx/langbox.hxx>

class SwWrtShell;
class CollatorRessource;

class SwSortDlg : public SvxStandardDialog
{
    FixedText           aColLbl;
    FixedText           aTypLbl;
    FixedText           aDirLbl;

    FixedLine           aSortFL;

    CheckBox            aKeyCB1;
    NumericField        aColEdt1;
    ListBox             aTypDLB1;
    RadioButton         aSortUpRB;
    RadioButton         aSortDnRB;

    CheckBox            aKeyCB2;
    NumericField        aColEdt2;
    ListBox             aTypDLB2;
    RadioButton         aSortUp2RB;
    RadioButton         aSortDn2RB;

    CheckBox            aKeyCB3;
    NumericField        aColEdt3;
    ListBox             aTypDLB3;
    RadioButton         aSortUp3RB;
    RadioButton         aSortDn3RB;

    FixedLine           aDirFL;
    RadioButton         aColumnRB;
    RadioButton         aRowRB;

    FixedLine           aDelimFL;
    RadioButton         aDelimTabRB;
    RadioButton         aDelimFreeRB;
    Edit                aDelimEdt;
    PushButton          aDelimPB;

    FixedLine           aLangFL;
    SvxLanguageBox      aLangLB;

    FixedLine           aSortOptFL;
    CheckBox            aCaseCB;

    OKButton            aOkBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    String aColTxt;
    String aRowTxt;
    String aNumericTxt;

    SwWrtShell          &rSh;
    CollatorRessource* pColRes;

    sal_uInt16 nX;
    sal_uInt16 nY;

    virtual void        Apply();
    sal_Unicode         GetDelimChar() const;

    DECL_LINK( CheckHdl, CheckBox * );
    DECL_LINK( DelimHdl, RadioButton* );
    DECL_LINK( LanguageHdl, ListBox* );
    DECL_LINK( DelimCharHdl, PushButton* );

public:
    SwSortDlg(Window * pParent, SwWrtShell &rSh);
    ~SwSortDlg();
};

#endif

