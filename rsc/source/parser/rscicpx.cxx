/*************************************************************************
 *
 *  $RCSfile: rscicpx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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

/****************** I N C L U D E S **************************************/
#pragma hdrstop

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>

#include <tools/rc.h>
#include <tools/vclrsc.hxx>

#ifndef _RSCMGR_HXX
#include <rscmgr.hxx>
#endif
#ifndef _RSCCLASS_HXX
#include <rscclass.hxx>
#endif
#ifndef _RSCCONT_HXX
#include <rsccont.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif
#ifndef _RSCSFX_HXX
#include <rscsfx.hxx>
#endif

#include "rsclex.hxx"
#include <rscyacc.yxx.h>

/*************************************************************************
|*    RscTypCont::InsWinBit()
*************************************************************************/
void RscTypCont::InsWinBit( RscTop * pClass, const ByteString & rName,
                            HASHID nVal )
{
    RscClient * pClient;

    // Clientvariablen einfuegen
    aBaseLst.Insert(
        pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                 &aWinBits, nVal ),
        LIST_APPEND );
    HASHID nId = aNmTb.Put( rName.GetBuffer(), VARNAME );
    pClass->SetVariable( nId, pClient, NULL,
                         VAR_NODATAINST, 0, nWinBitVarId );
}

#define INS_WINBIT( pClass, WinBit )        \
    InsWinBit( pClass, #WinBit, n##WinBit##Id );

/*************************************************************************
|*    RscTypCont::InitClassMgr()
*************************************************************************/
RscTop * RscTypCont::InitClassMgr()
{
    RscTop      *   pClassMgr;
    RscBaseCont *   pClass;
    HASHID          nId;

    aBaseLst.Insert( pClass =
        new RscBaseCont( HASH_NONAME, RSC_NOTYPE, NULL, FALSE ),
        LIST_APPEND );

    nId = pHS->Insert( "Resource" );
    pClassMgr = new RscMgr( nId, RSC_RESOURCE, pClass );
    aNmTb.Put( nId, CLASSNAME, pClassMgr );
    pClassMgr->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    {
        RscContWriteSrc *   pCont;

        // Variablen anlegen
        aBaseLst.Insert(
            pCont = new RscContExtraData( pHS->Insert( "ContExtradata" ),
                                          RSC_NOTYPE ),
            LIST_APPEND );
        pCont->SetTypeClass( &aShort, &aString );
        nRsc_EXTRADATA = nId = aNmTb.Put( "ExtraData", VARNAME );
        pClassMgr->SetVariable( nId, pCont );
    };
    nId = aNmTb.Put( "Comment", VARNAME );
    pClassMgr->SetVariable( nId, &aString, NULL, VAR_NORC );

    pClass->SetTypeClass( pClassMgr );

    return pClassMgr;
}

/*************************************************************************
|*    RscTypCont::InitClassString()
*************************************************************************/
RscTop * RscTypCont::InitClassString( RscTop * pSuper ){
    HASHID      nId;
    RscTop *    pClassString;

    nId = pHS->Insert( "String" );
    pClassString = new RscClass( nId, RSC_STRING, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassString );
    pClassString->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "Text", VARNAME );
    pClassString->SetVariable( nId, &aLangString );
    return( pClassString );
}

/*************************************************************************
|*    RscTypCont::InitClassBitmap()
*************************************************************************/
RscTop * RscTypCont::InitClassBitmap( RscTop * pSuper ){
    HASHID      nId;
    RscTop *    pClassBitmap;

    nId = pHS->Insert( "Bitmap" );
    pClassBitmap = new RscSysDepend( nId, RSC_BITMAP, pSuper );
    pClassBitmap->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassBitmap );

    // Die Klasse RscSysDepend behandelt die Variablen
    // "FILE" gesondert
    nId = aNmTb.Put( "File", VARNAME );
    pClassBitmap->SetVariable( nId, &aLangString, NULL, VAR_NORC );

    return( pClassBitmap );
}

/*************************************************************************
|*    RscTypCont::InitClassColor()
*************************************************************************/
RscTop * RscTypCont::InitClassColor( RscTop * pSuper, RscEnum * pColor ){
    HASHID      nId;
    RscTop *    pClassColor;

    // Klasse anlegen
    nId = pHS->Insert( "Color" );
    pClassColor = new RscClass( nId, RSC_COLOR, pSuper );
    pClassColor->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassColor );

    // Variablen anlegen
    nId = aNmTb.Put( "Red", VARNAME );
    pClassColor->SetVariable( nId, &aUShort );
    nId = aNmTb.Put( "Green", VARNAME );
    pClassColor->SetVariable( nId, &aUShort );
    nId = aNmTb.Put( "Blue", VARNAME );
    pClassColor->SetVariable( nId, &aUShort );
    nId = aNmTb.Put( "Predefine", VARNAME );
    pClassColor->SetVariable( nId, pColor );

    return( pClassColor );
}

/*************************************************************************
|*    RscTypCont::InitClassImage()
*************************************************************************/
RscTop * RscTypCont::InitClassImage( RscTop * pSuper, RscTop * pClassBitmap,
                                     RscTop * pClassColor )
{
    HASHID      nId;
    RscTop *    pClassImage;

    // Klasse anlegen
    nId = pHS->Insert( "Image" );
    pClassImage = new RscClass( nId, RSC_IMAGE, pSuper );
    pClassImage->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassImage );

    // Variablen anlegen
    nId = aNmTb.Put( "ImageBitmap", VARNAME );
    pClassImage->SetVariable( nId, pClassBitmap, NULL, 0, RSC_IMAGE_IMAGEBITMAP );
    nId = aNmTb.Put( "MaskBitmap", VARNAME );
    pClassImage->SetVariable( nId, pClassBitmap, NULL, 0, RSC_IMAGE_MASKBITMAP );
    nId = aNmTb.Put( "MaskColor", VARNAME );
    pClassImage->SetVariable( nId, pClassColor, NULL,
                              VAR_SVDYNAMIC, RSC_IMAGE_MASKCOLOR );

    return( pClassImage );
}

/*************************************************************************
|*    RscTypCont::InitClassImageList()
*************************************************************************/
RscTop * RscTypCont::InitClassImageList( RscTop * pSuper, RscTop * pClassBitmap,
                                         RscTop * pClassColor )
{
    HASHID      nId;
    RscTop *    pClassImageList;

    // Klasse anlegen
    nId = pHS->Insert( "ImageList" );
    pClassImageList = new RscClass( nId, RSC_IMAGELIST, pSuper );
    pClassImageList->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassImageList );

    // Variablen anlegen
    nId = aNmTb.Put( "ImageBitmap", VARNAME );
    pClassImageList->SetVariable( nId, pClassBitmap, NULL, 0,
                                  RSC_IMAGELIST_IMAGEBITMAP );
    nId = aNmTb.Put( "MaskBitmap", VARNAME );
    pClassImageList->SetVariable( nId, pClassBitmap, NULL, 0,
                                  RSC_IMAGELIST_MASKBITMAP );
    nId = aNmTb.Put( "MaskColor", VARNAME );
    pClassImageList->SetVariable( nId, pClassColor, NULL,
                                  VAR_SVDYNAMIC, RSC_IMAGELIST_MASKCOLOR );
    {
        RscCont * pCont;

        pCont = new RscCont( pHS->Insert( "USHORT *" ), RSC_NOTYPE );
        pCont->SetTypeClass( &aIdUShort );
        aBaseLst.Insert( pCont, LIST_APPEND );

        nId = aNmTb.Put( "IdList", VARNAME );
        pClassImageList->SetVariable( nId, pCont, NULL, 0,
                                      RSC_IMAGELIST_IDLIST );
    }
    nId = aNmTb.Put( "IdCount", VARNAME );
    pClassImageList->SetVariable( nId, &aUShort, NULL, 0,
                                  RSC_IMAGELIST_IDCOUNT );
    return( pClassImageList );
}

/*************************************************************************
|*    RscTypCont::InitClassWindow()
*************************************************************************/
RscTop * RscTypCont::InitClassWindow( RscTop * pSuper, RscEnum * pMapUnit,
                                 RscArray * pLangGeo )
{
    HASHID      nId;
    RscTop *    pClassWindow;

    // Klasse anlegen
    nId = pHS->Insert( "Window" );
    pClassWindow = new RscClass( nId, RSC_WINDOW, pSuper );
    pClassWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassWindow );

    // Variablen anlegen
    {
        RscFlag *   pFlag;
        RscClient * pClient;
        HASHID      nVarId, nDisableId, nOutputSizeId;

        aBaseLst.Insert( pFlag = new RscFlag( pHS->Insert( "FlagWndExtra" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );

        // Konstanten in Tabelle stellen
        nDisableId = pHS->Insert( "RSWND_DISABLE" );
        SETCONST( pFlag, nDisableId, RSWND_DISABLED );
        nOutputSizeId = pHS->Insert( "RSWND_OUTPUTSIZE" );
        SETCONST( pFlag, nOutputSizeId, RSWND_CLIENTSIZE );

        // Variable einfuegen
        nVarId = aNmTb.Put( "_RscExtraFlags", VARNAME );
        pClassWindow->SetVariable( nVarId, pFlag, NULL,
                                                                        VAR_HIDDEN | VAR_NOENUM );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nDisableId ),
            LIST_APPEND );
        nId = aNmTb.Put( "Disable", VARNAME );
        pClassWindow->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nOutputSizeId ),
            LIST_APPEND );
        nId = aNmTb.Put( "OutputSize", VARNAME );
        pClassWindow->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );
    }

    pClassWindow->SetVariable( nWinBitVarId, &aWinBits, NULL,
                               VAR_HIDDEN | VAR_NOENUM );

    INS_WINBIT(pClassWindow,Border)
    INS_WINBIT(pClassWindow,Hide)
    INS_WINBIT(pClassWindow,ClipChildren)
    INS_WINBIT(pClassWindow,SVLook)
    InsWinBit( pClassWindow, "DialogControl", nTabControlId );

    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassWindow->SetVariable( nId, &aIdLong );


    nRsc_XYMAPMODEId = nId = aNmTb.Put( "_XYMapMode", VARNAME );
    pClassWindow->SetVariable( nId, pMapUnit, NULL, 0, WINDOW_XYMAPMODE  );
    nRsc_X = nId = aNmTb.Put( "_X", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, NULL, 0, WINDOW_X  );
    nRsc_Y = nId = aNmTb.Put( "_Y", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, NULL, 0, WINDOW_Y  );

    nRsc_WHMAPMODEId = nId = aNmTb.Put( "_WHMapMode", VARNAME );
    pClassWindow->SetVariable( nId, pMapUnit, NULL, 0, WINDOW_WHMAPMODE  );
    nRsc_WIDTH = nId = aNmTb.Put( "_Width", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, NULL, 0, WINDOW_WIDTH  );
    nRsc_HEIGHT = nId = aNmTb.Put( "_Height", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, NULL, 0, WINDOW_HEIGHT  );

    nRsc_DELTALANG = nId = aNmTb.Put( "DeltaLang", VARNAME );
    pClassWindow->SetVariable( nId, pLangGeo, NULL, VAR_NORC | VAR_NOENUM);
    nId = aNmTb.Put( "Text", VARNAME );
    pClassWindow->SetVariable( nId, &aLangString, NULL, 0, WINDOW_TEXT );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassWindow->SetVariable( nId, &aLangString, NULL, 0, WINDOW_HELPTEXT );
    nId = aNmTb.Put( "QuickHelpText", VARNAME );
    pClassWindow->SetVariable( nId, &aLangString, NULL, 0, WINDOW_QUICKTEXT );
    nId = aNmTb.Put( "ExtraLong", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, NULL, 0, WINDOW_EXTRALONG );
    nId = aNmTb.Put( "UniqueId", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, NULL, 0, WINDOW_UNIQUEID );

    return( pClassWindow );
}

