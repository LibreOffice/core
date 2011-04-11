/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
