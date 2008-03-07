/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharedocdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:21:57 $
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

#ifndef SC_SHAREDOCDLG_HXX
#define SC_SHAREDOCDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <svx/simptabl.hxx>

class ScViewData;
class ScDocShell;


//=============================================================================
// class ScShareDocumentDlg
//=============================================================================

class ScShareDocumentDlg : public ModalDialog
{
private:
    CheckBox            maCbShare;
    FixedText           maFtWarning;
    FixedLine           maFlUsers;
    FixedText           maFtUsers;
    SvxSimpleTable      maLbUsers;
    FixedLine           maFlEnd;
    HelpButton          maBtnHelp;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;

    String              maStrTitleName;
    String              maStrTitleAccessed;
    String              maStrNoUserData;
    String              maStrUnkownUser;
    String              maStrExclusiveAccess;

    ScViewData*         mpViewData;
    ScDocShell*         mpDocShell;

    DECL_LINK( ToggleHandle, void* );

public:
                        ScShareDocumentDlg( Window* pParent, ScViewData* pViewData );
                        ~ScShareDocumentDlg();

    bool                IsShareDocumentChecked() const;
    void                UpdateView();
};

#endif
