/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tcpio.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:29:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
