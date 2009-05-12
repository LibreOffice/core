/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: admininvokationpage.hxx,v $
 * $Revision: 1.6 $
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

#ifndef EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX
#define EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX

#include "abspage.hxx"

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= AdminDialogInvokationPage
    //=====================================================================
    class AdminDialogInvokationPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aExplanation;
        PushButton      m_aInvokeAdminDialog;
        FixedText       m_aErrorMessage;

        sal_Bool        m_bSuccessfullyExecutedDialog;

    public:
        AdminDialogInvokationPage( OAddessBookSourcePilot* _pParent );

    protected:
        // TabDialog overridables
        virtual void        ActivatePage();
        virtual void        initializePage();

        // OImportPage overridables
        virtual bool        canAdvance() const;

    private:
        DECL_LINK( OnInvokeAdminDialog, void* );

        void implTryConnect();
        void implUpdateErrorMessage();
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX

