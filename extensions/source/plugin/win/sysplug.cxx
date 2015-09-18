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


#include <prewin.h>
#include <postwin.h>

#include <sal/log.hxx>

#include <plugin/impl.hxx>

#if defined _MSC_VER
#pragma warning (push,1)
#pragma warning (disable:4005)
#endif

#include <string.h>
#include <tchar.h>
#include <objbase.h>

#if defined _MSC_VER
#pragma warning (pop)
#endif

#include <list>
#include <map>
#include <algorithm>


extern NPNetscapeFuncs aNPNFuncs;

#include <tools/debug.hxx>


#if OSL_DEBUG_LEVEL > 1
void TRACE( char const * s );
void TRACEN( char const * s, long n );
#else
#define TRACE(x)
#define TRACEN(x,n)
#endif

std::shared_ptr<SysPlugData> CreateSysPlugData()
{
    return std::shared_ptr<SysPlugData>();
}


PluginComm_Impl::PluginComm_Impl( const OUString& /*rMIME*/, const OUString& rName, HWND /*hWnd*/ )
    : PluginComm( OUStringToOString( rName, RTL_TEXTENCODING_MS_1252 ) )
{
    // initialize plugin function table
    memset( &_NPPfuncs, 0, sizeof( _NPPfuncs ) );
#ifdef UNICODE
    _plDLL = ::LoadLibrary( rName.getStr() );
#else
    OString aStr( OUStringToOString( rName, RTL_TEXTENCODING_MS_1252 ) );
    _plDLL = ::LoadLibrary( aStr.getStr() );
#endif
    DBG_ASSERT( _plDLL, "### loading plugin dll failed!" );

    NPError (WINAPI * pEntry)( NPPluginFuncs* );
    retrieveFunction( _T("NP_GetEntryPoints"), (void**)&pEntry );

    _NPPfuncs.size = sizeof( _NPPfuncs );
    _NPPfuncs.version = 0;
    NPError nErr = (*pEntry)( &_NPPfuncs );

    SAL_WARN_IF(
        nErr != NPERR_NO_ERROR, "extensions.plugin",
        "NP_GetEntryPoints() failed");
    DBG_ASSERT( (_NPPfuncs.version >> 8) >= NP_VERSION_MAJOR,
                "### version failure!" );

    m_eCall = eNP_Initialize;
    execute();
}


PluginComm_Impl::~PluginComm_Impl()
{
    if (_plDLL)
    {
//          NPP_Shutdown();

        NPError (WINAPI * pShutdown)();
        if (retrieveFunction( _T("NP_Shutdown"), (void**)&pShutdown ))
        {
            NPError nErr = (*pShutdown)(); (void)nErr;
            DBG_ASSERT( nErr == NPERR_NO_ERROR, "### NP_Shutdown() failed!" );
        }

        BOOL bRet = (BOOL)::FreeLibrary( _plDLL ); (void)bRet;
        DBG_ASSERT( bRet, "### unloading plugin dll failed!" );
        _plDLL = NULL;
    }
}


BOOL PluginComm_Impl::retrieveFunction( TCHAR const * pName, void** ppFunc ) const
{
    if( ! _plDLL )
        return FALSE;

    *ppFunc = (void*)::GetProcAddress( _plDLL, pName );

    return (*ppFunc != NULL);
}



