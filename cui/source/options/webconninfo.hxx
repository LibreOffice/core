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
#ifndef _SVX_WEBCONNINFO_HXX
#define _SVX_WEBCONNINFO_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>

//........................................................................
namespace svx
{
//........................................................................

    class PasswordTable : public SvxSimpleTable
    {
    public:
        PasswordTable(SvxSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);

        void InsertHeaderItem( sal_uInt16 nColumn, const String& rText, HeaderBarItemBits nBits );
        void ResetTabs();
        void Resort( bool bForced );
    };

    //====================================================================
    //= class WebConnectionIfoDialog
    //====================================================================
    class WebConnectionInfoDialog : public ModalDialog
    {
    private:
        FixedInfo           m_aNeverShownFI;
        SvxSimpleTableContainer m_aPasswordsLBContainer;
        PasswordTable       m_aPasswordsLB;
        PushButton          m_aRemoveBtn;
        PushButton          m_aRemoveAllBtn;
        PushButton          m_aChangeBtn;
        FixedLine           m_aButtonsFL;
        CancelButton        m_aCloseBtn;
        HelpButton          m_aHelpBtn;
        sal_Int32           m_nPos;

    DECL_LINK( HeaderBarClickedHdl, SvxSimpleTable* );
    DECL_LINK(RemovePasswordHdl, void *);
    DECL_LINK(RemoveAllPasswordsHdl, void *);
    DECL_LINK(ChangePasswordHdl, void *);
    DECL_LINK( EntrySelectedHdl, void* );

    void FillPasswordList();

    public:
        WebConnectionInfoDialog( Window* pParent );
        ~WebConnectionInfoDialog();
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // #ifndef _SVX_WEBCONNINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
