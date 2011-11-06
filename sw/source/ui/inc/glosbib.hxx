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



#ifndef _GLOSBIB_HXX
#define _GLOSBIB_HXX

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <svx/stddlg.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <svtools/svtabbx.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>

class SwGlossaryHdl;
class SvStrings;

class FEdit : public Edit
{
    public:
        FEdit(Window * pParent, const ResId& rResId) :
            Edit(pParent, rResId){}

    virtual void KeyInput( const KeyEvent& rKEvent );
};
/* -----------------------------08.02.00 15:04--------------------------------

 ---------------------------------------------------------------------------*/
struct GlosBibUserData
{
    String sPath;
    String sGroupName;
    String sGroupTitle;
};
class SwGlossaryGroupTLB : public SvTabListBox
{
public:
    SwGlossaryGroupTLB(Window* pParent, const ResId& rResId) :
        SvTabListBox(pParent, rResId) {}

    virtual void    RequestHelp( const HelpEvent& rHEvt );
};

class SwGlossaryGroupDlg : public SvxStandardDialog
{
    FixedText           aBibFT;
    FEdit               aNameED;
    FixedText           aPathFT;
    ListBox             aPathLB;
    FixedText           aSelectFT;
    SwGlossaryGroupTLB  aGroupTLB;

    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    PushButton      aNewPB;
    PushButton      aDelPB;
    PushButton      aRenamePB;

    SvStrings*      pRemovedArr;
    SvStrings*      pInsertedArr;
    SvStrings*      pRenamedArr;

    SwGlossaryHdl   *pGlosHdl;

    String          sCreatedGroup;

    sal_Bool            IsDeleteAllowed(const String &rGroup);

protected:
    virtual void Apply();
    DECL_LINK( SelectHdl, SvTabListBox* );
    DECL_LINK( NewHdl, Button* );
    DECL_LINK( DeleteHdl, Button*  );
    DECL_LINK( ModifyHdl, Edit* );
    DECL_LINK( RenameHdl, Button*  );

public:
    SwGlossaryGroupDlg(Window * pParent,
                        const SvStrings* pPathArr,
                        SwGlossaryHdl *pGlosHdl);
    ~SwGlossaryGroupDlg();

    const String&       GetCreatedGroupName() const {return sCreatedGroup;}
};


#endif

