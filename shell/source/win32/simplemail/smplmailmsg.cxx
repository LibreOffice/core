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


#include <osl/file.h>
#include "smplmailmsg.hxx"

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::IllegalArgumentException;

using namespace cppu;

CSmplMailMsg::CSmplMailMsg( )
{
}

void SAL_CALL CSmplMailMsg::setBody( const ::rtl::OUString& aBody )
{
    m_aBody = aBody;
}

::rtl::OUString SAL_CALL CSmplMailMsg::getBody(  )
{
    return m_aBody;
}

void SAL_CALL CSmplMailMsg::setRecipient( const OUString& aRecipient )
{
    m_aRecipient = aRecipient;
}

OUString SAL_CALL CSmplMailMsg::getRecipient(  )
{
    return m_aRecipient;
}

void SAL_CALL CSmplMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
{
    m_CcRecipients = aCcRecipient;
}

Sequence< OUString > SAL_CALL CSmplMailMsg::getCcRecipient(  )
{
    return m_CcRecipients;
}

void SAL_CALL CSmplMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
{
    m_BccRecipients = aBccRecipient;
}

Sequence< OUString > SAL_CALL CSmplMailMsg::getBccRecipient(  )
{
    return m_BccRecipients;
}

void SAL_CALL CSmplMailMsg::setOriginator( const OUString& aOriginator )
{
    m_aOriginator = aOriginator;
}

OUString SAL_CALL CSmplMailMsg::getOriginator(  )
{
    return m_aOriginator;
}

void SAL_CALL CSmplMailMsg::setSubject( const OUString& aSubject )
{
    m_aSubject = aSubject;
}

OUString SAL_CALL CSmplMailMsg::getSubject(  )
{
    return m_aSubject;
}

void SAL_CALL CSmplMailMsg::setAttachement( const Sequence< OUString >& aAttachement )
{
    m_Attachements = aAttachement;
}

Sequence< OUString > SAL_CALL CSmplMailMsg::getAttachement(  )
{
    return m_Attachements;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
