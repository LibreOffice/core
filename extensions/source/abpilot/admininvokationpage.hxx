/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: admininvokationpage.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:37:12 $
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

#ifndef EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX
#define EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX

#ifndef EXTENSIONS_ABP_ABSPAGE_HXX
#include "abspage.hxx"
#endif

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

