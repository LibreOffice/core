/*************************************************************************
 *
 *  $RCSfile: cdeint.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-07 18:42:17 $
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
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <salunx.h>
#include <saldisp.hxx>
#include <salbmp.hxx>
#include <salframe.hxx>
#include <cdeint.hxx>
#include <soicon.hxx>
#include <strhelper.hxx>
#include <tools/debug.hxx>

void*   CDEIntegrator::pDtSvcLib = 0;
void*   CDEIntegrator::pXtLib = 0;
void*   CDEIntegrator::pXmLib = 0;
void*   CDEIntegrator::pMrmLib = 0;
void*   CDEIntegrator::pttLib = 0;
int     CDEIntegrator::nRefCount = 0;
char*   CDEIntegrator::pFallbackRes[] = {
    "title:                 OfficeCDEIntegrationShell",
    NULL
};
static char *ppDummyArgv[] = { "soffice.bin", NULL };
static int  nDummyArgc = 1;

// function pointers
// from DtSvc
XLIB_Boolean    (*CDEIntegrator::pDtAppInitialize)
    ( XtAppContext, Display*, Widget, char*, char* ) = 0;
void            (*CDEIntegrator::pDtDtsLoadDataTypes)() = 0;
void            (*CDEIntegrator::pDtDtsRelease)() = 0;
char*           (*CDEIntegrator::pDtDtsFileToAttributeValue)
    (const char*,const char*) = 0;
void            (*CDEIntegrator::pDtDtsFreeAttributeValue)( char* ) = 0;
Status      (*CDEIntegrator::pDtWsmGetWorkspaceInfo)( Display*, XLIB_Window, Atom,
                                                      DtWsmWorkspaceInfo** ) = 0;
void        (*CDEIntegrator::pDtWsmFreeWorkspaceInfo)( DtWsmWorkspaceInfo* ) = 0;
Status      (*CDEIntegrator::pDtWsmGetWorkspaceList)( Display*, XLIB_Window, Atom**,
                                                      int* ) = 0;
Status      (*CDEIntegrator::pDtWsmGetCurrentWorkspace)( Display*, XLIB_Window root, Atom* ) = 0;
Status      (*CDEIntegrator::pDtWsmGetWorkspacesOccupied)( Display*, XLIB_Window, Atom**, unsigned long* ) = 0;

// from Mrm
void            (*CDEIntegrator::pMrmInitialize)() = 0;

// from Xm
WidgetClass* CDEIntegrator::pxmDrawingAreaWidgetClass = 0;
WidgetClass* CDEIntegrator::pxmRowColumnWidgetClass = 0;
WidgetClass* CDEIntegrator::pxmPushButtonWidgetClass = 0;

// from Xt
void            (*CDEIntegrator::pXtToolkitInitialize)() = 0;
XtAppContext    (*CDEIntegrator::pXtCreateApplicationContext)() = 0;
Widget          (*CDEIntegrator::pXtAppCreateShell)
    ( char*, char*, WidgetClass, Display*, ArgList, Cardinal ) = 0;
Widget          (*CDEIntegrator::pXtVaCreateManagedWidget)
    ( char*, WidgetClass, Widget, ... ) = 0;
void            (*CDEIntegrator::pXtDisplayInitialize)
    ( XtAppContext, Display*, char*, char*, XrmOptionDescRec*, Cardinal,
      int*, char**) = 0;
void            (*CDEIntegrator::pXtAppSetFallbackResources)
    ( XtAppContext, char** ) = 0;
Widget          (*CDEIntegrator::pXtAppInitialize)
    (XtAppContext*, char*, XrmOptionDescList, Cardinal, int*, char**, char**,
     ArgList, Cardinal ) = 0;
Widget          (*CDEIntegrator::pXtSetLanguageProc)
    ( XtAppContext, XtLanguageProc, XtPointer ) = 0;
DtActionInvocationID (*CDEIntegrator::pDtActionInvoke)
    ( Widget, char*, DtActionArg*, int, char*, char*, char*, int,
      DtActionCallbackProc, XtPointer ) = 0;
void            (*CDEIntegrator::pXtRealizeWidget)( Widget ) = 0;
void            (*CDEIntegrator::pXtUnrealizeWidget)( Widget ) = 0;
XLIB_Boolean            (*CDEIntegrator::pXtIsRealized)( Widget ) = 0;
void            (*CDEIntegrator::pXtConfigureWidget)
    ( Widget, Position, Position, Dimension, Dimension, Dimension ) = 0;
void            (*CDEIntegrator::pXtAppProcessEvent)
    ( XtAppContext, XtInputMask ) = 0;
XtInputMask     (*CDEIntegrator::pXtAppPending)( XtAppContext ) = 0;
WidgetClass*    CDEIntegrator::pAppShellClass = 0;


CDEIntegrator::CDEIntegrator( SalFrame* pFrame ) :
        DtIntegrator( pFrame ),
        maAppWidget( 0 )
{
    meType = DtCDE;
    if( ! nRefCount )
        GlobalInit();
    nRefCount++;

    maAppWidget = pFrame->maFrameData.GetWidget();
    SalDisplay *pDisp = pFrame->maFrameData.GetDisplay();
    SalXLib    *pXlib = pDisp->GetXLib();
    maAppContext = pXlib->GetAppContext ();

    pDtAppInitialize( maAppContext, mpDisplay, maAppWidget,
                      ppDummyArgv[ 0 ], "Office" );

    pDtDtsLoadDataTypes();
}

CDEIntegrator::~CDEIntegrator()
{
    nRefCount--;
    if( ! nRefCount )
        GlobalDeInit();
}

void CDEIntegrator::GlobalInit()
{
    if( ! pDtSvcLib )
        pDtSvcLib = _LoadLibrary( "libDtSvc.so" );
    if( ! pttLib )
        pttLib = _LoadLibrary( "libtt.so" );
    if( ! pXmLib )
        pXmLib = _LoadLibrary( "libXm.so" );
    if( ! pMrmLib )
        pMrmLib = _LoadLibrary( "libMrm.so" );
    if( ! pXtLib )
        pXtLib  = _LoadLibrary( "libXt.so" );

    if( pDtSvcLib && pXtLib && pttLib && pXmLib && pMrmLib )
    {
        bSymbolLoadFailed = FALSE;

        pDtAppInitialize     = (XLIB_Boolean (*)( XtAppContext, Display*,
                                                  Widget, char*, char* ))
            _LoadSymbol( pDtSvcLib, "DtAppInitialize" );
        pDtDtsLoadDataTypes  = (void (*)())
            _LoadSymbol( pDtSvcLib, "DtDtsLoadDataTypes" );
        pDtDtsRelease        = (void    (*)())
            _LoadSymbol( pDtSvcLib, "DtDtsRelease" );
        pDtDtsFileToAttributeValue = (char* (*)(const char*,const char*))
            _LoadSymbol( pDtSvcLib, "DtDtsFileToAttributeValue" );
        pDtDtsFreeAttributeValue = (void (*)( char* ))
            _LoadSymbol( pDtSvcLib, "DtDtsFreeAttributeValue" );
        pDtActionInvoke = (DtActionInvocationID(*)
                           ( Widget, char*,DtActionArg*, int, char*,
                             char*, char*, int,
                             DtActionCallbackProc, XtPointer ))
            _LoadSymbol( pDtSvcLib, "DtActionInvoke" );
        pDtWsmGetWorkspaceInfo = (Status(*)( Display*, XLIB_Window, Atom,
                                             DtWsmWorkspaceInfo** ))
            _LoadSymbol( pDtSvcLib, "DtWsmGetWorkspaceInfo" );
        pDtWsmFreeWorkspaceInfo =  (void(*)( DtWsmWorkspaceInfo* ))
            _LoadSymbol( pDtSvcLib, "DtWsmFreeWorkspaceInfo" );
        pDtWsmGetWorkspaceList = (Status(*)( Display*, XLIB_Window, Atom**, int* ))
            _LoadSymbol( pDtSvcLib, "DtWsmGetWorkspaceList" );
        pDtWsmGetCurrentWorkspace = (Status(*)( Display*, XLIB_Window, Atom*))
            _LoadSymbol( pDtSvcLib, "DtWsmGetCurrentWorkspace" );
        pDtWsmGetWorkspacesOccupied = (Status(*)( Display*, XLIB_Window, Atom**, unsigned long* ))
            _LoadSymbol( pDtSvcLib, "DtWsmGetWorkspacesOccupied" );

        pXtToolkitInitialize        = (void (*)())
            _LoadSymbol( pXtLib, "XtToolkitInitialize" );
        pXtCreateApplicationContext = (XtAppContext (*)())
            _LoadSymbol( pXtLib, "XtCreateApplicationContext" );
        pXtAppCreateShell           = (Widget (*)( char*, char*, WidgetClass,
                                                   Display*, ArgList,
                                                   Cardinal ))
            _LoadSymbol( pXtLib, "XtAppCreateShell" );
        pXtDisplayInitialize = (void (*)( XtAppContext, Display*, char*,
                                          char*, XrmOptionDescRec*, Cardinal,
                                          int*, char**) )
            _LoadSymbol( pXtLib, "XtDisplayInitialize" );

        pXtVaCreateManagedWidget = (Widget (*)( char*, WidgetClass, Widget, ... ))
            _LoadSymbol( pXtLib, "XtVaCreateManagedWidget" );
        pXtSetLanguageProc = (Widget (*)( XtAppContext, XtLanguageProc, XtPointer ))
            _LoadSymbol( pXtLib, "XtSetLanguageProc" );
        pXtAppSetFallbackResources = (void (*)( XtAppContext, char** ))
            _LoadSymbol( pXtLib, "XtAppSetFallbackResources" );
        pXtAppInitialize = (Widget (*)(XtAppContext*, char*, XrmOptionDescList,
                                 Cardinal, int*, char**, char**, ArgList,
                                 Cardinal ))
            _LoadSymbol( pXtLib, "XtAppInitialize" );
        pXtRealizeWidget = (void(*)( Widget ))
            _LoadSymbol( pXtLib, "XtRealizeWidget" );
        pXtIsRealized = (XLIB_Boolean(*)( Widget ))
            _LoadSymbol( pXtLib, "XtIsRealized" );
        pXtUnrealizeWidget = (void(*)( Widget ))
            _LoadSymbol( pXtLib, "XtUnrealizeWidget" );
        pXtAppProcessEvent = (void(*)( XtAppContext, XtInputMask ))
            _LoadSymbol( pXtLib, "XtAppProcessEvent" );
        pXtAppPending = (XtInputMask(*)( XtAppContext ))
            _LoadSymbol( pXtLib, "XtAppPending" );
        pXtConfigureWidget = (void (*)( Widget, Position, Position, Dimension,
                                        Dimension, Dimension ))
            _LoadSymbol( pXtLib, "XtConfigureWidget" );
        pAppShellClass =
            (WidgetClass*)_LoadSymbol( pXtLib, "applicationShellWidgetClass" );
        pMrmInitialize = (void(*)())
            _LoadSymbol( pMrmLib, "MrmInitialize" );
        pxmDrawingAreaWidgetClass =
            (WidgetClass*)_LoadSymbol( pXmLib, "xmDrawingAreaWidgetClass" );
        pxmRowColumnWidgetClass =
            (WidgetClass*)_LoadSymbol( pXmLib, "xmRowColumnWidgetClass" );
        pxmPushButtonWidgetClass =
            (WidgetClass*)_LoadSymbol( pXmLib, "xmPushButtonWidgetClass" );
    }
    else
    {
        if( pXtLib )
            dlclose( pXtLib );
        if( pXmLib )
            dlclose( pXmLib );
        if( pMrmLib )
            dlclose( pMrmLib );
        if( pttLib )
            dlclose( pttLib );
        if( pDtSvcLib )
            dlclose( pDtSvcLib );
        pttLib = pMrmLib = pXmLib = pXtLib = pDtSvcLib = 0;
    }
}

void CDEIntegrator::GlobalDeInit()
{
    pDtDtsRelease();

    if( pXtLib )
        dlclose( pXtLib );
    if( pXmLib )
        dlclose( pXmLib );
    if( pMrmLib )
        dlclose( pMrmLib );
    if( pttLib )
        dlclose( pttLib );
    if( pDtSvcLib )
        dlclose( pDtSvcLib );
    pttLib = pMrmLib = pXmLib = pXtLib = pDtSvcLib = 0;
}

void CDEIntegrator::InvokeAction( const String& rAction, const String& rFiles )
{
#if 1
    String aActionLine( RTL_CONSTASCII_STRINGPARAM( "/usr/dt/bin/dtaction " ), RTL_TEXTENCODING_ASCII_US );
    aActionLine += rAction;
    aActionLine += ' ';
    aActionLine += rFiles;
    DtIntegrator::LaunchProcess( aActionLine );
#else
    int nTokens = GetCommandLineTokenCount( rFiles ), i;
    DtActionArg* pArgs = new DtActionArg[ nTokens ];
    char** pStrings = new char*[nTokens];
    for( i = 0; i < nTokens ; i++ )
    {
        pStrings[ i ] = strdup( GetCommandLineToken( i, rFiles ).GetStr() );
        pArgs[ i ].argClass = DtACTION_FILE;
        pArgs[ i ].u.file.name = pStrings[ i ];
    }

    pid_t nPID = fork();
    if( ! nPID )
        // child
    {
        // if the action fails for some reason, CDE tries to raise
        // an error box which leads to a SIGSEGV somewhere
        // in Motif (probably because we are not a real Motif app)
        // so execute this in its own process. if it dies, so what ?

        // this is not perfectly safe though. The second process
        // can cause an X Error wich comes back through our connection
        // at an unpredictable time. Some Actions do this reproducable
        // eg SDTAudio
        pDtActionInvoke( maAppWidget, const_cast<char*>(rAction.GetStr()), pArgs, nTokens,
                         NULL, NULL, NULL, True, NULL, NULL );
        _exit(0);
    }
    else
        // parent
        if( nPID > 0 )
            waitpid( nPID, NULL, 0 );

    // dtactioninvoke pops up the widget, pop it down again
    if( pXtIsRealized( maAppWidget ) )
        pXtUnrealizeWidget( maAppWidget );

    for( i = 0; i < nTokens; i++ )
        free( pStrings[i] );
    delete pStrings;
    delete pArgs;
#endif
}

BOOL CDEIntegrator::StartProcess( String& rFile, String& rParams, const String& rDir )
{
    String aFiles;
    if( rFile.GetChar( 0 ) != '"' )
    {
        aFiles  = '\"';
        aFiles += rFile;
        aFiles += '\"';
    }
    else
        aFiles = rFile;

    if( rParams.Len() )
    {
        aFiles += ' ';
        aFiles += rParams;
    }
    // first try to launch it
    if( LaunchProcess( aFiles, rDir ) )
        return TRUE;

    // if not successfull it must be a file of whatever type
    // look for an apropriate action
    ByteString aFile( rFile, gsl_getSystemTextEncoding() );
    char* pAttrValue = pDtDtsFileToAttributeValue( aFile.GetBuffer(), "ACTIONS" );
    if( pAttrValue )
    {
        // there is an action for this file. invoke the first one
        // (since we do not know which one
        String aAction( pAttrValue, strlen( pAttrValue ), gsl_getSystemTextEncoding() );
        pDtDtsFreeAttributeValue( pAttrValue );
        aAction =
            WhitespaceToSpace( aAction.GetToken( 0, ',' ) ).GetToken( 0, ' ' );
        if( ! aAction.Len() )
            return FALSE;

        BOOL bPreviousState =
            mpSalDisplay->GetXLib()->GetIgnoreXErrors();
        mpSalDisplay->GetXLib()->SetIgnoreXErrors( TRUE );

        InvokeAction( aAction, aFiles );

        XSync( mpDisplay, False );
        mpSalDisplay->GetXLib()->SetIgnoreXErrors( bPreviousState );

        return TRUE;
    }

    return FALSE;
}


static int getHexDigit( const char c )
{
    if( c >= '0' && c <= '9' )
        return (int)(c-'0');
    else if( c >= 'a' && c <= 'f' )
        return (int)(c-'a'+10);
    else if( c >= 'A' && c <= 'F' )
        return (int)(c-'A'+10);
    return -1;
}


BOOL CDEIntegrator::GetSystemLook( SystemLookInfo& rInfo )
{
    static Color aColors[ 8 ];
    static sal_Bool bRead = sal_False;

    if( ! bRead )
    {
        // get used palette from xrdb
        char **ppStringList = 0;
        int nStringCount;
        XTextProperty aTextProperty;
        aTextProperty.value = 0;
        int i;

        static Atom nResMgrAtom = XInternAtom( mpDisplay, "RESOURCE_MANAGER", False );

        if( XGetTextProperty( mpDisplay,
                              RootWindow( mpDisplay, 0 ),
                              &aTextProperty,
                              nResMgrAtom )
            && aTextProperty.value
            && XTextPropertyToStringList( &aTextProperty, &ppStringList, &nStringCount )
            )
        {
            // format of ColorPalette resource:
            // *n*ColorPalette: palettefile

            ByteString aLines;
            for( i=0; i < nStringCount; i++ )
                aLines += ppStringList[i];
            for( i = aLines.GetTokenCount( '\n' )-1; i >= 0; i-- )
            {
                ByteString aLine = aLines.GetToken( i, '\n' );
                int nIndex = aLine.Search( "ColorPalette" );
                if( nIndex != STRING_NOTFOUND )
                {
                    int nPos = nIndex;

                    nIndex+=12;
                    const char* pStr = aLine.GetBuffer() +nIndex;
                    while( *pStr && isspace( *pStr ) && *pStr != ':' )
                    {
                        pStr++;
                        nIndex++;
                    }
                    if( *pStr != ':' )
                        continue;
                    pStr++, nIndex++;
                    for( ; *pStr && isspace( *pStr ); pStr++, nIndex++ )
                        ;
                    if( ! *pStr )
                        continue;
                    int nIndex2 = nIndex;
                    for( ; *pStr && ! isspace( *pStr ); pStr++, nIndex2++ )
                        ;
                    ByteString aPaletteFile( aLine.Copy( nIndex, nIndex2 - nIndex ) );
                    // extract number before ColorPalette;
                    for( ; nPos >= 0 && aLine.GetChar( nPos ) != '*'; nPos-- )
                        ;
                    nPos--;
                    for( ; nPos >= 0 && aLine.GetChar( nPos ) != '*'; nPos-- )
                        ;
                    int nNumber = aLine.Copy( ++nPos ).ToInt32();

                    DBG_TRACE2( "found palette %d in resource \"%s\"", nNumber, aLine.GetBuffer() );

                    // found no documentation what this number actually means;
                    // might be the screen number. 0 seems to be the right one
                    // in most cases.
                    if( nNumber )
                        continue;

                    DBG_TRACE1( "Palette file is \"%s\".\n", aPaletteFile.GetBuffer() );

                    String aPath( aHomeDir );
                    aPath.AppendAscii( "/.dt/palettes/" );
                    aPath += String( aPaletteFile, gsl_getSystemTextEncoding() );

                    SvFileStream aStream( aPath, STREAM_READ );
                    if( ! aStream.IsOpen() )
                    {
                        aPath = String::CreateFromAscii( "/usr/dt/palettes/" );
                        aPath += String( aPaletteFile, gsl_getSystemTextEncoding() );
                        aStream.Open( aPath, STREAM_READ );
                        if( ! aStream.IsOpen() )
                            continue;
                    }

                    ByteString aBuffer;
                    for( nIndex = 0; nIndex < 8; nIndex++ )
                    {
                        aStream.ReadLine( aBuffer );
                        // format is "#RRRRGGGGBBBB"

                        DBG_TRACE1( "\t\"%s\".\n", aBuffer.GetBuffer() );

                        if( aBuffer.Len() )
                        {
                            const char* pArr = (const char*)aBuffer.GetBuffer()+1;
                            aColors[nIndex] = Color(
                                getHexDigit( pArr[1] )
                                | ( getHexDigit( pArr[0] ) << 4 ),
                                getHexDigit( pArr[5] )
                                | ( getHexDigit( pArr[4] ) << 4 ),
                                getHexDigit( pArr[9] )
                                | ( getHexDigit( pArr[8] ) << 4 )
                                );

                            DBG_TRACE1( "\t\t%lx\n", aColors[nIndex].GetColor() );
                        }
                    }

                    break;
                }
            }
        }

        if( ppStringList )
            XFreeStringList( ppStringList );
        if( aTextProperty.value )
            XFree( aTextProperty.value );
    }

    rInfo.windowActiveStart = aColors[0];
    rInfo.windowActiveEnd = aColors[0];
    rInfo.activeBorder = aColors[0];

    rInfo.windowInactiveStart = aColors[1];
    rInfo.windowInactiveEnd = aColors[1];
    rInfo.inactiveBorder = aColors[1];

    rInfo.activeForeground =
        aColors[ 0 ].GetBlue() < 128        ||
        aColors[ 0 ].GetGreen() < 128       ||
        aColors[ 0 ].GetRed() < 128
        ? Color( COL_WHITE ) : Color( COL_BLACK );
    rInfo.inactiveForeground =
        aColors[ 1 ].GetBlue() < 128        ||
        aColors[ 1 ].GetGreen() < 128       ||
        aColors[ 1 ].GetRed() < 128
        ? Color( COL_WHITE ) : Color( COL_BLACK );
    rInfo.foreground    = rInfo.inactiveForeground;
    rInfo.background    = aColors[ 1 ];

    return TRUE;
}
