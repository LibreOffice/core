/*************************************************************************
 *
 *  $RCSfile: sysplug.cxx,v $
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

#include <string>   // workaround for SUNPRO workshop include conflicts

#include <plugin/impl.hxx>

#include <dlfcn.h>

extern NPNetscapeFuncs aNPNFuncs;

UnxPluginComm::UnxPluginComm( const ::rtl::OString& library ) :
        PluginComm( library ),
        m_pLibrary( NULL ),
        m_bInit( FALSE )
{
    m_pLibrary = dlopen( library.getStr(), RTLD_NOW );

    // initialize plugin function table
    memset( &m_aFuncs, 0, sizeof( m_aFuncs ) );
    m_aFuncs.size       = sizeof( m_aFuncs );
    m_aFuncs.version    = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;

    if( m_pLibrary )
    {
        NPError (*initFunc)(NPNetscapeFuncs*, NPPluginFuncs* );
        NPError (*initPlugin)();

        initFunc = (NPError(*)(NPNetscapeFuncs*,NPPluginFuncs*))dlsym( m_pLibrary, "NP_Initialize" );
        initPlugin = (NPError(*)())dlsym( m_pLibrary, "NPP_Initialize" );
#ifdef DEBUG
        if( ! initFunc )
            fprintf( stderr, "could not get Symbol NP_Initialize\n" );
        if( ! initPlugin )
            fprintf( stderr, "could not get Symbol NPP_Initialize\n" );
#endif
        if( initFunc && initPlugin )
        {
            NPError aErr = initFunc( &aNPNFuncs, &m_aFuncs );
#ifdef DEBUG
            fprintf( stderr, "NP_Initialize returns %d\n", aErr );
#endif
            if( aErr )
            {
                aErr = initPlugin();
#ifdef DEBUG
                fprintf( stderr, "NPP_Initialize returns %d\n", aErr );
#endif
            }

            m_aFuncs.newp           = (NPP_NewUPP)dlsym( m_pLibrary, "NPP_New" );
            m_aFuncs.destroy        = (NPP_DestroyUPP)dlsym( m_pLibrary, "NPP_Destroy" );
            m_aFuncs.setwindow      = (NPP_SetWindowUPP)dlsym( m_pLibrary, "NPP_SetWindow" );
            m_aFuncs.newstream      = (NPP_NewStreamUPP)dlsym( m_pLibrary, "NPP_NewStream" );
            m_aFuncs.destroystream  = (NPP_DestroyStreamUPP)dlsym( m_pLibrary, "NPP_DestroyStream" );
            m_aFuncs.asfile         = (NPP_StreamAsFileUPP)dlsym( m_pLibrary, "NPP_StreamAsFile" );
            m_aFuncs.writeready     = (NPP_WriteReadyUPP)dlsym( m_pLibrary, "NPP_WriteReady" );
            m_aFuncs.write          = (NPP_WriteUPP)dlsym( m_pLibrary, "NPP_Write" );
            m_aFuncs.print          = (NPP_PrintUPP)dlsym( m_pLibrary, "NPP_Print" );
            m_aFuncs.event          = NULL;
            m_aFuncs.javaClass      = NULL;
            m_aFuncs.urlnotify      = (NPP_URLNotifyUPP)dlsym( m_pLibrary, "NPP_URLNotify" );
            m_aFuncs.getvalue       =(NPP_GetValueUPP)dlsym( m_pLibrary, "NPP_GetValue" );
            m_aFuncs.setvalue       =(NPP_SetValueUPP)dlsym( m_pLibrary, "NPP_SetValue" );

            m_bInit = TRUE;
        }
    }
    if( ! m_bInit )
    {
        dlclose( m_pLibrary );
        m_pLibrary = NULL;
    }
}

UnxPluginComm::~UnxPluginComm()
{
    if( m_bInit && m_pLibrary )
    {
        NPP_ShutdownUPP pFunc = (NPP_ShutdownUPP)dlsym( m_pLibrary,"NPP_Shutdown");
        if( pFunc)
            pFunc();
        dlclose( m_pLibrary );
    }
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_Destroy( NPP instance, NPSavedData** save )
{
    DBG_ASSERT( m_aFuncs.destroy, "### NPP_Destroy(): null pointer in NPP functions table!" );
    return (m_aFuncs.destroy
            ? m_aFuncs.destroy( instance, save )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    DBG_ASSERT( m_aFuncs.destroystream, "### NPP_DestroyStream(): null pointer in NPP functions table!" );
    return (m_aFuncs.destroystream
            ? m_aFuncs.destroystream( instance, stream, reason )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
                                  char* argn[], char* argv[], NPSavedData *saved )
{
    DBG_ASSERT( m_aFuncs.newp, "### NPP_New(): null pointer in NPP functions table!" );
    return (m_aFuncs.newp
            ? m_aFuncs.newp( pluginType, instance, mode, argc, argn, argv, saved )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                        NPBool seekable, uint16* stype )
{
    DBG_ASSERT( m_aFuncs.newstream, "### NPP_NewStream(): null pointer in NPP functions table!" );
    return (m_aFuncs.newstream
            ? m_aFuncs.newstream( instance, type, stream, seekable, stype )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
void UnxPluginComm::NPP_Print( NPP instance, NPPrint* platformPrint )
{
    DBG_ASSERT( m_aFuncs.print, "### NPP_Print(): null pointer in NPP functions table!" );
    if (m_aFuncs.print)
        m_aFuncs.print( instance, platformPrint );
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_SetWindow( NPP instance, NPWindow* window )
{
    DBG_ASSERT( m_aFuncs.setwindow, "### NPP_SetWindow(): null pointer in NPP functions table!" );
    return (m_aFuncs.setwindow
            ? m_aFuncs.setwindow( instance, window )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
void UnxPluginComm::NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname )
{
    DBG_ASSERT( m_aFuncs.asfile, "### NPP_StreamAsFile(): null pointer in NPP functions table!" );
    if (m_aFuncs.asfile)
        m_aFuncs.asfile( instance, stream, fname );
}

//--------------------------------------------------------------------------------------------------
void UnxPluginComm::NPP_URLNotify( NPP instance, const char* url, NPReason reason, void* notifyData )
{
    DBG_ASSERT( m_aFuncs.urlnotify, "### NPP_URLNotify(): null pointer in NPP functions table!" );
    if (m_aFuncs.urlnotify)
        m_aFuncs.urlnotify( instance, url, reason, notifyData );
}

//--------------------------------------------------------------------------------------------------
int32 UnxPluginComm::NPP_Write( NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer )
{
    DBG_ASSERT( m_aFuncs.write, "### NPP_Write(): null pointer in NPP functions table!" );
    return (m_aFuncs.write
            ? m_aFuncs.write( instance, stream, offset, len, buffer )
            : 0);
}

//--------------------------------------------------------------------------------------------------
int32 UnxPluginComm::NPP_WriteReady( NPP instance, NPStream* stream )
{
    DBG_ASSERT( m_aFuncs.writeready, "### NPP_WriteReady(): null pointer in NPP functions table!" );
    return (m_aFuncs.writeready
            ? m_aFuncs.writeready( instance, stream )
            : 0);
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_GetValue( NPP instance, NPPVariable variable, void *ret_alue )
{
    DBG_ASSERT( m_aFuncs.getvalue, "### NPP_GetValue(): null pointer in NPP functions table!" );
    return (m_aFuncs.getvalue
            ? m_aFuncs.getvalue( instance, variable, ret_alue )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_SetValue( NPP instance, NPNVariable variable, void *ret_alue )
{
    DBG_ASSERT( m_aFuncs.setvalue, "### NPP_SetValue(): null pointer in NPP functions table!" );
    return (m_aFuncs.setvalue
            ? m_aFuncs.setvalue( instance, variable, ret_alue )
            : NPERR_GENERIC_ERROR);
}

//--------------------------------------------------------------------------------------------------
void* UnxPluginComm::NPP_GetJavaClass()
{
    DBG_ERROR( "no java class available!" );
    return 0;
}

//--------------------------------------------------------------------------------------------------
NPError UnxPluginComm::NPP_Initialize()
{
    return NPERR_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------
void UnxPluginComm::NPP_Shutdown()
{
}

