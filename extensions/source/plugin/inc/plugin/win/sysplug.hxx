/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __PLUGIN_INC_WINPLUG_HXX
#define __PLUGIN_INC_WINPLUG_HXX

#pragma warning (push,1)
#pragma warning (disable:4005)

#include <tools/prewin.h>

#include <windows.h>
#include <tchar.h>
#include <winbase.h>

#include <tools/postwin.h>

#pragma pack( push, 8 )
#include <npsdk/npapi.h>
#include <npsdk/npupp.h>
#pragma pack( pop )

#pragma warning (pop)

#include <list>
#include <map>
#include <algorithm>

#include <plugin/plcom.hxx>
#include <vcl/threadex.hxx>

//==================================================================================================
class PluginComm_Impl :
    public PluginComm,
    public ::vcl::SolarThreadExecutor

{
    enum CallType {
        eNPP_Destroy,
        eNPP_DestroyStream,
        eNPP_GetJavaClass,
        eNPP_Initialize,
        eNPP_New,
        eNPP_NewStream,
        eNPP_Print,
        eNPP_SetWindow,
        eNPP_Shutdown,
        eNPP_StreamAsFile,
        eNPP_URLNotify,
        eNPP_Write,
        eNPP_WriteReady,
        eNPP_GetValue,
        eNPP_SetValue,
        eNP_Initialize
    };

    void*               m_aArgs[ 8 ];
    CallType            m_eCall;

    virtual long        doIt();
public:
                        PluginComm_Impl( const rtl::OUString& rMIME, const rtl::OUString& rName, HWND hWnd );
    virtual             ~PluginComm_Impl();

public:
    using PluginComm::NPP_Destroy;
    virtual NPError     NPP_Destroy( NPP instance, NPSavedData** save );
    virtual NPError     NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason );
    virtual void *      NPP_GetJavaClass();
    virtual NPError     NPP_Initialize();
    virtual NPError     NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
                                 char* argn[], char* argv[], NPSavedData *saved );
    virtual NPError     NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                       NPBool seekable, uint16* stype );
    virtual void        NPP_Print( NPP instance, NPPrint* platformPrint );

    using PluginComm::NPP_SetWindow;
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


