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

#include <plugin/unx/plugcon.hxx>

#include <unistd.h>
#include <dlfcn.h>

#include <osl/module.h>
#include <sal/log.hxx>

#include <config_vclplug.h>

#include <npwrap.hxx>

extern PluginConnector* pConnector;
extern XtAppContext app_context;
extern int wakeup_fd[];
extern Widget topLevel, topBox;
extern Display* pAppDisplay;
extern Display* pXtAppDisplay;
extern int nAppArguments;
extern char** pAppArguments;

// begin Netscape plugin api calls
extern "C" {

static void* l_NPN_MemAlloc( uint32_t nBytes )
{
    void* pMem = new char[nBytes];
    return pMem;
}

static void l_NPN_MemFree( void* pMem )
{
    delete [] static_cast<char*>(pMem);
}

static uint32_t l_NPN_MemFlush( uint32_t /*nSize*/ )
{
    return 0;
}

static NPError l_NPN_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return NPERR_GENERIC_ERROR;

    sal_uInt32 nFileID = pConnector->GetStreamID( stream );
    MediatorMessage* pMes=
        pConnector->
        Transact( eNPN_DestroyStream,
                  &nInstance, sizeof( nInstance ),
                  &nFileID, sizeof( nFileID ),
                  POST_STRING( stream->url ),
                  &reason, sizeof( reason ),
                  NULL );

    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    for( std::vector< NPStream* >::iterator it = pConnector->getStreamList().begin();
         it != pConnector->getStreamList().end(); ++it )
    {
        if( *it == stream )
        {
            pConnector->getStreamList().erase( it );
            break;
        }
    }
    delete [] stream->url;
    delete stream;
    // returns NPError
    NPError aRet = PluginConnector::GetNPError( pMes );
    delete pMes;
    return aRet;
}

#ifdef OJI
static JRIEnv* l_NPN_GetJavaEnv()
{
    // no java in this program
    SAL_INFO("extensions.plugin", "SNI: NPN_GetJavaEnv");
    return NULL;
}

static jref l_NPN_GetJavaPeer( NPP /*instance*/ )
{
    SAL_INFO("extensions.plugin", "SNI: NPN_GetJavaPeer");
    return NULL;
}
#endif

static NPError l_NPN_GetURL( NPP instance, const char* url, const char* window )
{
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return NPERR_GENERIC_ERROR;

    MediatorMessage* pMes=
        pConnector->
        Transact( eNPN_GetURL,
                  &nInstance, sizeof( nInstance ),
                  POST_STRING(url),
                  POST_STRING(window),
                  NULL );
    SAL_WARN_IF(!pMes, "extensions.plugin", "geturl: message unanswered");
    if( ! pMes )
        return NPERR_GENERIC_ERROR;

    // returns NPError
    NPError aRet = PluginConnector::GetNPError( pMes );
    SAL_WARN_IF(aRet, "extensions.plugin", "geturl returns " << aRet);
    delete pMes;
    return aRet;
}

static NPError l_NPN_GetURLNotify( NPP instance, const char* url, const char* target,
                            void* notifyData )
{
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return NPERR_GENERIC_ERROR;

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
    NPError aRet = PluginConnector::GetNPError( pMes );
    delete pMes;
    return aRet;
}

static NPError l_NPN_NewStream( NPP instance, NPMIMEType type, const char* target,
                         NPStream** stream )
    // stream is a return value
{
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return NPERR_GENERIC_ERROR;

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
    NPError aRet = PluginConnector::GetNPError( pMes );
    if( ! aRet )
    {
        NPStream* pStream = new NPStream;
        pStream->url = pMes->GetString();
        pStream->end = pMes->GetUINT32();
        pStream->lastmodified = pMes->GetUINT32();
        pStream->ndata = pStream->pdata = pStream->notifyData = nullptr;

        pConnector->getStreamList().push_back( pStream );
        *stream = pStream;
    }

    delete pMes;
    return aRet;
}

static NPError l_NPN_PostURLNotify( NPP instance, const char* url, const char* target, uint32_t len, const char* buf, NPBool file, void* notifyData )
{
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return NPERR_GENERIC_ERROR;

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

    NPError aRet = PluginConnector::GetNPError( pMes );
    delete pMes;
    return aRet;
}

