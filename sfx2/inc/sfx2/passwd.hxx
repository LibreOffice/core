/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: passwd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:26:31 $
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
#ifndef _SFX_PASSWD_HXX
#define _SFX_PASSWD_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
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
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

// defines ---------------------------------------------------------------

#define SHOWEXTRAS_NONE     ((USHORT)0x0000)
#define SHOWEXTRAS_USER     ((USHORT)0x0001)
#define SHOWEXTRAS_CONFIRM  ((USHORT)0x0002)
#define SHOWEXTRAS_ALL      ((USHORT)(SHOWEXTRAS_USER | SHOWEXTRAS_CONFIRM))

// class SfxPasswordDialog -----------------------------------------------

class SFX2_DLLPUBLIC SfxPasswordDialog : public ModalDialog
{
private:
    FixedText       maUserFT;
    Edit            maUserED;
    FixedText       maPasswordFT;
    Edit            maPasswordED;
    FixedText       maConfirmFT;
    Edit            maConfirmED;
    FixedLine       maPasswordBox;

    OKButton        maOKBtn;
    CancelButton    maCancelBtn;
    HelpButton      maHelpBtn;

    String          maConfirmStr;
    USHORT          mnMinLen;
    USHORT          mnExtras;

//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( EditModifyHdl, Edit* );
    DECL_DLLPRIVATE_LINK( OKHdl, OKButton* );
//#endif

public:
    SfxPasswordDialog( Window* pParent, const String* pGroupText = NULL );

    String          GetUser() const { return maUserED.GetText(); }
    String          GetPassword() const { return maPasswordED.GetText(); }
    String          GetConfirm() const { return maConfirmED.GetText(); }

    void            SetMinLen( USHORT Len );
    void            SetEditHelpId( ULONG nId ) { maPasswordED.SetHelpId( nId ); }
    void            ShowExtras( USHORT nExtras ) { mnExtras = nExtras; }

    virtual short   Execute();
};

#endif // #ifndef _SFX_PASSWD_HXX

