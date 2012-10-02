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

#include <osl/diagnose.h>
#include "cmdmailmsg.hxx"
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XNameAccess;
using rtl::OUString;
using osl::MutexGuard;

using namespace cppu;
using namespace com::sun::star::uno;


//------------------------------------------------

void SAL_CALL CmdMailMsg::setRecipient( const ::rtl::OUString& aRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aRecipient = aRecipient;
}

//------------------------------------------------

::rtl::OUString SAL_CALL CmdMailMsg::getRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aRecipient;
}

//------------------------------------------------

void SAL_CALL CmdMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_CcRecipients = aCcRecipient;
}

//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getCcRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_CcRecipients;
}

//------------------------------------------------

void SAL_CALL CmdMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_BccRecipients = aBccRecipient;
}

//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getBccRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_BccRecipients;
}

//------------------------------------------------

void SAL_CALL CmdMailMsg::setOriginator( const OUString& aOriginator )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aOriginator = aOriginator;
}

//------------------------------------------------

OUString SAL_CALL CmdMailMsg::getOriginator(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aOriginator;
}

//------------------------------------------------

void SAL_CALL CmdMailMsg::setSubject( const OUString& aSubject )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aSubject = aSubject;
}

//------------------------------------------------

OUString SAL_CALL CmdMailMsg::getSubject(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aSubject;
}

//------------------------------------------------

void SAL_CALL CmdMailMsg::setAttachement( const Sequence< ::rtl::OUString >& aAttachment )
    throw (IllegalArgumentException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_Attachments = aAttachment;
}

//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getAttachement(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_Attachments;
}

//------------------------------------------------

Any SAL_CALL CmdMailMsg::getByName( const OUString& aName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( "from" ) &&  !m_aOriginator.isEmpty() )
        return makeAny( m_aOriginator );

    else if( 0 == aName.compareToAscii( "to" ) &&  !m_aRecipient.isEmpty() )
        return makeAny( m_aRecipient );

    else if( 0 == aName.compareToAscii( "cc" ) &&  m_CcRecipients.getLength() )
        return makeAny( m_CcRecipients );

    else if( 0 == aName.compareToAscii( "bcc" ) &&  m_BccRecipients.getLength() )
        return makeAny( m_BccRecipients );

    else if( 0 == aName.compareToAscii( "subject" ) &&  !m_aSubject.isEmpty() )
        return makeAny( m_aSubject );

    else if( 0 == aName.compareToAscii( "attachment" ) &&  m_Attachments.getLength() )
        return makeAny( m_Attachments );

   throw NoSuchElementException( OUString("key not found: ") + aName,
        static_cast < XNameAccess * > (this) );
}

//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getElementNames(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    sal_Int32 nItems = 0;
    Sequence< OUString > aRet( 6 );

    if( !m_aOriginator.isEmpty() )
        aRet[nItems++] = OUString("from");

    if( !m_aRecipient.isEmpty() )
        aRet[nItems++] = OUString("to");

    if( m_CcRecipients.getLength() )
        aRet[nItems++] = OUString("cc");

    if( m_BccRecipients.getLength() )
        aRet[nItems++] = OUString("bcc");

    if( !m_aSubject.isEmpty() )
        aRet[nItems++] = OUString("subject");

    if( m_Attachments.getLength() )
        aRet[nItems++] = OUString("attachment");

    aRet.realloc( nItems );
    return aRet;
}

//------------------------------------------------

 sal_Bool SAL_CALL CmdMailMsg::hasByName( const ::rtl::OUString& aName )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( "from" ) &&  !m_aOriginator.isEmpty() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "to" ) &&  !m_aRecipient.isEmpty() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "cc" ) &&  m_CcRecipients.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "bcc" ) &&  m_BccRecipients.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "subject" ) &&  !m_aSubject.isEmpty() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "attachment" ) &&  m_Attachments.getLength() )
        return sal_True;

    return sal_False;
}

//------------------------------------------------

Type SAL_CALL CmdMailMsg::getElementType(  )
    throw (RuntimeException)
{
    // returning void for multi type container
    return Type();
}

//------------------------------------------------

sal_Bool SAL_CALL CmdMailMsg::hasElements(  )
    throw (RuntimeException)
{
    return 0 != getElementNames().getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
