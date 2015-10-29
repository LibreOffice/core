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

#ifndef INCLUDED_SHELL_SOURCE_CMDMAIL_CMDMAILMSG_HXX
#define INCLUDED_SHELL_SOURCE_CMDMAIL_CMDMAILMSG_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <com/sun/star/system/XSimpleMailMessage2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Reference.hxx>


// class declaration


class CmdMailMsg :
    public  cppu::WeakImplHelper<
        css::system::XSimpleMailMessage2,
        css::container::XNameAccess >
{
    OUString                        m_aBody;
    OUString                        m_aRecipient;
    OUString                        m_aOriginator;
    OUString                        m_aSubject;
    css::uno::Sequence< OUString >  m_CcRecipients;
    css::uno::Sequence< OUString >  m_BccRecipients;
    css::uno::Sequence< OUString >  m_Attachments;

    ::osl::Mutex                    m_aMutex;

public:

    CmdMailMsg() {};


    // XSimpleMailMessage


    virtual void SAL_CALL setBody( const OUString& aBody )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getBody(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setRecipient( const OUString& aRecipient )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getRecipient(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setCcRecipient( const css::uno::Sequence< OUString >& aCcRecipient )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getCcRecipient(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setBccRecipient( const css::uno::Sequence< OUString >& aBccRecipient )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getBccRecipient(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setOriginator( const OUString& aOriginator )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getOriginator(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setSubject( const OUString& aSubject )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getSubject(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setAttachement( const css::uno::Sequence< OUString >& aAttachement )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getAttachement(  )
        throw (css::uno::RuntimeException, std::exception) override;


    // XNameAccess


    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw (css::uno::RuntimeException, std::exception) override ;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw (css::uno::RuntimeException, std::exception) override;


    // XElementAccess


    virtual css::uno::Type SAL_CALL getElementType(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasElements(  )
        throw (css::uno::RuntimeException, std::exception) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
