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



#include <tools/stream.hxx>

#include <automation/commtypes.hxx>
#include "icommstream.hxx"


class SvCommStream : public ICommStream
{
protected:
    SvStream* pStream;
public:

    SvCommStream( SvStream* pIO );
    ~SvCommStream();

    ICommStream&        operator>>( comm_USHORT& rUShort );
    ICommStream&        operator>>( comm_ULONG& rULong );
    ICommStream&        operator>>( comm_BOOL& rChar );

    ICommStream&        operator<<( comm_USHORT nUShort );
    ICommStream&        operator<<( comm_ULONG nULong );
    ICommStream&        operator<<( comm_BOOL nChar );

    comm_ULONG      Read( void* pData, comm_ULONG nSize );
    comm_ULONG      Write( const void* pData, comm_ULONG nSize );

    comm_BOOL       IsEof() const;
    comm_ULONG      SeekRel( long nPos );
};
