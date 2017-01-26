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

#ifndef INCLUDED_SHELL_SOURCE_WIN32_SIMPLEMAIL_SMPLMAILMSG_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_SIMPLEMAIL_SMPLMAILMSG_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/system/XSimpleMailMessage2.hpp>


// class declaration


class CSmplMailMsg : public cppu::WeakImplHelper< css::system::XSimpleMailMessage2 >
{
public:
    CSmplMailMsg( );

    virtual void SAL_CALL setBody( const ::rtl::OUString& aBody ) override;

    virtual ::rtl::OUString SAL_CALL getBody(  ) override;


    virtual void SAL_CALL setRecipient( const OUString& aRecipient ) override;

    virtual OUString SAL_CALL getRecipient(  ) override;


    virtual void SAL_CALL setCcRecipient( const css::uno::Sequence< OUString >& aCcRecipient ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getCcRecipient(  ) override;


    virtual void SAL_CALL setBccRecipient( const css::uno::Sequence< OUString >& aBccRecipient ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getBccRecipient(  ) override;


    virtual void SAL_CALL setOriginator( const OUString& aOriginator ) override;

    virtual OUString SAL_CALL getOriginator(  ) override;


    virtual void SAL_CALL setSubject( const OUString& aSubject ) override;

    virtual OUString SAL_CALL getSubject(  ) override;


    virtual void SAL_CALL setAttachement( const css::uno::Sequence< OUString >& aAttachement ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getAttachement(  ) override;

private:
    OUString                        m_aBody;
    OUString                        m_aRecipient;
    OUString                        m_aOriginator;
    OUString                        m_aSubject;
    css::uno::Sequence< OUString >  m_CcRecipients;
    css::uno::Sequence< OUString >  m_BccRecipients;
    css::uno::Sequence< OUString >  m_Attachements;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
