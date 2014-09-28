/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_BTNDLG_HXX
#define INCLUDED_VCL_BTNDLG_HXX

#include <boost/ptr_container/ptr_vector.hpp>
#include <vcl/dllapi.h>
#include <vcl/dialog.hxx>

struct ImplBtnDlgItem;
class PushButton;

#define BUTTONDIALOG_BUTTON_NOTFOUND            ((sal_uInt16)0xFFFF)

#define BUTTONDIALOG_DEFBUTTON                  ((sal_uInt16)0x0001)
#define BUTTONDIALOG_OKBUTTON                   ((sal_uInt16)0x0002)
#define BUTTONDIALOG_CANCELBUTTON               ((sal_uInt16)0x0004)
#define BUTTONDIALOG_HELPBUTTON                 ((sal_uInt16)0x0008)
#define BUTTONDIALOG_FOCUSBUTTON                ((sal_uInt16)0x0010)

class VCL_DLLPUBLIC ButtonDialog : public Dialog
{
public:
                        ButtonDialog( vcl::Window* pParent, WinBits nStyle = WB_STDDIALOG );
    virtual             ~ButtonDialog();

    virtual void        Resize() SAL_OVERRIDE;
    virtual void        StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;

    virtual void        Click();

    void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }
    const Size&         GetPageSizePixel() const { return maPageSize; }

    sal_uInt16          GetCurButtonId() const { return mnCurButtonId; }

    void                AddButton( const OUString& rText, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                AddButton( StandardButtonType eType, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                RemoveButton( sal_uInt16 nId );

    void                Clear();
    sal_uInt16          GetButtonId( sal_uInt16 nButton ) const;
    PushButton*         GetPushButton( sal_uInt16 nId ) const;
    void                SetButtonText( sal_uInt16 nId, const OUString& rText );
    void                SetButtonHelpText( sal_uInt16 nId, const OUString& rText );

    void                SetFocusButton( sal_uInt16 nId = BUTTONDIALOG_BUTTON_NOTFOUND ) { mnFocusButtonId = nId; }
    sal_uInt16          GetFocusButton() const { return mnFocusButtonId; }

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }

protected:
                        ButtonDialog( WindowType nType );
    SAL_DLLPRIVATE long ImplGetButtonSize();

private:
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      ButtonDialog( const ButtonDialog & );
    SAL_DLLPRIVATE      ButtonDialog& operator=( const ButtonDialog& );

private:
    boost::ptr_vector<ImplBtnDlgItem> maItemList;
    Size                maPageSize;
    Size                maCtrlSize;
    long                mnButtonSize;
    sal_uInt16          mnCurButtonId;
    sal_uInt16          mnFocusButtonId;
    bool                mbFormat;
    Link                maClickHdl;

    SAL_DLLPRIVATE void             ImplInitButtonDialogData();
    SAL_DLLPRIVATE PushButton*      ImplCreatePushButton( sal_uInt16 nBtnFlags );
    SAL_DLLPRIVATE ImplBtnDlgItem*  ImplGetItem( sal_uInt16 nId ) const;
    DECL_DLLPRIVATE_LINK(           ImplClickHdl, PushButton* pBtn );
    SAL_DLLPRIVATE void             ImplPosControls();

};

#endif // INCLUDED_VCL_BTNDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