static NPError l_NPN_PostURL( NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file )
{
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return NPERR_GENERIC_ERROR;

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

    NPError aRet = PluginConnector::GetNPError( pMes );
    delete pMes;
    return aRet;
}

static NPError l_NPN_RequestRead( NPStream* stream, NPByteRange* rangeList )
{
    SAL_INFO("extensions.plugin", "pluginapp: NPN_RequestRead");

    NPByteRange* pRange = rangeList;
    sal_uInt32 nRanges = 0;
    while( pRange )
    {
        nRanges++;
        pRange = pRange->next;
    }

    sal_uInt32* pArray = new sal_uInt32[ 2 * nRanges ];
    pRange = rangeList;
    sal_uInt32 n = 0;
    while( pRange )
    {
        pArray[ 2*n    ] = (sal_uInt32)pRange->offset;
        pArray[ 2*n + 1] = (sal_uInt32)pRange->length;
        n++;
        pRange = pRange->next;
    }
    sal_uInt32 nFileID = pConnector->GetStreamID( stream );
    MediatorMessage* pMes = pConnector->
        Transact( eNPN_RequestRead,
                  &nFileID, sizeof( nFileID ),
                  &nRanges, sizeof( nRanges ),
                  pArray, sizeof( sal_uInt32 ) * 2 * nRanges,
                  NULL );

    if( ! pMes )
    {
        delete[] pArray;
        return NPERR_GENERIC_ERROR;
    }

    NPError aRet = PluginConnector::GetNPError( pMes );
    delete [] pArray;
    delete pMes;
    return aRet;
}

static void l_NPN_Status( NPP instance, const char* message )
{
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return;

    pConnector->Send( eNPN_Status,
                      &nInstance, sizeof( nInstance ),
                      POST_STRING( message ),
                      NULL );
}

static const char* l_NPN_UserAgent( NPP instance )
{
    static char* pAgent = nullptr;

    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
    {
        if( instance )
            return "Mozilla 3.0";
        else // e.g. flashplayer calls NPN_UserAgent with NULL
            nInstance = 0;
    }

    MediatorMessage* pMes = pConnector->
        Transact( eNPN_UserAgent,
                  &nInstance, sizeof( nInstance ),
                  NULL );

    if( ! pMes )
        return pAgent;

    if( pAgent )
        delete [] pAgent;
    pAgent = pMes->GetString();

    delete pMes;

    SAL_INFO("extensions.plugin", "NPN_UserAgent returns " << pAgent);

    return pAgent;
}

static int32_t l_NPN_Write( NPP instance, NPStream* stream, int32_t len, void* buffer )
{
    sal_uInt32 nFileID = pConnector->GetStreamID( stream );
    if( nFileID == PluginConnector::UnknownStreamID )
        return NPERR_GENERIC_ERROR;
    sal_uInt32 nInstance = pConnector->GetNPPID( instance );
    if( nInstance == PluginConnector::UnknownNPPID )
        return NPERR_GENERIC_ERROR;

    MediatorMessage* pMes = pConnector->
        Transact( eNPN_Write,
                  &nInstance, sizeof( nInstance ),
                  &nFileID, sizeof( nFileID ),
                  &len, sizeof( len ),
                  buffer, len,
                  NULL );

    if( ! pMes )
        return 0;

    sal_Int32 nRet = pMes->GetUINT32();
    return nRet;
}

static void l_NPN_ReloadPlugins( NPBool /*reloadPages*/ )
{
    SAL_INFO("extensions.plugin", "NPN_ReloadPlugins: SNI");
}

