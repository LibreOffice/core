/*************************************************************************
 *
 *  $RCSfile: retstrm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:05:14 $
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
#ifndef _RETSTRM_HXX
#define _RETSTRM_HXX

#ifndef _SBXVAR_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SMARTID_HXX_
#include "smartid.hxx"
#endif
#include "cmdbasestream.hxx"

class SvStream;

class RetStream: public CmdBaseStream
{

public:
    RetStream();
    ~RetStream();

//  CmdBaseStream::GenError;
//    void GenError( comm_ULONG nError, const comm_UniChar* aString, comm_USHORT nLenInChars ){CmdBaseStream::GenError( nError, aString, nLenInChars );}
//  new
    void GenError( SmartId aUId, String aString );

//  CmdBaseStream::GenReturn;
    void GenReturn( comm_USHORT nRet, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, nNr );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, &aUId, nNr );}
//  void GenReturn( comm_USHORT nRet, comm_ULONG nUId, const comm_UniChar* aString, comm_USHORT nLenInChars ){CmdBaseStream::GenReturn( nRet, nUId, aString, nLenInChars );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, &aUId, bBool );}
//  void GenReturn( comm_USHORT nRet, comm_ULONG nUId, comm_ULONG nNr, const comm_UniChar* aString, comm_USHORT nLenInChars, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, nUId, nNr, aString, nLenInChars, bBool );}
// MacroRecorder
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_USHORT nMethod ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod );}
//  void GenReturn( comm_USHORT nRet, comm_ULONG nUId, comm_USHORT nMethod, const comm_UniChar* aString, comm_USHORT nLenInChars ){CmdBaseStream::GenReturn( nRet, nUId, nMethod, aString, nLenInChars );}
//  void GenReturn( comm_USHORT nRet, comm_ULONG nUId, comm_USHORT nMethod, const comm_UniChar* aString, comm_USHORT nLenInChars, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, nUId, nMethod, aString, nLenInChars, bBool );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_USHORT nMethod, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod, bBool );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_USHORT nMethod, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod, nNr );}

//  new
    void GenReturn( USHORT nRet, SmartId aUId, String aString );
    void GenReturn( USHORT nRet, SmartId aUId, SbxValue &aValue );
    void GenReturn( USHORT nRet, SmartId aUId, ULONG nNr, String aString, BOOL bBool );
// MacroRecorder
    void GenReturn( USHORT nRet, SmartId aUId, USHORT nMethod, String aString );
    void GenReturn( USHORT nRet, SmartId aUId, USHORT nMethod, String aString, BOOL bBool );

    void Reset();
    SvStream* GetStream();



//  CmdBaseStream::Write;
    void Write( comm_USHORT nNr ){CmdBaseStream::Write( nNr );}
    void Write( comm_ULONG nNr ){CmdBaseStream::Write( nNr );}
//  void Write( const comm_UniChar* aString, comm_USHORT nLenInChars ){CmdBaseStream::Write( aString, nLenInChars );}
    void Write( comm_BOOL bBool ){CmdBaseStream::Write( bBool );}
//  new
    void Write( SbxValue &aValue );

// Complex Datatypes to be handled system dependent
    virtual void Write( SmartId* pId );
    virtual void Write( String *pString );

    SvStream *pSammel;
};

#endif
