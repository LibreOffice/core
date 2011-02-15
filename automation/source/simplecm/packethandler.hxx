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
    PacketHandler( ITransmiter* pTransmitter_, IReceiver* pReceiver_, comm_BOOL bMC = sal_False );

    comm_UINT16 GetReceiveProtocol() { return nReceiveProtocol; }
    comm_UINT16 GetReceiveHeaderType() { return nReceiveHeaderType; }

    comm_BOOL ReceiveData( void* &pData, comm_UINT32 &nLen );               /// Recieve DataPacket from Socket
    virtual comm_BOOL SendHandshake( HandshakeType aHandshakeType, const void* pData = NULL, comm_UINT32 nLen = 0 );
    virtual comm_BOOL TransferData( const void* pData, comm_UINT32 nLen, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );
};

#endif
