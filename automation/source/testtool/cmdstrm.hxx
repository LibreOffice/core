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
