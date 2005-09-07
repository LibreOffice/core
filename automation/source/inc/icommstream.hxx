/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: icommstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:17:43 $
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

#ifndef _AUTOMATION_ICOMMSTREAM_HXX_
#define _AUTOMATION_ICOMMSTREAM_HXX_

#include "commtypes.hxx"

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
