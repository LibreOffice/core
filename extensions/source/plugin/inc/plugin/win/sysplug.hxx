/*************************************************************************
 *
 *  $RCSfile: sysplug.hxx,v $
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

#ifndef __PLUGIN_INC_WINPLUG_HXX
#define __PLUGIN_INC_WINPLUG_HXX

#if _MSC_VER >= 1200
#include <tools/prewin.h>
#else
#include <tools/presys.h>
#endif
#if STLPORT_VERSION < 321
#include <list.h>
#include <map.h>
#include <algo.h>
#include <function.h>
#endif
#include <windows.h>
#include <tchar.h>
#include <winbase.h>
#if _MSC_VER >= 1200
#include <tools/postwin.h>
#else
#include <tools/postsys.h>
#endif
#if STLPORT_VERSION >= 321
#include <stl/list>
#include <stl/map>
#include <stl/algorithm>
#endif

#pragma pack( push, 8 )
#include <npsdk/npapi.h>
#include <npsdk/npupp.h>
#pragma pack( pop, 8 )

#include <plugin/plcom.hxx>


//==================================================================================================
class PluginComm_Impl
    : public PluginComm
{
public:
                        PluginComm_Impl( const rtl::OUString& rMIME, const rtl::OUString& rName, HWND hWnd );
    virtual             ~PluginComm_Impl();

public:
    virtual NPError     NPP_Destroy( NPP instance, NPSavedData** save );
    virtual NPError     NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason );
    virtual void *      NPP_GetJavaClass();
    virtual NPError     NPP_Initialize();
    virtual NPError     NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
                                 char* argn[], char* argv[], NPSavedData *saved );
    virtual NPError     NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                       NPBool seekable, uint16* stype );
    virtual void        NPP_Print( NPP instance, NPPrint* platformPrint );
    virtual NPError     NPP_SetWindow( NPP instance, NPWindow* window );
    virtual void        NPP_Shutdown();
    virtual void        NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname );
    virtual void        NPP_URLNotify( NPP instance, const char* url,
                                       NPReason reason, void* notifyData );
    virtual int32       NPP_Write( NPP instance, NPStream* stream, int32 offset,
                                   int32 len, void* buffer );
    virtual int32       NPP_WriteReady( NPP instance, NPStream* stream );
    virtual NPError     NPP_GetValue( NPP instance, NPPVariable variable, void *ret_alue );
    virtual NPError     NPP_SetValue( NPP instance, NPNVariable variable, void *ret_alue );

private:
    BOOL                retrieveFunction( TCHAR* pName, void** ppFunc ) const;

private:
    HINSTANCE           _plDLL;

    NPPluginFuncs       _NPPfuncs;
};


#endif


