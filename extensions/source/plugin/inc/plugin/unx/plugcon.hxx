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
#ifndef _PLUGCON_HXX
#define _PLUGCON_HXX

#include <stdarg.h>
#include <string.h>

#include <list>
#include <vector>
#include <plugin/unx/mediator.hxx>

#if defined SOLARIS
#define USE_MOTIF
#endif

#define Window      XLIB_Window
#define Font        XLIB_Font
#define KeyCode     XLIB_KeyCode
#define Time        XLIB_Time
#define Cursor      XLIB_Cursor
#define Region      XLIB_Region
#define String      XLIB_String
#define Boolean     XLIB_Boolean
#define XPointer    XLIB_XPointer
#include <X11/Xlib.h>
extern "C" {
#include <X11/Intrinsic.h>
}
#include <X11/Shell.h>
#include <X11/IntrinsicP.h>     /* Intrinsics Definitions*/
#include <X11/StringDefs.h>    /* Standard Name-String definitions*/
#if defined USE_MOTIF
#include <Xm/DrawingA.h>
#else
#  if defined DISABLE_XAW
#     include <X11/Composite.h>
#  else
#     include <X11/Xaw/Label.h>
#  endif
#endif
#include <X11/Xatom.h>
#ifndef XP_UNIX
#    define XP_UNIX
#endif
#define MOZ_X11
#include <stdio.h>
#ifdef SYSTEM_MOZILLA
#ifndef OJI
#  define OJI
#endif
#define MOZ_X11
#endif

//http://qa.openoffice.org/issues/show_bug.cgi?id=82545
//https://bugzilla.mozilla.org/show_bug.cgi?id=241262
#ifdef UNIX
#  ifndef _UINT32
#    if defined(__alpha) || defined(__LP64__)
       typedef unsigned int uint32;
#    else  /* __alpha */
       typedef unsigned long uint32;
#    endif
#    define _UINT32
#  endif
#  ifndef _INT32
#    if defined(__alpha) || defined(__LP64__)
       typedef int int32;
#    else  /* __alpha */
       typedef long int32;
#    endif
#    define _INT32
#  endif
#endif

#ifndef _NPAPI_H_
extern "C" {
#include <npsdk/npupp.h>
}
#include <npsdk/npapi.h>

#if NP_VERSION_MINOR < 17
// compatibility hack: compile with older NPN api header, but define
// some later introduced constants
// for gcc 3
#define NP_ABI_MASK 0x10000000
#define NPNVSupportsXEmbedBool ((NPNVariable)14)
#define NPPVpluginNeedsXEmbed  ((NPPVariable)14)
#define NPNVToolkit            ((int)(13 | NP_ABI_MASK))
#define NPNVGtk12 1
#define NPNVGtk2  2
#endif
#endif

#ifdef ENABLE_GTK
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#else
#define GtkWidget void
#endif

#undef Window
#undef Font
#undef KeyCode
#undef Time
#undef Cursor
#undef String
#undef Region
#undef Boolean
#undef XPointer

class ConnectorInstance
{
public:
    NPP                         instance;
    NPWindow                    window;
    NPSetWindowCallbackStruct   ws_info;
    char*                       pMimeType;
    void*                       pShell;
    void*                       pWidget;
    void*                       pForm;

    GtkWidget*                  pGtkWindow;
    GtkWidget*                  pGtkWidget;

    bool                        bShouldUseXEmbed;

    int nArg;
    char** argn;
    char** argv;
    char* pArgnBuf;
    char* pArgvBuf;
    NPSavedData aData;

    ConnectorInstance( NPP inst, char* type,
                       int args, char* pargnbuf, sal_uLong nargnbytes,
                       char* pargvbuf, sal_uLong nargvbytes,
                       char* savedata, sal_uLong savebytes );
    ~ConnectorInstance();
};

class PluginConnector : public Mediator
{
protected:
    vos::OMutex               m_aUserEventMutex;

    static std::vector<PluginConnector*>  allConnectors;

    DECL_LINK( NewMessageHdl, Mediator* );
    DECL_LINK( WorkOnNewMessageHdl, Mediator* );

    std::vector<NPStream*>              m_aNPWrapStreams;
    std::vector<ConnectorInstance*>     m_aInstances;

    sal_uLong   FillBuffer( char*&, const char*, sal_uLong, va_list );
public:
    PluginConnector( int nSocket );
    ~PluginConnector();

    virtual MediatorMessage* WaitForAnswer( sal_uLong nMessageID );
    MediatorMessage*    Transact( const char*, sal_uLong, ... );
    MediatorMessage*    Transact( sal_uInt32, ... );
    void                Respond( sal_uLong nID, char*, sal_uLong, ... );
    sal_uLong               Send( sal_uInt32, ... );

    static const sal_uInt32 UnknownStreamID = 0xffffffff;
    static const sal_uInt32 UnknownNPPID = 0xffffffff;

    sal_uInt32  GetStreamID( NPStream* pStream );
    sal_uInt32  GetNPPID( NPP );

    std::vector<NPStream*>& getStreamList() { return m_aNPWrapStreams; }

    NPError GetNPError( MediatorMessage* pMes )
    {
        NPError* pErr = (NPError*)pMes->GetBytes();
        NPError aErr = *pErr;
        delete [] pErr;
        return aErr;
    }

    void CallWorkHandler()
    {
        LINK( this, PluginConnector, WorkOnNewMessageHdl ).
            Call( (Mediator*)this );
    }

    ConnectorInstance* getInstance( NPP );
    ConnectorInstance* getInstanceById( sal_uInt32 );
};

enum CommandAtoms
{
        eNPN_GetURL,
        eNPN_GetURLNotify,
        eNPN_DestroyStream,
        eNPN_NewStream,
        eNPN_PostURLNotify,
        eNPN_PostURL,
        eNPN_RequestRead,
        eNPN_Status,
        eNPN_Version,
        eNPN_Write,
        eNPN_UserAgent,

        eNPP_DestroyStream,
        eNPP_Destroy,
        eNPP_DestroyPhase2,
        eNPP_NewStream,
        eNPP_New,
        eNPP_SetWindow,
        eNPP_StreamAsFile,
        eNPP_URLNotify,
        eNPP_WriteReady,
        eNPP_Write,
        eNPP_GetMIMEDescription,
        eNPP_Initialize,
        eNPP_Shutdown,

        eMaxCommand
};

const char* GetCommandName( CommandAtoms );

#define POST_STRING( x ) x ? x : const_cast<char*>(""), x ? strlen(x) : 1

#endif // _PLUGCON_HXX
