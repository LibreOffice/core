/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: communiio.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:28:20 $
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

#include "commtypes.hxx"

#define C_ERROR_NONE                0x0001
#define C_ERROR_PERMANENT           0x0002
#define C_ERROR_RETRY               0x0003
#define C_ERROR_TIMEOUT             0x0004

class ITransmiter
{
protected:
    comm_ULONG nLastSent;
public:
    ITransmiter() :nLastSent( 0 ){}
    virtual ~ITransmiter() {}
    virtual comm_USHORT TransferBytes( const void* pBuffer, comm_UINT32 nLen ) = 0;

    comm_ULONG GetLastSent() { return nLastSent; }
};

class IReceiver
{
protected:
    comm_ULONG nLastReceived;
public:
    IReceiver() :nLastReceived( 0 ){}
    virtual ~IReceiver() {;}
    virtual comm_USHORT ReceiveBytes( void* pBuffer, comm_UINT32 nLen ) = 0;

    comm_ULONG GetLastReceived() { return nLastReceived; }
};
