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


#ifndef _CMDSTRM_HXX
#define _CMDSTRM_HXX

#include <objtest.hxx>
#include <testapp.hxx>
#include "cmdbasestream.hxx"

class CmdStream : public CmdBaseStream
{
public:
    CmdStream();
    ~CmdStream();

    void WriteSortedParams( SbxArray* rPar, sal_Bool IsKeyString = sal_False );

    void GenCmdCommand( sal_uInt16 nNr, SbxArray* rPar );

    void GenCmdSlot( sal_uInt16 nNr, SbxArray* rPar );

    void GenCmdUNOSlot( const String &aURL );

    void GenCmdControl( comm_ULONG nUId, sal_uInt16 nMethodId, SbxArray* rPar );
    void GenCmdControl( String aUId, sal_uInt16 nMethodId, SbxArray* rPar );


    void GenCmdFlow( sal_uInt16 nArt );
    void GenCmdFlow( sal_uInt16 nArt, sal_uInt16 nNr1 );
    void GenCmdFlow( sal_uInt16 nArt, comm_ULONG nNr1 );
    void GenCmdFlow( sal_uInt16 nArt, String aString1 );

    void Reset(comm_ULONG nSequence);

    SvMemoryStream* GetStream();

    static CNames *pKeyCodes;           // Namen der Sondertasten  MOD1, F1, LEFT ...
    static ControlDefLoad __READONLY_DATA arKeyCodes [];

private:
    String WandleKeyEventString( String aKeys );    // Nutzt pKeyCodes.  <RETURN> <SHIFT LEFT LEFT>

    using CmdBaseStream::Write;
    void Write( comm_USHORT nNr ){CmdBaseStream::Write( nNr );}
    void Write( comm_ULONG nNr ){CmdBaseStream::Write( nNr );}
    void Write( const comm_UniChar* aString, comm_USHORT nLenInChars ){CmdBaseStream::Write( aString, nLenInChars );}
    void Write( comm_BOOL bBool ){CmdBaseStream::Write( bBool );}
//  new
    void Write( String aString, sal_Bool IsKeyString = sal_False );

    SvMemoryStream *pSammel;
};

#endif
