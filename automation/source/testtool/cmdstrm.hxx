/*************************************************************************
 *
 *  $RCSfile: cmdstrm.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mh $ $Date: 2002-11-18 15:53:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    void WriteSortedParams( SbxArray* rPar, BOOL IsKeyString = FALSE );

    void GenCmdCommand( USHORT nNr, SbxArray* rPar );

    void GenCmdSlot( USHORT nNr, SbxArray* rPar );

    void GenCmdUNOSlot( const String &aURL );

    void GenCmdControl( ULONG nUId, USHORT nMethodId, SbxArray* rPar );
    void GenCmdControl( String aUId, USHORT nMethodId, SbxArray* rPar );

/*  void GenCmdControl (ULONG nUId, USHORT nMethodId = 0);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, String aString);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, String aString, USHORT nNr);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, String aString, BOOL bBool);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, USHORT nNr);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, USHORT nNr1, USHORT nNr2);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, USHORT nNr, BOOL bBool);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, ULONG nNr);
    void GenCmdControl (ULONG nUId, USHORT nMethodId, BOOL bBool);*/

    void GenCmdFlow( USHORT nArt );
    void GenCmdFlow( USHORT nArt, USHORT nNr1 );
    void GenCmdFlow( USHORT nArt, ULONG nNr1 );
    void GenCmdFlow( USHORT nArt, String aString1 );

    void Reset(ULONG nSequence);

    SvMemoryStream* GetStream();

    static CNames *pKeyCodes;           // Namen der Sondertasten  MOD1, F1, LEFT ...
    static ControlDefLoad __READONLY_DATA arKeyCodes [];

private:
    String WandleKeyEventString( String aKeys );    // Nutzt pKeyCodes.  <RETURN> <SHIFT LEFT LEFT>

//  CmdBaseStream::Write;
    void Write( comm_USHORT nNr ){CmdBaseStream::Write( nNr );}
    void Write( comm_ULONG nNr ){CmdBaseStream::Write( nNr );}
    void Write( const comm_UniChar* aString, comm_USHORT nLenInChars ){CmdBaseStream::Write( aString, nLenInChars );}
    void Write( comm_BOOL bBool ){CmdBaseStream::Write( bBool );}
//  new
    void Write( String aString, BOOL IsKeyString = FALSE );

    SvMemoryStream *pSammel;
};

#endif
