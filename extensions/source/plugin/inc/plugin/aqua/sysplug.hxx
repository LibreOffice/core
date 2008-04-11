/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sysplug.hxx,v $
 * $Revision: 1.6 $
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
#ifndef __PLUGIN_INC_MACPLUG_HXX
#define __PLUGIN_INC_MACPLUG_HXX

#include <unistd.h>

#include <list>
#include <map>
#include <algorithm>
#include <premac.h>
#include <Carbon/Carbon.h>
#include <Security/cssmconfig.h>
#include <postmac.h>
#undef uint32
#include <npsdk/npapi.h>
#include <npsdk/npupp.h>

#include <plugin/plcom.hxx>
#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

#include <vcl/sysdata.hxx>


//==================================================================================================
class MacPluginComm : public PluginComm
{
private:
    static int  nConnCounter;

    pid_t       m_nCommPID;
public:
        MacPluginComm( const String& mimetype,
                       const String& library,
                       NSView* pParent,
                       int nDescriptor1,
                       int nDescriptor2
                       );
    virtual ~MacPluginComm();

    virtual NPError NPP_Destroy( NPP instance, NPSavedData** save );
    virtual NPError NPP_DestroyStream( NPP instance, NPStream* stream,
                                       NPError reason );
    virtual void* NPP_GetJavaClass();
    virtual NPError NPP_Initialize();
    virtual NPError NPP_New( NPMIMEType pluginType, NPP instance,
                             uint16 mode, int16 argc,
                             char* argn[], char* argv[], NPSavedData *saved );
    virtual NPError NPP_NewStream( NPP instance, NPMIMEType type,
                                   NPStream* stream,
                                   NPBool seekable, uint16* stype );
    virtual void NPP_Print( NPP instance, NPPrint* platformPrint );
    virtual NPError NPP_SetWindow( NPP instance, NPWindow* window );
    virtual void NPP_Shutdown();
    virtual void NPP_StreamAsFile( NPP instance, NPStream* stream,
                                   const char* fname );
    virtual void NPP_URLNotify( NPP instance, const char* url, NPReason reason,
                                void* notifyData );
    virtual int32 NPP_Write( NPP instance, NPStream* stream, int32 offset,
                             int32 len, void* buffer );
    virtual int32 NPP_WriteReady( NPP instance, NPStream* stream );
    virtual char* NPP_GetMIMEDescription();
    virtual NPError NPP_GetValue( NPP instance, NPPVariable variable, void* value );
    virtual NPError NPP_SetValue( NPP instance, NPNVariable variable,
                                  void *value);
};


#endif


