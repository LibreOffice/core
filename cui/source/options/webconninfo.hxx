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
        PasswordTable( Window* pParent, const ResId& rResId );

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
        PasswordTable       m_aPasswordsLB;
        PushButton          m_aRemoveBtn;
        PushButton          m_aRemoveAllBtn;
        PushButton          m_aChangeBtn;
        FixedLine           m_aButtonsFL;
        CancelButton        m_aCloseBtn;
        HelpButton          m_aHelpBtn;
        sal_Int32           m_nPos;

    DECL_LINK( HeaderBarClickedHdl, SvxSimpleTable* );
    DECL_LINK( RemovePasswordHdl, PushButton* );
    DECL_LINK( RemoveAllPasswordsHdl, PushButton* );
    DECL_LINK( ChangePasswordHdl, PushButton* );
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
