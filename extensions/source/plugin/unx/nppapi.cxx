/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/plugin/unx/nppapi.cxx,v 1.2 2003-03-25 16:03:42 hr Exp $

*************************************************************************/

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <plugin/impl.hxx>
#include <vcl/svapp.hxx>

PluginConnectorList PluginConnector::allConnectors;

PluginConnector::PluginConnector( int nSocket ) :
        Mediator( nSocket )
{
    allConnectors.Insert( this, LIST_APPEND );
    SetNewMessageHdl( LINK( this, PluginConnector, NewMessageHdl ) );
}

PluginConnector::~PluginConnector()
{
    NAMESPACE_VOS(OGuard) aGuard( m_aUserEventMutex );
    allConnectors.Remove( this );
}

IMPL_LINK( PluginConnector, NewMessageHdl, Mediator*, pMediator )
{
    if( allConnectors.GetPos( this ) == LIST_ENTRY_NOTFOUND )
        return 0;
    NAMESPACE_VOS(OGuard) aGuard( m_aUserEventMutex );
    return 0;
}

IMPL_LINK( PluginConnector, WorkOnNewMessageHdl, Mediator*, pMediator )
{
    if( allConnectors.GetPos( this ) == LIST_ENTRY_NOTFOUND )
        return 0;
/*
    {
        NAMESPACE_VOS(OGuard) aGuard( m_aUserEventMutex );
        m_aUserEventIDs.pop_front();
    }
*/

    MediatorMessage* pMessage;
    CommandAtoms nCommand;
    char* pRun;
    while( pMessage = GetNextMessage( FALSE ) )
    {
        nCommand = (CommandAtoms)pMessage->GetUINT32();
        medDebug( 1, "%s\n", GetCommandName( nCommand ) );
        switch( nCommand )
        {
            case eNPN_GetURL:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                char* pUrl          = pMessage->GetString();
                char* pWindow       = pMessage->GetString();
                NPError aRet = NPN_GetURL( instance, pUrl, pWindow );
                Respond( pMessage->m_nID,
                         (char*)(&aRet), sizeof( NPError ), NULL );
                delete pUrl;
                delete pWindow;
            }
            break;
            case eNPN_GetURLNotify:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                char* pUrl          = pMessage->GetString();
                char* pWindow       = pMessage->GetString();
                void** pNotifyData  = (void**)pMessage->GetBytes();
                NPError aRet = NPN_GetURLNotify( instance, pUrl, pWindow,
                                                 *pNotifyData );
                Respond( pMessage->m_nID,
                         (char*)(&aRet), sizeof( NPError ), NULL );
                delete pUrl;
                delete pWindow;
                delete pNotifyData;
            }
            break;
            case eNPN_DestroyStream:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                UINT32 nFileID      = pMessage->GetUINT32();
                char* pUrl          = pMessage->GetString();
                NPError* pReason    = (NPError*)pMessage->GetBytes();
                NPError aRet = NPERR_FILE_NOT_FOUND;
                if( nFileID < m_aNPWrapStreams.Count() )
                {
                    if( ! strcmp( m_aNPWrapStreams.GetObject( nFileID )->url, pUrl ) )
                    {
                        aRet =
                            NPN_DestroyStream( instance, m_aNPWrapStreams.GetObject( nFileID ),
                                               *pReason );
                        m_aNPWrapStreams.Remove( nFileID );
                    }
                    else
                        medDebug( 1, "StreamID %d has incoherent urls %s and %s\n",
                                  nFileID, pUrl, m_aNPWrapStreams.GetObject( nFileID )->url );
                }
                else
                    medDebug( 1, "Nonexistent StreamID %d\n", nFileID );

                Respond( pMessage->m_nID,
                         (char*)(&aRet), sizeof( NPError ), NULL );

                delete pUrl;
                delete pReason;
            }
            break;
            case eNPN_NewStream:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                NPMIMEType pType    = pMessage->GetString();
                char* pTarget       = pMessage->GetString();
                NPStream* pStream = new NPStream;

                NPError aRet = NPN_NewStream( instance, pType, pTarget, &pStream );

                if( aRet != NPERR_NO_ERROR )
                {
                    pStream->url = "";
                    pStream->end = 0;
                    pStream->lastmodified = 0;
                }
                else
                    m_aNPWrapStreams.Insert( pStream, LIST_APPEND );

                ULONG nLen = strlen( pStream->url );
                Respond( pMessage->m_nID,
                         (char*)&aRet, sizeof( aRet ),
                         pStream->url, nLen,
                         &pStream->end, sizeof(UINT32),
                         &pStream->lastmodified, sizeof(UINT32),
                         NULL );

                delete pTarget;
                delete pType;
            }
            break;
            case eNPN_PostURLNotify:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                char* pUrl      = pMessage->GetString();
                char* pTarget   = pMessage->GetString();
                UINT32 nLen     = pMessage->GetUINT32();
                char* pBuf      = (char*)pMessage->GetBytes();
                NPBool* pFile   = (NPBool*)pMessage->GetBytes();
                void** pNData   = (void**)pMessage->GetBytes();
                NPError aRet =
                    NPN_PostURLNotify( instance, pUrl, pTarget, nLen, pBuf, *pFile, *pNData );
                Respond( pMessage->m_nID, (char*)&aRet, sizeof( aRet ), NULL );
                delete pUrl;
                delete pTarget;
                delete pBuf;
                delete pFile;
                delete pNData;
            }
            break;
            case eNPN_PostURL:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                char* pUrl      = pMessage->GetString();
                char* pWindow   = pMessage->GetString();
                UINT32 nLen     = pMessage->GetUINT32();
                char* pBuf      = (char*)pMessage->GetBytes();
                NPBool* pFile   = (NPBool*)pMessage->GetBytes();
                NPError aRet =
                    NPN_PostURL( instance, pUrl, pWindow, nLen, pBuf, *pFile );
                Respond( pMessage->m_nID, (char*)&aRet, sizeof( aRet ), NULL );
                delete pUrl;
                delete pWindow;
                delete pBuf;
                delete pFile;
            }
            break;
            case eNPN_RequestRead:
            {
                UINT32 nFileID      = pMessage->GetUINT32();
                NPStream* pStream   = m_aNPWrapStreams.GetObject( nFileID );
                UINT32 nRanges      = pMessage->GetUINT32();
                UINT32* pArray      = (UINT32*)pMessage->GetBytes();
                // build ranges table
                NPByteRange* pFirst = new NPByteRange;
                NPByteRange* pRun   = pFirst;
                for( UINT32 n = 0; n < nRanges; n++ )
                {
                    pRun->offset = pArray[ 2*n ];
                    pRun->length = pArray[ 2*n+1 ];
                    pRun->next = n < nRanges-1 ? new NPByteRange : NULL;
                    pRun = pRun->next;
                }
                NPError aRet = NPN_RequestRead( pStream, pFirst );
                Respond( pMessage->m_nID, (char*)&aRet, sizeof( aRet ), NULL );
                while( pFirst )
                {
                    pRun = pFirst->next;
                    delete pFirst;
                    pFirst = pRun;
                }
                delete pArray;
            }
            break;
            case eNPN_Status:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                char* pString   = pMessage->GetString();
                NPN_Status( instance, pString );
                delete pString;
            }
            break;
            case eNPN_Version:
            {
                int major, minor, net_major, net_minor;
                NPN_Version( &major, &minor, &net_major, &net_minor );
                Respond( pMessage->m_nID,
                         (char*)&major, sizeof( int ),
                         &minor, sizeof( int ),
                         &net_major, sizeof( int ),
                         &net_minor, sizeof( int ),
                         NULL );
            }
            break;
            case eNPN_Write:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                UINT32 nFileID      = pMessage->GetUINT32();
                NPStream* pStream   = m_aNPWrapStreams.GetObject( nFileID );
                INT32 nLen          = pMessage->GetUINT32();
                void* pBuffer       = pMessage->GetBytes();
                INT32 nRet = NPN_Write( instance, pStream, nLen, pBuffer );
                Respond( pMessage->m_nID,
                         (char*)&nRet, sizeof( nRet ),
                         NULL );
                delete pBuffer;
                delete instance;
            }
            break;
            case eNPN_UserAgent:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                const char* pAnswer = NPN_UserAgent( instance );
                Respond( pMessage->m_nID,
                         (char*)pAnswer, strlen( pAnswer ),
                         NULL );
            }
            break;
            default:
                medDebug( 1, "caught unknown NPN request %d\n", nCommand );
        }

        delete pMessage;
    }
    return 0;
}

