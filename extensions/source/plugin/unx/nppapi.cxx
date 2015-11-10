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

#include <cstdarg>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#include <plugin/unx/plugcon.hxx>
#include <plugin/impl.hxx>

std::vector<PluginConnector*> PluginConnector::allConnectors;

PluginConnector::PluginConnector( int nSocket ) :
        Mediator( nSocket )
{
    allConnectors.push_back( this );
    SetNewMessageHdl( LINK( this, PluginConnector, NewMessageHdl ) );
}

PluginConnector::~PluginConnector()
{
    osl::MutexGuard aGuard( m_aUserEventMutex );
    for( std::vector< PluginConnector* >::iterator it = allConnectors.begin();
         it != allConnectors.end(); ++it )
    {
        if( *it == this )
        {
            allConnectors.erase( it );
            break;
        }
    }
}

IMPL_LINK_NOARG_TYPED( PluginConnector, NewMessageHdl, Mediator*, void )
{
    osl::MutexGuard aGuard( m_aUserEventMutex );
    bool bFound = false;
    for( std::vector< PluginConnector* >::iterator it = allConnectors.begin();
         it != allConnectors.end() && !bFound; ++it )
    {
        if( *it == this )
            bFound = true;
    }
    if( ! bFound )
        return;
    Application::PostUserEvent( LINK( this, PluginConnector, WorkOnNewMessageHdl ) );
}

