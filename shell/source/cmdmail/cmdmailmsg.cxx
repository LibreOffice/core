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