#define GET_INSTANCE() \
    UINT32 nInstance;  \
    nInstance = GetNPPID( instance )


#define POST_INSTANCE() (char*)&nInstance, sizeof( nInstance )

NPError UnxPluginComm::NPP_Destroy( NPP instance, NPSavedData** save )
{
    NPError aRet = NPERR_GENERIC_ERROR;
    GET_INSTANCE();
    MediatorMessage* pMes =
        Transact( eNPP_Destroy,
                  POST_INSTANCE(),
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    aRet = GetNPError( pMes );
    ULONG nSaveBytes;
    void* pSaveData = pMes->GetBytes( nSaveBytes );
    if( nSaveBytes == 4 && *(UINT32*)pSaveData == '0000' )
        *save = NULL;
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
    GET_INSTANCE();
    UINT32 nFileID = GetStreamID( stream );
    MediatorMessage* pMes =
        Transact( eNPP_DestroyStream,
                  POST_INSTANCE(),
                  &nFileID, sizeof( nFileID ),
                  &reason, sizeof( reason ),
                  NULL );
    m_aNPWrapStreams.Remove( stream );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    aRet = GetNPError( pMes );
    delete pMes;
    return aRet;
}

jref UnxPluginComm::NPP_GetJavaClass()
{
    return NULL;
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

NPError UnxPluginComm::NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
                 char* argn[], char* argv[], NPSavedData *saved )
{
    m_aInstances.Insert(
        new ConnectorInstance( instance, pluginType, 0,
                               NULL, 0, NULL, 0,
                               saved ? (char*)saved->buf : NULL,
                               saved ? saved->len : 0 ),
        LIST_APPEND );

    char *pArgnBuf, *pArgvBuf;
    int nArgnLen = 0, nArgvLen = 0;
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
                      saved->buf, saved->len,
                      NULL );
    else
        pMes =
            Transact( eNPP_New,
                      pluginType, strlen( pluginType ),
                      &mode, sizeof( mode ),
                      &argc, sizeof( argc ),
                      pArgnBuf, nArgnLen,
                      pArgvBuf, nArgvLen,
                      "0000", 4,
                      NULL );
    delete pArgnBuf;
    delete pArgvBuf;
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    NPError aRet = GetNPError( pMes );
    delete pMes;

    return aRet;
}

