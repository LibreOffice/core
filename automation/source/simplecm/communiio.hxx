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

#include <automation/commtypes.hxx>

#define C_ERROR_NONE                0x0001
#define C_ERROR_PERMANENT           0x0002
#define C_ERROR_RETRY               0x0003
#define C_ERROR_TIMEOUT             0x0004

class ITransmiter
{
protected:
    comm_ULONG nLastSent;
public:
    ITransmiter() :nLastSent( 0 ){}
    virtual ~ITransmiter() {}
    virtual comm_USHORT TransferBytes( const void* pBuffer, comm_UINT32 nLen ) = 0;

    comm_ULONG GetLastSent() { return nLastSent; }
};

class IReceiver
{
protected:
    comm_ULONG nLastReceived;
public:
    IReceiver() :nLastReceived( 0 ){}
    virtual ~IReceiver() {;}
    virtual comm_USHORT ReceiveBytes( void* pBuffer, comm_UINT32 nLen ) = 0;

    comm_ULONG GetLastReceived() { return nLastReceived; }
};