/*************************************************************************
|*    RscTypCont::InitClassSystemWindow()
*************************************************************************/
RscTop * RscTypCont::InitClassSystemWindow( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassSystemWindow;

    // Klasse anlegen
    nId = pHS->Insert( "SystemWindow" );
    pClassSystemWindow = new RscClass( nId, RSC_SYSWINDOW, pSuper );
    pClassSystemWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassSystemWindow );

    INS_WINBIT(pClassSystemWindow,Sizeable)
    INS_WINBIT(pClassSystemWindow,Moveable)
    InsWinBit( pClassSystemWindow, "Minable", nMinimizeId );
    InsWinBit( pClassSystemWindow, "Maxable", nMaximizeId );
    INS_WINBIT(pClassSystemWindow,Closeable)
    INS_WINBIT(pClassSystemWindow,App)

    return pClassSystemWindow ;
}

/*************************************************************************
|*    RscTypCont::InitClassWorkWindow()
*************************************************************************/
RscTop * RscTypCont::InitClassWorkWindow( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassWorkWindow;

    // Klasse anlegen
    nId = pHS->Insert( "WorkWindow" );
    pClassWorkWindow = new RscClass( nId, RSC_WORKWIN, pSuper );
    pClassWorkWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassWorkWindow );

    // Variablen anlegen
    {
        HASHID      nVarId;
        RscEnum   * pShow;

        aBaseLst.Insert( pShow = new RscEnum( pHS->Insert( "EnumShowState" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );

        SETCONST( pShow, "SHOW_NORMAL",        WORKWIN_SHOWNORMAL    );
        SETCONST( pShow, "SHOW_MINIMIZED",     WORKWIN_SHOWMINIMIZED );
        SETCONST( pShow, "SHOW_MAXIMIZED",     WORKWIN_SHOWMAXIMIZED );


        // Variable einfuegen
        nVarId = aNmTb.Put( "Show", VARNAME );
        pClassWorkWindow->SetVariable( nVarId, pShow, NULL );
    }

    return pClassWorkWindow;
}

/*************************************************************************
|*    RscTypCont::InitClassDialogBox()
*************************************************************************/
RscTop * RscTypCont::InitClassModalDialog( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassDialog;

    // Klasse anlegen
    nId = pHS->Insert( "ModalDialog" );
    pClassDialog = new RscClass( nId, RSC_MODALDIALOG, pSuper );
    pClassDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassDialog );

    InsWinBit( pClassDialog, "SysModal", nSysmodalId );

    return pClassDialog;
}

/*************************************************************************
|*    RscTypCont::InitClassModelessDialog()
*************************************************************************/
RscTop * RscTypCont::InitClassModelessDialog( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassDialog;

    // Klasse anlegen
    nId = pHS->Insert( "ModelessDialog" );
    pClassDialog = new RscClass( nId, RSC_MODELESSDIALOG, pSuper );
    pClassDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassDialog );

    return pClassDialog;
}

/*************************************************************************
|*    RscTypCont::InitClassControl()
*************************************************************************/
RscTop * RscTypCont::InitClassControl( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassControl;

    // Klasse anlegen
    nId = pHS->Insert( "Control" );
    pClassControl = new RscClass( nId, RSC_CONTROL, pSuper );
    pClassControl->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassControl );

    InsWinBit( pClassControl, "TabStop", nTabstopId );
    INS_WINBIT(pClassControl,Group)

    return pClassControl;
}

/*************************************************************************
|*    RscTypCont::InitClassPushButton()
*************************************************************************/
RscTop * RscTypCont::InitClassPushButton( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassPushButton;

    // Klasse anlegen
    nId = pHS->Insert( "PushButton" );
    pClassPushButton = new RscClass( nId, RSC_PUSHBUTTON, pSuper );
    pClassPushButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassPushButton );

    InsWinBit( pClassPushButton, "DefButton", nDefaultId );

    return pClassPushButton;
}

/*************************************************************************
|*    RscTypCont::InitClassTriStateBox()
*************************************************************************/
RscTop * RscTypCont::InitClassTriStateBox( RscTop * pSuper,
                                           RscEnum * pTriState )
{
    HASHID      nId;
    RscTop *    pClassTriStateBox;

    nId = pHS->Insert( "TriStateBox" );
    pClassTriStateBox = new RscClass( nId, RSC_TRISTATEBOX, pSuper );
    pClassTriStateBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassTriStateBox );

    // Variablen anlegen
    nId = aNmTb.Put( "State", VARNAME );
    pClassTriStateBox->SetVariable( nId, pTriState );
    nId = aNmTb.Put( "TriStateDisable", VARNAME );
    pClassTriStateBox->SetVariable( nId, &aBool );

    return( pClassTriStateBox );
}

/*************************************************************************
|*    RscTypCont::InitClassMenuButton()
*************************************************************************/
RscTop * RscTypCont::InitClassMenuButton( RscTop * pSuper,
                                           RscTop * pClassMenu )
{
    HASHID      nId;
    RscTop *    pClassMenuButton;

    nId = pHS->Insert( "MenuButton" );
    pClassMenuButton = new RscClass( nId, RSC_MENUBUTTON, pSuper );
    pClassMenuButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassMenuButton );

    // Variablen anlegen
    nId = aNmTb.Put( "ButtonMenu", VARNAME );
    pClassMenuButton->SetVariable( nId, pClassMenu, NULL, 0,
                                                                        RSCMENUBUTTON_MENU );

    return( pClassMenuButton );
}


/*************************************************************************
|*    RscTypCont::InitClassImageButton()
*************************************************************************/
RscTop * RscTypCont::InitClassImageButton( RscTop * pSuper,
                                           RscTop * pClassImage,
                                           RscEnum * pTriState )
{
    HASHID      nId;
    RscTop *    pClassImageButton;

    // Klasse anlegen
    nId = pHS->Insert( "ImageButton" );
    pClassImageButton = new RscClass( nId, RSC_IMAGEBUTTON, pSuper );
    pClassImageButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassImageButton );

    // Variablen anlegen
    {
        nId = aNmTb.Put( "ButtonImage", VARNAME );
        pClassImageButton->SetVariable( nId, pClassImage, NULL, 0,
                                         RSC_IMAGEBUTTON_IMAGE );
    }
    // Variablen anlegen
    {
        HASHID      nVarId;
        RscEnum   * pSymbol;

        aBaseLst.Insert( pSymbol = new RscEnum( pHS->Insert( "EnumSymbolButton" ),
                                                RSC_NOTYPE ), LIST_APPEND );

        SETCONST( pSymbol, "IMAGEBUTTON_DONTKNOW",      SYMBOL_DONTKNOW );
        SETCONST( pSymbol, "IMAGEBUTTON_IMAGE",         SYMBOL_IMAGE );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_UP",      SYMBOL_ARROW_UP );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_DOWN",    SYMBOL_ARROW_DOWN );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_LEFT",    SYMBOL_ARROW_LEFT );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_RIGHT",   SYMBOL_ARROW_RIGHT );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_UP",       SYMBOL_SPIN_UP );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_DOWN",     SYMBOL_SPIN_DOWN );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_LEFT",     SYMBOL_SPIN_LEFT );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_RIGHT",    SYMBOL_SPIN_RIGHT );
        SETCONST( pSymbol, "IMAGEBUTTON_FIRST",         SYMBOL_FIRST );
        SETCONST( pSymbol, "IMAGEBUTTON_LAST",          SYMBOL_LAST );
        SETCONST( pSymbol, "IMAGEBUTTON_PREV",          SYMBOL_PREV );
        SETCONST( pSymbol, "IMAGEBUTTON_NEXT",          SYMBOL_NEXT );
        SETCONST( pSymbol, "IMAGEBUTTON_PAGEUP",        SYMBOL_PAGEUP );
        SETCONST( pSymbol, "IMAGEBUTTON_PAGEDOWN",      SYMBOL_PAGEDOWN );
        SETCONST( pSymbol, "IMAGEBUTTON_PLAY",          SYMBOL_PLAY );
        SETCONST( pSymbol, "IMAGEBUTTON_REVERSEPLAY",   SYMBOL_REVERSEPLAY );
        SETCONST( pSymbol, "IMAGEBUTTON_STOP",          SYMBOL_STOP );
        SETCONST( pSymbol, "IMAGEBUTTON_PAUSE",         SYMBOL_PAUSE );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDSTART",     SYMBOL_WINDSTART );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDEND",       SYMBOL_WINDEND );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDBACKWARD",  SYMBOL_WINDBACKWARD );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDFORWARD",   SYMBOL_WINDFORWARD );

        // Variable einfuegen
        nVarId = aNmTb.Put( "Symbol", VARNAME );
        pClassImageButton->SetVariable( nVarId, pSymbol, NULL, 0,
                                        RSC_IMAGEBUTTON_SYMBOL );
    }
    nId = aNmTb.Put( "State", VARNAME );
    pClassImageButton->SetVariable( nId, pTriState, NULL, 0,
                                    RSC_IMAGEBUTTON_STATE );

    INS_WINBIT(pClassImageButton,Repeat)
    INS_WINBIT(pClassImageButton,SmallStyle)
    INS_WINBIT(pClassImageButton,RectStyle)

    return pClassImageButton;
}

/*************************************************************************
|*    RscTypCont::InitClassEdit()
*************************************************************************/
RscTop * RscTypCont::InitClassEdit( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassEdit;

    // Klasse anlegen
    nId                     = pHS->Insert( "Edit" );
    pClassEdit = new RscClass( nId, RSC_EDIT, pSuper );
    pClassEdit->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassEdit );

    INS_WINBIT(pClassEdit,Left)
    INS_WINBIT(pClassEdit,Center)
    INS_WINBIT(pClassEdit,Right)
    INS_WINBIT(pClassEdit,PassWord)
    INS_WINBIT(pClassEdit,ReadOnly)

    nId = aNmTb.Put( "MaxTextLength", VARNAME );
    pClassEdit->SetVariable( nId, &aUShort );

    return pClassEdit;
}

/*************************************************************************
|*    RscTypCont::InitClassMultiLineedit()
*************************************************************************/
RscTop * RscTypCont::InitClassMultiLineEdit( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassMultiLineEdit;

    // Klasse anlegen
    nId = pHS->Insert( "MultiLineEdit" );
    pClassMultiLineEdit = new RscClass( nId, RSC_MULTILINEEDIT, pSuper );
    pClassMultiLineEdit->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassMultiLineEdit );

    InsWinBit( pClassMultiLineEdit, "HScroll", nHscrollId );
    InsWinBit( pClassMultiLineEdit, "VScroll", nVscrollId );
    INS_WINBIT( pClassMultiLineEdit, IgnoreTab );

    return pClassMultiLineEdit;
}

