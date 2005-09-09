/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: masterpasscrtdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:23:28 $
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

#ifndef UUI_MASTERPASSCRTDLG_HXX
#define UUI_MASTERPASSCRTDLG_HXX

#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUESTMODE_HPP
#include <com/sun/star/task/PasswordRequestMode.hpp>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif

//============================================================================
class MasterPasswordCreateDialog : public ModalDialog
{
    FixedText       aFTMasterPasswordCrt;
    Edit            aEDMasterPasswordCrt;
    FixedText       aFTMasterPasswordRepeat;
    Edit            aEDMasterPasswordRepeat;
    OKButton        aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton      aHelpBtn;


    DECL_LINK( OKHdl_Impl, OKButton * );
    DECL_LINK( EditHdl_Impl, Edit * );

public:
    MasterPasswordCreateDialog( Window* pParent, ResMgr * pResMgr );

    String          GetMasterPassword() const { return aEDMasterPasswordCrt.GetText(); }

private:
    ResMgr*                                         pResourceMgr;
    sal_uInt16                                      nMinLen;
};

#endif // UUI_MASTERPASSCRTDLG_HXX
