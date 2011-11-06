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

#ifndef _AUTOMATION_ICOMMSTREAM_HXX_
#define _AUTOMATION_ICOMMSTREAM_HXX_

#include <automation/commtypes.hxx>

class ICommStream
{
public:

    ICommStream(){}
    virtual ~ICommStream(){}

    virtual ICommStream&    operator>>( comm_USHORT& rUShort )=0;
    virtual ICommStream&    operator>>( comm_ULONG& rULong )=0;
    virtual ICommStream&    operator>>( comm_BOOL& rChar )=0;

    virtual ICommStream&    operator<<( comm_USHORT nUShort )=0;
    virtual ICommStream&    operator<<( comm_ULONG nULong )=0;
    virtual ICommStream&    operator<<( comm_BOOL nChar )=0;

    virtual comm_ULONG          Read( void* pData, comm_ULONG nSize )=0;
    virtual comm_ULONG          Write( const void* pData, comm_ULONG nSize )=0;

    virtual comm_BOOL       IsEof() const=0;
    virtual comm_ULONG      SeekRel( long nPos )=0;

};

#endif
