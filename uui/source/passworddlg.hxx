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

#ifndef PASSWORDDLG_HXX
#define PASSWORDDLG_HXX

#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>

//============================================================================

class PasswordDialog : public ModalDialog
{
    FixedText       aFTPassword;
    Edit            aEDPassword;
    FixedText       aFTConfirmPassword;
    Edit            aEDConfirmPassword;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    FixedLine       aFixedLine1;
    sal_uInt16          nMinLen;
    String          aPasswdMismatch;


    DECL_LINK( OKHdl_Impl, OKButton * );

public:
    PasswordDialog( Window* pParent, ::com::sun::star::task::PasswordRequestMode nDlgMode, ResMgr * pResMgr, ::rtl::OUString& aDocURL,
            bool bOpenToModify = false, bool bIsSimplePasswordRequest = false );

    void            SetMinLen( sal_uInt16 nMin ) { nMinLen = nMin; }
    String          GetPassword() const { return aEDPassword.GetText(); }

private:
    ::com::sun::star::task::PasswordRequestMode     nDialogMode;
    ResMgr*                                         pResourceMgr;
};

#endif // PASSWORDDLG_HXX

