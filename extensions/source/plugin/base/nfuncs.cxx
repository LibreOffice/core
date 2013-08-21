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

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#undef OPTIONAL
#endif

#include <cstdarg>
#include <list>

#include <plugin/impl.hxx>
#include <vcl/svapp.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <osl/thread.h>
#include <osl/thread.hxx>
#include <stdio.h>
static FILE * s_file = 0;
void TRACE( char const * s )
{
    if (! s_file)
        s_file = stderr;
    if (s_file)
    {
        oslThreadIdentifier t = osl::Thread::getCurrentIdentifier();
        fprintf( s_file, "log [t_id=%" SAL_PRIuUINT32 "]: %s\n", t, s );
        fflush( s_file );
    }
}
void TRACEN( char const * s, long n )
{
    if (! s_file)
        s_file = stderr;
    if (s_file)
    {
        oslThreadIdentifier t = osl::Thread::getCurrentIdentifier();
        fprintf( s_file, "log [t_id=%" SAL_PRIuUINT32 "]: %s%ld\n", t, s, n );
        fflush( s_file );
    }
}
void TRACES( char const* s, char const* s2 )
{
    if (! s_file)
        s_file = stderr;
    if (s_file)
    {
        oslThreadIdentifier t = osl::Thread::getCurrentIdentifier();
        fprintf( s_file, "log [t_id=%" SAL_PRIuUINT32 "]: %s %s\n", t, s, s2 );
        fflush( s_file );
    }
}
#else
#define TRACE(x)
#define TRACEN(x,n)
#define TRACES(x,s)
#endif

using namespace com::sun::star::lang;


// Move deprecated functions which no longer appear in npapi.h before
// their use to avoid errors that they're undeclared at point of use
extern "C"
{
    const JRIEnvInterface** SAL_CALL NP_LOADDS  NPN_GetJavaEnv()
    {
        TRACE( "NPN_GetJavaEnv" );
        // no java in this program
        return NULL;
    }

    jref SAL_CALL NP_LOADDS  NPN_GetJavaPeer( NPP /*instance*/ )
    {
        TRACE( "NPN_GetJavaPeer" );
        return NULL;
    }
}

