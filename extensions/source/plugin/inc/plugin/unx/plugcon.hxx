/*************************************************************************
 *
 *  $RCSfile: plugcon.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-28 12:38:03 $
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
#ifndef _PLUGCON_HXX
#define _PLUGCON_HXX

#include <stdarg.h>
#include <string.h>

#include <list>

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _MEDIATOR_HXX
#include <plugin/unx/mediator.hxx>
#endif

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
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/IntrinsicP.h>     /* Intrinsics Definitions*/
#include <X11/StringDefs.h>    /* Standard Name-String definitions*/
#if defined USE_MOTIF
#include <Xm/DrawingA.h>
#else
#include <X11/Xaw/Label.h>
#endif
#include <X11/Xatom.h>
#define XP_UNIX
#include <stdio.h>
#ifndef _NPAPI_H_
#include <npsdk/npupp.h>
#include <npsdk/npapi.h>
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

    int nArg;
    char** argn;
    char** argv;
    char* pArgnBuf;
    char* pArgvBuf;
    NPSavedData aData;

    ConnectorInstance( NPP inst, char* type,
                       int args, char* pargnbuf, ULONG nargnbytes,
                       char* pargvbuf, ULONG nargvbytes,
                       char* savedata, ULONG savebytes );
    ~ConnectorInstance();
};

class PluginConnector;

DECLARE_LIST( NPStreamList, NPStream* );
DECLARE_LIST( InstanceList, ConnectorInstance* );
DECLARE_LIST( PluginConnectorList, PluginConnector* );

class PluginConnector : public Mediator
{
    friend NPError NPN_DestroyStream( NPP, NPStream*, NPError );
    friend NPError NPN_NewStream( NPP, NPMIMEType, const char*,
                                  NPStream** );
protected:
    NAMESPACE_VOS(OMutex)               m_aUserEventMutex;

    static PluginConnectorList          allConnectors;

    DECL_LINK( NewMessageHdl, Mediator* );
    DECL_LINK( WorkOnNewMessageHdl, Mediator* );

    NPStreamList    m_aNPWrapStreams;
    InstanceList    m_aInstances;

    ULONG   FillBuffer( char*&, char*, ULONG, va_list );
public:
    PluginConnector( int nSocket );
    ~PluginConnector();

    virtual MediatorMessage* WaitForAnswer( ULONG nMessageID );
    MediatorMessage*    Transact( char*, ULONG, ... );
    MediatorMessage*    Transact( UINT32, ... );
    void                Respond( ULONG nID, char*, ULONG, ... );
    ULONG               Send( UINT32, ... );

    UINT32  GetStreamID( NPStream* pStream );
    UINT32  GetNPPID( NPP );

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

char* GetCommandName( CommandAtoms );

#define POST_STRING( x ) x ? x : const_cast<char*>(""), x ? strlen(x) : 1

#endif // _PLUGCON_HXX