/*************************************************************************
|*    RscTypCont::InitClassScrollBar()
*************************************************************************/
RscTop * RscTypCont::InitClassScrollBar( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassScrollBar;

    // Klasse anlegen
    nId = pHS->Insert( "ScrollBar" );
    pClassScrollBar = new RscClass( nId, RSC_SCROLLBAR, pSuper );
    pClassScrollBar->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassScrollBar );

    // Variablen anlegen
    nId = aNmTb.Put( "MinPos", VARNAME );
    pClassScrollBar->SetVariable( nId, &aShort );
    {
        RSCINST     aDfltI;

        aDfltI = aShort.Create( NULL, RSCINST(), FALSE );
        aDfltI.pClass->SetNumber( aDfltI, 100 );
//      aDfltI.pClass->MakeDefault( aDfltI );

        nId = aNmTb.Put( "MaxPos", VARNAME );
        pClassScrollBar->SetVariable( nId, &aShort, &aDfltI );
    }
    nId = aNmTb.Put( "ThumbPos", VARNAME );
    pClassScrollBar->SetVariable( nId, &aShort );
    {
        RSCINST     aDfltI;

        aDfltI = aShort.Create( NULL, RSCINST(), FALSE );
        aDfltI.pClass->SetNumber( aDfltI, 1 );
//      aDfltI.pClass->MakeDefault( aDfltI );

        nId = aNmTb.Put( "PageSize", VARNAME);
        pClassScrollBar->SetVariable( nId, &aShort, &aDfltI  );
    }
    {
        RSCINST     aDfltI;

        aDfltI = aShort.Create( NULL, RSCINST(), FALSE );
        aDfltI.pClass->SetNumber( aDfltI, 1 );
//      aDfltI.pClass->MakeDefault( aDfltI );
        nId = aNmTb.Put( "LineSize", VARNAME );
        pClassScrollBar->SetVariable( nId, &aShort, &aDfltI );
    }
    nId = aNmTb.Put( "VisibleSize", VARNAME );
    pClassScrollBar->SetVariable( nId, &aShort );

    InsWinBit( pClassScrollBar, "HScroll", nHscrollId );
    InsWinBit( pClassScrollBar, "VScroll", nVscrollId );
    INS_WINBIT(pClassScrollBar,Drag)

    return pClassScrollBar;
}

/*************************************************************************
|*    RscTypCont::InitClassListBox()
*************************************************************************/
RscTop * RscTypCont::InitClassListBox( RscTop * pSuper, RscArray * pStrLst )
{
    HASHID      nId;
    RscTop *    pClassListBox;

    // Klasse anlegen
    nId = pHS->Insert( "ListBox" );
    pClassListBox = new RscClass( nId, RSC_LISTBOX, pSuper );
    pClassListBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassListBox );

    // Variablen anlegen
    INS_WINBIT(pClassListBox,Sort)
    INS_WINBIT(pClassListBox,DropDown)
    InsWinBit( pClassListBox, "HScroll", nHscrollId );
    InsWinBit( pClassListBox, "VScroll", nVscrollId );
    INS_WINBIT(pClassListBox,AutoSize)
    INS_WINBIT(pClassListBox,AutoHScroll)
    INS_WINBIT(pClassListBox,DDExtraWidth)

    {
        RSCINST aDflt = aUShort.Create( NULL, RSCINST(), FALSE );
        aDflt.pClass->SetNumber( aDflt, (USHORT)0xFFFF );
        nId = aNmTb.Put( "CurPos", VARNAME );
        pClassListBox->SetVariable( nId, &aUShort, &aDflt );
    }
    nId = aNmTb.Put( "StringList", VARNAME );
    pClassListBox->SetVariable( nId, pStrLst );

    return pClassListBox;
}

/*************************************************************************
|*    RscTypCont::InitClassMultiListBox()
*************************************************************************/
RscTop * RscTypCont::InitClassMultiListBox( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassMultiListBox;

    // Klasse anlegen
    nId = pHS->Insert( "MultiListBox" );
    pClassMultiListBox = new RscClass( nId, RSC_MULTILISTBOX, pSuper );
    pClassMultiListBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassMultiListBox );

    INS_WINBIT(pClassMultiListBox,SimpleMode)

    return pClassMultiListBox;
}

/*************************************************************************
|*    RscTypCont::InitClassComboBox()
*************************************************************************/
RscTop * RscTypCont::InitClassComboBox( RscTop * pSuper, RscArray * pStrLst )
{
    HASHID      nId;
    RscTop *    pClassComboBox;

    // Klasse anlegen
    nId = pHS->Insert( "ComboBox" );
    pClassComboBox = new RscClass( nId, RSC_COMBOBOX, pSuper );
    pClassComboBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassComboBox );

    // Variablen anlegen
    INS_WINBIT(pClassComboBox,DropDown)
    INS_WINBIT(pClassComboBox,Sort)
    InsWinBit( pClassComboBox, "HScroll", nHscrollId );
    InsWinBit( pClassComboBox, "VScroll", nVscrollId );
    INS_WINBIT(pClassComboBox,AutoSize)
    INS_WINBIT(pClassComboBox,AutoHScroll)
    INS_WINBIT(pClassComboBox,DDExtraWidth)

    nId = aNmTb.Put( "StringList", VARNAME );
    pClassComboBox->SetVariable( nId, pStrLst );

    return pClassComboBox;
}

#ifdef MAC
#pragma segment rsctypcont
#endif

/*************************************************************************
|*    RscTypCont::InitClassFixedText()
*************************************************************************/
RscTop * RscTypCont::InitClassFixedText( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassFixedText;

    // Klasse anlegen
    nId = pHS->Insert( "FixedText" );
    pClassFixedText = new RscClass( nId, RSC_TEXT, pSuper );
    pClassFixedText->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedText );

    // Variablen anlegen
    INS_WINBIT(pClassFixedText,Left)
    INS_WINBIT(pClassFixedText,Center)
    INS_WINBIT(pClassFixedText,Right)
    INS_WINBIT(pClassFixedText,WordBreak)
    INS_WINBIT(pClassFixedText,LeftLabel)
    INS_WINBIT(pClassFixedText,NoLabel)

    return pClassFixedText;
}

/*************************************************************************
|*    RscTypCont::InitClassFixedBitmap()
*************************************************************************/
RscTop * RscTypCont::InitClassFixedBitmap( RscTop * pSuper, RscTop * pClassBitmap )
{
    HASHID      nId;
    RscTop *    pClassFixedBitmap;

    // Klasse anlegen
    nId = pHS->Insert( "FixedBitmap" );
    pClassFixedBitmap = new RscClass( nId, RSC_FIXEDBITMAP, pSuper );
    pClassFixedBitmap->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedBitmap );

    INS_WINBIT(pClassFixedBitmap,Scale)

    // Variablen anlegen
    nId = aNmTb.Put( "Fixed", VARNAME );
    pClassFixedBitmap->SetVariable( nId, pClassBitmap, 0, 0, RSC_FIXEDBITMAP_BITMAP );

    return pClassFixedBitmap;
}

/*************************************************************************
|*    RscTypCont::InitClassFixedImage()
*************************************************************************/
RscTop * RscTypCont::InitClassFixedImage( RscTop * pSuper, RscTop * pClassImage )
{
    HASHID      nId;
    RscTop *    pClassFixedImage;

    // Klasse anlegen
    nId = pHS->Insert( "FixedImage" );
    pClassFixedImage = new RscClass( nId, RSC_FIXEDIMAGE, pSuper );
    pClassFixedImage->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedImage );

    // Variablen anlegen
    nId = aNmTb.Put( "Fixed", VARNAME );
    pClassFixedImage->SetVariable( nId, pClassImage, 0, 0, RSC_FIXEDIMAGE_IMAGE );

    return pClassFixedImage;
}

/*************************************************************************
|*    RscTypCont::InitClassImageRadioButton()
*************************************************************************/
RscTop * RscTypCont::InitClassImageRadioButton( RscTop * pSuper, RscTop * pClassImage )
{
    HASHID      nId;
    RscTop *    pClassImageRadioButton;

    // Klasse anlegen
    nId = pHS->Insert( "ImageRadioButton" );
    pClassImageRadioButton = new RscClass( nId, RSC_IMAGERADIOBUTTON, pSuper );
    pClassImageRadioButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassImageRadioButton );

    // Variablen anlegen
    INS_WINBIT(pClassImageRadioButton,TopImage)
    nId = aNmTb.Put( "RadioButtonImage", VARNAME );
    pClassImageRadioButton->SetVariable( nId, pClassImage, 0, 0, RSC_IMAGERADIOBUTTON_IMAGE );

    return pClassImageRadioButton;
}

