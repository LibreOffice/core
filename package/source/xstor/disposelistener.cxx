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
#include "precompiled_package.hxx"

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
    m_pStorage = NULL;
}

void SAL_CALL OChildDispListener_Impl::disposing( const lang::EventObject& Source )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // ObjectIsDisposed must not contain any locking!
    if ( m_pStorage && Source.Source.is() )
        m_pStorage->ChildIsDisposed( Source.Source );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
