/*************************************************************************
 *
 *  $RCSfile: sysplug.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
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
#if _MSC_VER >= 1200
#include <tools/prewin.h>
#else
#include <tools/presys.h>
#endif
#endif
#if STLPORT_VERSION < 321
#include <stl/map.h>
#endif
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <winreg.h>
#include <winbase.h>
#include <objbase.h>
#ifdef WNT
#if _MSC_VER >= 1200
#include <tools/postwin.h>
#else
#include <tools/postsys.h>
#endif
#endif
#if STLPORT_VERSION >= 321
#include <stl/list>
#include <stl/map>
#include <stl/algorithm>
#endif

extern NPNetscapeFuncs aNPNFuncs;

#include <tools/debug.hxx>

using namespace rtl;

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

    NPError nErr;
    NPError (WINAPI * pEntry)( NPPluginFuncs* );
    if (_plDLL && retrieveFunction( _T("NP_GetEntryPoints"), (void**)&pEntry ))
    {
        _NPPfuncs.size = sizeof( _NPPfuncs );
        _NPPfuncs.version = 0;
        nErr = (*pEntry)( &_NPPfuncs );

        DBG_ASSERT( nErr == NPERR_NO_ERROR, "### NP_GetEntryPoints() failed!" );
        DBG_ASSERT( (_NPPfuncs.version >> 8) >= NP_VERSION_MAJOR,
                    "### version failure!" );

        NPError (WINAPI * pInit)( NPNetscapeFuncs* );
        if (nErr == NPERR_NO_ERROR &&
            (_NPPfuncs.version >> 8) >= NP_VERSION_MAJOR &&
            (retrieveFunction( _T("NP_Initialize"), (void**)&pInit ) ||
             retrieveFunction( _T("NP_PluginInit"), (void**)&pInit )))
        {
            nErr = (*pInit)( &aNPNFuncs );
            DBG_ASSERT( nErr == NPERR_NO_ERROR, "### NP_Initialize() failed!" );

// WIN obsolete: called by NP_Initialize()
//              nErr = NPP_Initialize();
//              DBG_ASSERT( nErr == NPERR_NO_ERROR, "### NPP_Initialize() failed!" );
        }
    }
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
NPError PluginComm_Impl::NPP_Destroy( NPP instance, NPSavedData** save )
{
    DBG_ASSERT( _NPPfuncs.destroy, "### NPP_Destroy(): null pointer in NPP functions table!" );
    return (_NPPfuncs.destroy
            ? (*_NPPfuncs.destroy)( instance, save )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    DBG_ASSERT( _NPPfuncs.destroystream, "### NPP_DestroyStream(): null pointer in NPP functions table!" );
    return (_NPPfuncs.destroystream
            ? (*_NPPfuncs.destroystream)( instance, stream, reason )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
                                  char* argn[], char* argv[], NPSavedData *saved )
{
    DBG_ASSERT( _NPPfuncs.newp, "### NPP_New(): null pointer in NPP functions table!" );
    return (_NPPfuncs.newp
            ? (*_NPPfuncs.newp)( pluginType, instance, mode, argc, argn, argv, saved )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                        NPBool seekable, uint16* stype )
{
    DBG_ASSERT( _NPPfuncs.newstream, "### NPP_NewStream(): null pointer in NPP functions table!" );
    return (_NPPfuncs.newstream
            ? (*_NPPfuncs.newstream)( instance, type, stream, seekable, stype )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
void PluginComm_Impl::NPP_Print( NPP instance, NPPrint* platformPrint )
{
    DBG_ASSERT( _NPPfuncs.print, "### NPP_Print(): null pointer in NPP functions table!" );
    if (_NPPfuncs.print)
        (*_NPPfuncs.print)( instance, platformPrint );
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_SetWindow( NPP instance, NPWindow* window )
{
    DBG_ASSERT( _NPPfuncs.setwindow, "### NPP_SetWindow(): null pointer in NPP functions table!" );
    return (_NPPfuncs.setwindow
            ? (*_NPPfuncs.setwindow)( instance, window )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
void PluginComm_Impl::NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname )
{
    DBG_ASSERT( _NPPfuncs.asfile, "### NPP_StreamAsFile(): null pointer in NPP functions table!" );
    if (_NPPfuncs.asfile)
        (*_NPPfuncs.asfile)( instance, stream, fname );
}

//--------------------------------------------------------------------------------------------------
void PluginComm_Impl::NPP_URLNotify( NPP instance, const char* url, NPReason reason, void* notifyData )
{
    DBG_ASSERT( _NPPfuncs.urlnotify, "### NPP_URLNotify(): null pointer in NPP functions table!" );
    if (_NPPfuncs.urlnotify)
        (*_NPPfuncs.urlnotify)( instance, url, reason, notifyData );
}

//--------------------------------------------------------------------------------------------------
int32 PluginComm_Impl::NPP_Write( NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer )
{
    DBG_ASSERT( _NPPfuncs.write, "### NPP_Write(): null pointer in NPP functions table!" );
    return (_NPPfuncs.write
            ? (*_NPPfuncs.write)( instance, stream, offset, len, buffer )
            : 0);
}

//--------------------------------------------------------------------------------------------------
int32 PluginComm_Impl::NPP_WriteReady( NPP instance, NPStream* stream )
{
    DBG_ASSERT( _NPPfuncs.writeready, "### NPP_WriteReady(): null pointer in NPP functions table!" );
    return (_NPPfuncs.writeready
            ? (*_NPPfuncs.writeready)( instance, stream )
            : 0);
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_GetValue( NPP instance, NPPVariable variable, void *ret_alue )
{
    DBG_ASSERT( _NPPfuncs.getvalue, "### NPP_GetValue(): null pointer in NPP functions table!" );
    return (_NPPfuncs.getvalue
            ? (*_NPPfuncs.getvalue)( instance, variable, ret_alue )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError PluginComm_Impl::NPP_SetValue( NPP instance, NPNVariable variable, void *ret_alue )
{
    DBG_ASSERT( _NPPfuncs.setvalue, "### NPP_SetValue(): null pointer in NPP functions table!" );
    return (_NPPfuncs.setvalue
            ? (*_NPPfuncs.setvalue)( instance, variable, ret_alue )
            : NPERR_GENERIC_ERROR);
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
    NPP_ShutdownUPP pFunc;
    if (retrieveFunction( _T("NPP_Shutdown"), (void**)&pFunc ))
        (*pFunc)();
}


