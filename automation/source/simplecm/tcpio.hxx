/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tcpio.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:29:50 $
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

#ifndef TCPIO_HXX
#define TCPIO_HXX

#ifndef _VOS_SOCKET_HXX_ //autogen
#include <vos/socket.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include "communiio.hxx"

class TCPIO : public ITransmiter, public IReceiver
{
private:
    NAMESPACE_VOS(OStreamSocket) *pStreamSocket;
    vos::OMutex aMSocketReadAccess;
    vos::OMutex aMSocketWriteAccess;

public:

    ///
    TCPIO( NAMESPACE_VOS(OStreamSocket) *pSocket ):pStreamSocket( pSocket ){}
    virtual ~TCPIO(){}


    /// implement ITransmiter
    virtual comm_USHORT TransferBytes( const void* pBuffer, comm_UINT32 nLen );

    /// implement IReceiver
    virtual comm_USHORT ReceiveBytes( void* pBuffer, comm_UINT32 nLen );

    // helper
    void SetStreamSocket( NAMESPACE_VOS(OStreamSocket) *pSocket );

};

#endif
