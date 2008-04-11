/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tcpio.hxx,v $
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

#ifndef TCPIO_HXX
#define TCPIO_HXX

#include <vos/socket.hxx>
#include <vos/mutex.hxx>

#include "communiio.hxx"

class TCPIO : public ITransmiter, public IReceiver
{
private:
    NAMESPACE_VOS(OStreamSocket) *pStreamSocket;
    vos::OMutex aMSocketReadAccess;
    vos::OMutex aMSocketWriteAccess;

public:

    ///
    TCPIO( NAMESPACE_VOS(OStreamSocket) *pSocket ):pStreamSocket( pSocket ){}
    virtual ~TCPIO(){}


    /// implement ITransmiter
    virtual comm_USHORT TransferBytes( const void* pBuffer, comm_UINT32 nLen );

    /// implement IReceiver
    virtual comm_USHORT ReceiveBytes( void* pBuffer, comm_UINT32 nLen );

    // helper
    void SetStreamSocket( NAMESPACE_VOS(OStreamSocket) *pSocket );

};

#endif
