/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svcommstream.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-04-19 13:57:39 $
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

#include <tools/stream.hxx>

#include "commtypes.hxx"
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
