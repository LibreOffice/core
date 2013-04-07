/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/errcode.hxx>
#include <svl/svdde.hxx>
#include "ddectrl.hxx"
#include <basic/sberrors.hxx>

#define DDE_FREECHANNEL ((DdeConnection*)0xffffffff)

#define DDE_FIRSTERR    0x4000
#define DDE_LASTERR     0x4011

static const SbError nDdeErrMap[] =
{
    /* DMLERR_ADVACKTIMEOUT       */  0x4000, SbERR_DDE_TIMEOUT,
    /* DMLERR_BUSY                */  0x4001, SbERR_DDE_BUSY,
    /* DMLERR_DATAACKTIMEOUT      */  0x4002, SbERR_DDE_TIMEOUT,
    /* DMLERR_DLL_NOT_INITIALIZED */  0x4003, SbERR_DDE_ERROR,
    /* DMLERR_DLL_USAGE           */  0x4004, SbERR_DDE_ERROR,
    /* DMLERR_EXECACKTIMEOUT      */  0x4005, SbERR_DDE_TIMEOUT,
    /* DMLERR_INVALIDPARAMETER    */  0x4006, SbERR_DDE_ERROR,
    /* DMLERR_LOW_MEMORY          */  0x4007, SbERR_DDE_ERROR,
    /* DMLERR_MEMORY_ERROR        */  0x4008, SbERR_DDE_ERROR,
    /* DMLERR_NOTPROCESSED        */  0x4009, SbERR_DDE_NOTPROCESSED,
    /* DMLERR_NO_CONV_ESTABLISHED */  0x400a, SbERR_DDE_NO_CHANNEL,
    /* DMLERR_POKEACKTIMEOUT      */  0x400b, SbERR_DDE_TIMEOUT,
    /* DMLERR_POSTMSG_FAILED      */  0x400c, SbERR_DDE_QUEUE_OVERFLOW,
    /* DMLERR_REENTRANCY          */  0x400d, SbERR_DDE_ERROR,
    /* DMLERR_SERVER_DIED         */  0x400e, SbERR_DDE_PARTNER_QUIT,
    /* DMLERR_SYS_ERROR           */  0x400f, SbERR_DDE_ERROR,
    /* DMLERR_UNADVACKTIMEOUT     */  0x4010, SbERR_DDE_TIMEOUT,
    /* DMLERR_UNFOUND_QUEUE_ID    */  0x4011, SbERR_DDE_NO_CHANNEL
};

SbError SbiDdeControl::GetLastErr( DdeConnection* pConv )
{
    if( !pConv )
    {
        return 0;
    }
    long nErr = pConv->GetError();
    if( !nErr )
    {
        return 0;
    }
    if( nErr < DDE_FIRSTERR || nErr > DDE_LASTERR )
    {
        return SbERR_DDE_ERROR;
    }
    return nDdeErrMap[ 2 * (nErr - DDE_FIRSTERR) + 1 ];
}

IMPL_LINK_INLINE( SbiDdeControl,Data , DdeData*, pData,
{
    aData = OUString::createFromAscii( (const char*)(const void*)*pData );
    return 1;
}
)

SbiDdeControl::SbiDdeControl()
{
}

SbiDdeControl::~SbiDdeControl()
{
    TerminateAll();
}

size_t SbiDdeControl::GetFreeChannel()
{
    size_t nChannel = 0;
    size_t nListSize = aConvList.size();

    for (; nChannel < nListSize; ++nChannel)
    {
        if (aConvList[nChannel] == DDE_FREECHANNEL)
        {
            return nChannel+1;
        }
    }

    aConvList.push_back(DDE_FREECHANNEL);
    return nChannel+1;
}

SbError SbiDdeControl::Initiate( const OUString& rService, const OUString& rTopic,
                                 size_t& rnHandle )
{
    SbError nErr;
    DdeConnection* pConv = new DdeConnection( rService, rTopic );
    nErr = GetLastErr( pConv );
    if( nErr )
    {
        delete pConv;
        rnHandle = 0;
    }
    else
    {
        size_t nChannel = GetFreeChannel();
        aConvList[nChannel-1] = pConv;
        rnHandle = nChannel;
    }
    return 0;
}

SbError SbiDdeControl::Terminate( size_t nChannel )
{
    if (!nChannel || nChannel > aConvList.size())
    {
        return SbERR_DDE_NO_CHANNEL;
    }
    DdeConnection* pConv = aConvList[nChannel-1];

    if( pConv == DDE_FREECHANNEL )
    {
        return SbERR_DDE_NO_CHANNEL;
    }
    delete pConv;
    pConv = DDE_FREECHANNEL;

    return 0L;
}

SbError SbiDdeControl::TerminateAll()
{
    DdeConnection *conv;
    for (size_t nChannel = 0; nChannel < aConvList.size(); ++nChannel)
    {
        conv = aConvList[nChannel];

        if (conv != DDE_FREECHANNEL)
        {
            delete conv;
        }
    }

    aConvList.clear();

    return 0;
}

SbError SbiDdeControl::Request( size_t nChannel, const OUString& rItem, OUString& rResult )
{
    if (!nChannel || nChannel > aConvList.size())
    {
        return SbERR_DDE_NO_CHANNEL;
    }

    DdeConnection* pConv = aConvList[nChannel-1];

    if( pConv == DDE_FREECHANNEL )
    {
        return SbERR_DDE_NO_CHANNEL;
    }

    DdeRequest aRequest( *pConv, rItem, 30000 );
    aRequest.SetDataHdl( LINK( this, SbiDdeControl, Data ) );
    aRequest.Execute();
    rResult = aData;
    return GetLastErr( pConv );
}

SbError SbiDdeControl::Execute( size_t nChannel, const OUString& rCommand )
{
    if (!nChannel || nChannel > aConvList.size())
    {
        return SbERR_DDE_NO_CHANNEL;
    }

    DdeConnection* pConv = aConvList[nChannel-1];

    if( pConv == DDE_FREECHANNEL )
    {
        return SbERR_DDE_NO_CHANNEL;
    }
    DdeExecute aRequest( *pConv, rCommand, 30000 );
    aRequest.Execute();
    return GetLastErr( pConv );
}

SbError SbiDdeControl::Poke( size_t nChannel, const OUString& rItem, const OUString& rData )
{
    if (!nChannel || nChannel > aConvList.size())
    {
        return SbERR_DDE_NO_CHANNEL;
    }
    DdeConnection* pConv = aConvList[nChannel-1];

    if( pConv == DDE_FREECHANNEL )
    {
        return SbERR_DDE_NO_CHANNEL;
    }
    DdePoke aRequest( *pConv, rItem, DdeData(rData), 30000 );
    aRequest.Execute();
    return GetLastErr( pConv );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
