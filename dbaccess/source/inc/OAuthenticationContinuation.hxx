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

#ifndef _DBACCESS_OAUTHENTICATIONCONTINUATION_HXX_
#define _DBACCESS_OAUTHENTICATIONCONTINUATION_HXX_

#include "dbadllapi.hxx"

#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#include <com/sun/star/ucb/RememberAuthentication.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <tools/string.hxx>
#include <comphelper/interaction.hxx>


//........................................................................
namespace dbaccess
{
//........................................................................

class OOO_DLLPUBLIC_DBA OAuthenticationContinuation :
    public comphelper::OInteraction< com::sun::star::ucb::XInteractionSupplyAuthentication >
{
    sal_Bool    m_bRemberPassword : 1;      // remember the password for this session ?

    sal_Bool            m_bCanSetUserName;
    ::rtl::OUString     m_sUser;            // the user
    ::rtl::OUString     m_sPassword;        // the user's password

public:
    OAuthenticationContinuation();

    sal_Bool SAL_CALL canSetRealm(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setRealm( const ::rtl::OUString& Realm ) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL canSetUserName(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setUserName( const ::rtl::OUString& UserName ) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL canSetPassword(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setPassword( const ::rtl::OUString& Password ) throw(com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication > SAL_CALL getRememberPasswordModes( com::sun::star::ucb::RememberAuthentication& Default ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setRememberPassword( com::sun::star::ucb::RememberAuthentication Remember ) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL canSetAccount(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setAccount( const ::rtl::OUString& Account ) throw(com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication > SAL_CALL getRememberAccountModes( com::sun::star::ucb::RememberAuthentication& Default ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setRememberAccount( com::sun::star::ucb::RememberAuthentication Remember ) throw(com::sun::star::uno::RuntimeException);

    void            setCanChangeUserName( sal_Bool bVal )  { m_bCanSetUserName = bVal; }
    ::rtl::OUString getUser() const             { return m_sUser; }
    ::rtl::OUString getPassword() const         { return m_sPassword; }
    sal_Bool        getRememberPassword() const { return m_bRemberPassword; }
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBACCESS_OAUTHENTICATIONCONTINUATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
