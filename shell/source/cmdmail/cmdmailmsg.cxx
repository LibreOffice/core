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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#include "cmdmailmsg.hxx"

using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XNameAccess;
using rtl::OUString;
using osl::MutexGuard;

using namespace cppu;
using namespace com::sun::star::uno;


void SAL_CALL CmdMailMsg::setBody( const OUString& aBody )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aBody = aBody;
}

OUString SAL_CALL CmdMailMsg::getBody(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aBody;
}

void SAL_CALL CmdMailMsg::setRecipient( const OUString& aRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aRecipient = aRecipient;
}

OUString SAL_CALL CmdMailMsg::getRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aRecipient;
}

void SAL_CALL CmdMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_CcRecipients = aCcRecipient;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getCcRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_CcRecipients;
}

void SAL_CALL CmdMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_BccRecipients = aBccRecipient;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getBccRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_BccRecipients;
}

void SAL_CALL CmdMailMsg::setOriginator( const OUString& aOriginator )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aOriginator = aOriginator;
}

OUString SAL_CALL CmdMailMsg::getOriginator(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aOriginator;
}

void SAL_CALL CmdMailMsg::setSubject( const OUString& aSubject )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aSubject = aSubject;
}

OUString SAL_CALL CmdMailMsg::getSubject(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aSubject;
}

void SAL_CALL CmdMailMsg::setAttachement( const Sequence< OUString >& aAttachment )
    throw (IllegalArgumentException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_Attachments = aAttachment;
}

Sequence< OUString > SAL_CALL CmdMailMsg::getAttachement(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_Attachments;
}

Any SAL_CALL CmdMailMsg::getByName( const OUString& aName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "body" )) &&  m_aBody.getLength() )
        return makeAny( m_aBody );

    if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "from" )) &&  m_aOriginator.getLength() )
        return makeAny( m_aOriginator );

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "to" )) &&  m_aRecipient.getLength() )
        return makeAny( m_aRecipient );

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "cc" )) &&  m_CcRecipients.getLength() )
        return makeAny( m_CcRecipients );

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "bcc" )) &&  m_BccRecipients.getLength() )
        return makeAny( m_BccRecipients );

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "subject" )) &&  m_aSubject.getLength() )
        return makeAny( m_aSubject );

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "attachment" )) &&  m_Attachments.getLength() )
        return makeAny( m_Attachments );

   throw NoSuchElementException( OUString(RTL_CONSTASCII_USTRINGPARAM( "key not found: ")) + aName,
        static_cast < XNameAccess * > (this) );
}

Sequence< OUString > SAL_CALL CmdMailMsg::getElementNames(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    sal_Int32 nItems = 0;
    Sequence< OUString > aRet( 7 );

    if( m_aBody.getLength() )
        aRet[nItems++] = OUString(RTL_CONSTASCII_USTRINGPARAM( "body" ));

    if( m_aOriginator.getLength() )
        aRet[nItems++] = OUString(RTL_CONSTASCII_USTRINGPARAM( "from" ));

    if( m_aRecipient.getLength() )
        aRet[nItems++] = OUString(RTL_CONSTASCII_USTRINGPARAM( "to" ));

    if( m_CcRecipients.getLength() )
        aRet[nItems++] = OUString(RTL_CONSTASCII_USTRINGPARAM( "cc" ));

    if( m_BccRecipients.getLength() )
        aRet[nItems++] = OUString(RTL_CONSTASCII_USTRINGPARAM( "bcc" ));

    if( m_aSubject.getLength() )
        aRet[nItems++] = OUString(RTL_CONSTASCII_USTRINGPARAM( "subject" ));

    if( m_Attachments.getLength() )
        aRet[nItems++] = OUString(RTL_CONSTASCII_USTRINGPARAM( "attachment" ));

    aRet.realloc( nItems );
    return aRet;
}

 sal_Bool SAL_CALL CmdMailMsg::hasByName( const OUString& aName )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "body" )) &&  m_aBody.getLength() )
        return sal_True;

    if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "from" )) &&  m_aOriginator.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "to" )) &&  m_aRecipient.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "cc" )) &&  m_CcRecipients.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "bcc" )) &&  m_BccRecipients.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "subject" )) &&  m_aSubject.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "attachment" )) &&  m_Attachments.getLength() )
        return sal_True;

    return sal_False;
}

Type SAL_CALL CmdMailMsg::getElementType(  )
    throw (RuntimeException)
{
    // returning void for multi type container
    return Type();
}

sal_Bool SAL_CALL CmdMailMsg::hasElements(  )
    throw (RuntimeException)
{
    return 0 != getElementNames().getLength();
}
