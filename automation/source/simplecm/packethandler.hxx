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

    comm_BOOL ReceiveData( void* &pData, comm_UINT32 &nLen );               /// Receive DataPacket from Socket
    virtual comm_BOOL SendHandshake( HandshakeType aHandshakeType, const void* pData = NULL, comm_UINT32 nLen = 0 );
    virtual comm_BOOL TransferData( const void* pData, comm_UINT32 nLen, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );
};

#endif
