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


#ifndef _BREAK_HXX
#define _BREAK_HXX

#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

class SwWrtShell;

class SwBreakDlg: public SvxStandardDialog
{
    SwWrtShell     &rSh;
    FixedLine       aBreakFL;
    RadioButton     aLineBtn;
    RadioButton     aColumnBtn;
    RadioButton     aPageBtn;
    FixedText       aPageCollText;
    ListBox         aPageCollBox;
    CheckBox        aPageNumBox;
    NumericField    aPageNumEdit;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    String          aTemplate;
    sal_uInt16          nKind;
    sal_uInt16          nPgNum;

    sal_Bool            bHtmlMode;

    DECL_LINK( ClickHdl, void * );
    DECL_LINK( PageNumHdl, CheckBox * );
    DECL_LINK( PageNumModifyHdl, Edit * );
    DECL_LINK( OkHdl, Button * );

    void CheckEnable();

protected:
    virtual void Apply();

public:
    SwBreakDlg( Window *pParent, SwWrtShell &rSh );
    ~SwBreakDlg();

    String  GetTemplateName() { return aTemplate; }
    sal_uInt16  GetKind() { return nKind; }
    sal_uInt16  GetPageNumber() { return nPgNum; }
};

#endif

