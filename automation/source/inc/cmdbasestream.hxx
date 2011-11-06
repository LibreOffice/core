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

#ifndef _CMDBASESTREAM_HXX_
#define _CMDBASESTREAM_HXX_

#include <automation/commtypes.hxx>
#include "icommstream.hxx"

class CmdBaseStream
{
protected:
    ICommStream* pCommStream;
    CmdBaseStream();
    virtual ~CmdBaseStream();

public:

    void GenError( rtl::OString *pUId, comm_String *pString );

    void GenReturn( comm_USHORT nRet, comm_ULONG nNr );
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_ULONG nNr );
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_String *pString );
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_BOOL bBool );
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_ULONG nNr, comm_String *pString, comm_BOOL bBool );

    void GenReturn( comm_USHORT nRet, comm_USHORT nMethod, comm_ULONG nNr );
    void GenReturn( comm_USHORT nRet, comm_USHORT nMethod, comm_String *pString );
    void GenReturn( comm_USHORT nRet, comm_USHORT nMethod, comm_BOOL bBool );
    void GenReturn( comm_USHORT nRet, comm_USHORT nMethod, comm_USHORT nNr );

// MacroRecorder
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod ); // also used outside MacroRecorder
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_String *pString );
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_String *pString, comm_BOOL bBool );
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_BOOL bBool );
    void GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_ULONG nNr );

    void Read ( comm_USHORT &nNr );
    void Read ( comm_ULONG &nNr );
    void Read (comm_UniChar* &aString, comm_USHORT &nLenInChars );
    void Read ( comm_BOOL &bBool );
    comm_USHORT GetNextType();

    void Write( comm_USHORT nNr );
    void Write( comm_ULONG nNr );
    void Write( const comm_UniChar* aString, comm_USHORT nLenInChars );
    void Write( comm_BOOL bBool );

// Complex Datatypes to be handled system dependent
    virtual void Read ( comm_String *&pString );
    virtual void Read ( rtl::OString* &pId );

    virtual void Write( comm_String *pString );
    virtual void Write( rtl::OString* pId );
};

#endif
