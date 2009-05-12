/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svcommstream.hxx,v $
 * $Revision: 1.5 $
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

#include <tools/stream.hxx>

#include <automation/commtypes.hxx>
#include "icommstream.hxx"


class SvCommStream : public ICommStream
{
protected:
    SvStream* pStream;
public:

    SvCommStream( SvStream* pIO );
    ~SvCommStream();

    ICommStream&        operator>>( comm_USHORT& rUShort );
    ICommStream&        operator>>( comm_ULONG& rULong );
    ICommStream&        operator>>( comm_BOOL& rChar );

    ICommStream&        operator<<( comm_USHORT nUShort );
    ICommStream&        operator<<( comm_ULONG nULong );
    ICommStream&        operator<<( comm_BOOL nChar );

    comm_ULONG      Read( void* pData, comm_ULONG nSize );
    comm_ULONG      Write( const void* pData, comm_ULONG nSize );

    comm_BOOL       IsEof() const;
    comm_ULONG      SeekRel( long nPos );
};
