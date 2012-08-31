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



#ifndef SC_NAMEDLG_HXX
#define SC_NAMEDLG_HXX

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include "rangenam.hxx"
#include "anyrefdg.hxx"

class ScViewData;
class ScDocument;


//==================================================================

class ScNameDlg : public ScAnyRefDlg
{
private:
    FixedLine       aFlName;
    //ComboBox      aEdName;
    Edit                       aEdName;
    ListBox     aLBNames;

    FixedLine       aFlAssign;
    formula::RefEdit        aEdAssign;
    formula::RefButton      aRbAssign;

    FixedLine       aFlType;
    CheckBox        aBtnPrintArea;
    CheckBox        aBtnColHeader;
    CheckBox        aBtnCriteria;
    CheckBox        aBtnRowHeader;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;
    sal_Bool            bSaved;

    const String    aStrAdd;    // "Hinzufuegen"
    const String    aStrModify; // "Aendern"
    const String    errMsgInvalidSym;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    ScRangeName     aLocalRangeName;
    const ScAddress theCursorPos;
    Selection       theCurSel;
    SCTAB           aSelectedRangeScope;

#ifdef _NAMEDLG_CXX
private:
    void Init();
    void UpdateChecks();
    void UpdateNames();
    void CalcCurTableAssign( String& aAssign, sal_uInt16 nPos );

    // Handler:
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( EdModifyHdl, Edit * );
    DECL_LINK( NameSelectHdl, void * );
    DECL_LINK( AssignGetFocusHdl, void * );
#endif

protected:

    virtual void    RefInputDone( sal_Bool bForced = sal_False );


public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos );
                    ~ScNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;

    virtual void    SetActive();
    virtual sal_Bool    Close();

};



#endif // SC_NAMEDLG_HXX

