/*************************************************************************
 *
 *  $RCSfile: cmdmailmsg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 11:34:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _CMDMAILMSG_HXX_
#include "cmdmailmsg.hxx"
#endif

#ifndef _COM_SUN_STAR_URI_XEXTERNALURIREFERENCETRANSLATOR_HPP_
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#endif

#ifndef _COM_SUN_STAR_URI_EXTERNALURIREFERENCETRANSLATOR_HPP_
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

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
//
//------------------------------------------------

void SAL_CALL CmdMailMsg::setRecipient( const ::rtl::OUString& aRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aRecipient = aRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

::rtl::OUString SAL_CALL CmdMailMsg::getRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CmdMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_CcRecipients = aCcRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getCcRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_CcRecipients;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CmdMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_BccRecipients = aBccRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getBccRecipient(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_BccRecipients;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CmdMailMsg::setOriginator( const OUString& aOriginator )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aOriginator = aOriginator;
}

//------------------------------------------------
//
//------------------------------------------------

OUString SAL_CALL CmdMailMsg::getOriginator(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aOriginator;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CmdMailMsg::setSubject( const OUString& aSubject )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aSubject = aSubject;
}

//------------------------------------------------
//
//------------------------------------------------

OUString SAL_CALL CmdMailMsg::getSubject(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aSubject;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CmdMailMsg::setAttachement( const Sequence< ::rtl::OUString >& aAttachment )
    throw (IllegalArgumentException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_Attachments = aAttachment;
    sal_Int32 n = m_Attachments.getLength();
    if (n > 0) {
        Reference< com::sun::star::uri::XExternalUriReferenceTranslator >
            translator(
                com::sun::star::uri::ExternalUriReferenceTranslator::create(
                    m_xContext));
        for (sal_Int32 i = 0; i < n; ++i) {
            OUString external(
                translator->translateToExternal(m_Attachments[i]));
            if (external.getLength() == 0
                && m_Attachments[i].getLength() != 0)
            {
                throw RuntimeException(
                    (OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "Cannot translate URI reference to external"
                            " format: "))
                     + m_Attachments[i]),
                    static_cast< cppu::OWeakObject * >(this));
            }
            m_Attachments[i] = external;
        }
    }
}

//------------------------------------------------
//
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getAttachement(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_Attachments;
}

//------------------------------------------------
//
//------------------------------------------------

Any SAL_CALL CmdMailMsg::getByName( const OUString& aName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( "from" ) &&  m_aOriginator.getLength() )
        return makeAny( m_aOriginator );

    else if( 0 == aName.compareToAscii( "to" ) &&  m_aRecipient.getLength() )
        return makeAny( m_aRecipient );

    else if( 0 == aName.compareToAscii( "cc" ) &&  m_CcRecipients.getLength() )
        return makeAny( m_CcRecipients );

    else if( 0 == aName.compareToAscii( "bcc" ) &&  m_BccRecipients.getLength() )
        return makeAny( m_BccRecipients );

    else if( 0 == aName.compareToAscii( "subject" ) &&  m_aSubject.getLength() )
        return makeAny( m_aSubject );

    else if( 0 == aName.compareToAscii( "attachment" ) &&  m_Attachments.getLength() )
        return makeAny( m_Attachments );

   throw NoSuchElementException( OUString::createFromAscii( "key not found: ") + aName,
        static_cast < XNameAccess * > (this) );
}

//------------------------------------------------
//
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getElementNames(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    sal_Int32 nItems = 0;
    Sequence< OUString > aRet( 6 );

    if( m_aOriginator.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "from" );

    if( m_aRecipient.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "to" );

    if( m_CcRecipients.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "cc" );

    if( m_BccRecipients.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "bcc" );

    if( m_aSubject.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "subject" );

    if( m_Attachments.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "attachment" );

    aRet.realloc( nItems );
    return aRet;
}

//------------------------------------------------
//
//------------------------------------------------

 sal_Bool SAL_CALL CmdMailMsg::hasByName( const ::rtl::OUString& aName )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( "from" ) &&  m_aOriginator.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "to" ) &&  m_aRecipient.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "cc" ) &&  m_CcRecipients.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "bcc" ) &&  m_BccRecipients.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "subject" ) &&  m_aSubject.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "attachment" ) &&  m_Attachments.getLength() )
        return sal_True;

    return sal_False;
}

//------------------------------------------------
//
//------------------------------------------------

Type SAL_CALL CmdMailMsg::getElementType(  )
    throw (RuntimeException)
{
    // returning void for multi type container
    return Type();
}

//------------------------------------------------
//
//------------------------------------------------

sal_Bool SAL_CALL CmdMailMsg::hasElements(  )
    throw (RuntimeException)
{
    return 0 != getElementNames().getLength();
}
