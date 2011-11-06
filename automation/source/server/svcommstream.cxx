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

#include "svcommstream.hxx"

SvCommStream::SvCommStream( SvStream* pIO ) { pStream = pIO; }
SvCommStream::~SvCommStream() {}

ICommStream& SvCommStream::operator>>( comm_USHORT& rUShort ) { *pStream >> rUShort; return *this; }
ICommStream& SvCommStream::operator>>( comm_ULONG& rULong ) { *pStream >> rULong; return *this; }
ICommStream& SvCommStream::operator>>( comm_BOOL& rChar ) { *pStream >> rChar; return *this; }

ICommStream& SvCommStream::operator<<( comm_USHORT nUShort ) { *pStream << nUShort; return *this; }
ICommStream& SvCommStream::operator<<( comm_ULONG nULong ) { *pStream << nULong; return *this; }
ICommStream& SvCommStream::operator<<( comm_BOOL nChar ) { *pStream << nChar; return *this; }

comm_ULONG SvCommStream::Read( void* pData, comm_ULONG nSize ) { return pStream->Read( pData, nSize ); }
comm_ULONG SvCommStream::Write( const void* pData, comm_ULONG nSize ) { return pStream->Write( pData, nSize ); }

comm_BOOL SvCommStream::IsEof() const { return pStream->IsEof(); }
comm_ULONG SvCommStream::SeekRel( long nPos ) { return pStream->SeekRel( nPos ); }
