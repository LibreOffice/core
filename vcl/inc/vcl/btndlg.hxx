/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: btndlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:48:34 $
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

#ifndef _SV_BTNDLG_HXX
#define _SV_BTNDLG_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

struct ImplBtnDlgItem;
class ImplBtnDlgItemList;
class PushButton;

// ----------------------
// - ButtonDialog-Types -
// ----------------------

#define BUTTONDIALOG_BUTTON_NOTFOUND            ((USHORT)0xFFFF)

#define BUTTONDIALOG_DEFBUTTON                  ((USHORT)0x0001)
#define BUTTONDIALOG_OKBUTTON                   ((USHORT)0x0002)
#define BUTTONDIALOG_CANCELBUTTON               ((USHORT)0x0004)
#define BUTTONDIALOG_HELPBUTTON                 ((USHORT)0x0008)
#define BUTTONDIALOG_FOCUSBUTTON                ((USHORT)0x0010)

// ----------------
// - ButtonDialog -
// ----------------

class VCL_DLLPUBLIC ButtonDialog : public Dialog
{
private:
    ImplBtnDlgItemList* mpItemList;
    Size                maPageSize;
    Size                maCtrlSize;
    long                mnButtonSize;
    USHORT              mnCurButtonId;
    USHORT              mnFocusButtonId;
    BOOL                mbFormat;
    Link                maClickHdl;

    SAL_DLLPRIVATE void             ImplInitButtonDialogData();
    SAL_DLLPRIVATE PushButton*      ImplCreatePushButton( USHORT nBtnFlags );
    SAL_DLLPRIVATE ImplBtnDlgItem*  ImplGetItem( USHORT nId ) const;
    DECL_DLLPRIVATE_LINK(           ImplClickHdl, PushButton* pBtn );
    SAL_DLLPRIVATE void             ImplPosControls();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      ButtonDialog( const ButtonDialog & );
    SAL_DLLPRIVATE      ButtonDialog& operator=( const ButtonDialog& );
protected:
                        ButtonDialog( WindowType nType );
    SAL_DLLPRIVATE long ImplGetButtonSize();

public:
                        ButtonDialog( Window* pParent, WinBits nStyle = WB_STDDIALOG );
                        ButtonDialog( Window* pParent, const ResId& rResId );
                        ~ButtonDialog();

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );

    virtual void        Click();

    void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }
    const Size&         GetPageSizePixel() const { return maPageSize; }

    USHORT              GetCurButtonId() const { return mnCurButtonId; }

    void                AddButton( const XubString& rText, USHORT nId, USHORT nBtnFlags, long nSepPixel = 0 );
    void                AddButton( StandardButtonType eType, USHORT nId, USHORT nBtnFlags, long nSepPixel = 0 );
    void                AddButton( PushButton* pBtn, USHORT nId, USHORT nBtnFlags, long nSepPixel = 0 );
    void                RemoveButton( USHORT nId );
    void                Clear();
    USHORT              GetButtonCount() const;
    USHORT              GetButtonId( USHORT nButton ) const;
    PushButton*         GetPushButton( USHORT nId ) const;
    void                SetButtonText( USHORT nId, const XubString& rText );
    XubString           GetButtonText( USHORT nId ) const;
    void                SetButtonHelpText( USHORT nId, const XubString& rText );
    XubString           GetButtonHelpText( USHORT nId ) const;
    void                SetButtonHelpId( USHORT nId, ULONG nHelpId );
    ULONG               GetButtonHelpId( USHORT nId ) const;

    void                SetFocusButton( USHORT nId = BUTTONDIALOG_BUTTON_NOTFOUND ) { mnFocusButtonId = nId; }
    USHORT              GetFocusButton() const { return mnFocusButtonId; }

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
};

#endif  // _SV_BTNDLG_HXX
