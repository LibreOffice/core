/*************************************************************************
 *
 *  $RCSfile: asynceventnotifier.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tra $ $Date: 2001-12-11 16:35:45 $
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

#ifndef _ASYNCEVENTNOTIFIER_HXX_
#include "asynceventnotifier.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#include <process.h>

//------------------------------------------------
//
//------------------------------------------------

const sal_uInt32 MAX_WAIT_SHUTDOWN  = 5000; // msec

//------------------------------------------------
//
//------------------------------------------------

using namespace osl;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::ui::dialogs::XFilePickerListener;
using ::com::sun::star::ui::dialogs::FilePickerEvent;
using ::cppu::OBroadcastHelper;
using ::com::sun::star::uno::RuntimeException;

//------------------------------------------------
//
//------------------------------------------------

CAsyncFilePickerEventNotifier::CAsyncFilePickerEventNotifier( cppu::OBroadcastHelper& rBroadcastHelper ) :
    m_bRunFilePickerNotifierThread( true ),
    m_rBroadcastHelper( rBroadcastHelper )
{
    unsigned uThreadId;
    m_hFilePickerNotifierThread = (HANDLE)_beginthreadex(
        NULL, 0, CAsyncFilePickerEventNotifier::ThreadProc, this, 0, &uThreadId );

    OSL_ASSERT( NULL != m_hFilePickerNotifierThread );
}

//------------------------------------------------
//
//------------------------------------------------

CAsyncFilePickerEventNotifier::~CAsyncFilePickerEventNotifier( )
{
    m_bRunFilePickerNotifierThread = false;
    m_NotifyFilePickerEvent.set( );

    sal_uInt32 dwResult = WaitForSingleObject(
        m_hFilePickerNotifierThread, MAX_WAIT_SHUTDOWN );

    OSL_ENSURE( dwResult == WAIT_OBJECT_0, "filepicker event notifier thread could not terminate" );

    if ( WAIT_TIMEOUT == dwResult )
        TerminateThread( m_hFilePickerNotifierThread, 0 );

    CloseHandle( m_hFilePickerNotifierThread );
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncFilePickerEventNotifier::notifyEvent( FilePickerEventListenerMethod_t aListenerMethod, FilePickerEvent aEvent )
{
    MutexGuard aGuard( m_FilePickerEventListMutex );
    m_FilePickerEventList.push_back( std::make_pair( aListenerMethod, aEvent ) );
    m_NotifyFilePickerEvent.set( );
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncFilePickerEventNotifier::run( )
{
    while ( m_bRunFilePickerNotifierThread )
    {
        m_NotifyFilePickerEvent.wait( );

        if ( !m_rBroadcastHelper.bDisposed )
        {
            while ( m_FilePickerEventList.size() > 0 )
            {
                ClearableMutexGuard aGuard( m_FilePickerEventListMutex );

                FilePickerEventRecord_t nextEventRecord = m_FilePickerEventList.front();
                m_FilePickerEventList.pop_front();

                aGuard.clear();

                ::cppu::OInterfaceContainerHelper* pICHelper =
                    m_rBroadcastHelper.aLC.getContainer(getCppuType((Reference<XFilePickerListener>*)0) );

                if ( pICHelper )
                {
                    ::cppu::OInterfaceIteratorHelper iter( *pICHelper );

                    while( iter.hasMoreElements() )
                    {
                        Reference< XFilePickerListener > xFPListener( iter.next( ), ::com::sun::star::uno::UNO_QUERY );

                        try
                        {
                            if ( xFPListener.is() )
                                (xFPListener.get()->*nextEventRecord.first)(nextEventRecord.second);
                        }
                        catch( RuntimeException& )
                        {
                            OSL_ENSURE( sal_False, "RuntimeException during event dispatching" );
                        }
                    }
                }
            }
        }

        m_NotifyFilePickerEvent.reset( );
    }
}

//------------------------------------------------
//
//------------------------------------------------

unsigned int WINAPI CAsyncFilePickerEventNotifier::ThreadProc( LPVOID pParam )
{
    CAsyncFilePickerEventNotifier* pInst = reinterpret_cast< CAsyncFilePickerEventNotifier* >( pParam );
    OSL_ASSERT( pInst );
    pInst->run( );

    return 0;
}