/*************************************************************************
|*    RscTypCont::InitClassKeyCode()
*************************************************************************/
RscTop * RscTypCont::InitClassKeyCode( RscTop * pSuper, RscEnum * pKey )
{
    HASHID      nId;
    RscTop *    pClassKeyCode;

    // Klasse anlegen
    nId = pHS->Insert( "KeyCode" );
    pClassKeyCode = new RscClass( nId, RSC_KEYCODE, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassKeyCode );

    // Variablen anlegen
    nId = aNmTb.Put( "Code", VARNAME );
    pClassKeyCode->SetVariable( nId, pKey );

    {
        RscFlag *   pFlag;
        RscClient * pClient;
        HASHID      nVarId, nShiftId, nMod1Id, nMod2Id;

        aBaseLst.Insert( pFlag = new RscFlag( pHS->Insert( "FlagKeyModifier" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );

        // Konstanten in Tabelle stellen
        nShiftId = pHS->Insert( "KEY_SHIFT" );
        SETCONST( pFlag, nShiftId, KEY_SHIFT );
        nMod1Id = pHS->Insert( "KEY_MOD1" );
        SETCONST( pFlag, nMod1Id, KEY_MOD1 );
        nMod2Id = pHS->Insert( "KEY_MOD2" );
        SETCONST( pFlag, nMod2Id, KEY_MOD2 );

        // Variable einfuegen
        nVarId = aNmTb.Put( "_ModifierFlags", VARNAME );
        pClassKeyCode->SetVariable( nVarId, pFlag, NULL,
                                                                        VAR_HIDDEN | VAR_NOENUM );

        // Clientvariablen einfuegen
        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nShiftId ),
            LIST_APPEND );
        nId = aNmTb.Put( "Shift", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nMod1Id ),
            LIST_APPEND );
        nId = aNmTb.Put( "Modifier1", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );


        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nMod2Id ),
            LIST_APPEND );
        nId = aNmTb.Put( "Modifier2", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );
    }
    {
        HASHID      nVarId;
        RscEnum   * pKeyFunc;

        aBaseLst.Insert( pKeyFunc = new RscEnum( pHS->Insert( "EnumKeyFunc" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );

        SETCONST( pKeyFunc, "KEYFUNC_DONTKNOW",         KEYFUNC_DONTKNOW        );
        SETCONST( pKeyFunc, "KEYFUNC_NEW",                      KEYFUNC_NEW                     );
        SETCONST( pKeyFunc, "KEYFUNC_OPEN",                     KEYFUNC_OPEN            );
        SETCONST( pKeyFunc, "KEYFUNC_SAVE",                     KEYFUNC_SAVE            );
        SETCONST( pKeyFunc, "KEYFUNC_SAVEAS",           KEYFUNC_SAVEAS          );
        SETCONST( pKeyFunc, "KEYFUNC_PRINT",            KEYFUNC_PRINT           );
        SETCONST( pKeyFunc, "KEYFUNC_CLOSE",            KEYFUNC_CLOSE           );
        SETCONST( pKeyFunc, "KEYFUNC_QUIT",                     KEYFUNC_QUIT            );
        SETCONST( pKeyFunc, "KEYFUNC_CUT",                      KEYFUNC_CUT                     );
        SETCONST( pKeyFunc, "KEYFUNC_COPY",                     KEYFUNC_COPY            );
        SETCONST( pKeyFunc, "KEYFUNC_PASTE",            KEYFUNC_PASTE           );
        SETCONST( pKeyFunc, "KEYFUNC_UNDO",                     KEYFUNC_UNDO            );
        SETCONST( pKeyFunc, "KEYFUNC_REDO",                     KEYFUNC_REDO            );
        SETCONST( pKeyFunc, "KEYFUNC_DELETE",           KEYFUNC_DELETE          );
        SETCONST( pKeyFunc, "KEYFUNC_REPEAT",           KEYFUNC_REPEAT          );
        SETCONST( pKeyFunc, "KEYFUNC_FIND",                     KEYFUNC_FIND            );
        SETCONST( pKeyFunc, "KEYFUNC_PROPERTIES",       KEYFUNC_PROPERTIES      );
        SETCONST( pKeyFunc, "KEYFUNC_FRONT",            KEYFUNC_FRONT           );
        SETCONST( pKeyFunc, "KEYFUNC_FINDBACKWARD",     KEYFUNC_FINDBACKWARD );
        // Variable einfuegen
        nVarId = aNmTb.Put( "Function", VARNAME );
        pClassKeyCode->SetVariable( nVarId, pKeyFunc, NULL );
    }

    return pClassKeyCode;
}

/*************************************************************************
|*    RscTypCont::InitClassAccelItem()
*************************************************************************/
RscTop * RscTypCont::InitClassAccelItem( RscTop * pSuper,
                                                                                RscTop * pClassKeyCode )
{
    HASHID      nId;
    RscTop *    pClassAccelItem;

    // Klasse anlegen
    nId = pHS->Insert( "AcceleratorItem" );
    pClassAccelItem = new RscClass( nId, RSC_ACCELITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassAccelItem );

    // Variablen anlegen
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassAccelItem->SetVariable( nId, &aIdNoZeroUShort );
    nId = aNmTb.Put( "Disable", VARNAME );
    pClassAccelItem->SetVariable( nId, &aBool );
    nId = aNmTb.Put( "Key", VARNAME );
    pClassAccelItem->SetVariable( nId, pClassKeyCode, NULL, 0,
                                  ACCELITEM_KEY );

    return pClassAccelItem;
}

/*************************************************************************
|*    RscTypCont::InitClassAccelm()
*************************************************************************/
RscTop * RscTypCont::InitClassAccel( RscTop * pSuper, RscTop * pClassAccelItem )
{
    HASHID      nId;
    RscTop *    pClassAccel;

    // Klasse anlegen
    nId = pHS->Insert( "Accelerator" );
    pClassAccel = new RscClass( nId, RSC_ACCEL, pSuper );
    pClassAccel->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassAccel );

    // Variablen anlegen
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassAccel->SetVariable( nId, &aLangString );
    {
        RscCont * pCont;

        aBaseLst.Insert( pCont = new RscCont(
                                       pHS->Insert( "ContAcceleratorKey" ),
                                       RSC_NOTYPE ),
                         LIST_APPEND );
        pCont->SetTypeClass( pClassAccelItem );
        nId = aNmTb.Put( "ItemList", VARNAME );
        pClassAccel->SetVariable( nId, pCont );
    }

    return pClassAccel;
}

/*************************************************************************
|*    RscTypCont::InitClassMenuItem()
*************************************************************************/
RscTop * RscTypCont::InitClassMenuItem( RscTop * pSuper,
                                        RscTop * pClassBitmap,
                                                                                RscTop * pClassKeyCode )
{
    HASHID      nId;
    RscTop *    pClassMenuItem;

    // Klasse anlegen
    nId = pHS->Insert( "MenuItem" );
    pClassMenuItem = new RscClass( nId, RSC_MENUITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassMenuItem );

    // Variablen anlegen
    nId = aNmTb.Put( "Separator", VARNAME );
    pClassMenuItem->SetVariable( nId, &aBool, NULL, 0,
                                 RSC_MENUITEM_SEPARATOR );
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassMenuItem->SetVariable( nId, &aIdNoZeroUShort, NULL, 0,
                                 RSC_MENUITEM_ID );
    {
        RscFlag *   pFlag;
        RscClient * pClient;
        HASHID      nVarId, nAutoCheckId, nRadioCheckId;
        HASHID      nCheckableId, nAboutId, nHelpId;

        aBaseLst.Insert( pFlag = new RscFlag( pHS->Insert( "FlagMenuState" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );

        // Konstanten in Tabelle stellen
        nCheckableId = pHS->Insert( "MIB_CHECKABLE" );
        SETCONST( pFlag, nCheckableId, MIB_CHECKABLE );
        nAutoCheckId = pHS->Insert( "MIB_AUTOCHECK" );
        SETCONST( pFlag, nAutoCheckId, MIB_AUTOCHECK );
        nRadioCheckId = pHS->Insert( "MIB_RADIOCHECK" );
        SETCONST( pFlag, nRadioCheckId, MIB_RADIOCHECK );
        nAboutId = pHS->Insert( "MIB_ABOUT" );
        SETCONST( pFlag, nAboutId, MIB_ABOUT );
        nHelpId = pHS->Insert( "MIB_HELP" );
        SETCONST( pFlag, nHelpId, MIB_HELP );

        // Variable einfuegen
        nVarId = aNmTb.Put( "_MenuItemFlags", VARNAME );
        pClassMenuItem->SetVariable( nVarId, pFlag, NULL,
                                     VAR_HIDDEN | VAR_NOENUM,
                                     RSC_MENUITEM_STATUS );

        // Clientvariablen einfuegen
        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nCheckableId ),
            LIST_APPEND );
        nId = aNmTb.Put( "Checkable", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nAutoCheckId ),
            LIST_APPEND );
        nId = aNmTb.Put( "AutoCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nRadioCheckId ),
            LIST_APPEND );
        nId = aNmTb.Put( "RadioCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nAboutId ),
            LIST_APPEND );
        nId = aNmTb.Put( "About", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nHelpId ),
            LIST_APPEND );
        nId = aNmTb.Put( "Help", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

    }
    nId = aNmTb.Put( "Text", VARNAME );
    pClassMenuItem->SetVariable( nId, &aLangString, NULL, 0,
                                 RSC_MENUITEM_TEXT );
    nId = aNmTb.Put( "ItemBitmap", VARNAME );
    pClassMenuItem->SetVariable( nId, pClassBitmap, NULL, 0,
                                 RSC_MENUITEM_BITMAP );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassMenuItem->SetVariable( nId, &aLangString, NULL, 0,
                                 RSC_MENUITEM_HELPTEXT );
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassMenuItem->SetVariable( nId, &aIdLong, NULL, 0,
                                 RSC_MENUITEM_HELPID );
    nId = aNmTb.Put( "AccelKey", VARNAME );
    pClassMenuItem->SetVariable( nId, pClassKeyCode, NULL, 0,
                                 RSC_MENUITEM_KEYCODE );
    nId = aNmTb.Put( "Check", VARNAME );
    pClassMenuItem->SetVariable( nId, &aBool, NULL, 0,
                                 RSC_MENUITEM_CHECKED );
    nId = aNmTb.Put( "Disable", VARNAME );
    pClassMenuItem->SetVariable( nId, &aBool, NULL, 0,
                                 RSC_MENUITEM_DISABLE );
    nId = aNmTb.Put( "Command", VARNAME );
    pClassMenuItem->SetVariable( nId, &aString, NULL, 0,
                                    RSC_MENUITEM_COMMAND );

    return pClassMenuItem;
}

/*************************************************************************
|*    RscTypCont::InitClassMenu()
*************************************************************************/
RscTop * RscTypCont::InitClassMenu( RscTop * pSuper,
                                    RscTop * pClassMenuItem )
{
    HASHID      nId;
    RscTop *    pClassMenu;

    // Klasse anlegen
    nId = pHS->Insert( "Menu" );
    pClassMenu = new RscClass( nId, RSC_MENU, pSuper );
    pClassMenu->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassMenu );

    // Variablen anlegen
    {
        RscCont * pCont;

        aBaseLst.Insert( pCont = new RscCont( pHS->Insert( "ContMenuItem" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );
        pCont->SetTypeClass( pClassMenuItem );
        nId = aNmTb.Put( "ItemList", VARNAME );
        pClassMenu->SetVariable( nId, pCont, NULL, 0, RSC_MENU_ITEMS );
    }
    nId = aNmTb.Put( "Text", VARNAME );
    pClassMenu->SetVariable( nId, &aLangString, NULL, 0, RSC_MENU_TEXT );
    nId = aNmTb.Put( "DefaultItemId", VARNAME );
    pClassMenu->SetVariable( nId, &aIdUShort, NULL, 0,
                                 RSC_MENU_DEFAULTITEMID );

    return pClassMenu;
}

/*************************************************************************
|*    RscTypCont::InitClassMessageBox()
*************************************************************************/
RscTop * RscTypCont::InitClassMessBox( RscTop * pSuper,
                                       RscEnum * pMessButtons,
                                       RscEnum * pMessDefButton )
{
    HASHID      nId;
    RscTop *    pClassMessBox;

    // Klasse anlegen
    nId = pHS->Insert( "MessBox" );
    pClassMessBox = new RscClass( nId, RSC_MESSBOX, pSuper );
    pClassMessBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassMessBox );

    // Variablen anlegen
    nId = aNmTb.Put( "Buttons", VARNAME );
    pClassMessBox->SetVariable( nId, pMessButtons );
    nId = aNmTb.Put( "DefButton", VARNAME );
    pClassMessBox->SetVariable( nId, pMessDefButton );
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassMessBox->SetVariable( nId, &aIdLong );
    nId = aNmTb.Put( "SysModal", VARNAME );
    pClassMessBox->SetVariable( nId, &aBool );
    nId = aNmTb.Put( "Title", VARNAME );
    pClassMessBox->SetVariable( nId, &aLangString );
    nId = aNmTb.Put( "Message", VARNAME );
    pClassMessBox->SetVariable( nId, &aLangString );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassMessBox->SetVariable( nId, &aLangString );

    return pClassMessBox;
}

/*************************************************************************
|*    RscTypCont::InitClassSplitter()
*************************************************************************/
RscTop * RscTypCont::InitClassSplitter( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassSplitter;

    // Klasse anlegen
    nId = pHS->Insert( "Splitter" );
    pClassSplitter = new RscClass( nId, RSC_SPLITTER, pSuper );
    pClassSplitter->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassSplitter );

    InsWinBit( pClassSplitter, "HScroll", nHscrollId );
    InsWinBit( pClassSplitter, "VScroll", nVscrollId );

    return pClassSplitter;
}

