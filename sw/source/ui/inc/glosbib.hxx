/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: glosbib.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:17:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _GLOSBIB_HXX
#define _GLOSBIB_HXX

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

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

    FEdit               aNameED;
    ListBox             aPathLB;
    SwGlossaryGroupTLB  aGroupTLB;

    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    PushButton      aNewPB;
    PushButton      aDelPB;
    PushButton      aRenamePB;
    FixedText       aBibFT;
    FixedText       aPathFT;
    FixedText       aSelectFT;

    SvStrings*      pRemovedArr;
    SvStrings*      pInsertedArr;
    SvStrings*      pRenamedArr;

    SwGlossaryHdl   *pGlosHdl;

    String          sCreatedGroup;

    BOOL            IsDeleteAllowed(const String &rGroup);

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

