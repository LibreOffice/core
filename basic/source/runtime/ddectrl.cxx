/*************************************************************************
 *
 *  $RCSfile: ddectrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SVDDE_HXX //autogen
#include <svtools/svdde.hxx>
#endif
#pragma hdrstop
#include "ddectrl.hxx"
#ifndef _SBERRORS_HXX
#include <sberrors.hxx>
#endif

//#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

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
        return 0;
    long nErr = pConv->GetError();
    if( !nErr )
        return 0;
    if( nErr < DDE_FIRSTERR || nErr > DDE_LASTERR )
        return SbERR_DDE_ERROR;
    return nDdeErrMap[ 2*(nErr - DDE_FIRSTERR) + 1 ];
}

IMPL_LINK_INLINE( SbiDdeControl,Data , DdeData*, pData,
{
    aData = String::CreateFromAscii( (char*)(const void*)*pData );
    return 1;
}
)

SbiDdeControl::SbiDdeControl()
{
    pConvList = new DdeConnections;
    DdeConnection* pPtr = DDE_FREECHANNEL;
    pConvList->Insert( pPtr );
}

SbiDdeControl::~SbiDdeControl()
{
    TerminateAll();
    delete pConvList;
}

INT16 SbiDdeControl::GetFreeChannel()
{
    INT16 nListSize = (INT16)pConvList->Count();
    DdeConnection* pPtr = pConvList->First();
    pPtr = pConvList->Next(); // nullten eintrag ueberspringen
    INT16 nChannel;
    for( nChannel = 1; nChannel < nListSize; nChannel++ )
    {
        if( pPtr == DDE_FREECHANNEL )
            return nChannel;
        pPtr = pConvList->Next();
    }
    pPtr = DDE_FREECHANNEL;
    pConvList->Insert( pPtr, LIST_APPEND );
    return nChannel;
}

SbError SbiDdeControl::Initiate( const String& rService, const String& rTopic,
            INT16& rnHandle )
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
        INT16 nChannel = GetFreeChannel();
        pConvList->Replace( pConv, (ULONG)nChannel );
        rnHandle = nChannel;
    }
    return 0;
}

SbError SbiDdeControl::Terminate( INT16 nChannel )
{
    DdeConnection* pConv = pConvList->GetObject( (ULONG)nChannel );
    if( !nChannel || !pConv || pConv == DDE_FREECHANNEL )
        return SbERR_DDE_NO_CHANNEL;
    pConvList->Replace( DDE_FREECHANNEL, (ULONG)nChannel );
    delete pConv;
    return 0L;
}

SbError SbiDdeControl::TerminateAll()
{
    INT16 nChannel = (INT16)pConvList->Count();
    while( nChannel )
    {
        nChannel--;
        Terminate( nChannel );
    }

    pConvList->Clear();
    DdeConnection* pPtr = DDE_FREECHANNEL;
    pConvList->Insert( pPtr );

    return 0;
}

SbError SbiDdeControl::Request( INT16 nChannel, const String& rItem, String& rResult )
{
    DdeConnection* pConv = pConvList->GetObject( (ULONG)nChannel );
    if( !nChannel || !pConv || pConv == DDE_FREECHANNEL )
        return SbERR_DDE_NO_CHANNEL;

    DdeRequest aRequest( *pConv, rItem, 30000 );
    aRequest.SetDataHdl( LINK( this, SbiDdeControl, Data ) );
    aRequest.Execute();
    rResult = aData;
    return GetLastErr( pConv );
}

SbError SbiDdeControl::Execute( INT16 nChannel, const String& rCommand )
{
    DdeConnection* pConv = pConvList->GetObject( (ULONG)nChannel );
    if( !nChannel || !pConv || pConv == DDE_FREECHANNEL )
        return SbERR_DDE_NO_CHANNEL;
    DdeExecute aRequest( *pConv, rCommand, 30000 );
    aRequest.Execute();
    return GetLastErr( pConv );
}

SbError SbiDdeControl::Poke( INT16 nChannel, const String& rItem, const String& rData )
{
    DdeConnection* pConv = pConvList->GetObject( (ULONG)nChannel );
    if( !nChannel || !pConv || pConv == DDE_FREECHANNEL )
        return SbERR_DDE_NO_CHANNEL;
    DdePoke aRequest( *pConv, rItem, DdeData(rData), 30000 );
    aRequest.Execute();
    return GetLastErr( pConv );
}


