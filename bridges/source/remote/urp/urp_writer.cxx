/*************************************************************************
 *
 *  $RCSfile: urp_writer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jbu $ $Date: 2000-11-28 14:42:38 $
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
#include <assert.h>
#include <stdio.h>

#include <vos/thread.hxx>

#include <osl/mutex.hxx>
#include <osl/conditn.h>

#include <bridges/remote/connection.h>
#include <bridges/remote/remote.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <bridges/remote/counter.hxx>

#include "urp_writer.hxx"
#include "urp_bridgeimpl.hxx"
#include "urp_marshal.hxx"

#ifdef DEBUG
static MyCounter thisCounter( "DEBUG : WriterThread" );
#endif

using namespace ::osl;

namespace bridges_urp {

OWriterThread::OWriterThread( remote_Connection *pConnection, urp_BridgeImpl *pBridgeImpl) :
    m_pConnection( pConnection ),
    m_bAbort( sal_False ),
    m_pBridgeImpl( pBridgeImpl )
{
    m_oslCondition = osl_createCondition();
    osl_resetCondition( m_oslCondition );
    m_pConnection->acquire( m_pConnection );

#ifdef DEBUG
    thisCounter.acquire();
#endif
}

OWriterThread::~OWriterThread()
{
    osl_destroyCondition( m_oslCondition );
    m_pConnection->release( m_pConnection );
#ifdef DEBUG
    thisCounter.release();
#endif
}


// touch is called with locked m_marshalingMutex
void OWriterThread::touch( sal_Bool bImmediately )
{
      if( bImmediately || m_pBridgeImpl->m_blockMarshaler.getPos() > m_pBridgeImpl->m_properties.nFlushBlockSize )
      {
        write();
      }
}


void OWriterThread::abort()
{
    m_bAbort = sal_True;
    osl_setCondition( m_oslCondition );
}


// must be called with locked marshaling mutex
void OWriterThread::write()
{
     if( ! m_pBridgeImpl->m_blockMarshaler.empty() && ! m_bAbort )
     {
        m_pBridgeImpl->m_blockMarshaler.finish( m_pBridgeImpl->m_nMarshaledMessages);
        m_pBridgeImpl->m_nMarshaledMessages = 0;

        sal_Int32 nLength = m_pBridgeImpl->m_blockMarshaler.getSize();
        sal_Int8 *pBuf = m_pBridgeImpl->m_blockMarshaler.getBuffer();

        if( nLength != m_pConnection->write( m_pConnection, pBuf, nLength ))
        {
            m_pBridgeImpl->m_blockMarshaler.restart();
            return;
        }
        m_pConnection->flush( m_pConnection );
        m_pBridgeImpl->m_blockMarshaler.restart();
    }
}

void OWriterThread::sendEmptyMessage()
{
    // must be called with locked marshaling mutex
    sal_Int32 a[2] = {0,0};
    if( m_pConnection )
    {
        m_pConnection->write( m_pConnection , (sal_Int8*) a , sizeof( sal_Int32) *2 );
    }
}

void OWriterThread::run()
{
    while( sal_True )
    {
        // Wait for some work to do
        TimeValue value = { 0 , 1000 * m_pBridgeImpl->m_properties.nOnewayTimeoutMUSEC };
        osl_resetCondition( m_oslCondition );
        osl_waitCondition( m_oslCondition , &value );

        {
            // write to the socket
            MutexGuard guard( m_pBridgeImpl->m_marshalingMutex );
            m_bWaitForTimeout = sal_False;
            if( ! m_pBridgeImpl->m_blockMarshaler.empty() )
            {
                write();
            }
            osl_resetCondition( m_oslCondition );
        }
        if( m_bAbort )
        {
            break;
        }

    }
}


}

