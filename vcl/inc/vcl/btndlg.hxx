/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_BTNDLG_HXX
#define _SV_BTNDLG_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/dialog.hxx>
#include <vector>

struct ImplBtnDlgItem;
class PushButton;

typedef ::std::vector< ImplBtnDlgItem* > ImplBtnDlgItemList;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
