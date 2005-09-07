/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smplmailmsg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:09:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <osl/file.h>

#ifndef _SMPLMAILMSG_HXX_
#include "smplmailmsg.hxx"
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::IllegalArgumentException;
using rtl::OUString;
using osl::Mutex;

using namespace cppu;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

CSmplMailMsg::CSmplMailMsg( )
{
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CSmplMailMsg::setRecipient( const ::rtl::OUString& aRecipient )
    throw (RuntimeException)
{
    m_aRecipient = aRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

::rtl::OUString SAL_CALL CSmplMailMsg::getRecipient(  )
    throw (RuntimeException)
{
    return m_aRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CSmplMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
    throw (RuntimeException)
{
    m_CcRecipients = aCcRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

Sequence< OUString > SAL_CALL CSmplMailMsg::getCcRecipient(  )
    throw (RuntimeException)
{
    return m_CcRecipients;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CSmplMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
    throw (RuntimeException)
{
    m_BccRecipients = aBccRecipient;
}

//------------------------------------------------
//
//------------------------------------------------

Sequence< OUString > SAL_CALL CSmplMailMsg::getBccRecipient(  )
    throw (RuntimeException)
{
    return m_BccRecipients;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CSmplMailMsg::setOriginator( const OUString& aOriginator )
    throw (RuntimeException)
{
    m_aOriginator = aOriginator;
}

//------------------------------------------------
//
//------------------------------------------------

OUString SAL_CALL CSmplMailMsg::getOriginator(  )
    throw (RuntimeException)
{
    return m_aOriginator;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CSmplMailMsg::setSubject( const OUString& aSubject )
    throw (RuntimeException)
{
    m_aSubject = aSubject;
}

//------------------------------------------------
//
//------------------------------------------------

OUString SAL_CALL CSmplMailMsg::getSubject(  )
    throw (RuntimeException)
{
    return m_aSubject;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CSmplMailMsg::setAttachement( const Sequence< ::rtl::OUString >& aAttachement )
    throw (IllegalArgumentException, RuntimeException)
{
    m_Attachements = aAttachement;
}

//------------------------------------------------
//
//------------------------------------------------

Sequence< OUString > SAL_CALL CSmplMailMsg::getAttachement(  )
    throw (RuntimeException)
{
    return m_Attachements;
}
