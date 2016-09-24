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

#include "disposelistener.hxx"
#include "xstorage.hxx"

using namespace ::com::sun::star;

OChildDispListener_Impl::OChildDispListener_Impl( OStorage& aStorage )
: m_pStorage( &aStorage )
{}

OChildDispListener_Impl::~OChildDispListener_Impl()
{}

void OChildDispListener_Impl::OwnerIsDisposed()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pStorage = nullptr;
}

void SAL_CALL OChildDispListener_Impl::disposing( const lang::EventObject& Source )
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // ObjectIsDisposed must not contain any locking!
    if ( m_pStorage && Source.Source.is() )
        m_pStorage->ChildIsDisposed( Source.Source );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
