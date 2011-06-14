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

#include <automation/commtypes.hxx>

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

    comm_ULONG GetLastSent() const { return nLastSent; }
};

class IReceiver
{
protected:
    comm_ULONG nLastReceived;
public:
    IReceiver() :nLastReceived( 0 ){}
    virtual ~IReceiver() {;}
    virtual comm_USHORT ReceiveBytes( void* pBuffer, comm_UINT32 nLen ) = 0;

    comm_ULONG GetLastReceived() const { return nLastReceived; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
