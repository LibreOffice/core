/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

