/*************************************************************************
 *
 *  $RCSfile: nfuncs.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
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

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <stl/list>

#ifdef USE_NAMESPACE
using namespace std;
#endif


#include <plugin/impl.hxx>

//  #define TRACE(x) DBG_ERROR( (x) )
#define TRACE(x)


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

static ::rtl::OString normalizeURL( XPlugin_Impl* plugin, const ::rtl::OString& url )
{
    ::rtl::OString aLoadURL;
    if( url.indexOf( ":/" ) == -1 )
    {
        aLoadURL = ::rtl::OUStringToOString( plugin->getCreationURL(), plugin->getTextEncoding() );
        int nPos;
        if( ( nPos = aLoadURL.indexOf( "://" ) ) != -1 )
        {
            if( url.getLength() && url.getStr()[ 0 ] == '/' || url.indexOf( '/' ) != -1 )
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
    else
        aLoadURL = url;

    return aLoadURL;
}


extern "C" {

    void* SAL_CALL NP_LOADDS  NPN_MemAlloc( uint32 nBytes )
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

    uint32 SAL_CALL NP_LOADDS  NPN_MemFlush( uint32 nSize )
    {
        TRACE( "NPN_MemFlush" );
        return 0;
    }

    NPError SAL_CALL NP_LOADDS  NPN_DestroyStream( NPP instance, NPStream* stream, NPError reason )
    {
        TRACE( "NPN_DestroyStream" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        PluginStream* pStream = pImpl->getStreamFromNPStream( stream );
        if( pStream )
            delete pStream;

        return NPERR_NO_ERROR;
    }

    const JRIEnvInterface** SAL_CALL NP_LOADDS  NPN_GetJavaEnv()
    {
        TRACE( "NPN_GetJavaEnv" );
        // no java in this program
        return NULL;
    }

    void* SAL_CALL NP_LOADDS  NPN_GetJavaPeer( NPP instance )
    {
        TRACE( "NPN_GetJavaPeer" );
        return NULL;
    }

    NPError SAL_CALL NP_LOADDS  NPN_GetURL( NPP instance, const char* url, const char* window )
    {
        TRACE( "NPN_GetURL" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        ::rtl::OString aLoadURL = normalizeURL( pImpl, url );
        try
        {
            pImpl->enterPluginCallback();
            pImpl->getPluginContext()->
                getURL( pImpl,
                        ::rtl::OStringToOUString( aLoadURL, pImpl->getTextEncoding() ),
                        ::rtl::OStringToOUString( window, pImpl->getTextEncoding() )
                        );
            pImpl->leavePluginCallback();
        }
        catch( ::com::sun::star::plugin::PluginException& e )
        {
            pImpl->leavePluginCallback();
            return e.ErrorCode;
        }

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_GetURLNotify( NPP instance, const char* url, const char* target,
                          void* notifyData )
    {
        TRACE( "NPN_GetURLNotify" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        ::rtl::OString aLoadURL = normalizeURL( pImpl, url );
        PluginEventListener* pListener =
            new PluginEventListener( pImpl, url, aLoadURL.getStr(), notifyData );
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
                getURLNotify( pImpl,
                              ::rtl::OStringToOUString( aLoadURL, pImpl->getTextEncoding() ),
                              ::rtl::OStringToOUString( target, pImpl->getTextEncoding() ),
                              ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > ( pListener ) );
            pImpl->leavePluginCallback();
        }
        catch( ::com::sun::star::plugin::PluginException& e )
        {
            pImpl->leavePluginCallback();
            return e.ErrorCode;
        }

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
                    ::rtl::OStringToOUString( type, pImpl->getTextEncoding () ),
                    ::rtl::OStringToOUString( target, pImpl->getTextEncoding() ),
                    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource > ( pStream->getOutputStream(), UNO_QUERY )
                    );
            pImpl->leavePluginCallback();
        }
        catch( ::com::sun::star::plugin::PluginException& e )
        {
            pImpl->leavePluginCallback();
            return e.ErrorCode;
        }

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_PostURLNotify( NPP instance, const char* url, const char* target, uint32 len, const char* buf, NPBool file, void* notifyData )
    {
        TRACE( "NPN_PostURLNotify" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        ::com::sun::star::uno::Sequence<sal_Int8> Bytes( (sal_Int8*)buf, len );

        ::rtl::OString aPostURL = normalizeURL( pImpl, url );
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
                               ::rtl::OStringToOUString( aPostURL, pImpl->getTextEncoding() ),
                               ::rtl::OStringToOUString( target, pImpl->getTextEncoding() ),
                               Bytes,
                               file,
                               ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > ( pListener ) );
            pImpl->leavePluginCallback();
        }
        catch( ::com::sun::star::plugin::PluginException& e )
        {
            pImpl->leavePluginCallback();
            return e.ErrorCode;
        }

        return NPERR_NO_ERROR;
    }

    NPError SAL_CALL NP_LOADDS  NPN_PostURL( NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file )
    {
        TRACE( "NPN_PostURL" );
        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return NPERR_INVALID_INSTANCE_ERROR;

        ::com::sun::star::uno::Sequence<sal_Int8> Bytes( (sal_Int8*)buf, len );
        ::rtl::OString aPostURL = normalizeURL( pImpl, url );
        try
        {
            pImpl->enterPluginCallback();
            pImpl->getPluginContext()->
                postURL( pImpl,
                         ::rtl::OStringToOUString( aPostURL, pImpl->getTextEncoding() ),
                         ::rtl::OStringToOUString( window, pImpl->getTextEncoding () ),
                         Bytes,
                         file );
            pImpl->leavePluginCallback();
        }
        catch( ::com::sun::star::plugin::PluginException& e )
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
            if( ( pStream = (*iter)->getStreamFromNPStream( stream ) ) )
            {
                pPlugin = *iter;
                break;
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
            if( pBytes && nBytes < rangeList->length )
            {
                delete pBytes;
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
                pPlugin->getPluginComm()->
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
                displayStatusText( pImpl, ::rtl::OStringToOUString( message, pImpl->getTextEncoding() ) );
            pImpl->leavePluginCallback();
        }
        catch( ::com::sun::star::plugin::PluginException& )
        {
            pImpl->leavePluginCallback();
            return;
        }
    }

    const char* SAL_CALL NP_LOADDS  NPN_UserAgent( NPP instance )
    {
        TRACE( "NPN_UserAgent" );
        static char* pAgent = strdup( "Mozilla" );

        XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
        if( ! pImpl )
            return pAgent;

        ::rtl::OUString UserAgent;
        try
        {
            pImpl->enterPluginCallback();
            UserAgent = pImpl->getPluginContext()->
                getUserAgent( pImpl );
            pImpl->leavePluginCallback();
        }
        catch( ::com::sun::star::plugin::PluginException& )
        {
            pImpl->leavePluginCallback();
            return pAgent;
        }

        if( pAgent )
            free( pAgent );
        pAgent = strdup( ::rtl::OUStringToOString( UserAgent, pImpl->getTextEncoding() ).getStr() );

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

int32 SAL_CALL NP_LOADDS  NPN_Write( NPP instance, NPStream* stream, int32 len,
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
    TRACE( "NPN_GetValue" );
    XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );

#ifdef UNX
    // some special unix variables
    XPlugin_Impl* pInstance = pImpl ? pImpl : XPluginManager_Impl::getFirstXPlugin();
    if( ! pInstance )
        return NULL;

    switch( variable )
    {
        case NPNVxDisplay:
            *((Display**)value) = pInstance->getAppDisplay();
#ifdef DEBUG
            fprintf( stderr, "NPN_GetValue of display\n" );
#endif
            return NPERR_NO_ERROR;
            break;
        case NPNVxtAppContext:
            *((XtAppContext*)value) = pInstance->getAppContext();
#ifdef DEBUG
            fprintf( stderr, "NPN_GetValue of application context\n" );
#endif
            return NPERR_NO_ERROR;
    }
#endif

    if( ! pImpl )
        return 0;

    ::rtl::OUString aValue;
    try
    {
        pImpl->enterPluginCallback();
        aValue = pImpl->getPluginContext()->
            getValue( pImpl, (::com::sun::star::plugin::PluginVariable)variable );
        pImpl->leavePluginCallback();
    }
    catch( ::com::sun::star::plugin::PluginException& e )
    {
        pImpl->leavePluginCallback();
        return e.ErrorCode;
    }

    return NPERR_NO_ERROR;
}

void SAL_CALL NP_LOADDS  NPN_ReloadPlugins(NPBool reloadPages)
{
    TRACE( "NPN_ReloadPlugins" );
}


NPError SAL_CALL NP_LOADDS  NPN_SetValue(NPP instance, NPPVariable variable,
                         void *value)
{
    TRACE( "NPN_SetValue" );
    return 0;
}

void SAL_CALL NP_LOADDS  NPN_InvalidateRect(NPP instance, NPRect *invalidRect)
{
    TRACE( "NPN_InvalidateRect" );
}

void SAL_CALL NP_LOADDS  NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion)
{
    TRACE( "NPN_InvalidateRegion" );
}

void SAL_CALL NP_LOADDS  NPN_ForceRedraw(NPP instance)
{
    TRACE( "NPN_ForceRedraw" );
}

}
