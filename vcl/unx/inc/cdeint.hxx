/*************************************************************************
 *
 *  $RCSfile: cdeint.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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
#ifndef _SV_CDEINT_HXX
#define _SV_CDEINT_HXX

#ifndef _SV_DTINT_HXX
#include <dtint.hxx>
#endif

#include <prex.h>
#define Boolean XLIB_Boolean
#define Window  XLIB_Window
#include <X11/Intrinsic.h>
#include <dt/dt.h>
#include <dt/action.h>
#include <dt/wsm.h>
#undef Boolean
#undef Window
#include <postx.h>

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
class CDEIntegrator : public DtIntegrator
{
    friend DtIntegrator* DtIntegrator::CreateDtIntegrator( SalFrame* );
private:
    static void*        pDtSvcLib;
    static void*        pXmLib;
    static void*        pMrmLib;
    static void*        pXtLib;
    static void*        pttLib;
    static int          nRefCount;
    static char*        pFallbackRes[];

    // function pointers
    // from DtSvc
    static XLIB_Boolean (*pDtAppInitialize)
        ( XtAppContext, Display*, Widget, char*, char* );
    static void         (*pDtDtsLoadDataTypes)();
    static void         (*pDtDtsRelease)();
    static char*        (*pDtDtsFileToAttributeValue)
        (const char*,const char*);
    static void         (*pDtDtsFreeAttributeValue)( char* );
    static DtActionInvocationID (*pDtActionInvoke)
        ( Widget, char*, DtActionArg*, int, char*, char*, char*, int,
          DtActionCallbackProc, XtPointer );
    static Status       (*pDtWsmGetWorkspaceInfo)( Display*, XLIB_Window, Atom,
                                                   DtWsmWorkspaceInfo** );
    static void         (*pDtWsmFreeWorkspaceInfo)( DtWsmWorkspaceInfo* );
    static Status       (*pDtWsmGetWorkspaceList)( Display*, XLIB_Window, Atom**,
                                                   int* );
    static Status       (*pDtWsmGetCurrentWorkspace)( Display*, XLIB_Window root, Atom* );
    static Status       (*pDtWsmGetWorkspacesOccupied)( Display*, XLIB_Window, Atom**, unsigned long* );

    // from Mrm
    static void         (*pMrmInitialize)();

    // from Xm
    static WidgetClass* CDEIntegrator::pxmDrawingAreaWidgetClass;
    static WidgetClass* CDEIntegrator::pxmRowColumnWidgetClass;
    static WidgetClass* CDEIntegrator::pxmPushButtonWidgetClass;

    // from Xt
    static void         (*pXtToolkitInitialize)();
    static XtAppContext (*pXtCreateApplicationContext)();
    static Widget       (*pXtAppCreateShell)( char*, char*, WidgetClass,
                                       Display*, ArgList, Cardinal );
    static Widget       (*pXtVaCreateManagedWidget)( char*, WidgetClass,
                                                     Widget, ... );
    static void         (*pXtDisplayInitialize)( XtAppContext, Display*,
                                                 char*, char*,
                                                 XrmOptionDescRec*, Cardinal,
                                                 int*, char**);
    static Widget       (*pXtSetLanguageProc)( XtAppContext, XtLanguageProc, XtPointer );
    static void         (*pXtAppSetFallbackResources)( XtAppContext, char** );
    static Widget       (*pXtAppInitialize)(XtAppContext*, char*,
                                            XrmOptionDescList,
                                            Cardinal, int*, char**, char**,
                                            ArgList, Cardinal );
    static void         (*pXtRealizeWidget)( Widget );
    static void         (*pXtUnrealizeWidget)( Widget );
    static XLIB_Boolean (*pXtIsRealized)( Widget );
    static void         (*pXtConfigureWidget)
        ( Widget, Position, Position, Dimension, Dimension, Dimension );
    static void         (*pXtAppProcessEvent)( XtAppContext, XtInputMask );
    static XtInputMask  (*pXtAppPending)( XtAppContext );
    static WidgetClass* pAppShellClass;


    XtAppContext        maAppContext;
    Widget              maAppWidget;

    CDEIntegrator( SalFrame* );

    void GlobalInit();
    void GlobalDeInit();

    void InvokeAction( const String&, const String& );

public:
    virtual ~CDEIntegrator();

    virtual BOOL StartProcess( String&, String&, const String& rDir = String() );
    virtual BOOL GetSystemLook( SystemLookInfo& rInfo );
};

#endif
