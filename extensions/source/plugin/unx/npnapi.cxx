/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/plugin/unx/npnapi.cxx,v 1.3 2003-03-25 16:03:42 hr Exp $

*************************************************************************/
#include <plugin/unx/plugcon.hxx>

#include <dlfcn.h>

extern PluginConnector* pConnector;
extern NPWindow aNPWindow;
extern NPSetWindowCallbackStruct aNPSetWindowCallbackStruct;
extern XtAppContext app_context;
extern Widget topLevel;
extern Display* pAppDisplay;
extern int nAppArguments;
extern char** pAppArguments;
void* CreateNewShell( void** );

NPNetscapeFuncs aNetscapeFuncs =
{
    sizeof(aNetscapeFuncs),
    (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR,
    NPN_GetURL,
    NPN_PostURL,
    NPN_RequestRead,
    NPN_NewStream,
    NPN_Write,
    NPN_DestroyStream,
    NPN_Status,
    NPN_UserAgent,
    NPN_MemAlloc,
    NPN_MemFree,
    NPN_MemFlush,
    NPN_ReloadPlugins,
    NPN_GetJavaEnv,
    NPN_GetJavaPeer,
    NPN_GetURLNotify,
    NPN_PostURLNotify,
    NPN_GetValue,
    NPN_SetValue,
    NPN_InvalidateRect,
    NPN_InvalidateRegion,
    NPN_ForceRedraw
};

NPPluginFuncs aPluginFuncs =
{
    sizeof(aPluginFuncs),
    (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


void* pPluginLib = NULL;
char*(*pNPP_GetMIMEDescription)()                               = NULL;
NPError (*pNP_Initialize)(NPNetscapeFuncs*,NPPluginFuncs*)      = NULL;
NPError (*pNP_Shutdown)()                                       = NULL;

PluginConnectorList PluginConnector::allConnectors;

PluginConnector::PluginConnector( int nSocket ) :
        Mediator( nSocket )
{
    SetNewMessageHdl( LINK( this, PluginConnector, NewMessageHdl ) );
}

PluginConnector::~PluginConnector()
{
}

IMPL_LINK( PluginConnector, WorkOnNewMessageHdl, Mediator*, pMediator )
{
    MediatorMessage* pMessage;
    CommandAtoms nCommand;
    if( pMessage = GetNextMessage( FALSE ) )
    {
        nCommand = (CommandAtoms)pMessage->GetUINT32();
        medDebug( 1, "%s\n", GetCommandName( nCommand ) );
        switch( nCommand )
        {
            case eNPP_DestroyStream:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                UINT32 nFileID      = pMessage->GetUINT32();
                NPStream* pStream   = m_aNPWrapStreams.GetObject( nFileID );
                NPError aReason     = GetNPError( pMessage );
                aReason = aPluginFuncs.destroystream( instance, pStream, aReason );
                Respond( pMessage->m_nID,
                         (char*)&aReason, sizeof( aReason ),
                         NULL );
            }
            break;
            case eNPP_Destroy:
            {
                UINT32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances.GetObject( nInstance )->instance;
                NPSavedData* pSave = NULL;
                NPError aRet = aPluginFuncs.destroy( instance, &pSave );
                if( pSave )
                    Respond( pMessage->m_nID,
                             (char*)&aRet, sizeof( aRet ),
                             pSave->buf, pSave->len,
                             NULL );
                else
                    Respond( pMessage->m_nID,
                             (char*)&aRet, sizeof( aRet ),
                             "0000", 4,
                             NULL );
                delete pSave->buf;
                delete m_aInstances.Remove( nInstance );
                delete instance;
            }
            break;
            case eNPP_NewStream:
            {
                UINT32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances.GetObject( nInstance )->instance;
                char* pType             = pMessage->GetString();
                NPStream* pStream       = new NPStream;
                pStream->url            = pMessage->GetString();
                pStream->end            = pMessage->GetUINT32();
                pStream->lastmodified   = pMessage->GetUINT32();
                NPBool* pSeekable       = (NPBool*)pMessage->GetBytes();
                m_aNPWrapStreams.Insert( pStream, LIST_APPEND );
                uint16 nStype;
                NPError aRet = aPluginFuncs.newstream( instance, pType, pStream,
                                                       *pSeekable, &nStype );
                medDebug( aRet, "xhello: NPP_NewStream returns %d\n", (int) aRet );
                Respond( pMessage->m_nID,
                         (char*)&aRet, sizeof( aRet ),
                         &nStype, sizeof( nStype ),
                         NULL );
                delete pType;
            }
            break;
            case eNPP_New:
            {
                char* pType     = pMessage->GetString();
                uint16* pMode   = (uint16*)pMessage->GetBytes();
                int16*  pArgc   = (int16*)pMessage->GetBytes();
                NPP instance    = new NPP_t;
                instance->pdata = NULL;
                ULONG nArgnBytes, nArgvBytes;
                char* pArgn = (char*)pMessage->GetBytes( nArgnBytes );
                char* pArgv = (char*)pMessage->GetBytes( nArgvBytes );
                ULONG nSaveBytes;
                char* pSavedData = (char*)pMessage->GetBytes( nSaveBytes );
                ConnectorInstance* pInst =
                    new ConnectorInstance( instance, pType,
                                           *pArgc,
                                           pArgn, nArgnBytes,
                                           pArgv, nArgvBytes,
                                           pSavedData, nSaveBytes );
                m_aInstances.Insert( pInst, LIST_APPEND );
                NPError aRet;
                aRet = aPluginFuncs.newp( pInst->pMimeType, instance, *pMode, *pArgc,
                                          *pArgc ? pInst->argn : NULL,
                                          *pArgc ? pInst->argv : NULL,
                                          ( nSaveBytes == 4 && *(UINT32*)pSavedData == '0000' ) ?
                                          &(pInst->aData) : NULL );
                medDebug( aRet, "xhello: NPP_New returns %d\n", (int) aRet );
                Respond( pMessage->m_nID,
                         (char*)&aRet, sizeof( aRet ),
                         NULL );
                delete pMode;
                delete pArgc;
                delete pType;
            }
            break;
            case eNPP_SetWindow:
            {
                UINT32 nInstance        = pMessage->GetUINT32();
                ConnectorInstance* pInst= m_aInstances.GetObject( nInstance );
                NPWindow* pWindow       = (NPWindow*)pMessage->GetBytes();
                memcpy( &pInst->window, pWindow, sizeof( NPWindow ) );
                pInst->window.ws_info = &pInst->ws_info;
                memcpy( &pInst->ws_info, &aNPSetWindowCallbackStruct,
                        sizeof( NPSetWindowCallbackStruct ) );
                pInst->ws_info.type = 1;
                if( ! pInst->pWidget )
                {
                    pInst->pWidget = CreateNewShell( &(pInst->pShell) );

                    medDebug( 1, "Reparenting new widget %x to %x\n",
                              XtWindow( (Widget)pInst->pWidget ),
                              (XLIB_Window)pWindow->window );
                    XReparentWindow( pAppDisplay,
                                     XtWindow( (Widget)pInst->pShell ),
                                     (XLIB_Window)pWindow->window,
                                     0, 0 );
                    XSync( pAppDisplay, False );

                    XtRealizeWidget( (Widget)pInst->pWidget );
                    XtResizeWidget( (Widget)pInst->pShell,
                                    pWindow->width, pWindow->height, 0 );
                     XWithdrawWindow( pAppDisplay,
                                      XtWindow( topLevel ),
                                      DefaultScreen( pAppDisplay )
                                      );
                    XtMapWidget( (Widget)pInst->pWidget );
                    XRaiseWindow( pAppDisplay, XtWindow((Widget)pInst->pWidget) );
                    XSync( pAppDisplay, False );
                }

                pInst->window.window =
                    (void*)XtWindow( (Widget)pInst->pWidget );
                XtResizeWidget( (Widget)pInst->pShell,
                                pInst->window.width,
                                pInst->window.height,
                                0 );
                NPError aRet = aPluginFuncs.setwindow( pInst->instance, &pInst->window );
                medDebug( aRet, "xhello: NPP_SetWindow returns %d\n", (int) aRet );
                Respond( pMessage->m_nID,
                         (char*)&aRet, sizeof( aRet ),
                         NULL );
                delete pWindow;
            }
            break;
            case eNPP_StreamAsFile:
            {
                UINT32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances.GetObject( nInstance )->instance;
                UINT32 nFileID          = pMessage->GetUINT32();
                NPStream* pStream       = m_aNPWrapStreams.GetObject( nFileID );
                char* fname             = pMessage->GetString();
                medDebug( 1, "NPP_StreamAsFile %s\n", fname );
                aPluginFuncs.asfile( instance, pStream, fname );
                delete fname;
            }
            break;
            case eNPP_URLNotify:
            {
                UINT32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances.GetObject( nInstance )->instance;
                char* url               = pMessage->GetString();
                NPReason* pReason       = (NPReason*)pMessage->GetBytes();
                void** notifyData       = (void**)pMessage->GetBytes();
                aPluginFuncs.urlnotify( instance, url, *pReason, *notifyData );
                delete url;
                delete pReason;
                delete notifyData;
            }
            break;
            case eNPP_WriteReady:
            {
                UINT32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances.GetObject( nInstance )->instance;
                UINT32 nFileID          = pMessage->GetUINT32();
                NPStream* pStream       = m_aNPWrapStreams.GetObject( nFileID );
                int32 nRet = aPluginFuncs.writeready( instance, pStream );
                Respond( pMessage->m_nID,
                         (char*)&nRet, sizeof( nRet ),
                         NULL );
            }
            break;
            case eNPP_Write:
            {
                UINT32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances.GetObject( nInstance )->instance;
                UINT32 nFileID          = pMessage->GetUINT32();
                NPStream* pStream       = m_aNPWrapStreams.GetObject( nFileID );
                int32 offset            = pMessage->GetUINT32();
                ULONG len;
                char* buffer            = (char*)pMessage->GetBytes( len );
                int32 nRet = aPluginFuncs.write( instance, pStream, offset, len, buffer );
                Respond( pMessage->m_nID,
                         (char*)&nRet, sizeof( nRet ),
                         NULL );
                delete buffer;
            }
            break;
            case eNPP_GetMIMEDescription:
            {
                if( ! pNPP_GetMIMEDescription )
                    pNPP_GetMIMEDescription = (char*(*)())
                        dlsym( pPluginLib, "NPP_GetMIMEDescription" );
                char* pMIME = pNPP_GetMIMEDescription();
                Respond( pMessage->m_nID,
                         POST_STRING( pMIME ),
                         NULL );
            }
            break;
            case eNPP_Initialize:
            {

                pNP_Initialize =
                    (NPError(*)(NPNetscapeFuncs*, NPPluginFuncs*))
                    dlsym( pPluginLib, "NP_Initialize" );
                pNP_Shutdown = (NPError(*)())
                    dlsym( pPluginLib, "NP_Shutdown" );

                medDebug( 1, "entering NP_Initialize\n" );
                NPError aRet = pNP_Initialize( &aNetscapeFuncs, &aPluginFuncs );
                medDebug( 1, "xhello: NP_Initialize returns %d\n", (int) aRet );
                Respond( pMessage->m_nID, (char*)&aRet, sizeof( aRet ), NULL );
            }
            break;
            case eNPP_Shutdown:
            {
                pNP_Shutdown();
                dlclose( pPluginLib );
                exit( 0 );
            }
            break;
            default:
                medDebug( 1, "caught unknown NPP request %d\n", nCommand );
        }
        delete pMessage;
    }
    return 0;
}

// begin Netscape plugin api calls
extern "C" {

    void* NPN_MemAlloc( UINT32 nBytes )
    {
        void* pMem = malloc( nBytes );
        return pMem;
    }

    void NPN_MemFree( void* pMem )
{
    free( pMem );
}

    UINT32 NPN_MemFlush( UINT32 nSize )
{
    return 0;
}

NPError NPN_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes=
        pConnector->
        Transact( eNPN_DestroyStream,
                  &nInstance, sizeof( nInstance ),
                  pConnector->GetStreamID( stream ), sizeof( int ),
                  POST_STRING( stream->url ),
                  reason, sizeof( reason ),
                  NULL );

    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    pConnector->m_aNPWrapStreams.Remove( stream );
    delete stream->url;
    delete stream;
    // returns NPError
    NPError aRet = pConnector->GetNPError( pMes );
    delete pMes;
    return aRet;
}

    JRIEnv* NPN_GetJavaEnv()
{
    // no java in this program
    return NULL;
}

    jref NPN_GetJavaPeer( NPP instance )
{
    return NULL;
}

    NPError NPN_GetURL( NPP instance, const char* url, const char* window )
{
    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes=
        pConnector->
        Transact( eNPN_GetURL,
                  &nInstance, sizeof( nInstance ),
                  POST_STRING(url),
                  POST_STRING(window),
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    // returns NPError
    NPError aRet = pConnector->GetNPError( pMes );
    delete pMes;
    return aRet;
}

    NPError NPN_GetURLNotify( NPP instance, const char* url, const char* target,
                              void* notifyData )
{
    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes=
        pConnector->
        Transact( eNPN_GetURLNotify,
                  &nInstance, sizeof( nInstance ),
                  POST_STRING(url),
                  POST_STRING(target),
                  &notifyData, sizeof( void* ), // transmit the actual pointer
                  // since it is a pointer to private data fed back
                  // by NPP_URLNotify; this can be thought of as an ID
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    // returns NPError
    NPError aRet = pConnector->GetNPError( pMes );
    delete pMes;
    return aRet;
}

    NPError NPN_NewStream( NPP instance, NPMIMEType type, const char* target,
                           NPStream** stream )
        // stream is a return value
{
    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes=
        pConnector->
        Transact( eNPN_NewStream,
                  &nInstance, sizeof( nInstance ),
                  POST_STRING(type),
                  POST_STRING(target),
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    // returns a new NPStream and an error
    NPError aRet = pConnector->GetNPError( pMes );
    if( ! aRet )
    {
        NPStream* pStream = new NPStream;
        pStream->url = pMes->GetString();
        pStream->end = pMes->GetUINT32();
        pStream->lastmodified = pMes->GetUINT32();
        pConnector->m_aNPWrapStreams.Insert( pStream, LIST_APPEND );
        *stream = pStream;
    }

    delete pMes;
    return aRet;
}

    NPError NPN_PostURLNotify( NPP instance, const char* url, const char* target, UINT32 len, const char* buf, NPBool file, void* notifyData )
{
    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes = pConnector->
        Transact( eNPN_PostURLNotify,
                  &nInstance, sizeof( nInstance ),
                  POST_STRING( url ),
                  POST_STRING( target ),
                  &len, sizeof( len ),
                  buf, len,
                  &file, sizeof( NPBool ),
                  &notifyData, sizeof( void* ), // send the real pointer
                  NULL );

    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    NPError aRet = pConnector->GetNPError( pMes );
    delete pMes;
    return aRet;
}

NPError NPN_PostURL( NPP instance, const char* url, const char* window, UINT32 len, const char* buf, NPBool file )
{
    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes = pConnector->
        Transact( eNPN_PostURL,
                  &nInstance, sizeof( nInstance ),
                  POST_STRING( url ),
                  POST_STRING( window ),
                  &len, sizeof( len ),
                  buf, len,
                  &file, sizeof( NPBool ),
                  NULL );
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    NPError aRet = pConnector->GetNPError( pMes );
    delete pMes;
    return aRet;
}

NPError NPN_RequestRead( NPStream* stream, NPByteRange* rangeList )
{
    NPByteRange* pRange = rangeList;
    UINT32 nRanges = 0;
    while( pRange )
    {
        nRanges++;
        pRange = pRange->next;
    }

    UINT32* pArray = new UINT32[ 2 * nRanges ];
    pRange = rangeList;
    UINT32 n = 0;
    while( pRange )
    {
        pArray[ 2*n    ] = (UINT32)pRange->offset;
        pArray[ 2*n + 1] = (UINT32)pRange->length;
        n++;
        pRange = pRange->next;
    }
    UINT32 nFileID = pConnector->GetStreamID( stream );
    MediatorMessage* pMes = pConnector->
        Transact( eNPN_RequestRead,
                  &nFileID, sizeof( nFileID ),
                  &nRanges, sizeof( nRanges ),
                  pArray, sizeof( UINT32 ) * 2 * nRanges,
                  NULL );

    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    NPError aRet = pConnector->GetNPError( pMes );
    delete pArray;
    delete pMes;
    return aRet;
}

void NPN_Status( NPP instance, const char* message )
{
    UINT32 nInstance = pConnector->GetNPPID( instance );
    pConnector->Send( eNPN_Status,
                      &nInstance, sizeof( nInstance ),
                      POST_STRING( message ),
                      NULL );
}

const char* NPN_UserAgent( NPP instance )
{
    static char* pAgent = NULL;

    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes = pConnector->
        Transact( eNPN_UserAgent,
                  &nInstance, sizeof( nInstance ),
                  NULL );
    if( ! pMes )
        return pAgent;

    if( pAgent )
        delete pAgent;
    pAgent = pMes->GetString();

    delete pMes;
    return pAgent;
}

void NPN_Version( int* major, int* minor, int* net_major, int* net_minor )
{
    MediatorMessage* pMes = pConnector->
        Transact( eNPN_Version,
                  NULL );

    if( ! pMes )
        return;

    *major = pMes->GetUINT32();
    *minor = pMes->GetUINT32();
    *net_major = pMes->GetUINT32();
    *net_minor = pMes->GetUINT32();
    delete pMes;
}

int32 NPN_Write( NPP instance, NPStream* stream, int32 len,
                 void* buffer )
{
    UINT32 nFileID = pConnector->GetStreamID( stream );
    UINT32 nInstance = pConnector->GetNPPID( instance );
    MediatorMessage* pMes = pConnector->
        Transact( eNPN_Write,
                  &nInstance, sizeof( nInstance ),
                  &nFileID, sizeof( nFileID ),
                  &len, sizeof( len ),
                  buffer, len,
                  NULL );

    if( ! pMes )
        return 0;

    INT32 nRet = pMes->GetUINT32();
    return nRet;
}

void NPN_ReloadPlugins( NPBool reloadPages )
{
    medDebug( 1, "NPN_ReloadPlugins: SNI\n" );
}

NPError NPN_GetValue( NPP instance, NPNVariable variable, void* value )
{
    switch( variable )
    {
        case NPNVxDisplay:
            *((Display**)value) = pAppDisplay;
            medDebug( 1, "Display requested\n" );
            break;
        case NPNVxtAppContext:
            *((XtAppContext*)value) = app_context;
            medDebug( 1, "AppContext requested\n" );
            break;
        default:
            medDebug( 1, "unknown NPNVariable %d requested\n", variable );
            return 1;
    }
    return 0;
}

NPError NPN_SetValue(NPP instance, NPPVariable variable,
                         void *value)
{
    return 0;
}

void NPN_InvalidateRect(NPP instance, NPRect *invalidRect)
{
}

void NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion)
{
}

void NPN_ForceRedraw(NPP instance)
{
}

}

