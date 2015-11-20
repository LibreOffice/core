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


#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <sal/log.hxx>

#include <plugin/unx/plugcon.hxx>

#include <cstdarg>
#include <vector>

sal_uInt32 PluginConnector::GetStreamID( NPStream* pStream )
{
    size_t nLen = m_aNPWrapStreams.size();
    for( size_t i = 0; i < nLen; i++ )
        if( m_aNPWrapStreams[ i ] == pStream )
            return static_cast<sal_uInt32>(i);
    SAL_WARN("extensions.plugin", "NPStream has no ID");
    return UnknownStreamID;
}

sal_uInt32 PluginConnector::GetNPPID( NPP instance )
{
    size_t nLen = m_aInstances.size();
    for( size_t i=0; i <nLen; i++ )
        if( m_aInstances[ i ]->instance == instance )
            return static_cast<sal_uInt32>(i);
    SAL_WARN("extensions.plugin", "NPP has no ID");

    return UnknownNPPID;
}

struct PtrStruct
{
    char* pData;
    sal_uLong nBytes;

    PtrStruct( char* i_pData, sal_uLong i_nBytes )
    : pData( i_pData ), nBytes( i_nBytes ) {}
};

sal_uLong PluginConnector::FillBuffer( char*& rpBuffer,
                                   const char* pFunction,
                                   sal_uLong nFunctionLen,
                                   va_list ap )
{
    std::vector< PtrStruct > aList;
    aList.reserve( 5 );

    sal_uLong nDataSize = nFunctionLen + sizeof( sal_uLong );
    char* pNext;

    do {
        pNext = va_arg( ap, char* );
        if( pNext )
        {
            aList.push_back( PtrStruct( pNext, va_arg( ap, sal_uLong ) ) );
            nDataSize += aList.back().nBytes + sizeof(sal_uLong);
        }
    } while( pNext );

    rpBuffer = new char[ nDataSize ];
    char* pRun = rpBuffer;
    memcpy( pRun, &nFunctionLen, sizeof( nFunctionLen ) );
    pRun += sizeof( nFunctionLen );
    memcpy( pRun, pFunction, nFunctionLen );
    pRun += nFunctionLen;

    for( std::vector<PtrStruct>::const_iterator it = aList.begin(); it != aList.end(); ++it )
    {
        memcpy( pRun, &it->nBytes, sizeof( sal_uLong ) );
        pRun += sizeof( sal_uLong );
        memcpy( pRun, it->pData, it->nBytes );
        pRun += it->nBytes;
    }
    return nDataSize;
}

MediatorMessage* PluginConnector::Transact( const char* pFunction,
                                            sal_uLong nFunctionLen, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunctionLen );
    sal_uLong nSize = FillBuffer( pBuffer, pFunction, nFunctionLen, ap );
    va_end( ap );
    MediatorMessage* pRet = TransactMessage( nSize, pBuffer );
    delete[] pBuffer;
    return pRet;
}

MediatorMessage* PluginConnector::Transact( sal_uInt32 nFunction, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunction );
    sal_uLong nSize = FillBuffer( pBuffer, reinterpret_cast<char*>(&nFunction), sizeof( nFunction ), ap );
    va_end( ap );
    MediatorMessage* pRet = TransactMessage( nSize, pBuffer );
    delete[] pBuffer;
    return pRet;
}

sal_uLong PluginConnector::Send( sal_uInt32 nFunction, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunction );
    sal_uLong nSize = FillBuffer( pBuffer, reinterpret_cast<char*>(&nFunction), sizeof( nFunction ), ap );
    va_end( ap );
    sal_uLong nRet = SendMessage( nSize, pBuffer );
    delete[] pBuffer;
    return nRet;
}

void PluginConnector::Respond( sal_uLong nID,
                               char* pFunction,
                               sal_uLong nFunctionLen, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunctionLen );
    sal_uLong nSize = FillBuffer( pBuffer, pFunction, nFunctionLen, ap );
    va_end( ap );
    SendMessage( nSize, pBuffer, nID | ( 1 << 24 ) );
    delete[] pBuffer;
}