/*************************************************************************
|*    RscTypCont::InitClassSplitWindow()
*************************************************************************/
RscTop * RscTypCont::InitClassSplitWindow( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassSplitWindow;

    // Klasse anlegen
    nId = pHS->Insert( "SplitWindow" );
    pClassSplitWindow = new RscClass( nId, RSC_SPLITWINDOW, pSuper );
    pClassSplitWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassSplitWindow );

    INS_WINBIT(pClassSplitWindow,Sizeable)
    INS_WINBIT(pClassSplitWindow,NoSplitDraw)

    return pClassSplitWindow;
}

/*************************************************************************
|*    RscTypCont::InitClassTime()
*************************************************************************/
RscTop * RscTypCont::InitClassTime( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassTime;

    // Klasse anlegen
    nId = pHS->Insert( "Time" );
    pClassTime = new RscClass( nId, RSC_TIME, pSuper );
    pClassTime->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    aNmTb.Put( nId, CLASSNAME, pClassTime );

    // Variablen anlegen
    nId = aNmTb.Put( "Hour", VARNAME );
    pClassTime->SetVariable( nId, &a0to23Short, NULL, 0, TIME_HOUR  );

    nId = aNmTb.Put( "Minute", VARNAME );
    pClassTime->SetVariable( nId, &a0to59Short, NULL, 0, TIME_MINUTE  );

    nId = aNmTb.Put( "Second", VARNAME );
    pClassTime->SetVariable( nId, &a0to59Short, NULL, 0, TIME_SECOND  );

    nId = aNmTb.Put( "Sec100", VARNAME ); // weiss noch nich
    pClassTime->SetVariable( nId, &a0to99Short, NULL, 0, TIME_SEC100  );

    return pClassTime;
}

/*************************************************************************
|*    RscTypCont::InitClassDate()
*************************************************************************/
RscTop * RscTypCont::InitClassDate( RscTop * pSuper, RscEnum * /*pDayOfWeek*/ )
{
    HASHID      nId;
    RscTop *    pClassDate;

    // Klasse anlegen
    nId = pHS->Insert( "Date" );
    pClassDate = new RscClass( nId, RSC_DATE, pSuper );
    pClassDate->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    aNmTb.Put( nId, CLASSNAME, pClassDate );

    // Variablen anlegen
    nId = aNmTb.Put( "Year", VARNAME );
    pClassDate->SetVariable( nId, &a0to9999Short, NULL, 0, DATE_YEAR  );

    nId = aNmTb.Put( "Month", VARNAME );
    pClassDate->SetVariable( nId, &a1to12Short, NULL, 0, DATE_MONTH  );

    nId = aNmTb.Put( "Day", VARNAME );
    pClassDate->SetVariable( nId, &a1to31Short, NULL, 0, DATE_DAY  );

    return pClassDate;
}

/*************************************************************************
|*    RscTypCont::InitClassInt1()
*************************************************************************/
RscTop * RscTypCont::InitClassInt1( RscTop * pSuper,
                            RscEnum * pDateFormat,
                            RscEnum * pTimeFormat, RscEnum * pDayOfWeekFormat,
                            RscEnum * pMonthFormat )
{
    HASHID      nId;
    RscTop *    pClassInt;

    // Klasse anlegen
    nId = pHS->Insert( "SubInternational" );
    pClassInt = new RscClass( nId, RSC_NOTYPE, pSuper );
    //pClassInt->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "Language", VARNAME );
    pClassInt->SetVariable( nId, &aLangType, NULL, 0, INT1_LANGUAGE  );

    nId = aNmTb.Put( "FormatLanguage", VARNAME );
    pClassInt->SetVariable( nId, &aLangType, NULL, 0, INT1_FORMATLANGUAGE  );

    nId = aNmTb.Put( "DateFormat", VARNAME );
    pClassInt->SetVariable( nId, pDateFormat, NULL, 0, INT1_DATEFORMAT  );

    nId = aNmTb.Put( "DateDayLeadingZero", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0, INT1_DATEDAYLEADINDZERO  );

    nId = aNmTb.Put( "DateMonthLeadingZero", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0, INT1_DATEMONTHLEADINDZERO  );

    nId = aNmTb.Put( "DateCentury", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0, INT1_DATECENTURY  );

    nId = aNmTb.Put( "LongDateFormat", VARNAME );
    pClassInt->SetVariable( nId, pDateFormat, NULL, 0, INT1_LONGDATEFORMAT  );

    nId = aNmTb.Put( "LongDateDayOfWeekFormat", VARNAME );
    pClassInt->SetVariable( nId, pDayOfWeekFormat, NULL, 0,
                                                        INT1_LONGDATEDAYOFWEEKFORMAT  );

    nId = aNmTb.Put( "LongDateDayOfWeekSep", VARNAME );
    pClassInt->SetVariable( nId, &aString, NULL, 0, INT1_LONGDATEDAYOFWEEKSEP );

    nId = aNmTb.Put( "LongDateDayLeadingZero", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0,
                                                        INT1_LONGDATEDAYLEADINGZERO );

    nId = aNmTb.Put( "LongDateDaySep", VARNAME );
    pClassInt->SetVariable( nId, &aString, NULL, 0, INT1_LONGDATEDAYSEP );

    nId = aNmTb.Put( "LongDateMonthFormat", VARNAME );
    pClassInt->SetVariable( nId, pMonthFormat, NULL, 0,
                                                        INT1_LONGDATEMONTHFORMAT );

    nId = aNmTb.Put( "LongDateMonthSep", VARNAME );
    pClassInt->SetVariable( nId, &aString, NULL, 0, INT1_LONGDATEMONTHSEP );

    nId = aNmTb.Put( "LongDateCentury", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0, INT1_LONGDATECENTURY  );

    nId = aNmTb.Put( "LongDateYearSep", VARNAME );
    pClassInt->SetVariable( nId, &aString, NULL, 0, INT1_LONGDATEYEARSEP  );

    nId = aNmTb.Put( "TimeFormat", VARNAME );
    pClassInt->SetVariable( nId, pTimeFormat, NULL, 0, INT1_TIMEFORMAT  );

        // Maske voll zweite Struktur anfangen

        return pClassInt;
}


/*************************************************************************
|*    RscTypCont::InitClassInternational()
*************************************************************************/
RscTop * RscTypCont::InitClassInternational( RscTop * pSuper,
                                RscEnum * /*pDateFormat*/,
                                RscEnum * /*pTimeFormat*/, RscEnum * /*pWeekDayFormat*/,
                                RscEnum * /*pMonthFormat*/ )
{
    HASHID      nId;
    RscTop *    pClassInt;

    // Klasse anlegen
    nId = pHS->Insert( "International" );
    pClassInt = new RscClass( nId, RSC_INTERNATIONAL, pSuper );
    pClassInt->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    aNmTb.Put( nId, CLASSNAME, pClassInt );

    // Variablen anlegen
    nId = aNmTb.Put( "TimeLeadingZero", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0, INT2_TIMELEADINGZERO  );

    nId = aNmTb.Put( "TimeAM", VARNAME );
    pClassInt->SetVariable( nId, &aString, NULL, 0, INT2_TIMEAM  );

    nId = aNmTb.Put( "TimePM", VARNAME );
    pClassInt->SetVariable( nId, &aString, NULL, 0, INT2_TIMEPM  );

    nId = aNmTb.Put( "NumLeadingZero", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0, INT2_NUMLEADINGZERO  );

    nId = aNmTb.Put( "NumDigits", VARNAME );
    pClassInt->SetVariable( nId, &aUShort, NULL, 0, INT2_NUMDIGITS  );

    nId = aNmTb.Put( "CurrPositiveFormat", VARNAME );
    pClassInt->SetVariable( nId, &aUShort, NULL, 0, INT2_CURRPOSITIVEFORMAT  );

    nId = aNmTb.Put( "CurrNegativeFormat", VARNAME );
    pClassInt->SetVariable( nId, &aUShort, NULL, 0, INT2_CURRNEGATIVEFORMAT  );

    nId = aNmTb.Put( "CurrDigits", VARNAME );
    pClassInt->SetVariable( nId, &aUShort, NULL, 0, INT2_CURRDIGITS  );

    nId = aNmTb.Put( "NumTrailingZeros", VARNAME );
    pClassInt->SetVariable( nId, &aBool, NULL, 0, INT2_NUMTRAILINGZEROS  );

    return pClassInt;
}

/*************************************************************************
|*    RscTypCont::InitClassPatternFormatter()
*************************************************************************/
RscTop * RscTypCont::InitClassPatternFormatter( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassPattern;

    // Klasse anlegen
    nId = pHS->Insert( "PatternFormatter" );
    pClassPattern = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassPattern->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "StrictFormat", VARNAME );
    pClassPattern->SetVariable( nId, &aBool, NULL,
                                0, PATTERNFORMATTER_STRICTFORMAT );
    nId = aNmTb.Put( "EditMask", VARNAME );
    pClassPattern->SetVariable( nId, &aLangString, NULL,
                                0, PATTERNFORMATTER_EDITMASK );
    nId = aNmTb.Put( "LiteralMask", VARNAME );
    pClassPattern->SetVariable( nId, &aLangString, NULL,
                                0, PATTERNFORMATTER_LITTERALMASK );

        return pClassPattern;
}

/*************************************************************************
|*    RscTypCont::InitClassNumericFormatter()
*************************************************************************/
RscTop * RscTypCont::InitClassNumericFormatter( RscTop * pSuper,
                                                                                        RscTop * pClassI12 )
{
    HASHID      nId;
    RscTop *    pClassNumeric;

    // Klasse anlegen
    nId = pHS->Insert( "NumericFormatter" );
    pClassNumeric = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassNumeric->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "Minimum", VARNAME );
    pClassNumeric->SetVariable( nId, &aIdLong, NULL,
                                0, NUMERICFORMATTER_MIN );
    nId = aNmTb.Put( "Maximum", VARNAME );
    pClassNumeric->SetVariable( nId, &aIdLong, NULL,
                                0, NUMERICFORMATTER_MAX );
    nId = aNmTb.Put( "StrictFormat", VARNAME );
    pClassNumeric->SetVariable( nId, &aBool, NULL,
                                0, NUMERICFORMATTER_STRICTFORMAT );
    nId = aNmTb.Put( "Intnl", VARNAME );
    pClassNumeric->SetVariable( nId, pClassI12, NULL,
                                0, NUMERICFORMATTER_I12 );
    nId = aNmTb.Put( "DecimalDigits", VARNAME );
    pClassNumeric->SetVariable( nId, &aUShort, NULL,
                                0, NUMERICFORMATTER_DECIMALDIGITS );
    nId = aNmTb.Put( "Value", VARNAME );
    pClassNumeric->SetVariable( nId, &aIdLong, NULL,
                                0, NUMERICFORMATTER_VALUE );

    return pClassNumeric;
}

/*************************************************************************
|*    RscTypCont::InitClassMetricFormatter()
*************************************************************************/
RscTop * RscTypCont::InitClassMetricFormatter( RscTop * pSuper,
                                                                                        RscEnum * pFieldUnits )
{
    HASHID      nId;
    RscTop *    pClassMetric;

    // Klasse anlegen
    nId = pHS->Insert( "MetricFormatter" );
    pClassMetric = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassMetric->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "Unit", VARNAME );
    pClassMetric->SetVariable( nId, pFieldUnits, NULL,
                               0, METRICFORMATTER_UNIT );
    nId = aNmTb.Put( "CustomUnitText", VARNAME );
    pClassMetric->SetVariable( nId, &aLangString, NULL,
                               0, METRICFORMATTER_CUSTOMUNITTEXT );

    return pClassMetric;
}

