/*************************************************************************
 *
 *  $RCSfile: sysplug.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:18:31 $
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

#include <plugin/impl.hxx>

#ifdef WNT
#include <tools/prewin.h>
#endif

#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <winreg.h>
#include <winbase.h>
#include <objbase.h>

#ifdef WNT
#include <tools/postwin.h>
#endif

#include <list>
#include <map>
#include <algorithm>


extern NPNetscapeFuncs aNPNFuncs;

#include <tools/debug.hxx>

using namespace rtl;

#if OSL_DEBUG_LEVEL > 1
void TRACE( char const * s );
void TRACEN( char const * s, long n );
#else
#define TRACE(x)
#define TRACEN(x,n)
#endif


//--------------------------------------------------------------------------------------------------
PluginComm_Impl::PluginComm_Impl( const OUString& rMIME, const OUString& rName, HWND hWnd )
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

    NPError nErr = NPERR_NO_ERROR;
    NPError (WINAPI * pEntry)( NPPluginFuncs* );
    retrieveFunction( _T("NP_GetEntryPoints"), (void**)&pEntry );

    _NPPfuncs.size = sizeof( _NPPfuncs );
    _NPPfuncs.version = 0;
    nErr = (*pEntry)( &_NPPfuncs );

    DBG_ASSERT( nErr == NPERR_NO_ERROR, "### NP_GetEntryPoints() failed!" );
    DBG_ASSERT( (_NPPfuncs.version >> 8) >= NP_VERSION_MAJOR,
                "### version failure!" );

    m_eCall = eNP_Initialize;
    execute();
}

//--------------------------------------------------------------------------------------------------
PluginComm_Impl::~PluginComm_Impl()
{
    if (_plDLL)
    {
//          NPP_Shutdown();

        NPError (WINAPI * pShutdown)();
        if (retrieveFunction( _T("NP_Shutdown"), (void**)&pShutdown ))
        {
            NPError nErr = (*pShutdown)();
            DBG_ASSERT( nErr == NPERR_NO_ERROR, "### NP_Shutdown() failed!" );
        }

        BOOL bRet = ::FreeLibrary( _plDLL );
        DBG_ASSERT( bRet, "### unloading plugin dll failed!" );
        _plDLL = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
BOOL PluginComm_Impl::retrieveFunction( TCHAR* pName, void** ppFunc ) const
{
    if( ! _plDLL )
        return FALSE;

    *ppFunc = (void*)::GetProcAddress( _plDLL, pName );

    return (*ppFunc != NULL);
}

//--------------------------------------------------------------------------------------------------

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
                     (NPError)m_aArgs[2] )
                 : NPERR_GENERIC_ERROR);
        break;
    case eNPP_New:
        TRACE( "eNPP_New" );
        nRet = (_NPPfuncs.newp
                ? (*_NPPfuncs.newp)(
                    (NPMIMEType)m_aArgs[0],
                    (NPP)m_aArgs[1],
                    (uint16)m_aArgs[2],
                    (int16)m_aArgs[3],
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
                    (NPBool)m_aArgs[3],
                    (uint16*)m_aArgs[4] )
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
                (NPReason)m_aArgs[2],
                m_aArgs[3] );
        break;
    case eNPP_Write:
        TRACEN( "eNPP_Write n=", (int32)m_aArgs[3] );
        nRet = (_NPPfuncs.write
                ? (*_NPPfuncs.write)(
                    (NPP)m_aArgs[0],
                    (NPStream*)m_aArgs[1],
                    (int32)m_aArgs[2],
                    (int32)m_aArgs[3],
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
                    (NPPVariable)(int)m_aArgs[1],
                    m_aArgs[2] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_SetValue:
        TRACE( "eNPP_SetValue" );
        nRet = (_NPPfuncs.setvalue
                ? (*_NPPfuncs.setvalue)(
                    (NPP)m_aArgs[0],
                    (NPNVariable)(int)m_aArgs[1],
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

    }
    return nRet;
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_Destroy( NPP instance, NPSavedData** save )
{
    DBG_ASSERT( _NPPfuncs.destroy, "### NPP_Destroy(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Destroy;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)save;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    DBG_ASSERT( _NPPfuncs.destroystream, "### NPP_DestroyStream(): null pointer in NPP functions table!" );
    m_eCall = eNPP_DestroyStream;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)reason;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
                                  char* argn[], char* argv[], NPSavedData *saved )
{
    DBG_ASSERT( _NPPfuncs.newp, "### NPP_New(): null pointer in NPP functions table!" );
    m_eCall = eNPP_New;
    m_aArgs[0] = (void*)pluginType;
    m_aArgs[1] = (void*)instance;
    m_aArgs[2] = (void*)mode;
    m_aArgs[3] = (void*)argc;
    m_aArgs[4] = (void*)argn;
    m_aArgs[5] = (void*)argv;
    m_aArgs[6] = (void*)saved;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                        NPBool seekable, uint16* stype )
{
    DBG_ASSERT( _NPPfuncs.newstream, "### NPP_NewStream(): null pointer in NPP functions table!" );
    m_eCall = eNPP_NewStream;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)type;
    m_aArgs[2] = (void*)stream;
    m_aArgs[3] = (void*)seekable;
    m_aArgs[4] = (void*)stype;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
void PluginComm_Impl::NPP_Print( NPP instance, NPPrint* platformPrint )
{
    DBG_ASSERT( _NPPfuncs.print, "### NPP_Print(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Print;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)platformPrint;
    execute();
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_SetWindow( NPP instance, NPWindow* window )
{
    DBG_ASSERT( _NPPfuncs.setwindow, "### NPP_SetWindow(): null pointer in NPP functions table!" );
    m_eCall = eNPP_SetWindow;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)window;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
void PluginComm_Impl::NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname )
{
    DBG_ASSERT( _NPPfuncs.asfile, "### NPP_StreamAsFile(): null pointer in NPP functions table!" );
    m_eCall = eNPP_StreamAsFile;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)fname;
    execute();
}

//--------------------------------------------------------------------------------------------------
void PluginComm_Impl::NPP_URLNotify( NPP instance, const char* url, NPReason reason, void* notifyData )
{
    DBG_ASSERT( _NPPfuncs.urlnotify, "### NPP_URLNotify(): null pointer in NPP functions table!" );
    m_eCall = eNPP_URLNotify;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)url;
    m_aArgs[2] = (void*)reason;
    m_aArgs[3] = notifyData;
    execute();
}

//--------------------------------------------------------------------------------------------------
int32 PluginComm_Impl::NPP_Write( NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer )
{
    DBG_ASSERT( _NPPfuncs.write, "### NPP_Write(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Write;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)offset;
    m_aArgs[3] = (void*)len;
    m_aArgs[4] = buffer;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
int32 PluginComm_Impl::NPP_WriteReady( NPP instance, NPStream* stream )
{
    DBG_ASSERT( _NPPfuncs.writeready, "### NPP_WriteReady(): null pointer in NPP functions table!" );
    m_eCall = eNPP_WriteReady;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    return execute();
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_GetValue( NPP instance, NPPVariable variable, void *ret_value )
{
    DBG_ASSERT( _NPPfuncs.getvalue, "### NPP_GetValue(): null pointer in NPP functions table!" );
    m_eCall = eNPP_GetValue;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)variable;
    m_aArgs[2] = ret_value;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_SetValue( NPP instance, NPNVariable variable, void *set_value )
{
    DBG_ASSERT( _NPPfuncs.setvalue, "### NPP_SetValue(): null pointer in NPP functions table!" );
    m_eCall = eNPP_SetValue;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)variable;
    m_aArgs[2] = set_value;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
void * PluginComm_Impl::NPP_GetJavaClass()
{
    DBG_ERROR( "no java class available!" );
    return 0;
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_Initialize()
{
    return NPERR_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------
void PluginComm_Impl::NPP_Shutdown()
{
    m_eCall = eNPP_Shutdown;
    execute();
}
