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

#include "cmdmailmsg.hxx"

using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XNameAccess;

using namespace com::sun::star::uno;


void SAL_CALL CmdMailMsg::setBody( const OUString& aBody )
{
    std::scoped_lock aGuard( m_aMutex );
    m_aBody = aBody;
}

OUString SAL_CALL CmdMailMsg::getBody(  )
{
    std::scoped_lock aGuard( m_aMutex );
    return m_aBody;
}

void SAL_CALL CmdMailMsg::setRecipient( const OUString& aRecipient )
{
    std::scoped_lock aGuard( m_aMutex );
    m_aRecipient = aRecipient;
}

OUString SAL_CALL CmdMailMsg::getRecipient(  )
{
    std::scoped_lock aGuard( m_aMutex );
    return m_aRecipient;
}

void SAL_CALL CmdMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
{
    std::scoped_lock aGuard( m_aMutex );
    m_CcRecipients = aCcRecipient;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getCcRecipient(  )
{
    std::scoped_lock aGuard( m_aMutex );
    return m_CcRecipients;
}

void SAL_CALL CmdMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
{
    std::scoped_lock aGuard( m_aMutex );
    m_BccRecipients = aBccRecipient;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getBccRecipient(  )
{
    std::scoped_lock aGuard( m_aMutex );
    return m_BccRecipients;
}

void SAL_CALL CmdMailMsg::setOriginator( const OUString& aOriginator )
{
    std::scoped_lock aGuard( m_aMutex );
    m_aOriginator = aOriginator;
}

OUString SAL_CALL CmdMailMsg::getOriginator(  )
{
    std::scoped_lock aGuard( m_aMutex );
    return m_aOriginator;
}

void SAL_CALL CmdMailMsg::setSubject( const OUString& aSubject )
{
    std::scoped_lock aGuard( m_aMutex );
    m_aSubject = aSubject;
}

OUString SAL_CALL CmdMailMsg::getSubject(  )
{
    std::scoped_lock aGuard( m_aMutex );
    return m_aSubject;
}

void SAL_CALL CmdMailMsg::setAttachement( const Sequence< OUString >& aAttachment )
{
    std::scoped_lock aGuard( m_aMutex );
    m_Attachments = aAttachment;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getAttachement(  )
{
    std::scoped_lock aGuard( m_aMutex );
    return m_Attachments;
}

Any SAL_CALL CmdMailMsg::getByName( const OUString& aName )
{
    std::scoped_lock aGuard( m_aMutex );

    if( aName == "body" &&  !m_aBody.isEmpty() )
        return Any( m_aBody );

    if( aName == "from" &&  !m_aOriginator.isEmpty() )
        return Any( m_aOriginator );

    else if( aName == "to" &&  !m_aRecipient.isEmpty() )
        return Any( m_aRecipient );

    else if( aName == "cc" &&  m_CcRecipients.hasElements() )
        return Any( m_CcRecipients );

    else if( aName == "bcc" &&  m_BccRecipients.hasElements() )
        return Any( m_BccRecipients );

    else if( aName == "subject" &&  !m_aSubject.isEmpty() )
        return Any( m_aSubject );

    else if( aName == "attachment" &&  m_Attachments.hasElements() )
        return Any( m_Attachments );

    throw NoSuchElementException("key not found: " + aName,
        static_cast < XNameAccess * > (this) );
}

Sequence< OUString > SAL_CALL CmdMailMsg::getElementNames(  )
{
    std::scoped_lock aGuard( m_aMutex );

    sal_Int32 nItems = 0;
    Sequence< OUString > aRet( 7 );
    auto pRet = aRet.getArray();

    if( !m_aBody.isEmpty() )
        pRet[nItems++] = "body";

    if( !m_aOriginator.isEmpty() )
        pRet[nItems++] = "from";

    if( !m_aRecipient.isEmpty() )
        pRet[nItems++] = "to";

    if( m_CcRecipients.hasElements() )
        pRet[nItems++] = "cc";

    if( m_BccRecipients.hasElements() )
        pRet[nItems++] = "bcc";

    if( !m_aSubject.isEmpty() )
        pRet[nItems++] = "subject";

    if( m_Attachments.hasElements() )
        pRet[nItems++] = "attachment";

    aRet.realloc( nItems );
    return aRet;
}

 sal_Bool SAL_CALL CmdMailMsg::hasByName( const OUString& aName )
{
    std::scoped_lock aGuard( m_aMutex );

    if( aName == "body" &&  !m_aBody.isEmpty() )
        return true;

    if( aName == "from" &&  !m_aOriginator.isEmpty() )
        return true;

    else if( aName == "to" &&  !m_aRecipient.isEmpty() )
        return true;

    else if( aName == "cc" &&  m_CcRecipients.hasElements() )
        return true;

    else if( aName == "bcc" &&  m_BccRecipients.hasElements() )
        return true;

    else if( aName == "subject" &&  !m_aSubject.isEmpty() )
        return true;

    else if( aName == "attachment" &&  m_Attachments.hasElements() )
        return true;

    return false;
}

Type SAL_CALL CmdMailMsg::getElementType(  )
{
    // returning void for multi type container
    return Type();
}

sal_Bool SAL_CALL CmdMailMsg::hasElements(  )
{
    return getElementNames().hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