IMPL_LINK_NOARG_TYPED( PluginConnector, WorkOnNewMessageHdl, void*, void )
{
    bool bFound = false;
    for( std::vector< PluginConnector* >::iterator it = allConnectors.begin();
         it != allConnectors.end() && !bFound; ++it )
    {
        if( *it == this )
            bFound = true;
    }
    if( ! bFound )
        return;

    MediatorMessage* pMessage;
    CommandAtoms nCommand;
    while( (pMessage = GetNextMessage()) )
    {
        nCommand = (CommandAtoms)pMessage->GetUINT32();
        SAL_INFO("extensions.plugin", GetCommandName(nCommand));
        switch( nCommand )
        {
            case eNPN_GetURL:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                char* pUrl          = pMessage->GetString();
                char* pWindow       = pMessage->GetString();
                NPError aRet = NPN_GetURL( instance, pUrl, pWindow );
                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&aRet), sizeof( NPError ), NULL );
                delete [] pUrl;
                delete [] pWindow;
            }
            break;
            case eNPN_GetURLNotify:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                char* pUrl          = pMessage->GetString();
                char* pWindow       = pMessage->GetString();
                void** pNotifyData  = static_cast<void**>(pMessage->GetBytes());
                NPError aRet = NPN_GetURLNotify( instance, pUrl, pWindow,
                                                 *pNotifyData );
                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&aRet), sizeof( NPError ), NULL );
                delete [] pUrl;
                delete [] pWindow;
                delete [] pNotifyData;
            }
            break;
            case eNPN_DestroyStream:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                sal_uInt32 nFileID      = pMessage->GetUINT32();
                char* pUrl          = pMessage->GetString();
                NPError* pReason    = static_cast<NPError*>(pMessage->GetBytes());
                NPError aRet = NPERR_FILE_NOT_FOUND;
                if( nFileID < static_cast<sal_uInt32>(m_aNPWrapStreams.size()) )
                {
                    if( ! strcmp( m_aNPWrapStreams[ nFileID ]->url, pUrl ) )
                    {
                        aRet =
                            NPN_DestroyStream( instance, m_aNPWrapStreams[ nFileID ],
                                               *pReason );
                        m_aNPWrapStreams.erase( m_aNPWrapStreams.begin() + nFileID );
                    }
                    else
                        SAL_WARN(
                            "extensions.plugin",
                            "StreamID " << nFileID << " has incoherent urls "
                                << pUrl << " and "
                                << m_aNPWrapStreams[nFileID]->url);
                }
                else
                    SAL_WARN(
                        "extensions.plugin",
                        "nonexistent StreamID " << nFileID);

                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&aRet), sizeof( NPError ), NULL );

                delete [] pUrl;
                delete [] pReason;
            }
            break;
            case eNPN_NewStream:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                NPMIMEType pType    = pMessage->GetString();
                char* pTarget       = pMessage->GetString();

                NPStream* pStream = nullptr;

                NPError aRet = NPN_NewStream( instance, pType, pTarget, &pStream );

                if( aRet != NPERR_NO_ERROR )
                {
                    sal_uInt32 nDummy = 0;
                    Respond( pMessage->m_nID,
                             reinterpret_cast<char*>(&aRet), sizeof( aRet ),
                             "", 0,
                             &nDummy, sizeof(sal_uInt32),
                             &nDummy, sizeof(sal_uInt32),
                             NULL );
                }
                else
                {
                    m_aNPWrapStreams.push_back( pStream );

                    sal_uLong nLen = strlen( pStream->url );
                    Respond( pMessage->m_nID,
                             reinterpret_cast<char*>(&aRet), sizeof( aRet ),
                             pStream->url, nLen,
                             &pStream->end, sizeof(sal_uInt32),
                             &pStream->lastmodified, sizeof(sal_uInt32),
                             NULL );
                }

                delete [] pTarget;
                delete [] pType;
            }
            break;
            case eNPN_PostURLNotify:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                char* pUrl      = pMessage->GetString();
                char* pTarget   = pMessage->GetString();
                sal_uInt32 nLen     = pMessage->GetUINT32();
                char* pBuf      = static_cast<char*>(pMessage->GetBytes());
                NPBool* pFile   = static_cast<NPBool*>(pMessage->GetBytes());
                void** pNData   = static_cast<void**>(pMessage->GetBytes());
                NPError aRet =
                    NPN_PostURLNotify( instance, pUrl, pTarget, nLen, pBuf, *pFile, *pNData );
                Respond( pMessage->m_nID, reinterpret_cast<char*>(&aRet), sizeof( aRet ), NULL );
                delete [] pUrl;
                delete [] pTarget;
                delete [] pBuf;
                delete [] pFile;
                delete [] pNData;
            }
            break;
            case eNPN_PostURL:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                char* pUrl      = pMessage->GetString();
                char* pWindow   = pMessage->GetString();
                sal_uInt32 nLen     = pMessage->GetUINT32();
                char* pBuf      = static_cast<char*>(pMessage->GetBytes());
                NPBool* pFile   = static_cast<NPBool*>(pMessage->GetBytes());
                NPError aRet =
                    NPN_PostURL( instance, pUrl, pWindow, nLen, pBuf, *pFile );
                Respond( pMessage->m_nID, reinterpret_cast<char*>(&aRet), sizeof( aRet ), NULL );
                delete [] pUrl;
                delete [] pWindow;
                delete [] pBuf;
                delete [] pFile;
            }
            break;
            case eNPN_RequestRead:
            {
                sal_uInt32 nFileID      = pMessage->GetUINT32();
                NPStream* pStream   = m_aNPWrapStreams[ nFileID ];
                sal_uInt32 nRanges      = pMessage->GetUINT32();
                sal_uInt32* pArray      = static_cast<sal_uInt32*>(pMessage->GetBytes());
                // build ranges table
                NPByteRange* pFirst = new NPByteRange;
                NPByteRange* pRun   = pFirst;
                for( sal_uInt32 n = 0; n < nRanges; n++ )
                {
                    pRun->offset = pArray[ 2*n ];
                    pRun->length = pArray[ 2*n+1 ];
                    pRun->next = n < nRanges-1 ? new NPByteRange : nullptr;
                    pRun = pRun->next;
                }
                NPError aRet = NPN_RequestRead( pStream, pFirst );
                Respond( pMessage->m_nID, reinterpret_cast<char*>(&aRet), sizeof( aRet ), NULL );
                while( pFirst )
                {
                    pRun = pFirst->next;
                    delete pFirst;
                    pFirst = pRun;
                }
                delete [] pArray;
            }
            break;
            case eNPN_Status:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                char* pString   = pMessage->GetString();
                NPN_Status( instance, pString );
                delete [] pString;
            }
            break;
            case eNPN_Version:
            {
                int major, minor, net_major, net_minor;
                NPN_Version( &major, &minor, &net_major, &net_minor );
                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&major), sizeof( int ),
                         &minor, sizeof( int ),
                         &net_major, sizeof( int ),
                         &net_minor, sizeof( int ),
                         NULL );
            }
            break;
            case eNPN_Write:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                sal_uInt32 nFileID      = pMessage->GetUINT32();
                NPStream* pStream   = m_aNPWrapStreams[ nFileID ];
                sal_Int32 nLen          = pMessage->GetUINT32();
                void* pBuffer       = pMessage->GetBytes();
                sal_Int32 nRet = NPN_Write( instance, pStream, nLen, pBuffer );
                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&nRet), sizeof( nRet ),
                         NULL );
                delete [] static_cast<char*>(pBuffer);
                delete instance;
            }
            break;
            case eNPN_UserAgent:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                const char* pAnswer = NPN_UserAgent( instance );
                Respond( pMessage->m_nID,
                         const_cast<char*>(pAnswer), strlen( pAnswer ),
                         NULL );
            }
            break;
            default:
                SAL_WARN(
                    "extensions.plugin",
                    "caught unknown NPN request " << +nCommand);
        }

        delete pMessage;
    }
}

