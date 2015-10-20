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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_AQUA_SYSPLUG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_AQUA_SYSPLUG_HXX

#include <unistd.h>

#include <list>
#include <map>
#include <algorithm>
#include "premac.h"
#include <Carbon/Carbon.h>
#include <Security/cssmconfig.h>
#include "postmac.h"
#undef uint32

#define XP_MAC
#include "npapi.h"
#include "npsdk/npupp.h"

#include "plugin/plcom.hxx"

#include "vcl/sysdata.hxx"
#include "vcl/threadex.hxx"
#include "vcl/timer.hxx"
#include "osl/module.h"

#ifdef __OBJC__
@class NSView;
#else
class NSView;
#endif

class XPlugin_Impl;

namespace plugstringhelper
{
OUString           getString( CFStringRef i_xString );
OUString           getString( CFURLRef i_xURL );
CFMutableStringRef      createString( const OUString& i_rString );
CFURLRef                createURL( const OUString& i_rString );
OUString           getURLFromPath( const OUString& i_rPath );
CFURLRef                createURLFromPath( const OUString& i_rPath );
OUString           CFURLtoOSLURL( CFURLRef i_xURL );
}


class MacPluginComm :
    public PluginComm,
    public vcl::SolarThreadExecutor

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
        eNPP_HandleEvent,
        eNP_Initialize
    };

    void*               m_aArgs[ 8 ];
    CallType            m_eCall;

    virtual long        doIt();
public:
                        MacPluginComm( const OUString& rMIME, const OUString& rName, NSView* pView );
    virtual             ~MacPluginComm();

    // FIXME:
    // this actually should be from the NP headers
    // but currently we have too old a version
    // changes this when we have updated our headers
    typedef struct NP_CGContext
    {
        CGContextRef context;
        WindowRef window;
    } NP_CGContext;

public:
    virtual NPError     NPP_Destroy( NPP instance, NPSavedData** save );
    virtual NPError     NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason );
    virtual NPError     NPP_Initialize();
    virtual NPError     NPP_New( NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc,
                                 char* argn[], char* argv[], NPSavedData *saved );
    virtual NPError     NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                       NPBool seekable, uint16_t* stype );
    virtual void        NPP_Print( NPP instance, NPPrint* platformPrint );
    virtual NPError     NPP_SetWindow( NPP instance, NPWindow* window );
    virtual void        NPP_Shutdown();
    virtual void        NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname );
    virtual void        NPP_URLNotify( NPP instance, const char* url,
                                       NPReason reason, void* notifyData );
    virtual int32_t     NPP_Write( NPP instance, NPStream* stream, int32_t offset,
                                   int32_t len, void* buffer );
    virtual int32_t     NPP_WriteReady( NPP instance, NPStream* stream );
    virtual NPError     NPP_GetValue( NPP instance, NPPVariable variable, void *ret_value );
    virtual int16_t     NPP_HandleEvent( NPP instance, void* event );

    virtual NPError     NPP_SetWindow( XPlugin_Impl* );
    virtual NPError     NPP_Destroy( XPlugin_Impl*, NPSavedData** save );

    void                drawView( XPlugin_Impl* );
private:
    sal_Bool                retrieveFunction( const char* i_pName, void** i_ppFunc ) const;
    DECL_LINK_TYPED( NullTimerHdl, Timer*, void );

private:
    CFBundleRef         m_xBundle;
    oslModule           m_hPlugLib;
    NPPluginFuncs       m_aNPPfuncs;

    // timer for sending nullEvents
    AutoTimer*                      m_pNullTimer;
    std::list< XPlugin_Impl* >      m_aNullEventClients;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