static NPError l_NPN_GetValue( NPP, NPNVariable variable, void* value )
{
    /*
     * We want to handle values injected into a NPNVariable which aren't in
     * the old enum we build against, but that we know are in the new enum
     * we want to support
         */
    switch( (int)variable )
    {
        case NPNVxDisplay:
            *static_cast<Display**>(value) = pXtAppDisplay;
            SAL_INFO("extensions.plugin", "Display requested");
            break;
        case NPNVxtAppContext:
            *static_cast<XtAppContext*>(value) = app_context;
            SAL_INFO("extensions.plugin", "AppContext requested");
            break;
        case NPNVjavascriptEnabledBool:
            // no javascript
            *static_cast<NPBool*>(value) = false;
            SAL_INFO("extensions.plugin", "javascript enabled requested");
            break;
        case NPNVasdEnabledBool:
            // no SmartUpdate
            *static_cast<NPBool*>(value) = false;
            SAL_INFO("extensions.plugin", "smart update enabled requested");
            break;
        case NPNVisOfflineBool:
            // no offline browsing
            *static_cast<NPBool*>(value) = false;
            SAL_INFO("extensions.plugin", "offline browsing requested");
            break;
        case NPNVSupportsXEmbedBool:
            // asking xembed
            *static_cast<int*>(value) = int(true);
            SAL_INFO("extensions.plugin", "xembed requested");
            break;
        case NPNVToolkit:
#           if ENABLE_GTK
            *static_cast<int*>(value) = NPNVGtk2;
#           else
            *(int*)value = 0;
#           endif
            SAL_INFO("extensions.plugin", "toolkit requested");
            break;
        default:
            SAL_WARN(
                "extensions.plugin",
                "unknown NPNVariable " << +variable << " requested");
            return NPERR_INVALID_PARAM;
    }
    return NPERR_NO_ERROR;
}

static NPError l_NPN_SetValue(NPP /*instance*/, NPPVariable variable, void *value)
{
    SAL_INFO("extensions.plugin", "NPN_SetValue " << +variable << "=" << value);
    return 0;
}

static void l_NPN_InvalidateRect(NPP /*instance*/, NPRect* /*invalidRect*/)
{
    SAL_INFO("extensions.plugin", "NPN_InvalidateRect");
}

static void l_NPN_InvalidateRegion(NPP /*instance*/, NPRegion /*invalidRegion*/)
{
    SAL_INFO("extensions.plugin", "NPN_InvalidateRegion");
}

static void l_NPN_ForceRedraw(NPP /*instance*/)
{
    SAL_INFO("extensions.plugin", "NPN_ForceRedraw");
}

}

