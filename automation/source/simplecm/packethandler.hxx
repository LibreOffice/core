/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: packethandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 21:01:34 $
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

/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to work inside and outside the StarOffice environment.
 *    Only adaption of file commtypes.hxx should be necessary. Else it is a bug!
 *
 ************************************************************************/

#ifndef _TOOLS_PACKETHANDLER_HXX_
#define _TOOLS_PACKETHANDLER_HXX_

#include <automation/commtypes.hxx>
#include <automation/commdefines.hxx>

class ITransmiter;
class IReceiver;

class PacketHandler
{
private:
    unsigned char CalcCheckByte( comm_UINT32 nBytes );
    ITransmiter* pTransmitter;
    IReceiver* pReceiver;

    comm_BOOL bMultiChannel;

protected:
    comm_UINT16 nReceiveProtocol;
    comm_UINT16 nReceiveHeaderType;

public:
    PacketHandler( ITransmiter* pTransmitter_, IReceiver* pReceiver_, comm_BOOL bMC = FALSE );

    comm_UINT16 GetReceiveProtocol() { return nReceiveProtocol; }
    comm_UINT16 GetReceiveHeaderType() { return nReceiveHeaderType; }

    comm_BOOL ReceiveData( void* &pData, comm_UINT32 &nLen );               /// Recieve DataPacket from Socket
    virtual comm_BOOL SendHandshake( HandshakeType aHandshakeType, const void* pData = NULL, comm_UINT32 nLen = 0 );
    virtual comm_BOOL TransferData( const void* pData, comm_UINT32 nLen, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );
};

#endif
