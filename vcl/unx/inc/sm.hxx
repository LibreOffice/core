/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sm.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-01 10:36:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _VCL_SM_HXX
#define _VCL_SM_HXX

#include <tools/link.hxx>
#include <salunx.h>
#include <X11/SM/SMlib.h>
#include <salsession.hxx>

class SessionManagerClient
{
    static SmcConn          aSmcConnection;
    static ByteString       aClientID;
    static bool         bDocSaveDone;

    static void SaveYourselfProc(       SmcConn connection,
                                        SmPointer client_data,
                                        int save_type,
                                        Bool shutdown,
                                        int interact_style,
                                        Bool fast );
    static void DieProc(                SmcConn connection,
                                        SmPointer client_data );
    static void SaveCompleteProc(       SmcConn connection,
                                        SmPointer client_data );
    static void ShutdownCanceledProc(   SmcConn connection,
                                        SmPointer client_data );
    static void InteractProc(           SmcConn connection,
                                        SmPointer clientData );

    static const ByteString& getPreviousSessionID();

    DECL_STATIC_LINK( SessionManagerClient, ShutDownHdl, void* );
    DECL_STATIC_LINK( SessionManagerClient, ShutDownCancelHdl, void* );
    DECL_STATIC_LINK( SessionManagerClient, SaveYourselfHdl, void* );
    DECL_STATIC_LINK( SessionManagerClient, InteractionHdl, void* );
public:
    static VCL_DLLPUBLIC void open(); // needed by other plugins, so export
    static void close();

    static bool checkDocumentsSaved();
    static bool queryInteraction();
    static void saveDone();
    static void interactionDone();

    static String getExecName();
    static VCL_DLLPUBLIC const ByteString&  getSessionID();
};

class SalFrame;

class IceSalSession : public SalSession
{
public:
    IceSalSession();
    virtual ~IceSalSession();

    virtual void queryInteraction();
    virtual void interactionDone();
    virtual void saveDone();
    virtual bool cancelShutdown();

    static void handleOldX11SaveYourself( SalFrame* pFrame );
};

#endif