static NPNetscapeFuncs aNetscapeFuncs =
{
    sizeof(aNetscapeFuncs),
    (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR,
    l_NPN_GetURL,
    l_NPN_PostURL,
    l_NPN_RequestRead,
    l_NPN_NewStream,
    l_NPN_Write,
    l_NPN_DestroyStream,
    l_NPN_Status,
    l_NPN_UserAgent,
    l_NPN_MemAlloc,
    l_NPN_MemFree,
    l_NPN_MemFlush,
    l_NPN_ReloadPlugins,
#   ifdef OJI
    l_NPN_GetJavaEnv,
    l_NPN_GetJavaPeer,
#   else
    nullptr,
    nullptr,
#   endif
    l_NPN_GetURLNotify,
    l_NPN_PostURLNotify,
    l_NPN_GetValue,
    l_NPN_SetValue,
    l_NPN_InvalidateRect,
    l_NPN_InvalidateRegion,
    l_NPN_ForceRedraw
};

static NPPluginFuncs aPluginFuncs =
{
    sizeof(aPluginFuncs),
    (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};


oslModule pPluginLib = nullptr;
char*(*pNPP_GetMIMEDescription)()                               = nullptr;
NPError (*pNP_Initialize)(NPNetscapeFuncs*,NPPluginFuncs*)      = nullptr;
NPError (*pNP_Shutdown)()                                       = nullptr;

std::vector< PluginConnector* > PluginConnector::allConnectors;

PluginConnector::PluginConnector( int nSocket ) :
        Mediator( nSocket )
{
    SetNewMessageHdl( LINK( this, PluginConnector, NewMessageHdl ) );
}

PluginConnector::~PluginConnector()
{
}

IMPL_LINK_NOARG_TYPED( PluginConnector, WorkOnNewMessageHdl, void*, void )
{
    MediatorMessage* pMessage;
    CommandAtoms nCommand;
    while( (pMessage = GetNextMessage()) )
    {
        nCommand = (CommandAtoms)pMessage->GetUINT32();
        SAL_INFO(
            "extensions.plugin", "pluginapp: " << GetCommandName(nCommand));
        switch( nCommand )
        {
            case eNPP_DestroyStream:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                NPP instance        = m_aInstances[ nInstance ]->instance;
                sal_uInt32 nFileID      = pMessage->GetUINT32();
                NPStream* pStream   = m_aNPWrapStreams[ nFileID ];
                NPError aReason     = GetNPError( pMessage );
                m_aNPWrapStreams.erase( m_aNPWrapStreams.begin() + nFileID );

                aReason = aPluginFuncs.destroystream( instance, pStream, aReason );
                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&aReason), sizeof( aReason ),
                         NULL );

                delete [] pStream->url;
                delete pStream;
            }
            break;
            case eNPP_Destroy:
            {
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                ConnectorInstance* pInst= m_aInstances[ nInstance ];

                // some plugin rely on old netscapes behaviour
                // to first destroy the widget and then destroy
                // the instance, so mimic that behaviour here
                if( pInst->pShell )
                    XtDestroyWidget( static_cast<Widget>(pInst->pShell) );

                pInst->pWidget = pInst->pShell = nullptr;

                // the other side will call eNPP_DestroyPhase2 after this
                NPError aReason = NPERR_NO_ERROR;
                Respond( pMessage->m_nID, reinterpret_cast<char*>(&aReason), sizeof( aReason ), NULL );
            }
            break;
            case eNPP_DestroyPhase2:
            {
                // now really destroy the instance
                sal_uInt32 nInstance    = pMessage->GetUINT32();
                ConnectorInstance* pInst= m_aInstances[ nInstance ];
                NPP instance        = pInst->instance;
                NPSavedData* pSave = nullptr;

                NPError aRet = aPluginFuncs.destroy( instance, &pSave );
                if( pSave )
                {
                    Respond( pMessage->m_nID,
                             reinterpret_cast<char*>(&aRet), sizeof( aRet ),
                             pSave->buf, pSave->len,
                             NULL );
                    delete [] static_cast<char*>(pSave->buf);
                }
                else
                    Respond( pMessage->m_nID,
                             reinterpret_cast<char*>(&aRet), sizeof( aRet ),
                             "0000", 4,
                             NULL );

                #if ENABLE_GTK
                if( pInst->pGtkWindow )
                    g_object_unref( G_OBJECT(pInst->pGtkWindow) );
                if( pInst->pGtkWidget )
                    g_object_unref( G_OBJECT(pInst->pGtkWidget) );
                #endif

                m_aInstances.erase( m_aInstances.begin() + nInstance );
                delete pInst;
                delete instance;
                SAL_INFO(
                    "extensions.plugin",
                    "destroyed instance (returning " << aRet << ")");
            }
            break;
            case eNPP_NewStream:
            {
                sal_uInt32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances[ nInstance ]->instance;
                char* pType             = pMessage->GetString();
                NPStream* pStream       = new NPStream;
                pStream->url            = pMessage->GetString();
                pStream->end            = pMessage->GetUINT32();
                pStream->lastmodified   = pMessage->GetUINT32();
                pStream->pdata = pStream->ndata = pStream->notifyData = nullptr;
                NPBool* pSeekable       = static_cast<NPBool*>(pMessage->GetBytes());
                m_aNPWrapStreams.push_back( pStream );
                uint16_t nStype = NP_ASFILE;
                NPError aRet = aPluginFuncs.newstream( instance, pType, pStream,
                                                       *pSeekable, &nStype );
                SAL_INFO(
                    "extensions.plugin",
                    "pluginapp: NPP_NewStream(" << instance << ", " << pType
                        << ", " << pStream << ", "
                        << (*pSeekable ? "seekable" : "not seekable") << ", "
                        << &nStype << ") returns " << aRet
                        << "; stream = { pdata = " << pStream->pdata
                        << ", ndata = " << pStream->ndata << ", url = "
                        << pStream->url << ", end = " << pStream->end
                        << ", lastmodified = " << pStream->lastmodified
                        << ", notifyData = " << pStream->notifyData << " }");
                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&aRet), sizeof( aRet ),
                         &nStype, sizeof( nStype ),
                         NULL );
                delete [] pType;
                delete [] pSeekable;
            }
            break;
            case eNPP_New:
            {
                char* pType     = pMessage->GetString();
                uint16_t* pMode   = static_cast<uint16_t*>(pMessage->GetBytes());
                int16_t*  pArgc   = static_cast<int16_t*>(pMessage->GetBytes());
                NPP instance    = new NPP_t;
                instance->pdata = instance->ndata = nullptr;
                sal_uLong nArgnBytes, nArgvBytes;
                char* pArgn = static_cast<char*>(pMessage->GetBytes( nArgnBytes ));
                char* pArgv = static_cast<char*>(pMessage->GetBytes( nArgvBytes ));
                sal_uLong nSaveBytes;
                char* pSavedData = static_cast<char*>(pMessage->GetBytes( nSaveBytes ));
                ConnectorInstance* pInst =
                    new ConnectorInstance( instance, pType,
                                           *pArgc,
                                           pArgn, nArgnBytes,
                                           pArgv, nArgvBytes,
                                           pSavedData, nSaveBytes );
                m_aInstances.push_back( pInst );
                NPError aRet;
                aRet = aPluginFuncs.newp( pInst->pMimeType, instance, *pMode, *pArgc,
                                          pInst->nArg ? pInst->argn : nullptr,
                                          pInst->nArg ? pInst->argv : nullptr,
                                          ( nSaveBytes == 4 && *reinterpret_cast<sal_uInt32*>(pSavedData) == 0 ) ?
                                          &(pInst->aData) : nullptr );
                SAL_INFO(
                    "extensions.plugin",
                    "pluginapp: NPP_New( " << pInst->pMimeType << ", "
                        << instance << ", " << *pMode << ", " << pInst->nArg
                        << ", " << pInst->argn << ", " << pInst->argv << ", "
                        << &pInst->aData << ") returns" << aRet);
                for( int i = 0; i < pInst->nArg; i++ )
                    SAL_INFO(
                        "extensions.plugin",
                        "   \"" << pInst->argn[i] << "\"=\"" << pInst->argv[i]
                            << "\"");

                #if ENABLE_GTK
                // check if XEMBED is to be used
                // ask for Bool. there seems to be no clear definition whether the
                // return value should be an int or unsigned char
                // int can hold both and will be nonzero in case of "true"
                if( aPluginFuncs.getvalue )
                {
                    int bNeedsXEmbed = 0;
                    NPError error = aPluginFuncs.getvalue( instance, NPPVpluginNeedsXEmbed, static_cast<void *>(&bNeedsXEmbed) );
                    if( error == NPERR_NO_ERROR )
                        pInst->bShouldUseXEmbed = (bNeedsXEmbed != 0);
                    SAL_INFO(
                        "extensions.plugin",
                        "should use xembed = "
                            << (pInst->bShouldUseXEmbed ? "true" : "false"));
                }
                #endif

                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&aRet), sizeof( aRet ),
                         NULL );
                delete [] pMode;
                delete [] pArgc;
                delete [] pType;
            }
            break;
            case eNPP_SetWindow:
            {
                sal_uInt32 nInstance        = pMessage->GetUINT32();
                ConnectorInstance* pInst= m_aInstances[ nInstance ];
                NPWindow* pWindow       = static_cast<NPWindow*>(pMessage->GetBytes());

                if( pWindow->width < 1 )
                    pWindow->width = 1;
                if( pWindow->height < 1 )
                    pWindow->height = 1;

                #if ENABLE_GTK
                if( pInst->bShouldUseXEmbed )
                {
                    if( ! pInst->pGtkWidget )
                    {
                        SAL_INFO(
                            "extensions.plugin",
                            "creating gtk plug and socket");

                        pInst->pGtkWindow = gtk_plug_new((GdkNativeWindow)reinterpret_cast<sal_uIntPtr>(pWindow->window));
                        gtk_widget_show( pInst->pGtkWindow );
                        pInst->pGtkWidget = gtk_socket_new();
                        gtk_widget_show( pInst->pGtkWidget );
                        gtk_container_add( GTK_CONTAINER(pInst->pGtkWindow), pInst->pGtkWidget );
                        gtk_widget_show_all( pInst->pGtkWindow );
                        pInst->window.window = reinterpret_cast<void *>(gtk_socket_get_id( GTK_SOCKET(pInst->pGtkWidget ) ));

                        XSync( pAppDisplay, False );

                        XMapWindow( pAppDisplay, GDK_WINDOW_XWINDOW(pInst->pGtkWindow->window) );

                        XSync( pAppDisplay, False );
                    }

                    // update widget size; alas out parent is not yet really XEMBED conformant
                    gtk_widget_set_size_request( pInst->pGtkWidget, pWindow->width, pWindow->height );
                    gtk_window_resize( GTK_WINDOW(pInst->pGtkWindow), pWindow->width, pWindow->height );

                    GdkScreen* pGdkScreen = gtk_widget_get_screen( pInst->pGtkWidget );
                    Screen* pScreen = ScreenOfDisplay( pAppDisplay, gdk_screen_get_number( pGdkScreen ) );

                    pInst->window.x                 = 0;
                    pInst->window.y                 = 0;
                    pInst->window.width             = pWindow->width;
                    pInst->window.height            = pWindow->height;
                    pInst->window.clipRect.left     = 0;
                    pInst->window.clipRect.top      = 0;
                    pInst->window.clipRect.right    = pWindow->width;
                    pInst->window.clipRect.bottom   = pWindow->height;
                    pInst->window.ws_info           = &pInst->ws_info;
                    pInst->window.type              = NPWindowTypeWindow;
                    pInst->ws_info.type             = NP_SETWINDOW;
                    pInst->ws_info.display          = pAppDisplay;
                    pInst->ws_info.visual           = DefaultVisualOfScreen( pScreen );
                    pInst->ws_info.colormap         = DefaultColormapOfScreen( pScreen );
                    pInst->ws_info.depth            = DefaultDepthOfScreen( pScreen );
                }
                else
                #endif
                {
                    if( ! pInst->pWidget )
                    {
                        pInst->pWidget = CreateNewShell( &(pInst->pShell), reinterpret_cast<Window>(pWindow->window) );
                    }

                    // fill in NPWindow and NPCallbackStruct
                    pInst->window.window            = reinterpret_cast<void*>(XtWindow( static_cast<Widget>(pInst->pWidget) ));
                    pInst->window.x                 = 0;
                    pInst->window.y                 = 0;
                    pInst->window.width             = pWindow->width;
                    pInst->window.height            = pWindow->height;
                    pInst->window.clipRect.left     = 0;
                    pInst->window.clipRect.top      = 0;
                    pInst->window.clipRect.right    = pWindow->width;
                    pInst->window.clipRect.bottom   = pWindow->height;
                    pInst->window.ws_info           = &pInst->ws_info;
                    pInst->window.type              = NPWindowTypeWindow;
                    pInst->ws_info.type             = NP_SETWINDOW;
                    pInst->ws_info.display          = XtDisplay( static_cast<Widget>(pInst->pWidget) );
                    pInst->ws_info.visual           = DefaultVisualOfScreen( XtScreen( static_cast<Widget>(pInst->pWidget) ) );
                    pInst->ws_info.colormap         = DefaultColormapOfScreen( XtScreen( static_cast<Widget>(pInst->pWidget) ) );
                    pInst->ws_info.depth            = DefaultDepthOfScreen( XtScreen( static_cast<Widget>(pInst->pWidget) ) );

                    XtResizeWidget( static_cast<Widget>(pInst->pShell),
                                    pInst->window.width,
                                    pInst->window.height,
                                    0 );
                    XtResizeWidget( static_cast<Widget>(pInst->pWidget),
                                    pInst->window.width,
                                    pInst->window.height,
                                    0 );
                }

                NPError aRet = aPluginFuncs.setwindow( pInst->instance, &pInst->window );
                SAL_INFO(
                    "extensions.plugin",
                    "pluginapp: NPP_SetWindow returns " << aRet);
                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&aRet), sizeof( aRet ),
                         NULL );
                delete [] reinterpret_cast<char*>(pWindow);
            }
            break;
            case eNPP_StreamAsFile:
            {
                sal_uInt32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances[ nInstance ]->instance;
                sal_uInt32 nFileID          = pMessage->GetUINT32();
                NPStream* pStream       = m_aNPWrapStreams[ nFileID ];
                char* fname             = pMessage->GetString();
                SAL_INFO(
                    "extensions.plugin",
                    "pluginapp: NPP_StreamAsFile " << fname);
                aPluginFuncs.asfile( instance, pStream, fname );
                delete [] fname;
            }
            break;
            case eNPP_URLNotify:
            {
                sal_uInt32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances[ nInstance ]->instance;
                char* url               = pMessage->GetString();
                NPReason* pReason       = static_cast<NPReason*>(pMessage->GetBytes());
                void** notifyData       = static_cast<void**>(pMessage->GetBytes());
                aPluginFuncs.urlnotify( instance, url, *pReason, *notifyData );
                delete [] url;
                delete [] pReason;
                delete [] notifyData;
            }
            break;
            case eNPP_WriteReady:
            {
                sal_uInt32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances[ nInstance ]->instance;
                sal_uInt32 nFileID          = pMessage->GetUINT32();
                NPStream* pStream       = m_aNPWrapStreams[ nFileID ];
                int32_t nRet = aPluginFuncs.writeready( instance, pStream );

                SAL_INFO(
                    "extensions.plugin",
                    "pluginapp: NPP_WriteReady(" << instance << ", " << pStream
                        << ") (stream id = " << nFileID << ") returns "
                        << nRet);

                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&nRet), sizeof( nRet ),
                         NULL );
            }
            break;
            case eNPP_Write:
            {
                sal_uInt32 nInstance        = pMessage->GetUINT32();
                NPP instance            = m_aInstances[ nInstance ]->instance;
                sal_uInt32 nFileID          = pMessage->GetUINT32();
                NPStream* pStream       = m_aNPWrapStreams[ nFileID ];
                int32_t offset            = pMessage->GetUINT32();
                sal_uLong len;
                char* buffer            = static_cast<char*>(pMessage->GetBytes( len ));
                int32_t nRet = aPluginFuncs.write( instance, pStream, offset, len, buffer );

                SAL_INFO(
                    "extensions.plugin",
                    "pluginapp: NPP_Write(" << instance << ", " << pStream
                        << ", " << offset << ", " << len << ", " << buffer
                        << ") returns " << nRet << "; stream = { pdata = "
                        << pStream->pdata << ", ndata = " << pStream->ndata
                        << ", url = " << pStream->url << ", end = "
                        << pStream->end << ", lastmodified = "
                        << pStream->lastmodified << ", notifyData = "
                        << pStream->notifyData << " }");

                Respond( pMessage->m_nID,
                         reinterpret_cast<char*>(&nRet), sizeof( nRet ),
                         NULL );
                delete [] buffer;
            }
            break;
            case eNPP_GetMIMEDescription:
            {
                if( ! pNPP_GetMIMEDescription )
                    pNPP_GetMIMEDescription = reinterpret_cast<char*(*)()>(
                        osl_getAsciiFunctionSymbol( pPluginLib, "NPP_GetMIMEDescription" ));
                char* pMIME = pNPP_GetMIMEDescription();
                Respond( pMessage->m_nID,
                         POST_NONCONST_STRING( pMIME ),
                         NULL );
            }
            break;
            case eNPP_Initialize:
            {

                pNP_Initialize =
                    reinterpret_cast<NPError(*)(NPNetscapeFuncs*, NPPluginFuncs*)>(
                    osl_getAsciiFunctionSymbol( pPluginLib, "NP_Initialize" ));
                SAL_WARN_IF(
                    !pNP_Initialize, "extensions.plugin",
                    "no NP_Initialize, " << dlerror());
                pNP_Shutdown = reinterpret_cast<NPError(*)()>(
                    osl_getAsciiFunctionSymbol( pPluginLib, "NP_Shutdown" ));
                SAL_WARN_IF(
                    !pNP_Initialize, "extensions.plugin",
                    "no NP_Shutdown, " << dlerror());

                SAL_INFO("extensions.plugin", "entering NP_Initialize");
                NPError aRet = pNP_Initialize( &aNetscapeFuncs, &aPluginFuncs );
                SAL_INFO(
                    "extensions.plugin",
                    "pluginapp: NP_Initialize returns " << aRet);
                Respond( pMessage->m_nID, reinterpret_cast<char*>(&aRet), sizeof( aRet ), NULL );
            }
            break;
            case eNPP_Shutdown:
            {
                bool bSuccess = (4 == write(wakeup_fd[1], "xxxx", 4));
                SAL_WARN_IF(!bSuccess, "extensions.plugin", "short write");
            }
            break;
            default:
                SAL_WARN(
                    "extensions.plugin",
                    "caught unknown NPP request " << +nCommand);
                break;
        }
        delete pMessage;
    }
}

void LoadAdditionalLibs( const char* _pPluginLib )
{
    SAL_INFO("extensions.plugin", "LoadAdditionalLibs " << _pPluginLib);

    if( ! strncmp( _pPluginLib, "libflashplayer.so", 17 ) )
    {
        /*  #b4951312# flash 7 implicitly assumes a gtk application
         *  if the API version is greater or equal to 12 (probably
         *  because they think they run in mozilla then). In that
         *  case they try to find gtk within the process and crash
         *  when they don't find it.
         */
        aNetscapeFuncs.version = 11;
        aPluginFuncs.version = 11;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
