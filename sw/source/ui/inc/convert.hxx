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


#ifndef _SW_CONVERT_HXX
#define _SW_CONVERT_HXX

#include <vcl/fixed.hxx>
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/edit.hxx>
#include <sfx2/basedlgs.hxx>
#include <textcontrolcombo.hxx>

class SwTableAutoFmt;
class SwView;
class SwWrtShell;
class NumericField;
struct SwInsertTableOptions;

class SwConvertTableDlg: public SfxModalDialog
{
    RadioButton     aTabBtn;
    RadioButton     aSemiBtn;
    RadioButton     aParaBtn;
    RadioButton     aOtherBtn;
    Edit            aOtherEd;
    CheckBox        aKeepColumn;
    FixedLine       aDelimFL;

    CheckBox        aHeaderCB;
    CheckBox        aRepeatHeaderCB;

    FixedText       aRepeatHeaderFT;    // "dummy" to build before and after FT
    FixedText           aRepeatHeaderBeforeFT;
    NumericField        aRepeatHeaderNF;
    FixedText           aRepeatHeaderAfterFT;
    TextControlCombo    aRepeatHeaderCombo;

    FixedLine       aOptionsFL;
    CheckBox        aDontSplitCB;
    CheckBox        aBorderCB;
    PushButton      aAutoFmtBtn;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    String          sConvertTextTable;
    SwTableAutoFmt* pTAutoFmt;
    SwWrtShell*     pShell;

    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK( BtnHdl, Button* );
    DECL_LINK( CheckBoxHdl, CheckBox *pCB = 0 );
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );

public:
    SwConvertTableDlg( SwView& rView, bool bToTable );
    ~SwConvertTableDlg();

    void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTblOpts,
                    SwTableAutoFmt *& prTAFmt );
};


#endif
