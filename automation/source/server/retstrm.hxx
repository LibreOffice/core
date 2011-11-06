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


#ifndef _RETSTRM_HXX
#define _RETSTRM_HXX

#include <basic/sbxvar.hxx>
#include "cmdbasestream.hxx"

class SvStream;

class RetStream: public CmdBaseStream
{

public:
    RetStream();
    ~RetStream();

    using CmdBaseStream::GenError;
//  new
    void GenError( rtl::OString aUId, String aString );

    using CmdBaseStream::GenReturn;
    void GenReturn( comm_USHORT nRet, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, nNr );}
    void GenReturn( comm_USHORT nRet, rtl::OString aUId, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, &aUId, nNr );}
    void GenReturn( comm_USHORT nRet, rtl::OString aUId, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, &aUId, bBool );}

// MacroRecorder
    void GenReturn( comm_USHORT nRet, rtl::OString aUId, comm_USHORT nMethod ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod );} // also used outside MacroRecorder
    void GenReturn( comm_USHORT nRet, rtl::OString aUId, comm_USHORT nMethod, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod, bBool );}
    void GenReturn( comm_USHORT nRet, rtl::OString aUId, comm_USHORT nMethod, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod, nNr );}

//  new
    void GenReturn( sal_uInt16 nRet, rtl::OString aUId, String aString );
    void GenReturn( sal_uInt16 nRet, rtl::OString aUId, comm_ULONG nNr, String aString, sal_Bool bBool );

// needed for RemoteCommand and Profiling
    void GenReturn( sal_uInt16 nRet, sal_uInt16 nMethod, SbxValue &aValue );
    void GenReturn( sal_uInt16 nRet, sal_uInt16 nMethod, String aString );

// MacroRecorder
    void GenReturn( sal_uInt16 nRet, rtl::OString aUId, comm_USHORT nMethod, String aString );
    void GenReturn( sal_uInt16 nRet, rtl::OString aUId, comm_USHORT nMethod, String aString, sal_Bool bBool );

    void Reset();
    SvStream* GetStream();



    using CmdBaseStream::Write;
    void Write( comm_USHORT nNr ){CmdBaseStream::Write( nNr );}
    void Write( comm_ULONG nNr ){CmdBaseStream::Write( nNr );}
    void Write( comm_BOOL bBool ){CmdBaseStream::Write( bBool );}
//  new
    void Write( SbxValue &aValue );

// Complex Datatypes to be handled system dependent
    virtual void Write( rtl::OString* pId );
    virtual void Write( String *pString );

    SvStream *pSammel;
};

#endif
