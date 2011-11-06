/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"

#include "tcpio.hxx"

/// implement ITransmiter
comm_USHORT TCPIO::TransferBytes( const void* pBuffer, comm_UINT32 nLen )
{
    vos::OGuard aGuard( aMSocketWriteAccess );
    if ( !pStreamSocket )
    {
        nLastSent = 0;
        return C_ERROR_PERMANENT;
    }
    nLastSent = pStreamSocket->write( pBuffer, nLen );
    if ( nLastSent == nLen )
        return C_ERROR_NONE;
    return C_ERROR_PERMANENT;
}


/// implement IReceiver
comm_USHORT TCPIO::ReceiveBytes( void* pBuffer, comm_UINT32 nLen )
{
    vos::OGuard aGuard( aMSocketReadAccess );
    if ( !pStreamSocket )
    {
        nLastReceived = 0;
        return C_ERROR_PERMANENT;
    }
    nLastReceived = pStreamSocket->read( pBuffer, nLen );
    if ( nLastReceived == nLen )
        return C_ERROR_NONE;
    return C_ERROR_PERMANENT;
}


// helper
void TCPIO::SetStreamSocket( vos::OStreamSocket *pSocket )
{
    vos::OGuard aRGuard( aMSocketReadAccess );
    vos::OGuard aWGuard( aMSocketWriteAccess );
    pStreamSocket = pSocket;
}