MediatorMessage* PluginConnector::WaitForAnswer( sal_uLong nMessageID )
{
    if( ! m_bValid )
        return nullptr;

    nMessageID &= 0x00ffffff;
    while( m_pListener )
    {
        {
            osl::MutexGuard aGuard( m_aQueueMutex );
            for( size_t i = 0; i < m_aMessageQueue.size(); i++ )
            {
                MediatorMessage* pMessage = m_aMessageQueue[ i ];
                sal_uLong nID = pMessage->m_nID;
                if(  ( nID & 0xff000000 ) &&
                     ( ( nID & 0x00ffffff ) == nMessageID ) )
                {
                    m_aMessageQueue.erase( m_aMessageQueue.begin() + i );
                    return pMessage;
                }
            }
        }
        if( ! m_aMessageQueue.empty() )
            CallWorkHandler();
        WaitForMessage( 2000 );
    }
    return nullptr;
}

ConnectorInstance::ConnectorInstance( NPP inst, char* type,
                                      int args, char* pargnbuf, sal_uLong nargnbytes,
                                      char* pargvbuf, sal_uLong nargvbytes,
                                      char* savedata, sal_uLong savebytes ) :
        instance( inst ),
        pShell( nullptr ),
        pWidget( nullptr ),
        pGtkWindow( nullptr ),
        pGtkWidget( nullptr ),
        bShouldUseXEmbed( false ),
        nArg( args ),
        pArgnBuf( pargnbuf ),
        pArgvBuf( pargvbuf )
{
    memset( &window, 0, sizeof(window) );
    pMimeType = new char[ strlen( type ) + 1 ];
    strcpy( pMimeType, type );
    aData.len = savebytes;
    aData.buf = savedata;
    argn = new char*[ nArg ];
    argv = new char*[ nArg ];
    int i;
    char* pRun = pArgnBuf;
    for( i = 0; i < nArg; i++ )
    {
        argn[i] = pRun;
        while( *pRun != 0 && (sal_uLong)(pRun - pArgnBuf) < nargnbytes )
            pRun++;
        if( (sal_uLong)(pRun - pArgnBuf) < nargnbytes )
            pRun++;
    }
    pRun = pArgvBuf;
    for( i = 0; i < nArg; i++ )
    {
        argv[i] = pRun;
        while( *pRun != 0 && (sal_uLong)(pRun - pArgvBuf) < nargvbytes )
            pRun++;
        if( (sal_uLong)(pRun - pArgvBuf) < nargvbytes )
            pRun++;
    }
}

ConnectorInstance::~ConnectorInstance()
{
    delete [] pMimeType;
    delete [] argn;
    delete [] argv;
    delete [] pArgnBuf;
    delete [] pArgvBuf;
    delete [] static_cast<char*>(aData.buf);
}

const char* GetCommandName( CommandAtoms eCommand )
{
    switch( eCommand )
    {
        case eNPN_GetURL:               return "NPN_GetURL";
        case eNPN_GetURLNotify:         return "NPN_GetURLNotify";
        case eNPN_DestroyStream:        return "NPN_DestroyStream";
        case eNPN_NewStream:            return "NPN_NewStream";
        case eNPN_PostURLNotify:        return "NPN_PostURLNotify";
        case eNPN_PostURL:              return "NPN_PostURL";
        case eNPN_RequestRead:          return "NPN_RequestRead";
        case eNPN_Status:               return "NPN_Status";
        case eNPN_Version:              return "NPN_Version";
        case eNPN_Write:                return "NPN_Write";
        case eNPN_UserAgent:            return "NPN_UserAgent";

        case eNPP_DestroyStream:        return "NPP_DestroyStream";
        case eNPP_Destroy:              return "NPP_Destroy";
        case eNPP_DestroyPhase2:        return "NPP_DestroyPhase2";
        case eNPP_NewStream:            return "NPP_NewStream";
        case eNPP_New:                  return "NPP_New";
        case eNPP_SetWindow:            return "NPP_SetWindow";
        case eNPP_StreamAsFile:         return "NPP_StreamAsFile";
        case eNPP_URLNotify:            return "NPP_URLNotify";
        case eNPP_WriteReady:           return "NPP_WriteReady";
        case eNPP_Write:                return "NPP_Write";
        case eNPP_GetMIMEDescription:   return "NPP_GetMIMEDescription";
        case eNPP_Initialize:           return "NPP_Initialize";
        case eNPP_Shutdown:             return "NPP_Shutdown";

        case eMaxCommand:               return "eMaxCommand";
        default:                        return "unknown command";
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
