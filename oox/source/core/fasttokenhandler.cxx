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

#include "oox/core/fasttokenhandler.hxx"
#include <osl/mutex.hxx>
#include "oox/token/tokenmap.hxx"

using ::rtl::OUString;
using ::osl::Mutex;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

namespace oox {
namespace core {

// ============================================================================

namespace {

Mutex& lclGetTokenMutex()
{
    static Mutex aMutex;
    return aMutex;
}

} // namespace

// ============================================================================

FastTokenHandler::FastTokenHandler() :
    mrTokenMap( StaticTokenMap::get() )
{
}

FastTokenHandler::~FastTokenHandler()
{
}

sal_Int32 FastTokenHandler::getToken( const OUString& rIdentifier ) throw( RuntimeException )
{
    MutexGuard aGuard( lclGetTokenMutex() );
    return mrTokenMap.getTokenFromUnicode( rIdentifier );
}

OUString FastTokenHandler::getIdentifier( sal_Int32 nToken ) throw( RuntimeException )
{
    MutexGuard aGuard( lclGetTokenMutex() );
    return mrTokenMap.getUnicodeTokenName( nToken );
}

Sequence< sal_Int8 > FastTokenHandler::getUTF8Identifier( sal_Int32 nToken ) throw( RuntimeException )
{
    MutexGuard aGuard( lclGetTokenMutex() );
    return mrTokenMap.getUtf8TokenName( nToken );
}

sal_Int32 FastTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& rIdentifier ) throw( RuntimeException )
{
    MutexGuard aGuard( lclGetTokenMutex() );
    return mrTokenMap.getTokenFromUtf8( rIdentifier );
}

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