long PluginComm_Impl::doIt()
{
    long nRet = 0;
    switch( m_eCall )
    {
    case eNP_Initialize:
    {
        TRACE( "eNP_Initialize" );
        NPError (WINAPI * pInit)( NPNetscapeFuncs* );
        if ((_NPPfuncs.version >> 8) >= NP_VERSION_MAJOR &&
            (retrieveFunction( _T("NP_Initialize"), (void**)&pInit ) ||
             retrieveFunction( _T("NP_PluginInit"), (void**)&pInit )))
        {
            nRet = (*pInit)( &aNPNFuncs );
        }
        else
        {
            nRet = NPERR_GENERIC_ERROR;
        }
        DBG_ASSERT( nRet == NPERR_NO_ERROR, "### NP_Initialize() failed!" );
    }
    break;
    case eNPP_Destroy:
        TRACE( "eNPP_Destroy" );
        nRet = (_NPPfuncs.destroy
                ? (*_NPPfuncs.destroy)(
                    (NPP)m_aArgs[0],
                    (NPSavedData**)m_aArgs[1] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_DestroyStream:
        TRACE( "eNPP_DestroyStream" );
        nRet =  (_NPPfuncs.destroystream
                 ? (*_NPPfuncs.destroystream)(
                     (NPP)m_aArgs[0],
                     (NPStream*)m_aArgs[1],
                     (NPError)(sal_IntPtr)m_aArgs[2] )
                 : NPERR_GENERIC_ERROR);
        break;
    case eNPP_New:
        TRACE( "eNPP_New" );
        nRet = (_NPPfuncs.newp
                ? (*_NPPfuncs.newp)(
                    (NPMIMEType)m_aArgs[0],
                    (NPP)m_aArgs[1],
                    (uint16_t)(sal_IntPtr)m_aArgs[2],
                    (int16_t)(sal_IntPtr)m_aArgs[3],
                    (char**)m_aArgs[4],
                    (char**)m_aArgs[5],
                    (NPSavedData*)m_aArgs[6] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_NewStream:
        TRACE( "eNPP_NewStream" );
        nRet = (_NPPfuncs.newstream
                ? (*_NPPfuncs.newstream)(
                    (NPP)m_aArgs[0],
                    (NPMIMEType)m_aArgs[1],
                    (NPStream*)m_aArgs[2],
                    (NPBool)(sal_IntPtr)m_aArgs[3],
                    (uint16_t*)m_aArgs[4] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_Print:
        TRACE( "eNPP_Print" );
        if (_NPPfuncs.print)
            (*_NPPfuncs.print)(
                (NPP)m_aArgs[0],
                (NPPrint*)m_aArgs[1] );
        break;
    case eNPP_SetWindow:
    {
        TRACE( "eNPP_SetWindow" );
        nRet = (_NPPfuncs.setwindow
                ? (*_NPPfuncs.setwindow)(
                    (NPP)m_aArgs[0],
                    (NPWindow*)m_aArgs[1] )
                : NPERR_GENERIC_ERROR);
        break;
    }
    case eNPP_StreamAsFile:
        TRACE( "eNPP_StreamAsFile" );
        if (_NPPfuncs.asfile)
            (*_NPPfuncs.asfile)(
                (NPP)m_aArgs[0],
                (NPStream*)m_aArgs[1],
                (char*)m_aArgs[2] );
        break;
    case eNPP_URLNotify:
        TRACE( "eNPP_URLNotify" );
        if (_NPPfuncs.urlnotify)
            (*_NPPfuncs.urlnotify)(
                (NPP)m_aArgs[0],
                (char*)m_aArgs[1],
                (NPReason)(sal_IntPtr)m_aArgs[2],
                m_aArgs[3] );
        break;
    case eNPP_Write:
        TRACEN( "eNPP_Write n=", (int32_t)m_aArgs[3] );
        nRet = (_NPPfuncs.write
                ? (*_NPPfuncs.write)(
                    (NPP)m_aArgs[0],
                    (NPStream*)m_aArgs[1],
                    (sal_IntPtr)m_aArgs[2],
                    (sal_IntPtr)m_aArgs[3],
                    m_aArgs[4] )
                : 0);
        break;
    case eNPP_WriteReady:
        TRACE( "eNPP_WriteReady" );
        nRet = (_NPPfuncs.writeready
                ? (*_NPPfuncs.writeready)(
                    (NPP)m_aArgs[0],
                    (NPStream*)m_aArgs[1] )
                : 0);
        break;
    case eNPP_GetValue:
        TRACE( "eNPP_GetValue" );
        nRet = (_NPPfuncs.getvalue
                ? (*_NPPfuncs.getvalue)(
                    (NPP)m_aArgs[0],
                    (NPPVariable)(sal_IntPtr)m_aArgs[1],
                    m_aArgs[2] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_SetValue:
        TRACE( "eNPP_SetValue" );
        nRet = (_NPPfuncs.setvalue
                ? (*_NPPfuncs.setvalue)(
                    (NPP)m_aArgs[0],
                    (NPNVariable)(sal_IntPtr)m_aArgs[1],
                    m_aArgs[2] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_Shutdown:
    {
        TRACE( "eNPP_Shutdown" );
        NPP_ShutdownUPP pFunc;
        if (retrieveFunction( _T("NPP_Shutdown"), (void**)&pFunc ))
            (*pFunc)();
    }
    break;
    case eNPP_Initialize:
        TRACE( "eNPP_Initialize" );
        OSL_FAIL( "NPP_Initialize: not implemented!" );
        break;
    case eNPP_GetJavaClass:
        TRACE( "eNPP_GetJavaClass" );
        OSL_FAIL( "NPP_GetJavaClass: not implemented!" );
        break;
    }
    return nRet;
}


NPError PluginComm_Impl::NPP_Destroy( NPP instance, NPSavedData** save )
{
    DBG_ASSERT( _NPPfuncs.destroy, "### NPP_Destroy(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Destroy;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)save;
    return (NPError)execute();
}


NPError PluginComm_Impl::NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    DBG_ASSERT( _NPPfuncs.destroystream, "### NPP_DestroyStream(): null pointer in NPP functions table!" );
    m_eCall = eNPP_DestroyStream;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = reinterpret_cast< void * >(static_cast< sal_IntPtr >(reason));
    return (NPError)execute();
}


NPError PluginComm_Impl::NPP_New( NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc,
                                  char* argn[], char* argv[], NPSavedData *saved )
{
    DBG_ASSERT( _NPPfuncs.newp, "### NPP_New(): null pointer in NPP functions table!" );
    m_eCall = eNPP_New;
    m_aArgs[0] = (void*)pluginType;
    m_aArgs[1] = (void*)instance;
    m_aArgs[2] = reinterpret_cast< void * >(static_cast< sal_uIntPtr >(mode));
    m_aArgs[3] = reinterpret_cast< void * >(static_cast< sal_IntPtr >(argc));
    m_aArgs[4] = (void*)argn;
    m_aArgs[5] = (void*)argv;
    m_aArgs[6] = (void*)saved;
    return (NPError)execute();
}


NPError PluginComm_Impl::NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                        NPBool seekable, uint16_t* stype )
{
    DBG_ASSERT( _NPPfuncs.newstream, "### NPP_NewStream(): null pointer in NPP functions table!" );
    m_eCall = eNPP_NewStream;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)type;
    m_aArgs[2] = (void*)stream;
    m_aArgs[3] = reinterpret_cast< void * >(
        static_cast< sal_uIntPtr >(seekable));
    m_aArgs[4] = (void*)stype;
    return (NPError)execute();
}


void PluginComm_Impl::NPP_Print( NPP instance, NPPrint* platformPrint )
{
    DBG_ASSERT( _NPPfuncs.print, "### NPP_Print(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Print;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)platformPrint;
    execute();
}


NPError PluginComm_Impl::NPP_SetWindow( NPP instance, NPWindow* window )
{
    DBG_ASSERT( _NPPfuncs.setwindow, "### NPP_SetWindow(): null pointer in NPP functions table!" );
    m_eCall = eNPP_SetWindow;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)window;
    return (NPError)execute();
}


void PluginComm_Impl::NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname )
{
    DBG_ASSERT( _NPPfuncs.asfile, "### NPP_StreamAsFile(): null pointer in NPP functions table!" );
    m_eCall = eNPP_StreamAsFile;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)fname;
    execute();
}


void PluginComm_Impl::NPP_URLNotify( NPP instance, const char* url, NPReason reason, void* notifyData )
{
    DBG_ASSERT( _NPPfuncs.urlnotify, "### NPP_URLNotify(): null pointer in NPP functions table!" );
    m_eCall = eNPP_URLNotify;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)url;
    m_aArgs[2] = reinterpret_cast< void * >(static_cast< sal_IntPtr >(reason));
    m_aArgs[3] = notifyData;
    execute();
}


int32_t PluginComm_Impl::NPP_Write( NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer )
{
    DBG_ASSERT( _NPPfuncs.write, "### NPP_Write(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Write;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)(sal_IntPtr)offset;
    m_aArgs[3] = (void*)(sal_IntPtr)len;
    m_aArgs[4] = buffer;
    return (NPError)execute();
}


int32_t PluginComm_Impl::NPP_WriteReady( NPP instance, NPStream* stream )
{
    DBG_ASSERT( _NPPfuncs.writeready, "### NPP_WriteReady(): null pointer in NPP functions table!" );
    m_eCall = eNPP_WriteReady;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    return execute();
}


NPError PluginComm_Impl::NPP_GetValue( NPP instance, NPPVariable variable, void *ret_value )
{
    DBG_ASSERT( _NPPfuncs.getvalue, "### NPP_GetValue(): null pointer in NPP functions table!" );
    m_eCall = eNPP_GetValue;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)variable;
    m_aArgs[2] = ret_value;
    return (NPError)execute();
}


NPError PluginComm_Impl::NPP_Initialize()
{
    return NPERR_NO_ERROR;
}


void PluginComm_Impl::NPP_Shutdown()
{
    m_eCall = eNPP_Shutdown;
    execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
