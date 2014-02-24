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

#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/system/XSimpleMailMessage2.hpp>


// class declaration


class CSmplMailMsg :
    public  cppu::WeakImplHelper1< com::sun::star::system::XSimpleMailMessage2 >
{
public:
    CSmplMailMsg( );

    virtual void SAL_CALL setBody( const ::rtl::OUString& aBody )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getBody(  )
        throw (::com::sun::star::uno::RuntimeException);



    virtual void SAL_CALL setRecipient( const OUString& aRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);



    virtual void SAL_CALL setCcRecipient( const ::com::sun::star::uno::Sequence< OUString >& aCcRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getCcRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);



    virtual void SAL_CALL setBccRecipient( const ::com::sun::star::uno::Sequence< OUString >& aBccRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getBccRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);



    virtual void SAL_CALL setOriginator( const OUString& aOriginator )
        throw (::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getOriginator(  )
        throw (::com::sun::star::uno::RuntimeException);



    virtual void SAL_CALL setSubject( const OUString& aSubject )
        throw (::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getSubject(  )
        throw (::com::sun::star::uno::RuntimeException);



    virtual void SAL_CALL setAttachement( const ::com::sun::star::uno::Sequence< OUString >& aAttachement )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAttachement(  )
        throw (::com::sun::star::uno::RuntimeException);

private:
    OUString                                   m_aBody;
    OUString                                   m_aRecipient;
    OUString                                   m_aOriginator;
    OUString                                   m_aSubject;
    com::sun::star::uno::Sequence< OUString >  m_CcRecipients;
    com::sun::star::uno::Sequence< OUString >  m_BccRecipients;
    com::sun::star::uno::Sequence< OUString >  m_Attachements;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