NPError UnxPluginComm::NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                       NPBool seekable, uint16* stype )
{
    NPError aRet = NPERR_GENERIC_ERROR;
    GET_INSTANCE();

    m_aNPWrapStreams.Insert( stream, LIST_APPEND );
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
    uint16* pSType = (uint16*)pMes->GetBytes();
    *stype = *pSType;
    delete pSType;
    delete pMes;
    return aRet;
}

void UnxPluginComm::NPP_Print( NPP instance, NPPrint* platformPrint )
{
}

NPError UnxPluginComm::NPP_SetWindow( NPP instance, NPWindow* window )
{
    NPError aRet = NPERR_GENERIC_ERROR;
    GET_INSTANCE();

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
    UINT32 nFileID = GetStreamID( stream );

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

int32 UnxPluginComm::NPP_Write( NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer )
{
    GET_INSTANCE();
    UINT32 nFileID = GetStreamID( stream );
    MediatorMessage* pMes =
        Transact( eNPP_Write,
                  POST_INSTANCE(),
                  &nFileID, sizeof( nFileID ),
                  &offset, sizeof( offset ),
                  buffer, len,
                  NULL );
    if( ! pMes )
        return 0;

    int32 aRet = pMes->GetUINT32();
    delete pMes;
    return aRet;
}

int32 UnxPluginComm::NPP_WriteReady( NPP instance, NPStream* stream )
{
    GET_INSTANCE();
    UINT32 nFileID = GetStreamID( stream );
    MediatorMessage* pMes =
        Transact( eNPP_WriteReady,
                  POST_INSTANCE(),
                  &nFileID, sizeof( nFileID ),
                  NULL );

    if( ! pMes )
        return 0;

    int32 aRet = pMes->GetUINT32();
    delete pMes;
    return aRet;
}

char* UnxPluginComm::NPP_GetMIMEDescription()
{
    static char* pDesc = NULL;
    MediatorMessage* pMes =
        Transact( eNPP_GetMIMEDescription,
                  NULL );
    if( ! pMes )
        return "";

    if( pDesc )
        delete pDesc;
    pDesc = pMes->GetString();
    delete pMes;
    return pDesc;
}

NPError UnxPluginComm::NPP_GetValue( NPP instance, NPPVariable variable, void* value )
{
    return 0;
}

NPError UnxPluginComm::NPP_SetValue( NPP instance, NPNVariable variable, void* value )
{
    return 0;
}