/*************************************************************************
|*    RscTypCont::InitClassCurrencyFormatter()
*************************************************************************/
RscTop * RscTypCont::InitClassCurrencyFormatter
(
    RscTop * pSuper,
    RscEnum * /* pFieldUnits */)
{
    HASHID      nId;
    RscTop *    pClassCurrency;

    // Klasse anlegen
    nId = pHS->Insert( "CurrencyFormatter" );
    pClassCurrency = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassCurrency->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    return pClassCurrency;
}

/*************************************************************************
|*    RscTypCont::InitClassDateFormatter()
*************************************************************************/
RscTop * RscTypCont::InitClassDateFormatter( RscTop * pSuper,
                        RscTop * pClassDate, RscTop * pClassI12 )
{
    HASHID      nId;
    RscTop *    pClassDateF;

    // Klasse anlegen
    nId = pHS->Insert( "DateFormatter" );
    pClassDateF = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassDateF->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "Minimum", VARNAME );
    pClassDateF->SetVariable( nId, pClassDate, NULL,
                                                                        0, DATEFORMATTER_MIN );
    nId = aNmTb.Put( "Maximum", VARNAME );
    pClassDateF->SetVariable( nId, pClassDate, NULL,
                                                                        0, DATEFORMATTER_MAX );
    nId = aNmTb.Put( "LongFormat", VARNAME );
    pClassDateF->SetVariable( nId, &aBool, NULL,
                                                                        0, DATEFORMATTER_LONGFORMAT );
    nId = aNmTb.Put( "StrictFormat", VARNAME );
    pClassDateF->SetVariable( nId, &aBool, NULL,
                                                                        0, DATEFORMATTER_STRICTFORMAT );
    nId = aNmTb.Put( "Intnl", VARNAME );
    pClassDateF->SetVariable( nId, pClassI12, NULL,
                                                                        0, DATEFORMATTER_I12 );
    nId = aNmTb.Put( "Value", VARNAME );
    pClassDateF->SetVariable( nId, pClassDate, NULL,
                                                                        0, DATEFORMATTER_VALUE );

        return pClassDateF;
}

/*************************************************************************
|*    RscTypCont::InitClassTimeFormatter()
*************************************************************************/
RscTop * RscTypCont::InitClassTimeFormatter( RscTop * pSuper,
                        RscTop * pClassTime, RscTop * pClassI12,
                        RscEnum * pTimeFieldFormat )
{
    HASHID      nId;
    RscTop *    pClassTimeF;

    // Klasse anlegen
    nId = pHS->Insert( "TimeFormatter" );
    pClassTimeF = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassTimeF->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "Minimum", VARNAME );
    pClassTimeF->SetVariable( nId, pClassTime, NULL,
                              0, TIMEFORMATTER_MIN );
    nId = aNmTb.Put( "Maximum", VARNAME );
    pClassTimeF->SetVariable( nId, pClassTime, NULL,
                              0, TIMEFORMATTER_MAX );
    nId = aNmTb.Put( "Format", VARNAME );
    pClassTimeF->SetVariable( nId, pTimeFieldFormat, NULL,
                              0, TIMEFORMATTER_TIMEFIELDFORMAT );
    nId = aNmTb.Put( "Duration", VARNAME );
    pClassTimeF->SetVariable( nId, &aBool, NULL,
                              0, TIMEFORMATTER_DURATION );
    nId = aNmTb.Put( "StrictFormat", VARNAME );
    pClassTimeF->SetVariable( nId, &aBool, NULL,
                              0, TIMEFORMATTER_STRICTFORMAT );
    nId = aNmTb.Put( "Intnl", VARNAME );
    pClassTimeF->SetVariable( nId, pClassI12, NULL,
                              0, TIMEFORMATTER_I12 );
    nId = aNmTb.Put( "Value", VARNAME );
    pClassTimeF->SetVariable( nId, pClassTime, NULL,
                              0, TIMEFORMATTER_VALUE );

    return pClassTimeF;
}

/*************************************************************************
|*    RscTypCont::InitClassSpinField()
*************************************************************************/
RscTop * RscTypCont::InitClassSpinField( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassSpinField;

    // Klasse anlegen
    nId = pHS->Insert( "SpinField" );
    pClassSpinField = new RscClass( nId, RSC_SPINFIELD, pSuper );
    pClassSpinField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassSpinField );

    INS_WINBIT(pClassSpinField,Repeat)
    INS_WINBIT(pClassSpinField,Spin)

    return pClassSpinField;
}

/*************************************************************************
|*    RscTypCont::InitClassPatternField()
*************************************************************************/
RscTop * RscTypCont::InitClassPatternField( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassPatternField;

    // Klasse anlegen
    nId = pHS->Insert( "PatternField" );
    pClassPatternField = new RscClass( nId, RSC_PATTERNFIELD, pSuper );
    pClassPatternField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassPatternField );

    return pClassPatternField;
}

/*************************************************************************
|*    RscTypCont::InitClassNumericField()
*************************************************************************/
RscTop * RscTypCont::InitClassNumericField( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassNumericField;

    // Klasse anlegen
    nId = pHS->Insert( "NumericField" );
    pClassNumericField = new RscClass( nId, RSC_NUMERICFIELD, pSuper );
    pClassNumericField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassNumericField );

    // Variablen anlegen
    nId = aNmTb.Put( "First", VARNAME );
    pClassNumericField->SetVariable( nId, &aIdLong, NULL,
                                                                        0, NUMERICFIELD_FIRST );
    nId = aNmTb.Put( "Last", VARNAME );
    pClassNumericField->SetVariable( nId, &aIdLong, NULL,
                                                                        0, NUMERICFIELD_LAST );
    nId = aNmTb.Put( "SpinSize", VARNAME );
    pClassNumericField->SetVariable( nId, &aIdLong, NULL,
                                                                        0, NUMERICFIELD_SPINSIZE  );

    return pClassNumericField;
}

/*************************************************************************
|*    RscTypCont::InitClassMetricField()
*************************************************************************/
RscTop * RscTypCont::InitClassMetricField( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassMetricField;

    // Klasse anlegen
    nId = pHS->Insert( "MetricField" );
    pClassMetricField = new RscClass( nId, RSC_METRICFIELD, pSuper );
    pClassMetricField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassMetricField );

    // Variablen anlegen
    nId = aNmTb.Put( "First", VARNAME );
    pClassMetricField->SetVariable( nId, &aIdLong, NULL,
                                    0, METRICFIELD_FIRST );
    nId = aNmTb.Put( "Last", VARNAME );
    pClassMetricField->SetVariable( nId, &aIdLong, NULL,
                                    0, METRICFIELD_LAST );
    nId = aNmTb.Put( "SpinSize", VARNAME );
    pClassMetricField->SetVariable( nId, &aIdLong, NULL,
                                    0, METRICFIELD_SPINSIZE  );

    return pClassMetricField;
}

/*************************************************************************
|*    RscTypCont::InitClassCurrencyField()
*************************************************************************/
RscTop * RscTypCont::InitClassCurrencyField
(
    const char * pClassName,
    USHORT nRT,
    RscTop * pSuper
)
{
    HASHID      nId;
    RscTop *    pClassCurrencyField;

    // Klasse anlegen
    nId = pHS->Insert( pClassName );
    pClassCurrencyField = new RscClass( nId, nRT, pSuper );
    pClassCurrencyField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassCurrencyField );

    // Variablen anlegen
    nId = aNmTb.Put( "First", VARNAME );
    pClassCurrencyField->SetVariable( nId, &aIdLong, NULL,
                                      0, CURRENCYFIELD_FIRST );
    nId = aNmTb.Put( "Last", VARNAME );
    pClassCurrencyField->SetVariable( nId, &aIdLong, NULL,
                                      0, CURRENCYFIELD_LAST );
    nId = aNmTb.Put( "SpinSize", VARNAME );
    pClassCurrencyField->SetVariable( nId, &aIdLong, NULL,
                                      0, CURRENCYFIELD_SPINSIZE  );

    return pClassCurrencyField;
}

/*************************************************************************
|*    RscTypCont::InitClassDateField()
*************************************************************************/
RscTop * RscTypCont::InitClassDateField( RscTop * pSuper, RscTop * pClassDate )
{
    HASHID      nId;
    RscTop *    pClassDateField;

    // Klasse anlegen
    nId = pHS->Insert( "DateField" );
    pClassDateField = new RscClass( nId, RSC_DATEFIELD, pSuper );
    pClassDateField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassDateField );

    // Variablen anlegen
    nId = aNmTb.Put( "First", VARNAME );
    pClassDateField->SetVariable( nId, pClassDate, NULL, 0, DATEFIELD_FIRST );
    nId = aNmTb.Put( "Last", VARNAME );
    pClassDateField->SetVariable( nId, pClassDate, NULL, 0, DATEFIELD_LAST );

    return pClassDateField;
}

/*************************************************************************
|*    RscTypCont::InitClassTimeField()
*************************************************************************/
RscTop * RscTypCont::InitClassTimeField( RscTop * pSuper, RscTop * pClassTime )
{
    HASHID      nId;
    RscTop *    pClassTimeField;

    // Klasse anlegen
    nId = pHS->Insert( "TimeField" );
    pClassTimeField = new RscClass( nId, RSC_TIMEFIELD, pSuper );
    pClassTimeField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassTimeField );

    // Variablen anlegen
    nId = aNmTb.Put( "First", VARNAME );
    pClassTimeField->SetVariable( nId, pClassTime, NULL, 0, TIMEFIELD_FIRST );
    nId = aNmTb.Put( "Last", VARNAME );
    pClassTimeField->SetVariable( nId, pClassTime, NULL, 0, TIMEFIELD_LAST );

    return pClassTimeField;
}

/*************************************************************************
|*    RscTypCont::InitClassPatternBox()
*************************************************************************/
RscTop * RscTypCont::InitClassPatternBox( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassPatternBox;

    // Klasse anlegen
    nId = pHS->Insert( "PatternBox" );
    pClassPatternBox = new RscClass( nId, RSC_PATTERNBOX, pSuper );
    pClassPatternBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassPatternBox );

    return pClassPatternBox;
}

/*************************************************************************
|*    RscTypCont::InitClassNumericBox()
*************************************************************************/
RscTop * RscTypCont::InitClassNumericBox( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassNumericBox;

    // Klasse anlegen
    nId = pHS->Insert( "NumericBox" );
    pClassNumericBox = new RscClass( nId, RSC_NUMERICBOX, pSuper );
    pClassNumericBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassNumericBox );

    // Variablen anlegen

    return pClassNumericBox;
}

/*************************************************************************
|*    RscTypCont::InitClassMetricBox()
*************************************************************************/
RscTop * RscTypCont::InitClassMetricBox( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassMetricBox;

    // Klasse anlegen
    nId = pHS->Insert( "MetricBox" );
    pClassMetricBox = new RscClass( nId, RSC_METRICBOX, pSuper );
    pClassMetricBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassMetricBox );

    // Variablen anlegen

    return pClassMetricBox;
}

