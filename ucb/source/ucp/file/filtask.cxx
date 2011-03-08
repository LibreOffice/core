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
#include "precompiled_ucb.hxx"
#include "filtask.hxx"
#include "filglob.hxx"

/******************************************************************************/
/*                                                                            */
/*                              TaskHandling                                  */
/*                                                                            */
/******************************************************************************/


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;



TaskManager::TaskManager()
    : m_nCommandId( 0 )
{
}



TaskManager::~TaskManager()
{
}



void SAL_CALL
TaskManager::startTask(
    sal_Int32 CommandId,
    const uno::Reference< XCommandEnvironment >& xCommandEnv )
    throw( DuplicateCommandIdentifierException )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
    {
        throw DuplicateCommandIdentifierException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ),
            uno::Reference< uno::XInterface >() );
    }
    m_aTaskMap[ CommandId ] = TaskHandling( xCommandEnv );
}



void SAL_CALL
TaskManager::endTask( sal_Int32 CommandId,
                      const rtl::OUString& aUncPath,
                      BaseContent* pContent)
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it == m_aTaskMap.end() )
        return;

    sal_Int32 ErrorCode = it->second.getInstalledError();
    sal_Int32 MinorCode = it->second.getMinorErrorCode();
    bool isHandled = it->second.isHandled();

    Reference< XCommandEnvironment > xComEnv
        = it->second.getCommandEnvironment();

    m_aTaskMap.erase( it );

    if( ErrorCode != TASKHANDLER_NO_ERROR )
        throw_handler(
            ErrorCode,
            MinorCode,
            xComEnv,
            aUncPath,
            pContent,
            isHandled);
}



void SAL_CALL
TaskManager::abort( sal_Int32 CommandId )
{
    if( CommandId )
    {
        osl::MutexGuard aGuard( m_aMutex );
        TaskMap::iterator it = m_aTaskMap.find( CommandId );
        if( it == m_aTaskMap.end() )
            return;
        else
            it->second.abort();
    }
}


void SAL_CALL TaskManager::clearError( sal_Int32 CommandId )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
        it->second.clearError();
}


void SAL_CALL TaskManager::retrieveError( sal_Int32 CommandId,
                                          sal_Int32 &ErrorCode,
                                          sal_Int32 &minorCode)
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
    {
        ErrorCode = it->second.getInstalledError();
        minorCode = it->second. getMinorErrorCode();
    }
}



void SAL_CALL TaskManager::installError( sal_Int32 CommandId,
                                         sal_Int32 ErrorCode,
                                         sal_Int32 MinorCode )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
        it->second.installError( ErrorCode,MinorCode );
}



sal_Int32 SAL_CALL
TaskManager::getCommandId( void )
{
    osl::MutexGuard aGuard( m_aMutex );
    return ++m_nCommandId;
}



void SAL_CALL TaskManager::handleTask(
    sal_Int32 CommandId,
    const uno::Reference< task::XInteractionRequest >& request )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    uno::Reference< task::XInteractionHandler > xInt;
    if( it != m_aTaskMap.end() )
    {
        xInt = it->second.getInteractionHandler();
        if( xInt.is() )
            xInt->handle( request );
        it->second.setHandled();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
