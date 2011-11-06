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


#ifndef _SW_JAVAEDIT_HXX
#define _SW_JAVAEDIT_HXX

// include ---------------------------------------------------------------

#include <svx/stddlg.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_SVMEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwWrtShell;
class SwFldMgr;
class SwScriptField;

namespace sfx2 { class FileDialogHelper; }

// class SwJavaEditDialog -------------------------------------------------

class SwJavaEditDialog : public SvxStandardDialog
{
private:
    FixedText           aTypeFT;
    Edit                aTypeED;
    RadioButton         aUrlRB;
    RadioButton         aEditRB;
    PushButton          aUrlPB;
    Edit                aUrlED;
    MultiLineEdit       aEditED;
    FixedLine           aPostItFL;

    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    ImageButton         aPrevBtn;
    ImageButton         aNextBtn;
    HelpButton          aHelpBtn;

    String              aText;
    String              aType;

    sal_Bool                bNew;
    sal_Bool                bIsUrl;

    SwScriptField*          pFld;
    SwFldMgr*               pMgr;
    SwWrtShell*             pSh;
    sfx2::FileDialogHelper* pFileDlg;
    Window*                 pOldDefDlgParent;

    DECL_LINK( OKHdl, Button* );
    DECL_LINK( PrevHdl, Button* );
    DECL_LINK( NextHdl, Button* );
    DECL_LINK( RadioButtonHdl, RadioButton* pBtn );
    DECL_LINK( InsertFileHdl, PushButton * );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper * );

    virtual void    Apply();

    void            CheckTravel();
    void            SetFld();

    using Window::GetText;
    using Window::GetType;

public:
    SwJavaEditDialog(Window* pParent, SwWrtShell* pWrtSh);
    ~SwJavaEditDialog();

    String              GetText() { return aText; }

    String              GetType() { return aType; }

    sal_Bool                IsUrl() { return bIsUrl; }
    sal_Bool                IsNew() { return bNew; }
    sal_Bool                IsUpdate();
};


#endif

