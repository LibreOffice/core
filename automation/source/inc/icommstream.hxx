/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: icommstream.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _AUTOMATION_ICOMMSTREAM_HXX_
#define _AUTOMATION_ICOMMSTREAM_HXX_

#include <automation/commtypes.hxx>

class ICommStream
{
public:

    ICommStream(){}
    virtual ~ICommStream(){}

    virtual ICommStream&    operator>>( comm_USHORT& rUShort )=0;
    virtual ICommStream&    operator>>( comm_ULONG& rULong )=0;
    virtual ICommStream&    operator>>( comm_BOOL& rChar )=0;

    virtual ICommStream&    operator<<( comm_USHORT nUShort )=0;
    virtual ICommStream&    operator<<( comm_ULONG nULong )=0;
    virtual ICommStream&    operator<<( comm_BOOL nChar )=0;

    virtual comm_ULONG          Read( void* pData, comm_ULONG nSize )=0;
    virtual comm_ULONG          Write( const void* pData, comm_ULONG nSize )=0;

    virtual comm_BOOL       IsEof() const=0;
    virtual comm_ULONG      SeekRel( long nPos )=0;

};

#endif
