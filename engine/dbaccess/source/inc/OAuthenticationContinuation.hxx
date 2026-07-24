/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <config_options.h>

#include <dbadllapi.hxx>

#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>

#include <rtl/ustring.hxx>
#include <comphelper/interaction.hxx>

namespace dbaccess
{

class UNLESS_MERGELIBS(OOO_DLLPUBLIC_DBA) OAuthenticationContinuation :
    public comphelper::OInteraction< css::ucb::XInteractionSupplyAuthentication >
{
    bool    m_bRememberPassword : 1;      // remember the password for this session ?

    bool            m_bCanSetUserName;
    OUString     m_sUser;            // the user
    OUString     m_sPassword;        // the user's password

public:
    OAuthenticationContinuation();

    bool canSetRealm(  ) override;
    void setRealm( const OUString& Realm ) override;
    bool canSetUserName(  ) override;
    void setUserName( const OUString& UserName ) override;
    bool canSetPassword(  ) override;
    void setPassword( const OUString& Password ) override;
    cpo::uno::Sequence< css::ucb::RememberAuthentication > getRememberPasswordModes( css::ucb::RememberAuthentication& Default ) override;
    void setRememberPassword( css::ucb::RememberAuthentication Remember ) override;
    bool canSetAccount(  ) override;
    void setAccount( const OUString& Account ) override;
    cpo::uno::Sequence< css::ucb::RememberAuthentication > getRememberAccountModes( css::ucb::RememberAuthentication& Default ) override;
    void setRememberAccount( css::ucb::RememberAuthentication Remember ) override;

    void            setCanChangeUserName( bool bVal )  { m_bCanSetUserName = bVal; }
    const OUString& getUser() const             { return m_sUser; }
    const OUString& getPassword() const         { return m_sPassword; }
    bool        getRememberPassword() const { return m_bRememberPassword; }
};

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
