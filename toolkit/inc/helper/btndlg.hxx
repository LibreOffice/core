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

#include <vcl/toolkit/dialog.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vector>
#include <memory>

struct ImplBtnDlgItem;
class PushButton;
class Button;

#define BUTTONDIALOG_BUTTON_NOTFOUND            (sal_uInt16(0xFFFF))

enum class ButtonDialogFlags
{
    NONE                 = 0x0000,
    Default              = 0x0001,
    OK                   = 0x0002,
    Cancel               = 0x0004,
    Help                 = 0x0008,
    Focus                = 0x0010,
};
namespace o3tl
{
    template<> struct typed_flags<ButtonDialogFlags> : is_typed_flags<ButtonDialogFlags, 0x001f> {};
}

class ButtonDialog : public Dialog
{
public:
    virtual             ~ButtonDialog() override;
    virtual void        dispose() override;

    virtual void        Resize() override;
    virtual void        StateChanged( StateChangedType nStateChange ) override;

    void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }

    void                AddButton( StandardButtonType eType, sal_uInt16 nId, ButtonDialogFlags nBtnFlags, tools::Long nSepPixel = 0 );
    void                RemoveButton( sal_uInt16 nId );

protected:
                        ButtonDialog( WindowType eType );
    tools::Long                ImplGetButtonSize();

private:
                        ButtonDialog( const ButtonDialog & ) = delete;
                        ButtonDialog& operator=( const ButtonDialog& ) = delete;

private:
    std::vector<std::unique_ptr<ImplBtnDlgItem>> m_ItemList;
    Size                maPageSize;
    Size                maCtrlSize;
    tools::Long                mnButtonSize;
    sal_uInt16          mnCurButtonId;
    sal_uInt16          mnFocusButtonId;
    bool                mbFormat;

    void             ImplInitButtonDialogData();
    VclPtr<PushButton> ImplCreatePushButton( ButtonDialogFlags nBtnFlags );
    DECL_LINK( ImplClickHdl, Button* pBtn, void );
    void             ImplPosControls();

};

#endif // INCLUDED_VCL_BTNDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
