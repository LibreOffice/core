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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_WEBCONNINFO_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_WEBCONNINFO_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/stdctrl.hxx>


namespace svx
{


    class PasswordTable : public SvSimpleTable
    {
    public:
        PasswordTable(SvSimpleTableContainer& rParent, WinBits nBits);

        void InsertHeaderItem(sal_uInt16 nColumn, const OUString& rText, HeaderBarItemBits nBits);
        void setColWidths();
        void Resort( bool bForced );
        virtual void Resize() override;
    };

    class WebConnectionInfoDialog : public ModalDialog
    {
    private:
        VclPtr<PasswordTable> m_pPasswordsLB;
        VclPtr<PushButton>    m_pRemoveBtn;
        VclPtr<PushButton>    m_pRemoveAllBtn;
        VclPtr<PushButton>    m_pChangeBtn;
        sal_Int32      m_nPos;

        DECL_LINK_TYPED( HeaderBarClickedHdl, SvSimpleTable*, void );
        DECL_LINK_TYPED( RemovePasswordHdl, Button*, void );
        DECL_LINK_TYPED( RemoveAllPasswordsHdl, Button*, void );
        DECL_LINK_TYPED( ChangePasswordHdl, Button*, void );
        DECL_LINK_TYPED( EntrySelectedHdl, SvTreeListBox*, void );

        void FillPasswordList();

    public:
        explicit WebConnectionInfoDialog( vcl::Window* pParent );
        virtual ~WebConnectionInfoDialog();
        virtual void dispose() override;
    };


}


#endif // INCLUDED_CUI_SOURCE_OPTIONS_WEBCONNINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
