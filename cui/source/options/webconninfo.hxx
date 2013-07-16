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
#include <svtools/simptabl.hxx>
#include <svtools/stdctrl.hxx>

//........................................................................
namespace svx
{
//........................................................................

    class PasswordTable : public SvSimpleTable
    {
    public:
        PasswordTable(SvSimpleTableContainer& rParent, WinBits nBits);

        void InsertHeaderItem(sal_uInt16 nColumn, const OUString& rText, HeaderBarItemBits nBits);
        void setColWidths();
        void Resort( bool bForced );
        virtual void Resize();
    };

    //====================================================================
    //= class WebConnectionIfoDialog
    //====================================================================
    class WebConnectionInfoDialog : public ModalDialog
    {
    private:
        PasswordTable* m_pPasswordsLB;
        PushButton*    m_pRemoveBtn;
        PushButton*    m_pRemoveAllBtn;
        PushButton*    m_pChangeBtn;
        sal_Int32      m_nPos;

    DECL_LINK( HeaderBarClickedHdl, SvSimpleTable* );
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
