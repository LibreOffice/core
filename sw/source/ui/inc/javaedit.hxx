/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javaedit.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:26:33 $
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
#ifndef _SW_JAVAEDIT_HXX
#define _SW_JAVAEDIT_HXX

// include ---------------------------------------------------------------

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

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
#include <vcl/imagebtn.hxx>
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

    BOOL                bNew;
    BOOL                bIsUrl;

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

public:
    SwJavaEditDialog(Window* pParent, SwWrtShell* pWrtSh);
    ~SwJavaEditDialog();

    String              GetText() { return aText; }
    String              GetType() { return aType; }
    BOOL                IsUrl() { return bIsUrl; }
    BOOL                IsNew() { return bNew; }
    BOOL                IsUpdate();
};


#endif

