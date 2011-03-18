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

#include <boost/ptr_container/ptr_vector.hpp>

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/dialog.hxx>

struct ImplBtnDlgItem;
class PushButton;

// ----------------------
// - ButtonDialog-Types -
// ----------------------

#define BUTTONDIALOG_BUTTON_NOTFOUND            ((sal_uInt16)0xFFFF)

#define BUTTONDIALOG_DEFBUTTON                  ((sal_uInt16)0x0001)
#define BUTTONDIALOG_OKBUTTON                   ((sal_uInt16)0x0002)
#define BUTTONDIALOG_CANCELBUTTON               ((sal_uInt16)0x0004)
#define BUTTONDIALOG_HELPBUTTON                 ((sal_uInt16)0x0008)
#define BUTTONDIALOG_FOCUSBUTTON                ((sal_uInt16)0x0010)

// ----------------
// - ButtonDialog -
// ----------------

class VCL_DLLPUBLIC ButtonDialog : public Dialog
{
private:
    boost::ptr_vector<ImplBtnDlgItem> maItemList;
    Size                maPageSize;
    Size                maCtrlSize;
    long                mnButtonSize;
    sal_uInt16              mnCurButtonId;
    sal_uInt16              mnFocusButtonId;
    sal_Bool                mbFormat;
    Link                maClickHdl;

    SAL_DLLPRIVATE void             ImplInitButtonDialogData();
    SAL_DLLPRIVATE PushButton*      ImplCreatePushButton( sal_uInt16 nBtnFlags );
    SAL_DLLPRIVATE ImplBtnDlgItem*  ImplGetItem( sal_uInt16 nId ) const;
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

    sal_uInt16              GetCurButtonId() const { return mnCurButtonId; }

    void                AddButton( const XubString& rText, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                AddButton( StandardButtonType eType, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                AddButton( PushButton* pBtn, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                RemoveButton( sal_uInt16 nId );
    void                Clear();
    sal_uInt16              GetButtonCount() const;
    sal_uInt16              GetButtonId( sal_uInt16 nButton ) const;
    PushButton*         GetPushButton( sal_uInt16 nId ) const;
    void                SetButtonText( sal_uInt16 nId, const XubString& rText );
    XubString           GetButtonText( sal_uInt16 nId ) const;
    void                SetButtonHelpText( sal_uInt16 nId, const XubString& rText );
    XubString           GetButtonHelpText( sal_uInt16 nId ) const;
    void                SetButtonHelpId( sal_uInt16 nId, const rtl::OString& rHelpId );
    rtl::OString        GetButtonHelpId( sal_uInt16 nId ) const;

    void                SetFocusButton( sal_uInt16 nId = BUTTONDIALOG_BUTTON_NOTFOUND ) { mnFocusButtonId = nId; }
    sal_uInt16              GetFocusButton() const { return mnFocusButtonId; }

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
};

#endif  // _SV_BTNDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
