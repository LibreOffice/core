/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmdbasestream.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:17:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to work inside and outside the StarOffice environment.
 *    Only adaption of file commtypes.hxx should be necessary. Else it is a bug!
 *
 ************************************************************************/

#ifndef _CMDBASESTREAM_HXX_
#define _CMDBASESTREAM_HXX_

#include "commtypes.hxx"
#include "icommstream.hxx"

class CmdBaseStream
{
protected:
    ICommStream* pCommStream;
    CmdBaseStream();
    virtual ~CmdBaseStream();

public:

    void GenError( SmartId *pUId, comm_String *pString );

    void GenReturn( comm_USHORT nRet, comm_ULONG nNr );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_ULONG nNr );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_String *pString );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_BOOL bBool );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_ULONG nNr, comm_String *pString, comm_BOOL bBool );

// MacroRecorder
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_String *pString );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_String *pString, comm_BOOL bBool );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_BOOL bBool );
    void GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_ULONG nNr );

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
    virtual void Read ( SmartId* &pId );

    virtual void Write( comm_String *pString );
    virtual void Write( SmartId* pId );
};

#endif
