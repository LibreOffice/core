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



#ifndef SC_DBNAMDLG_HXX
#define SC_DBNAMDLG_HXX

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#include "anyrefdg.hxx"
#include "dbcolect.hxx"
#include "expftext.hxx"

class ScViewData;
class ScDocument;


//============================================================================

class ScDbNameDlg : public ScAnyRefDlg
{
public:
                    ScDbNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScDbNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    FixedLine           aFlName;
    ComboBox            aEdName;

    FixedLine           aFlAssign;
    formula::RefEdit            aEdAssign;
    formula::RefButton          aRbAssign;

    FixedLine           aFlOptions;
    CheckBox            aBtnHeader;
    CheckBox            aBtnDoSize;
    CheckBox            aBtnKeepFmt;
    CheckBox            aBtnStripData;
    ScExpandedFixedText aFTSource;     //@18.09.97 erweiterter FixedText
    FixedText           aFTOperations;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;

    sal_Bool            bSaved;


    const String    aStrAdd;    // "Hinzufuegen"
    const String    aStrModify; // "Aendern"
    const String    aStrNoName; // "unbenannt"
    const String    aStrInvalid;

    String          aStrSource;
    String          aStrOperations;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    sal_Bool            bRefInputMode;
    ScAddress::Details aAddrDetails;

    ScDBCollection  aLocalDbCol;
    ScRange         theCurArea;
    List            aRemoveList;
    Timer       SynFocusTimer;
    DECL_LINK( FocusToComoboxHdl, Timer* );

#ifdef _DBNAMDLG_CXX
private:
    void            Init();
    void            UpdateNames();
    void            UpdateDBData( const String& rStrName );
    void            SetInfoStrings( const ScDBData* pDBData );

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( NameModifyHdl, void * );
    DECL_LINK( AssModifyHdl, void * );
#endif
};



#endif // SC_DBNAMDLG_HXX

