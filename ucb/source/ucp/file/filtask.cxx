/*************************************************************************
 *
 *  $RCSfile: filtask.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:26:43 $
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
#ifndef _FILTASK_HXX_
#include "filtask.hxx"
#endif
#ifndef _FILGLOB_HXX_
#include "filglob.hxx"
#endif

/*********************************************************************************/
/*                                                                               */
/*                                 TaskHandling                                  */
/*                                                                               */
/*********************************************************************************/


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
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
    {
        throw DuplicateCommandIdentifierException();
    }
    m_aTaskMap[ CommandId ] = TaskHandling( xCommandEnv );
}



void SAL_CALL
TaskManager::endTask( shell * pShell,
                      sal_Int32 CommandId,
                      const rtl::OUString& aUncPath,
                      BaseContent* pContent)
{
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it == m_aTaskMap.end() )
        return;

    sal_Int32 ErrorCode = it->second.getInstalledError();
    sal_Int32 MinorCode = it->second.getMinorErrorCode();
    bool isHandled = it->second.isHandled();

    Reference< XCommandEnvironment > xComEnv = it->second.getCommandEnvironment();

    m_aTaskMap.erase( it );

    if( ErrorCode != TASKHANDLER_NO_ERROR )
        throw_handler(
            pShell,
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
        vos::OGuard aGuard( m_aMutex );
        TaskMap::iterator it = m_aTaskMap.find( CommandId );
        if( it == m_aTaskMap.end() )
            return;
        else
            it->second.abort();
    }
}


bool SAL_CALL TaskManager::isAborted( sal_Int32 CommandId )
{
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it == m_aTaskMap.end() || it->second.isAborted() )
        return false;
    else
        return true;
}


void SAL_CALL TaskManager::clearError( sal_Int32 CommandId )
{
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
        it->second.clearError();
}


void SAL_CALL TaskManager::retrieveError( sal_Int32 CommandId,
                                          sal_Int32 &ErrorCode,
                                          sal_Int32 &minorCode)
{
    vos::OGuard aGuard( m_aMutex );
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
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
        it->second.installError( ErrorCode,MinorCode );
}



sal_Int32 SAL_CALL
TaskManager::getCommandId( void )
{
    vos::OGuard aGuard( m_aMutex );
    return ++m_nCommandId;
}



uno::Reference< task::XInteractionHandler > SAL_CALL
TaskManager::getInteractionHandler( sal_Int32 CommandId )
{
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it == m_aTaskMap.end() )
        return uno::Reference< task::XInteractionHandler >( 0 );
    else
        return it->second.getInteractionHandler();
}



uno::Reference< XProgressHandler > SAL_CALL
TaskManager::getProgressHandler( sal_Int32 CommandId )
{
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it == m_aTaskMap.end() )
        return uno::Reference< XProgressHandler >( 0 );
    else
        return it->second.getProgressHandler();
}


uno::Reference< XCommandEnvironment > SAL_CALL
TaskManager::getCommandEnvironment( sal_Int32 CommandId )
{
    vos::OGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it == m_aTaskMap.end() )
        return uno::Reference< XCommandEnvironment >( 0 );
    else
        return it->second.getCommandEnvironment();
}


void SAL_CALL TaskManager::handleTask( sal_Int32 CommandId,
                                       const uno::Reference< task::XInteractionRequest >& request )
{
    vos::OGuard aGuard( m_aMutex );
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





