/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

using cpo::uno::Sequence;

using namespace cppu;

CSmplMailMsg::CSmplMailMsg( )
{
}

void CSmplMailMsg::setBody( const OUString& aBody )
{
    m_aBody = aBody;
}

OUString CSmplMailMsg::getBody(  )
{
    return m_aBody;
}

void CSmplMailMsg::setRecipient( const OUString& aRecipient )
{
    m_aRecipient = aRecipient;
}

OUString CSmplMailMsg::getRecipient(  )
{
    return m_aRecipient;
}

void CSmplMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient )
{
    m_CcRecipients = aCcRecipient;
}

Sequence< OUString > CSmplMailMsg::getCcRecipient(  )
{
    return m_CcRecipients;
}

void CSmplMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient )
{
    m_BccRecipients = aBccRecipient;
}

Sequence< OUString > CSmplMailMsg::getBccRecipient(  )
{
    return m_BccRecipients;
}

void CSmplMailMsg::setOriginator( const OUString& aOriginator )
{
    m_aOriginator = aOriginator;
}

OUString CSmplMailMsg::getOriginator(  )
{
    return m_aOriginator;
}

void CSmplMailMsg::setSubject( const OUString& aSubject )
{
    m_aSubject = aSubject;
}

OUString CSmplMailMsg::getSubject(  )
{
    return m_aSubject;
}

void CSmplMailMsg::setAttachement( const Sequence< OUString >& aAttachement )
{
    m_Attachements = aAttachement;
}

Sequence< OUString > CSmplMailMsg::getAttachement(  )
{
    return m_Attachements;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
