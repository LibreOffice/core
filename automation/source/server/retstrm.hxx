/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: retstrm.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _RETSTRM_HXX
#define _RETSTRM_HXX

#include <basic/sbxvar.hxx>
#include <vcl/smartid.hxx>
#include "cmdbasestream.hxx"

class SvStream;

class RetStream: public CmdBaseStream
{

public:
    RetStream();
    ~RetStream();

    using CmdBaseStream::GenError;
//    void GenError( comm_ULONG nError, const comm_UniChar* aString, comm_USHORT nLenInChars ){CmdBaseStream::GenError( nError, aString, nLenInChars );}
//  new
    void GenError( SmartId aUId, String aString );

    using CmdBaseStream::GenReturn;
    void GenReturn( comm_USHORT nRet, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, nNr );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, &aUId, nNr );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, &aUId, bBool );}
// MacroRecorder
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_USHORT nMethod ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_USHORT nMethod, comm_BOOL bBool ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod, bBool );}
    void GenReturn( comm_USHORT nRet, SmartId aUId, comm_USHORT nMethod, comm_ULONG nNr ){CmdBaseStream::GenReturn( nRet, &aUId, nMethod, nNr );}

//  new
    void GenReturn( USHORT nRet, SmartId aUId, String aString );
    void GenReturn( USHORT nRet, SmartId aUId, SbxValue &aValue );
    void GenReturn( USHORT nRet, SmartId aUId, comm_ULONG nNr, String aString, BOOL bBool );
// MacroRecorder
    void GenReturn( USHORT nRet, SmartId aUId, comm_USHORT nMethod, String aString );
    void GenReturn( USHORT nRet, SmartId aUId, comm_USHORT nMethod, String aString, BOOL bBool );

    void Reset();
    SvStream* GetStream();



    using CmdBaseStream::Write;
    void Write( comm_USHORT nNr ){CmdBaseStream::Write( nNr );}
    void Write( comm_ULONG nNr ){CmdBaseStream::Write( nNr );}
    void Write( comm_BOOL bBool ){CmdBaseStream::Write( bBool );}
//  new
    void Write( SbxValue &aValue );

// Complex Datatypes to be handled system dependent
    virtual void Write( SmartId* pId );
    virtual void Write( String *pString );

    SvStream *pSammel;
};

#endif
