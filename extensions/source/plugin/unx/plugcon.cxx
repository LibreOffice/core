/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/plugin/unx/plugcon.cxx,v 1.3 2003-03-25 16:03:42 hr Exp $

*************************************************************************/
#include <plugin/unx/plugcon.hxx>
#include <cstdarg>

UINT32 PluginConnector::GetStreamID( NPStream* pStream )
{
    for( ULONG i = 0; i < m_aNPWrapStreams.Count(); i++ )
        if( m_aNPWrapStreams.GetObject( i ) == pStream )
            return i;
    medDebug( 1, "Error: NPStream has no ID\n" );
    return ~0;
}

UINT32 PluginConnector::GetNPPID( NPP instance )
{
    for( ULONG i=0; i < m_aInstances.Count(); i++ )
        if( m_aInstances.GetObject( i )->instance == instance )
            return i;
    medDebug( 1, "Error: NPP has no ID\n" );
    return ~0;
}

struct PtrStruct
{
    char* pData;
    ULONG nBytes;
};

DECLARE_LIST( PtrStructList, PtrStruct* );

ULONG PluginConnector::FillBuffer( char*& rpBuffer,
                                   char* pFunction,
                                   ULONG nFunctionLen,
                                   va_list ap )
{
    PtrStructList aList;
    PtrStruct* pPtrStruct;
    ULONG nDataSize = nFunctionLen + sizeof( ULONG );
    char* pNext;

    do {
        pNext = va_arg( ap, char* );
        if( pNext )
        {
            pPtrStruct = new PtrStruct;
            pPtrStruct->pData = pNext;
            pPtrStruct->nBytes = va_arg( ap, ULONG );
            nDataSize += pPtrStruct->nBytes + sizeof(ULONG);
            aList.Insert( pPtrStruct, LIST_APPEND );
        }
    } while( pNext );

    rpBuffer = new char[ nDataSize ];
    char* pRun = rpBuffer;
    memcpy( pRun, &nFunctionLen, sizeof( nFunctionLen ) );
    pRun += sizeof( nFunctionLen );
    memcpy( pRun, pFunction, nFunctionLen );
    pRun += nFunctionLen;

    while( pPtrStruct = aList.Remove( (ULONG) 0 ) )
    {
        memcpy( pRun, &pPtrStruct->nBytes, sizeof( ULONG ) );
        pRun += sizeof( ULONG );
        memcpy( pRun, pPtrStruct->pData, pPtrStruct->nBytes );
        pRun += pPtrStruct->nBytes;
        delete pPtrStruct;
    }
    return nDataSize;
}

MediatorMessage* PluginConnector::Transact( char* pFunction,
                                            ULONG nFunctionLen, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunctionLen );
    ULONG nSize = FillBuffer( pBuffer, pFunction, nFunctionLen, ap );
    va_end( ap );
    return TransactMessage( nSize, pBuffer );
}

MediatorMessage* PluginConnector::Transact( UINT32 nFunction, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunction );
    ULONG nSize = FillBuffer( pBuffer, (char*)&nFunction, sizeof( nFunction ), ap );
    va_end( ap );
    return TransactMessage( nSize, pBuffer );
}

ULONG PluginConnector::Send( UINT32 nFunction, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunction );
    ULONG nSize = FillBuffer( pBuffer, (char*)&nFunction, sizeof( nFunction ), ap );
    va_end( ap );
    return SendMessage( nSize, pBuffer );
}

void PluginConnector::Respond( ULONG nID,
                               char* pFunction,
                               ULONG nFunctionLen, ... )
{
    va_list ap;
    char* pBuffer;

    va_start( ap, nFunctionLen );
    ULONG nSize = FillBuffer( pBuffer, pFunction, nFunctionLen, ap );
    va_end( ap );
    SendMessage( nSize, pBuffer, nID | ( 1 << 24 ) );
}

MediatorMessage* PluginConnector::WaitForAnswer( ULONG nMessageID )
{
    if( ! m_bValid )
        return NULL;

    nMessageID &= 0x00ffffff;
    while( m_pListener )
    {
        {
            NAMESPACE_VOS(OGuard) aGuard( m_aQueueMutex );
            for( int i = 0; i < m_aMessageQueue.Count(); i++ )
            {
                ULONG nID = m_aMessageQueue.GetObject( i )->m_nID;
                if(  ( nID & 0xff000000 ) &&
                     ( ( nID & 0x00ffffff ) == nMessageID ) )
                    return m_aMessageQueue.Remove( i );
            }
        }
        if( m_aMessageQueue.Count() )
            CallWorkHandler();
        WaitForMessage( 2000 );
    }
    return NULL;
}

ConnectorInstance::ConnectorInstance( NPP inst, char* type,
                                      int args, char* pargnbuf, ULONG nargnbytes,
                                      char* pargvbuf, ULONG nargvbytes,
                                      char* savedata, ULONG savebytes ) :
        instance( inst ),
        nArg( args ),
        pArgnBuf( pargnbuf ),
        pArgvBuf( pargvbuf ),
        pShell( NULL ),
        pWidget( NULL )
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
        while( *pRun != 0 && (ULONG)(pRun - pArgnBuf) < nargnbytes )
            pRun++;
        if( (ULONG)(pRun - pArgnBuf) < nargnbytes )
            pRun++;
    }
    pRun = pArgvBuf;
    for( i = 0; i < nArg; i++ )
    {
        argv[i] = pRun;
        while( *pRun != 0 && (ULONG)(pRun - pArgvBuf) < nargvbytes )
            pRun++;
        if( (ULONG)(pRun - pArgvBuf) < nargvbytes )
            pRun++;
    }
}

ConnectorInstance::~ConnectorInstance()
{
    delete pMimeType;
    delete argn;
    delete argv;
    delete pArgnBuf;
    delete pArgvBuf;
    delete aData.buf;
}

char* GetCommandName( CommandAtoms eCommand )
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
    return NULL;
}
