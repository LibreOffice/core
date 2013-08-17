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

#ifndef _DBACCESS_OAUTHENTICATIONCONTINUATION_HXX_
#define _DBACCESS_OAUTHENTICATIONCONTINUATION_HXX_

#include "dbadllapi.hxx"

#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#include <com/sun/star/ucb/RememberAuthentication.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>
#include <comphelper/interaction.hxx>

namespace dbaccess
{

class OOO_DLLPUBLIC_DBA OAuthenticationContinuation :
    public comphelper::OInteraction< com::sun::star::ucb::XInteractionSupplyAuthentication >
{
    sal_Bool    m_bRemberPassword : 1;      // remember the password for this session ?

    sal_Bool            m_bCanSetUserName;
    OUString     m_sUser;            // the user
    OUString     m_sPassword;        // the user's password

public:
    OAuthenticationContinuation();

    sal_Bool SAL_CALL canSetRealm(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setRealm( const OUString& Realm ) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL canSetUserName(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setUserName( const OUString& UserName ) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL canSetPassword(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setPassword( const OUString& Password ) throw(com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication > SAL_CALL getRememberPasswordModes( com::sun::star::ucb::RememberAuthentication& Default ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setRememberPassword( com::sun::star::ucb::RememberAuthentication Remember ) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL canSetAccount(  ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setAccount( const OUString& Account ) throw(com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication > SAL_CALL getRememberAccountModes( com::sun::star::ucb::RememberAuthentication& Default ) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL setRememberAccount( com::sun::star::ucb::RememberAuthentication Remember ) throw(com::sun::star::uno::RuntimeException);

    void            setCanChangeUserName( sal_Bool bVal )  { m_bCanSetUserName = bVal; }
    OUString getUser() const             { return m_sUser; }
    OUString getPassword() const         { return m_sPassword; }
    sal_Bool        getRememberPassword() const { return m_bRemberPassword; }
};

}   // namespace dbaccess

#endif // _DBACCESS_OAUTHENTICATIONCONTINUATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
