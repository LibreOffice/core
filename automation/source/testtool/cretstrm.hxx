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


#ifndef _CRETSTRM_HXX
#define _CRETSTRM_HXX

#include <tools/stream.hxx>
#include <basic/sbxvar.hxx>

#include "cmdbasestream.hxx"

class CRetStream : public CmdBaseStream
{
    SvStream *pSammel;
    sal_uInt16 nId;

public:
    CRetStream( SvStream *pIn );
    ~CRetStream();

    using CmdBaseStream::Read;
    void Read ( comm_USHORT &nNr ){CmdBaseStream::Read ( nNr );}
    void Read ( comm_ULONG &nNr ){CmdBaseStream::Read ( nNr );}
//  void Read ( comm_UniChar* &aString, comm_USHORT &nLenInChars ){CmdBaseStream::Read ( aString, nLenInChars );}
    virtual void Read ( rtl::OString* &pId ){CmdBaseStream::Read ( pId );}
    void Read ( comm_BOOL &bBool ){CmdBaseStream::Read ( bBool );}
//  new
    void Read( String &aString );
    void Read( SbxValue &aValue );
};

#endif
