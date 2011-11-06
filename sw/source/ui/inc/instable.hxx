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


#ifndef _INSTABLE_HXX
#define _INSTABLE_HXX

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <sfx2/basedlgs.hxx>
#include <actctrl.hxx>
#include <textcontrolcombo.hxx>

class SwWrtShell;
class SwTableAutoFmt;
class SwView;
struct SwInsertTableOptions;


class SwInsTableDlg : public SfxModalDialog
{
    FixedText       aNameFT;
    TableNameEdit   aNameEdit;

    FixedLine       aFL;
    FixedText       aColLbl;
    NumericField    aColEdit;
    FixedText       aRowLbl;
    NumericField    aRowEdit;

    FixedLine       aOptionsFL;
    CheckBox        aHeaderCB;
    CheckBox        aRepeatHeaderCB;
    FixedText       aRepeatHeaderFT;    // "dummy" to build before and after FT
    FixedText       aRepeatHeaderBeforeFT;
    NumericField    aRepeatHeaderNF;
    FixedText       aRepeatHeaderAfterFT;
    TextControlCombo    aRepeatHeaderCombo;

    CheckBox        aDontSplitCB;
    CheckBox        aBorderCB;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    PushButton      aAutoFmtBtn;

    SwWrtShell*     pShell;
    SwTableAutoFmt* pTAutoFmt;
    sal_Int64       nEnteredValRepeatHeaderNF;

    DECL_LINK( ModifyName, Edit * );
    DECL_LINK( ModifyRowCol, NumericField * );
    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK( CheckBoxHdl, CheckBox *pCB = 0 );
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );
    DECL_LINK( ModifyRepeatHeaderNF_Hdl, void* p = 0 );

public:
    SwInsTableDlg( SwView& rView );
    ~SwInsTableDlg();

    void GetValues( String& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                    SwInsertTableOptions& rInsTblOpts, String& rTableAutoFmtName,
                    SwTableAutoFmt *& prTAFmt );
};

#endif