/*************************************************************************
|*    RscTypCont::InitClassCurrencyBox()
*************************************************************************/
RscTop * RscTypCont::InitClassCurrencyBox
(
    const char * pClassName,
    USHORT nRT,
    RscTop * pSuper
)
{
    HASHID      nId;
    RscTop *    pClassCurrencyBox;

    // Klasse anlegen
    nId = pHS->Insert( pClassName );
    pClassCurrencyBox = new RscClass( nId, nRT, pSuper );
    pClassCurrencyBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassCurrencyBox );

    // Variablen anlegen

    return pClassCurrencyBox;
}

/*************************************************************************
|*    RscTypCont::InitClassDateBox()
*************************************************************************/
RscTop * RscTypCont::InitClassDateBox( RscTop * pSuper,
                                       RscTop * /*pClassDate*/ )
{
    HASHID      nId;
    RscTop *    pClassDateBox;

    // Klasse anlegen
    nId = pHS->Insert( "DateBox" );
    pClassDateBox = new RscClass( nId, RSC_DATEBOX, pSuper );
    pClassDateBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassDateBox );

    // Variablen anlegen

    return pClassDateBox;
}

/*************************************************************************
|*    RscTypCont::InitClassTimeBox()
*************************************************************************/
RscTop * RscTypCont::InitClassTimeBox( RscTop * pSuper,
                                       RscTop * /*pClassTime*/ )
{
    HASHID      nId;
    RscTop *    pClassTimeBox;

    // Klasse anlegen
    nId = pHS->Insert( "TimeBox" );
    pClassTimeBox = new RscClass( nId, RSC_TIMEBOX, pSuper );
    pClassTimeBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassTimeBox );

    // Variablen anlegen

    return pClassTimeBox;
}

/*************************************************************************
|*    RscTypCont::InitClassDockWindow()
*************************************************************************/
RscTop * RscTypCont::InitClassDockingWindow( RscTop * pSuper,
                                             RscEnum * pMapUnit )
{
    HASHID      nId;
    RscTop *    pClassDockWindow;

    // Klasse anlegen
    nId = pHS->Insert( "DockingWindow" );
    pClassDockWindow = new RscClass( nId, RSC_DOCKINGWINDOW, pSuper );
    pClassDockWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassDockWindow );

    // Variablen anlegen
    nId = aNmTb.Put( "_FloatingPosMapMode", VARNAME );
    pClassDockWindow->SetVariable( nId, pMapUnit, NULL, 0,
                                   RSC_DOCKINGWINDOW_XYMAPMODE  );
    nId = aNmTb.Put( "_FloatingPosX", VARNAME );
    pClassDockWindow->SetVariable( nId, &aShort, NULL, 0,
                                   RSC_DOCKINGWINDOW_X );
    nId = aNmTb.Put( "_FloatingPosY", VARNAME );
    pClassDockWindow->SetVariable( nId, &aShort, NULL, 0,
                                   RSC_DOCKINGWINDOW_Y );
    nId = aNmTb.Put( "FloatingMode", VARNAME );
    pClassDockWindow->SetVariable( nId, &aBool, NULL, 0,
                                   RSC_DOCKINGWINDOW_FLOATING );

    INS_WINBIT(pClassDockWindow,Moveable)
    INS_WINBIT(pClassDockWindow,Sizeable)
    INS_WINBIT(pClassDockWindow,EnableResizing)
    INS_WINBIT(pClassDockWindow,Closeable)
    INS_WINBIT(pClassDockWindow,HideWhenDeactivate);
    INS_WINBIT(pClassDockWindow,Zoomable);
    INS_WINBIT(pClassDockWindow,Dockable);

    return pClassDockWindow;
}

/*************************************************************************
|*    RscTypCont::InitClassToolBoxItem()
*************************************************************************/
RscTop * RscTypCont::InitClassToolBoxItem( RscTop * pSuper,
                                           RscTop * pClassBitmap,
                                           RscTop * pClassImage,
                                           RscEnum * pTriState )
{
    HASHID      nId;
    RscTop *    pClassToolBoxItem;

    // Klasse anlegen
    nId = pHS->Insert( "ToolBoxItem" );
    pClassToolBoxItem = new RscClass( nId, RSC_TOOLBOXITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassToolBoxItem );

    // Variablen anlegen
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aIdNoZeroUShort, NULL, 0,
                                    RSC_TOOLBOXITEM_ID );
    {
        RscEnum   * pEnum;

        aBaseLst.Insert(
            pEnum = new RscEnum( pHS->Insert( "EnumToolBoxItemType" ),
                                 RSC_NOTYPE ), LIST_APPEND );
        SETCONST( pEnum, "TOOLBOXITEM_BUTTON", TOOLBOXITEM_BUTTON );
        SETCONST( pEnum, "TOOLBOXITEM_SPACE", TOOLBOXITEM_SPACE );
        SETCONST( pEnum, "TOOLBOXITEM_SEPARATOR", TOOLBOXITEM_SEPARATOR );
        SETCONST( pEnum, "TOOLBOXITEM_BREAK", TOOLBOXITEM_BREAK );

        // Variable einfuegen
        nId = aNmTb.Put( "Type", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pEnum, NULL, 0,
                                  RSC_TOOLBOXITEM_TYPE  );
    }
    {
        RscFlag *   pFlag;
        RscClient * pClient;
        HASHID      nVarId, nAutoCheckId, nRadioCheckId, nCheckableId, nLeftId, nAutoSizeId, nDropDownId;

        aBaseLst.Insert( pFlag = new RscFlag( pHS->Insert( "FlagToolBoxState" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );

        // Konstanten in Tabelle stellen
        nCheckableId = pHS->Insert( "TIB_CHECKABLE" );
        SETCONST( pFlag, nCheckableId, TIB_CHECKABLE );
        nAutoCheckId = pHS->Insert( "TIB_AUTOCHECK" );
        SETCONST( pFlag, nAutoCheckId, TIB_AUTOCHECK );
        nRadioCheckId = pHS->Insert( "TIB_RADIOCHECK" );
        SETCONST( pFlag, nRadioCheckId, TIB_RADIOCHECK );
        nLeftId = pHS->Insert( "TIB_LEFT" );
        SETCONST( pFlag, nLeftId, TIB_LEFT );
        nAutoSizeId = pHS->Insert( "TIB_AUTOSIZE" );
        SETCONST( pFlag, nAutoSizeId, TIB_AUTOSIZE );
        nDropDownId = pHS->Insert( "TIB_DROPDOWN" );
        SETCONST( pFlag, nDropDownId, TIB_DROPDOWN );

        // Variable einfuegen
        nVarId = aNmTb.Put( "_ToolBoxItemFlags", VARNAME );
        pClassToolBoxItem->SetVariable( nVarId, pFlag, NULL,
                                     VAR_HIDDEN | VAR_NOENUM,
                                     RSC_TOOLBOXITEM_STATUS );

        // Clientvariablen einfuegen
        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nCheckableId ), LIST_APPEND );
        nId = aNmTb.Put( "Checkable", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nAutoCheckId ), LIST_APPEND );
        nId = aNmTb.Put( "AutoCheck", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nRadioCheckId ), LIST_APPEND );
        nId = aNmTb.Put( "RadioCheck", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nLeftId ), LIST_APPEND );
        nId = aNmTb.Put( "Left", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nAutoSizeId ), LIST_APPEND );
        nId = aNmTb.Put( "AutoSize", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, nVarId );

        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     pFlag, nDropDownId ), LIST_APPEND );
        nId = aNmTb.Put( "DropDown", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, nVarId );
    }
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aIdLong, NULL, 0,
                                    RSC_TOOLBOXITEM_HELPID  );
    nId = aNmTb.Put( "Text", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aLangString, NULL, 0,
                                    RSC_TOOLBOXITEM_TEXT );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aLangString, NULL, 0,
                                    RSC_TOOLBOXITEM_HELPTEXT );
    nId = aNmTb.Put( "ItemBitmap", VARNAME );
    pClassToolBoxItem->SetVariable( nId, pClassBitmap, NULL, 0,
                                    RSC_TOOLBOXITEM_BITMAP );
    nId = aNmTb.Put( "ItemImage", VARNAME );
    pClassToolBoxItem->SetVariable( nId, pClassImage, NULL, 0,
                                    RSC_TOOLBOXITEM_IMAGE );
    nId = aNmTb.Put( "Disable", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aBool, NULL, 0,
                                    RSC_TOOLBOXITEM_DISABLE );

    nId = aNmTb.Put( "State", VARNAME );
    pClassToolBoxItem->SetVariable( nId, pTriState, NULL, 0,
                                    RSC_TOOLBOXITEM_STATE );
    nId = aNmTb.Put( "Hide", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aBool, NULL, 0,
                                    RSC_TOOLBOXITEM_HIDE );
    nId = aNmTb.Put( "Hide", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aBool, NULL, 0,
                                    RSC_TOOLBOXITEM_HIDE );
    nId = aNmTb.Put( "Command", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aString, NULL, 0,
                                    RSC_TOOLBOXITEM_COMMAND );

    return pClassToolBoxItem;
}