NPNetscapeFuncs aNPNFuncs =
{
    sizeof( NPNetscapeFuncs ),
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

static OString normalizeURL( XPlugin_Impl* plugin, const OString& url )
{
    OString aLoadURL;
    if( url.indexOf( ':' ) == -1 )
    {
        aLoadURL = OUStringToOString( plugin->getCreationURL(), plugin->getTextEncoding() );
        int nPos;
        if( ( nPos = aLoadURL.indexOf( "://" ) ) != -1 )
        {
            if( !url.isEmpty() && (url.getStr()[ 0 ] == '/' || url.indexOf( '/' ) != -1) )
            {
                // this means same server but new path
                nPos = aLoadURL.indexOf( '/', nPos+3 );

                if( nPos != -1 )
                    aLoadURL = aLoadURL.copy( 0, url.getStr()[0] == '/' ? nPos : nPos+1 );
            }
            else
            {
                // same server but new file
                nPos = aLoadURL.lastIndexOf( '/' );
                aLoadURL = aLoadURL.copy( 0, nPos+1 );
            }
            aLoadURL += url;
        }
        else
            aLoadURL = url;
    }
    else if( url.indexOf( ":/" ) != -1 )
        aLoadURL = url;

    return aLoadURL;
}

struct AsynchronousGetURL
{
    OUString                        aUrl;
    OUString                        aTarget;
    Reference< XEventListener >     xListener;

    DECL_LINK( getURL, XPlugin_Impl* );
};

IMPL_LINK( AsynchronousGetURL, getURL, XPlugin_Impl*, pImpl )
{
    try
    {
        pImpl->enterPluginCallback();
        if( xListener.is() )
            pImpl->getPluginContext()->
                getURLNotify( pImpl,
                              aUrl,
                              aTarget,
                              xListener );
        else
            pImpl->getPluginContext()->
                getURL( pImpl,
                        aUrl,
                        aTarget );
    }
    catch(const ::com::sun::star::plugin::PluginException&)
    {
    }
    pImpl->leavePluginCallback();
    delete this;
    return 0;
}


extern "C" {

    void* SAL_CALL NP_LOADDS  NPN_MemAlloc( uint32_t nBytes )
    {
        TRACE( "NPN_MemAlloc" );
        void* pMem = malloc( nBytes );
        return pMem;
    }

    void SAL_CALL NP_LOADDS  NPN_MemFree( void* pMem )
    {
        TRACE( "NPN_MemFree" );
        free( pMem );
    }

    uint32_t SAL_CALL NP_LOADDS  NPN_MemFlush( uint32_t /*nSize*/ )
    {
        TRACE( "NPN_MemFlush" );
        return 0;
    }

    NPError SAL_CALL NP_LOADDS  NPN_DestroyStream( NPP instance, NPStream* stream, NPError /*reason*/ )
    {
        TRACE( "NPN_DestroyStream" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        PluginStream* pStream = pImpl->getStreamFromNPStream( stream );
        if( pStream )
        {
            if( pStream->getStreamType() == InputStream )
                static_cast<PluginInputStream*>(pStream)->releaseSelf();
            else
                delete pStream;
        }

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_GetURL( NPP instance, const char* url, const char* window )
    {
        TRACES( "NPN_GetURL", url );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        AsynchronousGetURL* pAsync = new AsynchronousGetURL();

        OString aLoadURL = normalizeURL( pImpl, url );
        TRACES( "NPN_GetURL", aLoadURL.getStr() );
        pAsync->aUrl = OStringToOUString( aLoadURL, pImpl->getTextEncoding() );
        pAsync->aTarget = OStringToOUString( window, pImpl->getTextEncoding() );
        pImpl->setLastGetUrl( aLoadURL );
        Application::PostUserEvent( LINK( pAsync, AsynchronousGetURL, getURL ), pImpl );
        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_GetURLNotify( NPP instance, const char* url, const char* target,
                          void* notifyData )
    {
        TRACES( "NPN_GetURLNotify", url );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        OString aLoadURL = normalizeURL( pImpl, url );
        if( aLoadURL.isEmpty() )
            return NPERR_INVALID_URL;

        AsynchronousGetURL* pAsync = new AsynchronousGetURL();
        PluginEventListener* pListener =
            new PluginEventListener( pImpl, url, aLoadURL.getStr(), notifyData );
        if( ! target || ! *target )
        {
            // stream will be fed back to plugin,
            // notify immediately after destruction of stream
            pImpl->addPluginEventListener( pListener );
            pListener = NULL;
        }
        pAsync->aUrl        = OStringToOUString( aLoadURL, pImpl->getTextEncoding() );
        pAsync->aTarget     = OStringToOUString( target, pImpl->getTextEncoding() );
        pAsync->xListener   = pListener;
        pImpl->setLastGetUrl( aLoadURL );
        Application::PostUserEvent( LINK( pAsync, AsynchronousGetURL, getURL ), pImpl );

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_NewStream( NPP instance, NPMIMEType type, const char* target,
                           NPStream** stream )
        // stream is a return value
    {
        TRACE( "NPN_NewStream" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        PluginOutputStream* pStream = new PluginOutputStream( pImpl,
                                                              "", 0, 0 );
        *stream = pStream->getStream();

        try
        {
            pImpl->enterPluginCallback();
            pImpl->getPluginContext()->
                newStream(
                    pImpl,
                    OStringToOUString( type, pImpl->getTextEncoding () ),
                    OStringToOUString( target, pImpl->getTextEncoding() ),
                    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource > ( pStream->getOutputStream(), UNO_QUERY )
                    );
            pImpl->leavePluginCallback();
        }
        catch( const ::com::sun::star::plugin::PluginException& e )
        {
            pImpl->leavePluginCallback();
            return e.ErrorCode;
        }

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_PostURLNotify( NPP instance, const char* url, const char* target, uint32_t len, const char* buf, NPBool file, void* notifyData )
    {
        TRACE( "NPN_PostURLNotify" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        ::com::sun::star::uno::Sequence<sal_Int8> Bytes( (sal_Int8*)buf, len );

        OString aPostURL = normalizeURL( pImpl, url );
        PluginEventListener* pListener =
            new PluginEventListener( pImpl, url, aPostURL.getStr(), notifyData );

        if( ! target || ! *target )
        {
            // stream will be fed back to plugin,
            // notify immediately after destruction of stream
            pImpl->addPluginEventListener( pListener );
            pListener = NULL;
        }

        try
        {
            pImpl->enterPluginCallback();
            pImpl->getPluginContext()->
                postURLNotify( pImpl,
                               OStringToOUString( aPostURL, pImpl->getTextEncoding() ),
                               OStringToOUString( target, pImpl->getTextEncoding() ),
                               Bytes,
                               file,
                               ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > ( pListener ) );
            pImpl->leavePluginCallback();
        }
        catch( const ::com::sun::star::plugin::PluginException& e )
        {
            pImpl->leavePluginCallback();
            return e.ErrorCode;
        }

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_PostURL( NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file )
    {
        TRACE( "NPN_PostURL" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        ::com::sun::star::uno::Sequence<sal_Int8> Bytes( (sal_Int8*)buf, len );
        OString aPostURL = normalizeURL( pImpl, url );
        try
        {
            pImpl->enterPluginCallback();
            pImpl->getPluginContext()->
                postURL( pImpl,
                         OStringToOUString( aPostURL, pImpl->getTextEncoding() ),
                         OStringToOUString( window, pImpl->getTextEncoding () ),
                         Bytes,
                         file );
            pImpl->leavePluginCallback();
        }
        catch( const ::com::sun::star::plugin::PluginException& e )
        {
            pImpl->leavePluginCallback();
            return e.ErrorCode;
        }

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_RequestRead( NPStream* stream, NPByteRange* rangeList )
    {
        TRACE( "NPN_RequestRead" );
        if( ! rangeList )
            return NPERR_NO_ERROR;

        ::std::list<XPlugin_Impl*>& rList = PluginManager::get().getPlugins();
        ::std::list<XPlugin_Impl*>::iterator iter;
        XPlugin_Impl* pPlugin = NULL;
        PluginStream* pStream = NULL;
        for( iter = rList.begin(); iter!= rList.end(); ++iter )
        {
            pStream = (*iter)->getStreamFromNPStream( stream );
            if( pStream )
            {
                pPlugin = *iter;
                break;
            }
        }
        if( ! pPlugin )
            return NPERR_INVALID_INSTANCE_ERROR;
        if( ! pStream || pStream->getStreamType() != InputStream )
            return NPERR_FILE_NOT_FOUND;

        PluginInputStream* pInputStream = (PluginInputStream*)pStream;
        sal_Int8* pBytes = NULL;
        int   nBytes = 0;
        pPlugin->enterPluginCallback();
        while( rangeList )
        {
            if( pBytes && nBytes < (int)rangeList->length )
            {
                delete [] pBytes;
                pBytes = NULL;
            }
            if( ! pBytes )
                pBytes = new sal_Int8[ nBytes = rangeList->length ];
            int nRead =
                pInputStream->read( rangeList->offset, pBytes, rangeList->length );
            int nPos = 0;
            int nNow;
            do
            {
                nNow = pPlugin->getPluginComm()->
                    NPP_WriteReady( pPlugin->getNPPInstance(),
                                    stream );
                nNow = pPlugin->getPluginComm()->
                    NPP_Write( pPlugin->getNPPInstance(),
                               stream,
                               rangeList->offset + nPos,
                               nNow,
                               pBytes+nPos );
                nPos += nNow;
                nRead -= nNow;
            } while( nRead > 0 && nNow );
            rangeList = rangeList->next;
        }
        pPlugin->leavePluginCallback();

        return NPERR_NO_ERROR;
    }

    void SAL_CALL NP_LOADDS  NPN_Status( NPP instance, const char* message )
    {
        TRACE( "NPN_Status" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return;

        try
        {
            pImpl->enterPluginCallback();
            pImpl->getPluginContext()->
                displayStatusText( pImpl, OStringToOUString( message, pImpl->getTextEncoding() ) );
            pImpl->leavePluginCallback();
        }
        catch( const ::com::sun::star::plugin::PluginException& )
        {
            pImpl->leavePluginCallback();
            return;
        }
    }

    const char* SAL_CALL NP_LOADDS  NPN_UserAgent( NPP instance )
    {
        static char* pAgent = strdup( "Mozilla 3.0" );

        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( pImpl )
        {
            OUString UserAgent;
            try
            {
                pImpl->enterPluginCallback();
                UserAgent = pImpl->getPluginContext()->
                    getUserAgent( pImpl );
                pImpl->leavePluginCallback();
                if( pAgent )
                    free( pAgent );
                pAgent = strdup( OUStringToOString( UserAgent, pImpl->getTextEncoding() ).getStr() );
            }
            catch( const ::com::sun::star::plugin::PluginException& )
            {
                pImpl->leavePluginCallback();
            }
        }

        TRACES( "NPN_UserAgent: returning", pAgent );

        return pAgent;
    }

void SAL_CALL NP_LOADDS  NPN_Version( int* major, int* minor, int* net_major, int* net_minor )
{
    TRACE( "NPN_Version" );
    *major = 4;
    *minor = 0;
    *net_major = 4;
    *net_minor = 5;
}

int32_t SAL_CALL NP_LOADDS  NPN_Write( NPP instance, NPStream* stream, int32_t len,
                 void* buffer )
{
    TRACE( "NPN_Write" );
    XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
    if( ! pImpl )
        return 0;

    PluginStream* pStream = pImpl->getStreamFromNPStream( stream );
    if( ! pStream || pStream->getStreamType() != OutputStream )
        return 0;

    pImpl->enterPluginCallback();
    ::com::sun::star::uno::Sequence<sal_Int8> Bytes( (sal_Int8*)buffer, len );
    ((PluginOutputStream*)pStream)->getOutputStream()->writeBytes( Bytes );
    pImpl->leavePluginCallback();

    return len;
}


NPError SAL_CALL NP_LOADDS  NPN_GetValue( NPP instance, NPNVariable variable, void* value )
{
    TRACEN( "NPN_GetValue: ", variable );
    XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );

    if( ! pImpl )
        return 0;

    NPError aResult( NPERR_NO_ERROR );

    switch( variable )
    {
        case NPNVxDisplay:
            // Unix only, handled in sysdep part
        case NPNVxtAppContext:
            // Unix only, handled in sysdep part
         default:
            aResult = NPERR_INVALID_PARAM;
            break;
        #ifdef MACOSX
        case 2000: // NPNVsupportsQuickDrawBool
            *(NPBool*)value = false;
            break;
        case 2001: // NPNVsupportsCoreGraphicsBool
            *(NPBool*)value = true;
            break;
        #endif
        case NPNVjavascriptEnabledBool:
            // no javascript
            *(NPBool*)value = false;
            break;
        case NPNVasdEnabledBool:
            // no SmartUpdate
            *(NPBool*)value = false;
            break;
        case NPNVisOfflineBool:
            // no offline browsing
            *(NPBool*)value = false;
            break;
    }

    return aResult;
}

void SAL_CALL NP_LOADDS  NPN_ReloadPlugins(NPBool /*reloadPages*/)
{
    TRACE( "NPN_ReloadPlugins" );
}


NPError SAL_CALL NP_LOADDS  NPN_SetValue( NPP instance,
                                          NPPVariable variable,
                                          void* value )
{
    NPError nError = NPERR_NO_ERROR;
    TRACEN( "NPN_SetValue ", variable );
    #ifdef MACOSX
    NPN_SetValue_Impl(instance, variable, value);
    #else
    (void)instance;
    (void)variable;
    (void)value;
    #endif
    return nError;
}

void SAL_CALL NP_LOADDS  NPN_InvalidateRect(NPP instance, NPRect* /*invalidRect*/)
{
    TRACE( "NPN_InvalidateRect" );

    #ifdef MACOSX
    NPN_ForceRedraw( instance );
    #else
    (void)instance;
    #endif
}

void SAL_CALL NP_LOADDS  NPN_InvalidateRegion(NPP instance, NPRegion /*invalidRegion*/)
{
    TRACE( "NPN_InvalidateRegion" );

    #ifdef MACOSX
    NPN_ForceRedraw( instance );
    #else
    (void)instance;
    #endif
}

void SAL_CALL NP_LOADDS  NPN_ForceRedraw(NPP instance)
{
    TRACE( "NPN_ForceRedraw" );
    #ifdef MACOSX
    NPN_ForceRedraw_Impl(instance);
    #else
    (void)instance;
    #endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
