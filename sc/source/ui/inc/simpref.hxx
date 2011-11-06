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



#ifndef SC_SIMPREF_HXX
#define SC_SIMPREF_HXX

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

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

class ScSimpleRefDlg: public ScAnyRefDlg
{
private:
    Link            aCloseHdl;
    Link            aDoneHdl;
    Link            aAbortedHdl;
    Link            aChangeHdl;

    FixedText       aFtAssign;
    formula::RefEdit        aEdAssign;
    formula::RefButton      aRbAssign;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    sal_Bool            bRefInputMode;

    ScRange         theCurArea;
    sal_Bool            bCloseFlag;
    sal_Bool            bAutoReOpen;
    sal_Bool            bCloseOnButtonUp;
    sal_Bool            bSingleCell;
    sal_Bool            bMultiSelection;

    void            Init();

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );


protected:

    virtual void    RefInputDone( sal_Bool bForced = sal_False );

public:
                    ScSimpleRefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScSimpleRefDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

    void            StartRefInput();

    virtual String  GetRefString() const;
    virtual void    SetRefString(const String &rStr);
    virtual void    FillInfo(SfxChildWinInfo&) const;

    void            SetCloseHdl( const Link& rLink );
    void            SetUnoLinks( const Link& rDone, const Link& rAbort,
                                const Link& rChange );

    void            SetFlags( sal_Bool bSetCloseOnButtonUp, sal_Bool bSetSingleCell, sal_Bool bSetMultiSelection );

    void            SetAutoReOpen(sal_Bool bFlag) {bAutoReOpen=bFlag;}
};



#endif // SC_DBNAMDLG_HXX

