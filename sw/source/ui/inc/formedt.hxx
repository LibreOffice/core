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


#ifndef _FORMEDT_HXX
#define _FORMEDT_HXX

#include <svx/stddlg.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#include "toxe.hxx"

class SwWrtShell;
class SwForm;

/*--------------------------------------------------------------------
     Beschreibung:  Markierung fuer Verzeichniseintrag einfuegen
 --------------------------------------------------------------------*/

class SwIdxFormDlg : public SvxStandardDialog
{
    DECL_LINK( EntryHdl, Button * );
    DECL_LINK( PageHdl, Button * );
    DECL_LINK( TabHdl, Button * );
    DECL_LINK( JumpHdl, Button * );
    DECL_LINK( StdHdl, Button * );
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( EnableSelectHdl, ListBox * );
    DECL_LINK( DoubleClickHdl, Button * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( AssignHdl, Button * );
    void            UpdatePattern();
    void            Apply();

    ListBox         aEntryLB;
    OKButton        aOKBtn;
    CancelButton    aCancelBT;
    FixedText       aLevelFT;
    Edit            aEntryED;
    PushButton      aEntryBT;
    PushButton      aTabBT;
    PushButton      aPageBT;
    PushButton      aJumpBT;
    FixedLine       aEntryFL;
    FixedText       aLevelFT2;
    ListBox         aLevelLB;
    FixedText       aTemplateFT;
    ListBox         aParaLayLB;
    PushButton      aStdBT;
    PushButton      aAssignBT;
    FixedLine       aFormatFL;

    SwWrtShell     &rSh;
    SwForm         *pForm;
    sal_uInt16          nAktLevel;
    sal_Bool            bLastLinkIsEnd;

public:
    SwIdxFormDlg( Window* pParent, SwWrtShell &rShell, const SwForm& rForm );
    ~SwIdxFormDlg();

    static sal_Bool     IsNoNum(SwWrtShell& rSh, const String& rName);
    const SwForm&   GetTOXForm();
};

inline const SwForm& SwIdxFormDlg::GetTOXForm()
{
    return *pForm;
}

#endif
