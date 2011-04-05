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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include <osl/diagnose.h>

#include <osl/file.h>
#include "smplmailmsg.hxx"

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

void SAL_CALL CSmplMailMsg::setRecipient( const ::rtl::OUString& aRecipient )
    throw (RuntimeException)
{
    m_aRecipient = aRecipient;
}

//------------------------------------------------

::rtl::OUString SAL_CALL CSmplMailMsg::getRecipient(  )
    throw (RuntimeException)
{
    return m_aRecipient;
}

//------------------------------------------------

void SAL_CALL CSmplMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
    throw (RuntimeException)
{
    m_CcRecipients = aCcRecipient;
}

//------------------------------------------------

Sequence< OUString > SAL_CALL CSmplMailMsg::getCcRecipient(  )
    throw (RuntimeException)
{
    return m_CcRecipients;
}

//------------------------------------------------

void SAL_CALL CSmplMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
    throw (RuntimeException)
{
    m_BccRecipients = aBccRecipient;
}

//------------------------------------------------

Sequence< OUString > SAL_CALL CSmplMailMsg::getBccRecipient(  )
    throw (RuntimeException)
{
    return m_BccRecipients;
}

//------------------------------------------------

void SAL_CALL CSmplMailMsg::setOriginator( const OUString& aOriginator )
    throw (RuntimeException)
{
    m_aOriginator = aOriginator;
}

//------------------------------------------------

OUString SAL_CALL CSmplMailMsg::getOriginator(  )
    throw (RuntimeException)
{
    return m_aOriginator;
}

//------------------------------------------------

void SAL_CALL CSmplMailMsg::setSubject( const OUString& aSubject )
    throw (RuntimeException)
{
    m_aSubject = aSubject;
}

//------------------------------------------------

OUString SAL_CALL CSmplMailMsg::getSubject(  )
    throw (RuntimeException)
{
    return m_aSubject;
}

//------------------------------------------------

void SAL_CALL CSmplMailMsg::setAttachement( const Sequence< ::rtl::OUString >& aAttachement )
    throw (IllegalArgumentException, RuntimeException)
{
    m_Attachements = aAttachement;
}

//------------------------------------------------

Sequence< OUString > SAL_CALL CSmplMailMsg::getAttachement(  )
    throw (RuntimeException)
{
    return m_Attachements;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
