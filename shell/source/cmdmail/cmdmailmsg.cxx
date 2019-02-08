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

using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XNameAccess;
using osl::MutexGuard;

using namespace cppu;
using namespace com::sun::star::uno;


void SAL_CALL CmdMailMsg::setBody( const OUString& aBody )
{
    MutexGuard aGuard( m_aMutex );
    m_aBody = aBody;
}

OUString SAL_CALL CmdMailMsg::getBody(  )
{
    MutexGuard aGuard( m_aMutex );
    return m_aBody;
}

void SAL_CALL CmdMailMsg::setRecipient( const OUString& aRecipient )
{
    MutexGuard aGuard( m_aMutex );
    m_aRecipient = aRecipient;
}

OUString SAL_CALL CmdMailMsg::getRecipient(  )
{
    MutexGuard aGuard( m_aMutex );
    return m_aRecipient;
}

void SAL_CALL CmdMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
{
    MutexGuard aGuard( m_aMutex );
    m_CcRecipients = aCcRecipient;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getCcRecipient(  )
{
    MutexGuard aGuard( m_aMutex );
    return m_CcRecipients;
}

void SAL_CALL CmdMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
{
    MutexGuard aGuard( m_aMutex );
    m_BccRecipients = aBccRecipient;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getBccRecipient(  )
{
    MutexGuard aGuard( m_aMutex );
    return m_BccRecipients;
}

void SAL_CALL CmdMailMsg::setOriginator( const OUString& aOriginator )
{
    MutexGuard aGuard( m_aMutex );
    m_aOriginator = aOriginator;
}

OUString SAL_CALL CmdMailMsg::getOriginator(  )
{
    MutexGuard aGuard( m_aMutex );
    return m_aOriginator;
}

void SAL_CALL CmdMailMsg::setSubject( const OUString& aSubject )
{
    MutexGuard aGuard( m_aMutex );
    m_aSubject = aSubject;
}

OUString SAL_CALL CmdMailMsg::getSubject(  )
{
    MutexGuard aGuard( m_aMutex );
    return m_aSubject;
}

void SAL_CALL CmdMailMsg::setAttachement( const Sequence< OUString >& aAttachment )
{
    MutexGuard aGuard( m_aMutex );
    m_Attachments = aAttachment;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getAttachement(  )
{
    MutexGuard aGuard( m_aMutex );
    return m_Attachments;
}

Any SAL_CALL CmdMailMsg::getByName( const OUString& aName )
{
    MutexGuard aGuard( m_aMutex );

    if( aName == "body" &&  !m_aBody.isEmpty() )
        return makeAny( m_aBody );

    if( aName == "from" &&  !m_aOriginator.isEmpty() )
        return makeAny( m_aOriginator );

    else if( aName == "to" &&  !m_aRecipient.isEmpty() )
        return makeAny( m_aRecipient );

    else if( aName == "cc" &&  m_CcRecipients.getLength() )
        return makeAny( m_CcRecipients );

    else if( aName == "bcc" &&  m_BccRecipients.getLength() )
        return makeAny( m_BccRecipients );

    else if( aName == "subject" &&  !m_aSubject.isEmpty() )
        return makeAny( m_aSubject );

    else if( aName == "attachment" &&  m_Attachments.getLength() )
        return makeAny( m_Attachments );

    throw NoSuchElementException("key not found: " + aName,
        static_cast < XNameAccess * > (this) );
}

Sequence< OUString > SAL_CALL CmdMailMsg::getElementNames(  )
{
    MutexGuard aGuard( m_aMutex );

    sal_Int32 nItems = 0;
    Sequence< OUString > aRet( 7 );

    if( !m_aBody.isEmpty() )
        aRet[nItems++] = "body";

    if( !m_aOriginator.isEmpty() )
        aRet[nItems++] = "from";

    if( !m_aRecipient.isEmpty() )
        aRet[nItems++] = "to";

    if( m_CcRecipients.getLength() )
        aRet[nItems++] = "cc";

    if( m_BccRecipients.getLength() )
        aRet[nItems++] = "bcc";

    if( !m_aSubject.isEmpty() )
        aRet[nItems++] = "subject";

    if( m_Attachments.getLength() )
        aRet[nItems++] = "attachment";

    aRet.realloc( nItems );
    return aRet;
}

 sal_Bool SAL_CALL CmdMailMsg::hasByName( const OUString& aName )
{
    MutexGuard aGuard( m_aMutex );

    if( aName == "body" &&  !m_aBody.isEmpty() )
        return true;

    if( aName == "from" &&  !m_aOriginator.isEmpty() )
        return true;

    else if( aName == "to" &&  !m_aRecipient.isEmpty() )
        return true;

    else if( aName == "cc" &&  m_CcRecipients.getLength() )
        return true;

    else if( aName == "bcc" &&  m_BccRecipients.getLength() )
        return true;

    else if( aName == "subject" &&  !m_aSubject.isEmpty() )
        return true;

    else if( aName == "attachment" &&  m_Attachments.getLength() )
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
    return 0 != getElementNames().getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
