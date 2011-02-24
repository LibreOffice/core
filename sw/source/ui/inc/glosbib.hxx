/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