/*************************************************************************
|*    RscTypCont::InitClassToolBox()
*************************************************************************/
RscTop * RscTypCont::InitClassToolBox( RscTop * pSuper,
                                       RscTop * pClassToolBoxItem,
                                       RscTop * pClassImageList )
{
    HASHID      nId;
    RscTop *    pClassToolBox;

    // Klasse anlegen
    nId = pHS->Insert( "ToolBox" );
    pClassToolBox = new RscClass( nId, RSC_TOOLBOX, pSuper );
    pClassToolBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassToolBox );

    // Variablen anlegen
    {
        RscEnum   * pEnum;

        aBaseLst.Insert(
            pEnum = new RscEnum( pHS->Insert( "EnumButtonType" ),
                                 RSC_NOTYPE ), LIST_APPEND );
        SETCONST( pEnum, "BUTTON_SYMBOL",               BUTTON_SYMBOL );
        SETCONST( pEnum, "BUTTON_TEXT",                 BUTTON_TEXT );
        SETCONST( pEnum, "BUTTON_SYMBOLTEXT",   BUTTON_SYMBOLTEXT );

        // Variable einfuegen
        nId = aNmTb.Put( "ButtonType", VARNAME );
        pClassToolBox->SetVariable( nId, pEnum, NULL, 0,
                                  RSC_TOOLBOX_BUTTONTYPE  );
    }
    {
        RscEnum   * pEnum;

        aBaseLst.Insert(
            pEnum = new RscEnum( pHS->Insert( "EnumToolBoxAlign" ),
                                 RSC_NOTYPE ), LIST_APPEND );
        SETCONST( pEnum, "BOXALIGN_TOP",                BOXALIGN_TOP );
        SETCONST( pEnum, "BOXALIGN_LEFT",               BOXALIGN_LEFT );
        SETCONST( pEnum, "BOXALIGN_RIGHT",              BOXALIGN_RIGHT );
        SETCONST( pEnum, "BOXALIGN_BOTTOM",             BOXALIGN_BOTTOM );

        // Variable einfuegen
        nId = aNmTb.Put( "Align", VARNAME );
        pClassToolBox->SetVariable( nId, pEnum, NULL, 0,
                                  RSC_TOOLBOX_ALIGN  );
    }
    nId = aNmTb.Put( "LineCount", VARNAME );
    pClassToolBox->SetVariable( nId, &aIdNoZeroUShort, NULL, 0,
                                RSC_TOOLBOX_LINECOUNT  );
    nId = aNmTb.Put( "FloatingLines", VARNAME );
    pClassToolBox->SetVariable( nId, &aUShort, NULL, 0,
                                RSC_TOOLBOX_FLOATLINES  );
    nId = aNmTb.Put( "Customize", VARNAME );
    pClassToolBox->SetVariable( nId, &aBool, NULL, 0,
                                RSC_TOOLBOX_CUSTOMIZE );
    nId = aNmTb.Put( "MenuStrings", VARNAME );
    pClassToolBox->SetVariable( nId, &aBool, NULL, 0,
                                RSC_TOOLBOX_MENUSTRINGS );
    nId = aNmTb.Put( "ItemImageList", VARNAME );
    pClassToolBox->SetVariable( nId, pClassImageList, NULL, 0,
                                RSC_TOOLBOX_ITEMIMAGELIST );
    {
        RscCont * pCont;

        aBaseLst.Insert( pCont = new RscCont( pHS->Insert( "ContToolBoxItem" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );
        pCont->SetTypeClass( pClassToolBoxItem );
        nId = aNmTb.Put( "ItemList", VARNAME );
        pClassToolBox->SetVariable( nId, pCont, NULL, 0,
                                    RSC_TOOLBOX_ITEMLIST );
    }
    INS_WINBIT(pClassToolBox,Scroll)
    INS_WINBIT(pClassToolBox,LineSpacing)
    INS_WINBIT(pClassToolBox,RectStyle)

    return pClassToolBox;
}

/*************************************************************************
|*    RscTypCont::InitClassStatusBar()
*************************************************************************/
RscTop * RscTypCont::InitClassStatusBar( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassStatusBar;

    // Klasse anlegen
    nId = pHS->Insert( "StatusBar" );
    pClassStatusBar = new RscClass( nId, RSC_STATUSBAR, pSuper );
    pClassStatusBar->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassStatusBar );

    // Variablen anlegen
    INS_WINBIT(pClassStatusBar,Left)
    INS_WINBIT(pClassStatusBar,Right)

    return pClassStatusBar;
}

/*************************************************************************
|*    RscTypCont::InitClassMoreButton()
*************************************************************************/
RscTop * RscTypCont::InitClassMoreButton( RscTop * pSuper, RscEnum * pMapUnit )
{
    HASHID      nId;
    RscTop *    pClassMoreButton;

    // Klasse anlegen
    nId = pHS->Insert( "MoreButton" );
    pClassMoreButton = new RscClass( nId, RSC_MOREBUTTON, pSuper );
    pClassMoreButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassMoreButton );

    // Variablen anlegen
    nId = aNmTb.Put( "State", VARNAME );
    pClassMoreButton->SetVariable( nId, &aBool, NULL, 0,
                                   RSC_MOREBUTTON_STATE );
    nId = aNmTb.Put( "MapUnit", VARNAME );
    pClassMoreButton->SetVariable( nId, pMapUnit, NULL, 0,
                                   RSC_MOREBUTTON_MAPUNIT );
    nId = aNmTb.Put( "Delta", VARNAME );
    pClassMoreButton->SetVariable( nId, &aUShort, NULL, 0,
                                   RSC_MOREBUTTON_DELTA );

    return pClassMoreButton;
}

/*************************************************************************
|*    RscTypCont::InitClassFloatingWindow()
*************************************************************************/
RscTop * RscTypCont::InitClassFloatingWindow( RscTop * pSuper,
                                              RscEnum * pMapUnit )
{
    HASHID      nId;
    RscTop *    pClassFloatingWindow;

    // Klasse anlegen
    nId = pHS->Insert( "FloatingWindow" );
    pClassFloatingWindow = new RscClass( nId, RSC_FLOATINGWINDOW, pSuper );
    pClassFloatingWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFloatingWindow );

    // Variablen anlegen
    nId = aNmTb.Put( "_ZoomInMapMode", VARNAME );
    pClassFloatingWindow->SetVariable( nId, pMapUnit, NULL, 0,
                                       RSC_FLOATINGWINDOW_WHMAPMODE  );
    nId = aNmTb.Put( "_ZoomInWidth", VARNAME );
    pClassFloatingWindow->SetVariable( nId, &aShort, NULL, 0,
                                       RSC_FLOATINGWINDOW_WIDTH  );
    nId = aNmTb.Put( "_ZoomInHeight", VARNAME );
    pClassFloatingWindow->SetVariable( nId, &aShort, NULL, 0,
                                       RSC_FLOATINGWINDOW_HEIGHT  );
    nId = aNmTb.Put( "ZoomIn", VARNAME );
    pClassFloatingWindow->SetVariable( nId, &aBool, NULL, 0,
                                       RSC_FLOATINGWINDOW_ZOOMIN );

    INS_WINBIT(pClassFloatingWindow,Zoomable)
    INS_WINBIT(pClassFloatingWindow,HideWhenDeactivate)
    INS_WINBIT(pClassFloatingWindow,EnableResizing)

    return pClassFloatingWindow;
}

/*************************************************************************
|*    RscTypCont::InitClassTabControlItem()
*************************************************************************/
RscTop * RscTypCont::InitClassTabControlItem( RscTop * pSuper,
                                              RscTop * pClassTabPage )
{
    HASHID      nId;
    RscTop *    pClassTabControlItem;

    // Klasse anlegen
    nId = pHS->Insert( "PageItem" );
    pClassTabControlItem = new RscClass( nId, RSC_TABCONTROLITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassTabControlItem );

    // Variablen anlegen
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassTabControlItem->SetVariable( nId, &aIdNoZeroUShort, NULL, 0,
                                       RSC_TABCONTROLITEM_ID );
    nId = aNmTb.Put( "Text", VARNAME );
    pClassTabControlItem->SetVariable( nId, &aLangString, NULL, 0,
                                       RSC_TABCONTROLITEM_TEXT );
    nId = aNmTb.Put( "PageResID", VARNAME );
    pClassTabControlItem->SetVariable( nId, &aIdUShort, NULL, 0,
                                       RSC_TABCONTROLITEM_PAGERESID );

    return pClassTabControlItem;
}

/*************************************************************************
|*    RscTypCont::InitClassTabControl()
*************************************************************************/
RscTop * RscTypCont::InitClassTabControl( RscTop * pSuper,
                                          RscTop * pClassTabControlItem )
{
    HASHID      nId;
    RscTop *    pClassTabControl;

    // Klasse anlegen
    nId = pHS->Insert( "TabControl" );
    pClassTabControl = new RscClass( nId, RSC_TABCONTROL, pSuper );
    pClassTabControl->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassTabControl );

    // Variablen anlegen
    {
        RscCont * pCont;

        aBaseLst.Insert( pCont = new RscCont( pHS->Insert( "ContTabControlItem" ),
                                              RSC_NOTYPE ),
                         LIST_APPEND );
        pCont->SetTypeClass( pClassTabControlItem );
        nId = aNmTb.Put( "PageList", VARNAME );
        pClassTabControl->SetVariable( nId, pCont, NULL, 0,
                                       RSC_TABCONTROL_ITEMLIST );
    }

    return pClassTabControl;
}

/*************************************************************************
|*    RscTypCont::InitClassSfxFamilyStyleItem()
*************************************************************************/
RscTop * RscTypCont::InitClassSfxStyleFamilyItem( RscTop * pSuper,
                                                  RscTop * pClassBitmap,
                                                  RscTop * pClassImage,
                                                  RscArray * pStrLst )
{
    HASHID      nId;
    RscTop *    pClassSfxFamilyStyleItem;

    // Klasse anlegen
    nId = pHS->Insert( "SfxStyleFamilyItem" );
    pClassSfxFamilyStyleItem = new RscClass( nId, RSC_SFX_STYLE_FAMILY_ITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassSfxFamilyStyleItem );

    nId = aNmTb.Put( "FilterList", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, pStrLst, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_LIST );
    nId = aNmTb.Put( "StyleBitmap", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, pClassBitmap, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_BITMAP );
    nId = aNmTb.Put( "Text", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, &aLangString, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_TEXT );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, &aLangString, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_HELPTEXT );
    {
        RscEnum * pSfxStyleFamily;
        pSfxStyleFamily = new RscEnum( pHS->Insert( "StyleFamily" ),
                                    RSC_NOTYPE );

        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_PARA", SFX_STYLE_FAMILY_PARA );
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_CHAR", SFX_STYLE_FAMILY_CHAR );
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_FRAME",SFX_STYLE_FAMILY_FRAME);
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_PAGE", SFX_STYLE_FAMILY_PAGE );
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_PSEUDO", SFX_STYLE_FAMILY_PSEUDO );
        aBaseLst.Insert( pSfxStyleFamily );

        nId = aNmTb.Put( "StyleFamily", VARNAME );
        pClassSfxFamilyStyleItem->SetVariable( nId, pSfxStyleFamily, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_STYLEFAMILY );
    }
    nId = aNmTb.Put( "StyleImage", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, pClassImage, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_IMAGE );
    return pClassSfxFamilyStyleItem;
}

/*************************************************************************
|*    RscTypCont::InitClassSfxTemplateDialogm()
*************************************************************************/
RscTop * RscTypCont::InitClassSfxTemplateDialog( RscTop * pSuper,
                                                 RscTop * pClassFamilyStyleItem )
{
    HASHID      nId;
    RscTop *    pClassSfxTemplateDialog;

    // Klasse anlegen
    nId = pHS->Insert( "SfxStyleFamilies" );
    pClassSfxTemplateDialog = new RscClass( nId, RSC_SFX_STYLE_FAMILIES, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassSfxTemplateDialog );

    // Variablen anlegen
    {
        RscCont * pCont;

        aBaseLst.Insert( pCont = new RscCont(
                                   pHS->Insert( "ContFamilyStyleItem" ),
                                   RSC_NOTYPE ),
                         LIST_APPEND );
        pCont->SetTypeClass( pClassFamilyStyleItem );
        nId = aNmTb.Put( "StyleFamilyList", VARNAME );
        pClassSfxTemplateDialog->SetVariable( nId, pCont );
    }

    return pClassSfxTemplateDialog;
}

/*************************************************************************
|*    RscTypCont::InitClassSfxSlotInfo()
*************************************************************************/
RscTop * RscTypCont::InitClassSfxSlotInfo( RscTop * pSuper )
{
    HASHID      nId;
    RscTop *    pClassSfxSlotInfo;

    // Klasse anlegen
    nId = pHS->Insert( "SfxSlotInfo" );
    pClassSfxSlotInfo = new RscClass( nId, RSC_SFX_SLOT_INFO, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassSfxSlotInfo );

    nId = aNmTb.Put( "SlotName", VARNAME );
    pClassSfxSlotInfo->SetVariable( nId, &aLangString, NULL, 0,
                                    RSC_SFX_SLOT_INFO_SLOTNAME );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassSfxSlotInfo->SetVariable( nId, &aLangString, NULL, 0,
                                    RSC_SFX_SLOT_INFO_HELPTEXT );
    return pClassSfxSlotInfo;
}