#define GET_INSTANCE() \
    sal_uInt32 nInstance;  \
    nInstance = GetNPPID( instance );

#define GET_INSTANCE_RET( err ) \
    GET_INSTANCE() \
    if( nInstance == PluginConnector::UnknownNPPID ) \
        return err


#define POST_INSTANCE() reinterpret_cast<char*>(&nInstance), sizeof( nInstance )

NPError UnxPluginComm::NPP_Destroy( NPP instance, NPSavedData** save )
{
    NPError aRet = NPERR_GENERIC_ERROR;
    GET_INSTANCE_RET( aRet );
    MediatorMessage* pMes =
        Transact( eNPP_Destroy,
                  POST_INSTANCE(),
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;
    delete pMes;

    pMes = Transact( eNPP_DestroyPhase2,
                     POST_INSTANCE(),
                     NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    aRet = GetNPError( pMes );
    sal_uLong nSaveBytes;
    void* pSaveData = pMes->GetBytes( nSaveBytes );
    if( nSaveBytes == 4 && *static_cast<sal_uInt32*>(pSaveData) == 0 )
        *save = nullptr;
    else
    {
        *save = new NPSavedData;
        (*save)->len = nSaveBytes;
        (*save)->buf = pSaveData;
    }
    delete pMes;

    return aRet;
}

NPError UnxPluginComm::NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    NPError aRet = NPERR_GENERIC_ERROR;
    GET_INSTANCE_RET( aRet );
    sal_uInt32 nFileID = GetStreamID( stream );
    if( nFileID == PluginConnector::UnknownStreamID )
        return NPERR_GENERIC_ERROR;

    MediatorMessage* pMes =
        Transact( eNPP_DestroyStream,
                  POST_INSTANCE(),
                  &nFileID, sizeof( nFileID ),
                  &reason, sizeof( reason ),
                  NULL );
    m_aNPWrapStreams.erase( m_aNPWrapStreams.begin() + nFileID );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    aRet = GetNPError( pMes );
    delete pMes;
    return aRet;
}

NPError UnxPluginComm::NPP_Initialize()
{
    MediatorMessage* pMes =
        Transact( eNPP_Initialize,
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    NPError aRet = GetNPError( pMes );
    delete pMes;
    return aRet;
}

NPError UnxPluginComm::NPP_New( NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc,
                 char* argn[], char* argv[], NPSavedData *saved )
{
    m_aInstances.push_back(
        new ConnectorInstance( instance, pluginType, 0,
                               nullptr, 0, nullptr, 0,
                               saved ? static_cast<char*>(saved->buf) : nullptr,
                               saved ? saved->len : 0 ) );

    char *pArgnBuf, *pArgvBuf;
    size_t nArgnLen = 0, nArgvLen = 0;
    int i;
    for( i = 0; i < argc; i++ )
    {
        nArgnLen += strlen( argn[i] ) +1;
        nArgvLen += strlen( argv[i] ) +1;
    }
    pArgnBuf = new char[ nArgnLen ];
    pArgvBuf = new char[ nArgvLen ];
    char* pRunArgn = pArgnBuf;
    char* pRunArgv = pArgvBuf;
    for( i = 0; i < argc; i++ )
    {
        strcpy( pRunArgn, argn[i] );
        strcpy( pRunArgv, argv[i] );
        pRunArgn += strlen( argn[i] ) +1;
        pRunArgv += strlen( argv[i] ) +1;
    }

    MediatorMessage* pMes;
    if( saved )
        pMes =
            Transact( eNPP_New,
                      pluginType, strlen( pluginType ),
                      &mode, sizeof( mode ),
                      &argc, sizeof( argc ),
                      pArgnBuf, nArgnLen,
                      pArgvBuf, nArgvLen,
                      saved->buf, static_cast<size_t>(saved->len),
                      NULL );
    else
        pMes =
            Transact( eNPP_New,
                      pluginType, strlen( pluginType ),
                      &mode, sizeof( mode ),
                      &argc, sizeof( argc ),
                      pArgnBuf, nArgnLen,
                      pArgvBuf, nArgvLen,
                      "0000", size_t(4),
                      NULL );
    delete [] pArgnBuf;
    delete [] pArgvBuf;
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    NPError aRet = GetNPError( pMes );
    delete pMes;

    return aRet;
}

NPError UnxPluginComm::NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                       NPBool seekable, uint16_t* stype )
{
    NPError aRet = NPERR_GENERIC_ERROR;
    GET_INSTANCE_RET( aRet );

    m_aNPWrapStreams.push_back( stream );
    MediatorMessage* pMes =
        Transact( eNPP_NewStream,
                  POST_INSTANCE(),
                  type, strlen( type ),
                  stream->url, strlen( stream->url ),
                  &stream->end, sizeof( stream->end ),
                  &stream->lastmodified, sizeof( stream->lastmodified ),
                  &seekable, sizeof( seekable ),
                  NULL );

    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    aRet = GetNPError( pMes );
    uint16_t* pSType = static_cast<uint16_t*>(pMes->GetBytes());
    *stype = *pSType;

    delete [] pSType;
    delete pMes;
    return aRet;
}

NPError UnxPluginComm::NPP_SetWindow( NPP instance, NPWindow* window )
{
    NPError aRet = NPERR_GENERIC_ERROR;
    GET_INSTANCE_RET( aRet );

    MediatorMessage* pMes =
        Transact( eNPP_SetWindow,
                  POST_INSTANCE(),
                  window, sizeof( NPWindow ),
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    aRet = GetNPError( pMes );
    delete pMes;
    return aRet;
}

void UnxPluginComm::NPP_Shutdown()
{
    Send( eNPP_Shutdown, NULL );
}

void UnxPluginComm::NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname )
{
    GET_INSTANCE();
    sal_uInt32 nFileID = GetStreamID( stream );
    if( nFileID == PluginConnector::UnknownStreamID )
        return;

    Send( eNPP_StreamAsFile,
          POST_INSTANCE(),
          &nFileID, sizeof( nFileID ),
          fname, strlen( fname ),
          NULL );
}

void UnxPluginComm::NPP_URLNotify( NPP instance, const char* url, NPReason reason, void* notifyData )
{
    GET_INSTANCE();

    Send( eNPP_URLNotify,
          POST_INSTANCE(),
          url, strlen( url ),
          &reason, sizeof( reason ),
          &notifyData, sizeof( void* ),
          NULL );
}

int32_t UnxPluginComm::NPP_Write( NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer )
{
    GET_INSTANCE_RET( -1 );
    sal_uInt32 nFileID = GetStreamID( stream );
    if( nFileID == PluginConnector::UnknownStreamID )
        return -1;

    MediatorMessage* pMes =
        Transact( eNPP_Write,
                  POST_INSTANCE(),
                  &nFileID, sizeof( nFileID ),
                  &offset, sizeof( offset ),
                  buffer, static_cast<size_t>(len),
                  NULL );
    if( ! pMes )
        return 0;

    int32_t aRet = pMes->GetUINT32();
    delete pMes;

    return aRet;
}

int32_t UnxPluginComm::NPP_WriteReady( NPP instance, NPStream* stream )
{
    GET_INSTANCE_RET( -1 );
    sal_uInt32 nFileID = GetStreamID( stream );
    if( nFileID == PluginConnector::UnknownStreamID )
        return -1;

    MediatorMessage* pMes =
        Transact( eNPP_WriteReady,
                  POST_INSTANCE(),
                  &nFileID, sizeof( nFileID ),
                  NULL );

    if( ! pMes )
        return 0;

    int32_t aRet = pMes->GetUINT32();
    delete pMes;

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
