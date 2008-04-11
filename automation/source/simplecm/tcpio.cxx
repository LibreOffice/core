/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tcpio.cxx,v $
 * $Revision: 1.5 $
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
void TCPIO::SetStreamSocket( NAMESPACE_VOS(OStreamSocket) *pSocket )
{
    vos::OGuard aRGuard( aMSocketReadAccess );
    vos::OGuard aWGuard( aMSocketWriteAccess );
    pStreamSocket = pSocket;
}
