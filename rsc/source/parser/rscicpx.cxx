/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <stdlib.h>
#include <stdio.h>

#include <tools/rc.h>
#include <tools/wintypes.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>

#include <rscmgr.hxx>
#include <rscclass.hxx>
#include <rsccont.hxx>
#include <rscdb.hxx>
#include <rsc/rscsfx.hxx>
#include <rsclex.hxx>
#include <rscyacc.hxx>

void RscTypCont::InsWinBit( RscTop * pClass, const OString& rName,
                            Atom nVal )
{
    RscClient * pClient;

    // Clientvariablen einfuegen
    aBaseLst.push_back(
        pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, &aWinBits, nVal )
    );
    Atom nId = aNmTb.Put( rName.getStr(), VARNAME );
    pClass->SetVariable( nId, pClient, NULL,
                         VAR_NODATAINST, 0, nWinBitVarId );
}

#define INS_WINBIT( pClass, WinBit )        \
    InsWinBit( pClass, #WinBit, n##WinBit##Id );

RscTop * RscTypCont::InitClassMgr()
{
    RscTop      *   pClassMgr;
    RscBaseCont *   pClass;
    Atom            nId;

    aBaseLst.push_back( pClass = new RscBaseCont( InvalidAtom, RSC_NOTYPE, NULL, false ) );

    nId = pHS->getID( "Resource" );
    pClassMgr = new RscMgr( nId, RSC_RESOURCE, pClass );
    aNmTb.Put( nId, CLASSNAME, pClassMgr );
    pClassMgr->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    {
        RscContWriteSrc *   pCont;

        // Variablen anlegen
        aBaseLst.push_back( pCont = new RscContExtraData( pHS->getID( "ContExtradata" ), RSC_NOTYPE ) );
        pCont->SetTypeClass( &aShort, &aString );
        nRsc_EXTRADATA = nId = aNmTb.Put( "ExtraData", VARNAME );
        pClassMgr->SetVariable( nId, pCont );
    };
    nId = aNmTb.Put( "Comment", VARNAME );
    pClassMgr->SetVariable( nId, &aString, NULL, VAR_NORC );

    pClass->SetTypeClass( pClassMgr );

    return pClassMgr;
}

RscTop * RscTypCont::InitClassString( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassString;

    nId = pHS->getID( "String" );
    pClassString = new RscClass( nId, RSC_STRING, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassString );
    pClassString->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // Variablen anlegen
    nId = aNmTb.Put( "Text", VARNAME );
    pClassString->SetVariable( nId, &aLangString );
    return pClassString;
}

RscTop * RscTypCont::InitClassBitmap( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassBitmap;

    nId = pHS->getID( "Bitmap" );
    pClassBitmap = new RscSysDepend( nId, RSC_BITMAP, pSuper );
    pClassBitmap->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassBitmap );

    // Die Klasse RscSysDepend behandelt die Variablen
    // "FILE" gesondert
    nId = aNmTb.Put( "File", VARNAME );
    pClassBitmap->SetVariable( nId, &aLangString, NULL, VAR_NORC );

    return pClassBitmap;
}

RscTop * RscTypCont::InitClassColor( RscTop * pSuper, RscEnum * pColor )
{
    Atom        nId;
    RscTop *    pClassColor;

    // Klasse anlegen
    nId = pHS->getID( "Color" );
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

    return pClassColor;
}

RscTop * RscTypCont::InitClassImage( RscTop * pSuper, RscTop * pClassBitmap,
                                     RscTop * pClassColor )
{
    Atom        nId;
    RscTop *    pClassImage;

    // Klasse anlegen
    nId = pHS->getID( "Image" );
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

    return pClassImage;
}

RscTop * RscTypCont::InitClassImageList( RscTop * pSuper,
                                         RscTop * pClassColor, RscCont * pStrLst )
{
    Atom        nId;
    RscTop *    pClassImageList;

    // Klasse anlegen
    nId = pHS->getID( "ImageList" );
    pClassImageList = new RscClass( nId, RSC_IMAGELIST, pSuper );
    pClassImageList->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassImageList );

    nId = aNmTb.Put( "Prefix", VARNAME );
    pClassImageList->SetVariable( nId, &aString );

    nId = aNmTb.Put( "MaskColor", VARNAME );
    pClassImageList->SetVariable( nId, pClassColor, NULL,
                                  VAR_SVDYNAMIC, RSC_IMAGELIST_MASKCOLOR );

    RscCont * pCont = new RscCont( pHS->getID( "sal_uInt16 *" ), RSC_NOTYPE );
    pCont->SetTypeClass( &aIdUShort );
    aBaseLst.push_back( pCont );
    nId = aNmTb.Put( "IdList", VARNAME );
    pClassImageList->SetVariable( nId, pCont, NULL, 0,
                                      RSC_IMAGELIST_IDLIST );

    nId = aNmTb.Put( "FileList", VARNAME );
    pClassImageList->SetVariable( nId, pStrLst );

    nId = aNmTb.Put( "IdCount", VARNAME );
    pClassImageList->SetVariable( nId, &aUShort, NULL, 0,
                                  RSC_IMAGELIST_IDCOUNT );
    return pClassImageList;
}

RscTop * RscTypCont::InitClassWindow( RscTop * pSuper, RscEnum * pMapUnit,
                                      RscArray * pLangGeo )
{
    Atom        nId;
    RscTop *    pClassWindow;

    // Klasse anlegen
    nId = pHS->getID( "Window" );
    pClassWindow = new RscClass( nId, RSC_WINDOW, pSuper );
    pClassWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassWindow );

    // Variablen anlegen
    {
        RscFlag *   pFlag;
        RscClient * pClient;
        Atom        nVarId, nDisableId, nOutputSizeId;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagWndExtra" ), RSC_NOTYPE ) );

        // Konstanten in Tabelle stellen
        nDisableId = pHS->getID( "RSWND_DISABLE" );
        SETCONST( pFlag, nDisableId, RSWND_DISABLED );
        nOutputSizeId = pHS->getID( "RSWND_OUTPUTSIZE" );
        SETCONST( pFlag, nOutputSizeId, RSWND_CLIENTSIZE );

        // Variable einfuegen
        nVarId = aNmTb.Put( "_RscExtraFlags", VARNAME );
        pClassWindow->SetVariable( nVarId, pFlag, NULL,
                                                                        VAR_HIDDEN | VAR_NOENUM );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nDisableId )
        );
        nId = aNmTb.Put( "Disable", VARNAME );
        pClassWindow->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nOutputSizeId )
        );
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
    pClassWindow->SetVariable( nId, &aStringLiteral, NULL, 0, WINDOW_HELPID );


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
    pClassWindow->SetVariable( nId, &aString, NULL, 0, WINDOW_UNIQUEID );

    // BorderStyle
    RscEnum* pBorderStyleEnum = new RscEnum( pHS->getID( "WindowBorderStyle" ), RSC_NOTYPE );
    aBaseLst.push_back( pBorderStyleEnum );

    // Konstanten in Tabelle stellen
    SETCONST( pBorderStyleEnum, pHS->getID( "WINDOW_BORDER_NORMAL" ),    WINDOW_BORDER_NORMAL );
    SETCONST( pBorderStyleEnum, pHS->getID( "WINDOW_BORDER_MONO" ),      WINDOW_BORDER_MONO );
    SETCONST( pBorderStyleEnum, pHS->getID( "WINDOW_BORDER_MENU" ),      WINDOW_BORDER_MENU );
    SETCONST( pBorderStyleEnum, pHS->getID( "WINDOW_BORDER_NOBORDER" ),  WINDOW_BORDER_NOBORDER );

    // Variable einfuegen
    nId = aNmTb.Put( "BorderStyle", VARNAME );
    pClassWindow->SetVariable( nId, pBorderStyleEnum, NULL,
                                    0,
                                    WINDOW_BORDER_STYLE );

    return pClassWindow;
}

RscTop * RscTypCont::InitClassSystemWindow( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassSystemWindow;

    // Klasse anlegen
    nId = pHS->getID( "SystemWindow" );
    pClassSystemWindow = new RscClass( nId, RSC_SYSWINDOW, pSuper );
    pClassSystemWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassSystemWindow );

    INS_WINBIT(pClassSystemWindow,Sizeable)
    INS_WINBIT(pClassSystemWindow,Moveable)
    InsWinBit( pClassSystemWindow, "Minable", nMinimizeId );
    InsWinBit( pClassSystemWindow, "Maxable", nMaximizeId );
    INS_WINBIT(pClassSystemWindow,Closeable)
    INS_WINBIT(pClassSystemWindow,App)
    INS_WINBIT(pClassSystemWindow,SysWin)

    return pClassSystemWindow ;
}

RscTop * RscTypCont::InitClassWorkWindow( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassWorkWindow;

    // Klasse anlegen
    nId = pHS->getID( "WorkWindow" );
    pClassWorkWindow = new RscClass( nId, RSC_WORKWIN, pSuper );
    pClassWorkWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassWorkWindow );

    // Variablen anlegen
    {
        Atom        nVarId;
        RscEnum   * pShow;

        aBaseLst.push_back( pShow = new RscEnum( pHS->getID( "EnumShowState" ), RSC_NOTYPE ) );

        SETCONST( pShow, "SHOW_NORMAL",        WORKWIN_SHOWNORMAL    );
        SETCONST( pShow, "SHOW_MINIMIZED",     WORKWIN_SHOWMINIMIZED );
        SETCONST( pShow, "SHOW_MAXIMIZED",     WORKWIN_SHOWMAXIMIZED );


        // Variable einfuegen
        nVarId = aNmTb.Put( "Show", VARNAME );
        pClassWorkWindow->SetVariable( nVarId, pShow, NULL );
    }

    return pClassWorkWindow;
}

RscTop * RscTypCont::InitClassModalDialog( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassDialog;

    // Klasse anlegen
    nId = pHS->getID( "ModalDialog" );
    pClassDialog = new RscClass( nId, RSC_MODALDIALOG, pSuper );
    pClassDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassDialog );

    InsWinBit( pClassDialog, "SysModal", nSysmodalId );

    return pClassDialog;
}

RscTop * RscTypCont::InitClassModelessDialog( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassDialog;

    // Klasse anlegen
    nId = pHS->getID( "ModelessDialog" );
    pClassDialog = new RscClass( nId, RSC_MODELESSDIALOG, pSuper );
    pClassDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassDialog );

    return pClassDialog;
}

RscTop * RscTypCont::InitClassControl( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassControl;

    // Klasse anlegen
    nId = pHS->getID( "Control" );
    pClassControl = new RscClass( nId, RSC_CONTROL, pSuper );
    pClassControl->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassControl );

    InsWinBit( pClassControl, "TabStop", nTabstopId );
    INS_WINBIT(pClassControl,Group)

    return pClassControl;
}

RscTop * RscTypCont::InitClassCheckBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassCheckBox;

    // Klasse anlegen
    nId = pHS->getID( "CheckBox" );
    pClassCheckBox = new RscClass( nId, RSC_CHECKBOX, pSuper );
    pClassCheckBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassCheckBox );

    // Variablen anlegen
    INS_WINBIT( pClassCheckBox, WordBreak )
    INS_WINBIT( pClassCheckBox, Top )
    INS_WINBIT( pClassCheckBox, VCenter )
    INS_WINBIT( pClassCheckBox, Bottom )

    nId = aNmTb.Put( "Check", VARNAME );
    pClassCheckBox->SetVariable( nId, &aBool );

    return pClassCheckBox;
}

RscTop * RscTypCont::InitClassPushButton( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassPushButton;

    // Klasse anlegen
    nId = pHS->getID( "PushButton" );
    pClassPushButton = new RscClass( nId, RSC_PUSHBUTTON, pSuper );
    pClassPushButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassPushButton );

    InsWinBit( pClassPushButton, "DefButton", nDefaultId );
    INS_WINBIT( pClassPushButton, Top )
    INS_WINBIT( pClassPushButton, VCenter )
    INS_WINBIT( pClassPushButton, Bottom )

    return pClassPushButton;
}

RscTop * RscTypCont::InitClassTriStateBox( RscTop * pSuper,
                                           RscEnum * pTriState )
{
    Atom        nId;
    RscTop *    pClassTriStateBox;

    nId = pHS->getID( "TriStateBox" );
    pClassTriStateBox = new RscClass( nId, RSC_TRISTATEBOX, pSuper );
    pClassTriStateBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassTriStateBox );

    // Variablen anlegen
    nId = aNmTb.Put( "State", VARNAME );
    pClassTriStateBox->SetVariable( nId, pTriState );
    nId = aNmTb.Put( "TriStateDisable", VARNAME );
    pClassTriStateBox->SetVariable( nId, &aBool );

    return pClassTriStateBox;
}

RscTop * RscTypCont::InitClassMenuButton( RscTop * pSuper,
                                          RscTop * pClassMenu )
{
    Atom        nId;
    RscTop *    pClassMenuButton;

    nId = pHS->getID( "MenuButton" );
    pClassMenuButton = new RscClass( nId, RSC_MENUBUTTON, pSuper );
    pClassMenuButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassMenuButton );

    // Variablen anlegen
    nId = aNmTb.Put( "ButtonMenu", VARNAME );
    pClassMenuButton->SetVariable( nId, pClassMenu, NULL, 0,
                                                                        RSCMENUBUTTON_MENU );

    return pClassMenuButton;
}


RscTop * RscTypCont::InitClassImageButton( RscTop * pSuper,
                                           RscTop * pClassImage,
                                           RscEnum * pTriState )
{
    Atom        nId;
    RscTop *    pClassImageButton;

    // Klasse anlegen
    nId = pHS->getID( "ImageButton" );
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
        Atom        nVarId;
        RscEnum   * pSymbol;

        aBaseLst.push_back( pSymbol = new RscEnum( pHS->getID( "EnumSymbolButton" ), RSC_NOTYPE ) );

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

RscTop * RscTypCont::InitClassEdit( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassEdit;

    // Klasse anlegen
    nId                     = pHS->getID( "Edit" );
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

RscTop * RscTypCont::InitClassMultiLineEdit( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassMultiLineEdit;

    // Klasse anlegen
    nId = pHS->getID( "MultiLineEdit" );
    pClassMultiLineEdit = new RscClass( nId, RSC_MULTILINEEDIT, pSuper );
    pClassMultiLineEdit->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassMultiLineEdit );

    INS_WINBIT( pClassMultiLineEdit, HScroll );
    INS_WINBIT( pClassMultiLineEdit, VScroll );
    INS_WINBIT( pClassMultiLineEdit, IgnoreTab );
    INS_WINBIT( pClassMultiLineEdit, AutoVScroll )

    return pClassMultiLineEdit;
}

RscTop * RscTypCont::InitClassScrollBar( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassScrollBar;

    // Klasse anlegen
    nId = pHS->getID( "ScrollBar" );
    pClassScrollBar = new RscClass( nId, RSC_SCROLLBAR, pSuper );
    pClassScrollBar->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassScrollBar );

    // Variablen anlegen
    nId = aNmTb.Put( "MinPos", VARNAME );
    pClassScrollBar->SetVariable( nId, &aShort );
    {
        RSCINST     aDfltI;

        aDfltI = aShort.Create( NULL, RSCINST(), false );
        aDfltI.pClass->SetNumber( aDfltI, 100 );
//      aDfltI.pClass->MakeDefault( aDfltI );

        nId = aNmTb.Put( "MaxPos", VARNAME );
        pClassScrollBar->SetVariable( nId, &aShort, &aDfltI );
    }
    nId = aNmTb.Put( "ThumbPos", VARNAME );
    pClassScrollBar->SetVariable( nId, &aShort );
    {
        RSCINST     aDfltI;

        aDfltI = aShort.Create( NULL, RSCINST(), false );
        aDfltI.pClass->SetNumber( aDfltI, 1 );
//      aDfltI.pClass->MakeDefault( aDfltI );

        nId = aNmTb.Put( "PageSize", VARNAME);
        pClassScrollBar->SetVariable( nId, &aShort, &aDfltI  );
    }
    {
        RSCINST     aDfltI;

        aDfltI = aShort.Create( NULL, RSCINST(), false );
        aDfltI.pClass->SetNumber( aDfltI, 1 );
//      aDfltI.pClass->MakeDefault( aDfltI );
        nId = aNmTb.Put( "LineSize", VARNAME );
        pClassScrollBar->SetVariable( nId, &aShort, &aDfltI );
    }
    nId = aNmTb.Put( "VisibleSize", VARNAME );
    pClassScrollBar->SetVariable( nId, &aShort );

    INS_WINBIT( pClassScrollBar, HScroll );
    INS_WINBIT( pClassScrollBar, VScroll );
    INS_WINBIT( pClassScrollBar, Drag )

    return pClassScrollBar;
}

RscTop * RscTypCont::InitClassListBox( RscTop * pSuper, RscArray * pStrLst )
{
    Atom        nId;
    RscTop *    pClassListBox;

    // Klasse anlegen
    nId = pHS->getID( "ListBox" );
    pClassListBox = new RscClass( nId, RSC_LISTBOX, pSuper );
    pClassListBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassListBox );

    // Variablen anlegen
    INS_WINBIT(pClassListBox,Sort)
    INS_WINBIT(pClassListBox,DropDown)
    INS_WINBIT(pClassListBox,HScroll);
    INS_WINBIT(pClassListBox,VScroll);
    INS_WINBIT(pClassListBox,AutoSize)
    INS_WINBIT(pClassListBox,AutoHScroll)
    INS_WINBIT(pClassListBox,DDExtraWidth)

    {
        RSCINST aDflt = aUShort.Create( NULL, RSCINST(), false );
        aDflt.pClass->SetNumber( aDflt, (sal_uInt16)0xFFFF );
        nId = aNmTb.Put( "CurPos", VARNAME );
        pClassListBox->SetVariable( nId, &aUShort, &aDflt );
    }
    nId = aNmTb.Put( "StringList", VARNAME );
    pClassListBox->SetVariable( nId, pStrLst );

    return pClassListBox;
}

RscTop * RscTypCont::InitClassMultiListBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassMultiListBox;

    // Klasse anlegen
    nId = pHS->getID( "MultiListBox" );
    pClassMultiListBox = new RscClass( nId, RSC_MULTILISTBOX, pSuper );
    pClassMultiListBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassMultiListBox );

    INS_WINBIT(pClassMultiListBox,SimpleMode)

    return pClassMultiListBox;
}

RscTop * RscTypCont::InitClassComboBox( RscTop * pSuper, RscArray * pStrLst )
{
    Atom        nId;
    RscTop *    pClassComboBox;

    // Klasse anlegen
    nId = pHS->getID( "ComboBox" );
    pClassComboBox = new RscClass( nId, RSC_COMBOBOX, pSuper );
    pClassComboBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassComboBox );

    // Variablen anlegen
    INS_WINBIT(pClassComboBox,DropDown)
    INS_WINBIT(pClassComboBox,Sort)
    INS_WINBIT(pClassComboBox,HScroll);
    INS_WINBIT(pClassComboBox,VScroll);
    INS_WINBIT(pClassComboBox,AutoSize)
    INS_WINBIT(pClassComboBox,AutoHScroll)
    INS_WINBIT(pClassComboBox,DDExtraWidth)

    nId = aNmTb.Put( "StringList", VARNAME );
    pClassComboBox->SetVariable( nId, pStrLst );

    return pClassComboBox;
}

RscTop * RscTypCont::InitClassFixedText( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassFixedText;

    // Klasse anlegen
    nId = pHS->getID( "FixedText" );
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
    INS_WINBIT(pClassFixedText,Top)
    INS_WINBIT(pClassFixedText,VCenter)
    INS_WINBIT(pClassFixedText,Bottom)

    return pClassFixedText;
}

RscTop * RscTypCont::InitClassFixedBitmap( RscTop * pSuper, RscTop * pClassBitmap )
{
    Atom        nId;
    RscTop *    pClassFixedBitmap;

    // Klasse anlegen
    nId = pHS->getID( "FixedBitmap" );
    pClassFixedBitmap = new RscClass( nId, RSC_FIXEDBITMAP, pSuper );
    pClassFixedBitmap->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedBitmap );

    INS_WINBIT(pClassFixedBitmap,Scale)

    // Variablen anlegen
    nId = aNmTb.Put( "Fixed", VARNAME );
    pClassFixedBitmap->SetVariable( nId, pClassBitmap, 0, 0, RSC_FIXEDBITMAP_BITMAP );

    return pClassFixedBitmap;
}

RscTop * RscTypCont::InitClassFixedImage( RscTop * pSuper, RscTop * pClassImage )
{
    Atom        nId;
    RscTop *    pClassFixedImage;

    // Klasse anlegen
    nId = pHS->getID( "FixedImage" );
    pClassFixedImage = new RscClass( nId, RSC_FIXEDIMAGE, pSuper );
    pClassFixedImage->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedImage );

    // Variablen anlegen
    nId = aNmTb.Put( "Fixed", VARNAME );
    pClassFixedImage->SetVariable( nId, pClassImage, 0, 0, RSC_FIXEDIMAGE_IMAGE );

    return pClassFixedImage;
}

RscTop * RscTypCont::InitClassRadioButton( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassRadioButton;

    // Klasse anlegen
    nId = pHS->getID( "RadioButton" );
    pClassRadioButton = new RscClass( nId, RSC_RADIOBUTTON, pSuper );
    pClassRadioButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassRadioButton );

    // Variablen anlegen
    INS_WINBIT( pClassRadioButton, WordBreak )
    INS_WINBIT( pClassRadioButton, Top )
    INS_WINBIT( pClassRadioButton, VCenter )
    INS_WINBIT( pClassRadioButton, Bottom )

    nId = aNmTb.Put( "Check", VARNAME );
    pClassRadioButton->SetVariable( nId, &aBool );

    return pClassRadioButton;
}

RscTop * RscTypCont::InitClassKeyCode( RscTop * pSuper, RscEnum * pKey )
{
    Atom        nId;
    RscTop *    pClassKeyCode;

    // Klasse anlegen
    nId = pHS->getID( "KeyCode" );
    pClassKeyCode = new RscClass( nId, RSC_KEYCODE, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassKeyCode );

    // Variablen anlegen
    nId = aNmTb.Put( "Code", VARNAME );
    pClassKeyCode->SetVariable( nId, pKey );

    {
        RscFlag *   pFlag;
        RscClient * pClient;
        Atom        nVarId, nShiftId, nMod1Id, nMod2Id;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagKeyModifier" ), RSC_NOTYPE ) );

        // Konstanten in Tabelle stellen
        nShiftId = pHS->getID( "KEY_SHIFT" );
        SETCONST( pFlag, nShiftId, KEY_SHIFT );
        nMod1Id = pHS->getID( "KEY_MOD1" );
        SETCONST( pFlag, nMod1Id, KEY_MOD1 );
        nMod2Id = pHS->getID( "KEY_MOD2" );
        SETCONST( pFlag, nMod2Id, KEY_MOD2 );

        // Variable einfuegen
        nVarId = aNmTb.Put( "_ModifierFlags", VARNAME );
        pClassKeyCode->SetVariable( nVarId, pFlag, NULL,
                                                                        VAR_HIDDEN | VAR_NOENUM );

        // Clientvariablen einfuegen
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nShiftId )
        );
        nId = aNmTb.Put( "Shift", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nMod1Id )
        );
        nId = aNmTb.Put( "Modifier1", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );


        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nMod2Id )
        );
        nId = aNmTb.Put( "Modifier2", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, NULL,
                                   VAR_NODATAINST, 0, nVarId );
    }
    {
        Atom        nVarId;
        RscEnum   * pKeyFunc;

        aBaseLst.push_back( pKeyFunc = new RscEnum( pHS->getID( "EnumKeyFunc" ), RSC_NOTYPE ) );

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

RscTop * RscTypCont::InitClassAccelItem( RscTop * pSuper,
                                         RscTop * pClassKeyCode )
{
    Atom        nId;
    RscTop *    pClassAccelItem;

    // Klasse anlegen
    nId = pHS->getID( "AcceleratorItem" );
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

RscTop * RscTypCont::InitClassAccel( RscTop * pSuper, RscTop * pClassAccelItem )
{
    Atom        nId;
    RscTop *    pClassAccel;

    // Klasse anlegen
    nId = pHS->getID( "Accelerator" );
    pClassAccel = new RscClass( nId, RSC_ACCEL, pSuper );
    pClassAccel->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassAccel );

    // Variablen anlegen
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassAccel->SetVariable( nId, &aLangString );
    {
        RscCont * pCont;

        aBaseLst.push_back(
            pCont = new RscCont( pHS->getID( "ContAcceleratorKey" ), RSC_NOTYPE )
        );
        pCont->SetTypeClass( pClassAccelItem );
        nId = aNmTb.Put( "ItemList", VARNAME );
        pClassAccel->SetVariable( nId, pCont );
    }

    return pClassAccel;
}

RscTop * RscTypCont::InitClassMenuItem( RscTop * pSuper,
                                        RscTop * pClassBitmap,
                                                                                RscTop * pClassKeyCode )
{
    Atom        nId;
    RscTop *    pClassMenuItem;

    // Klasse anlegen
    nId = pHS->getID( "MenuItem" );
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
        Atom        nVarId, nAutoCheckId, nRadioCheckId;
        Atom        nCheckableId, nAboutId, nHelpId;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagMenuState" ), RSC_NOTYPE ) );

        // Konstanten in Tabelle stellen
        nCheckableId = pHS->getID( "MIB_CHECKABLE" );
        SETCONST( pFlag, nCheckableId, MIB_CHECKABLE );
        nAutoCheckId = pHS->getID( "MIB_AUTOCHECK" );
        SETCONST( pFlag, nAutoCheckId, MIB_AUTOCHECK );
        nRadioCheckId = pHS->getID( "MIB_RADIOCHECK" );
        SETCONST( pFlag, nRadioCheckId, MIB_RADIOCHECK );
        nAboutId = pHS->getID( "MIB_ABOUT" );
        SETCONST( pFlag, nAboutId, MIB_ABOUT );
        nHelpId = pHS->getID( "MIB_HELP" );
        SETCONST( pFlag, nHelpId, MIB_HELP );

        // Variable einfuegen
        nVarId = aNmTb.Put( "_MenuItemFlags", VARNAME );
        pClassMenuItem->SetVariable( nVarId, pFlag, NULL,
                                     VAR_HIDDEN | VAR_NOENUM,
                                     RSC_MENUITEM_STATUS );

        // Clientvariablen einfuegen
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nCheckableId )
        );
        nId = aNmTb.Put( "Checkable", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nAutoCheckId )
        );
        nId = aNmTb.Put( "AutoCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nRadioCheckId )
        );
        nId = aNmTb.Put( "RadioCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nAboutId )
        );
        nId = aNmTb.Put( "About", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, NULL,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nHelpId )
        );
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
    pClassMenuItem->SetVariable( nId, &aStringLiteral, NULL, 0,
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

RscTop * RscTypCont::InitClassMenu( RscTop * pSuper,
                                    RscTop * pClassMenuItem )
{
    Atom        nId;
    RscTop *    pClassMenu;

    // Klasse anlegen
    nId = pHS->getID( "Menu" );
    pClassMenu = new RscClass( nId, RSC_MENU, pSuper );
    pClassMenu->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassMenu );

    // Variablen anlegen
    {
        RscCont * pCont;

        aBaseLst.push_back( pCont = new RscCont( pHS->getID( "ContMenuItem" ), RSC_NOTYPE ) );
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

RscTop * RscTypCont::InitClassMessBox( RscTop * pSuper,
                                       RscEnum * pMessButtons,
                                       RscEnum * pMessDefButton )
{
    Atom        nId;
    RscTop *    pClassMessBox;

    // Klasse anlegen
    nId = pHS->getID( "MessBox" );
    pClassMessBox = new RscClass( nId, RSC_MESSBOX, pSuper );
    pClassMessBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassMessBox );

    // Variablen anlegen
    nId = aNmTb.Put( "Buttons", VARNAME );
    pClassMessBox->SetVariable( nId, pMessButtons );
    nId = aNmTb.Put( "DefButton", VARNAME );
    pClassMessBox->SetVariable( nId, pMessDefButton );
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassMessBox->SetVariable( nId, &aStringLiteral );
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

RscTop * RscTypCont::InitClassSplitter( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassSplitter;

    // Klasse anlegen
    nId = pHS->getID( "Splitter" );
    pClassSplitter = new RscClass( nId, RSC_SPLITTER, pSuper );
    pClassSplitter->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassSplitter );

    INS_WINBIT(pClassSplitter,HScroll);
    INS_WINBIT(pClassSplitter,VScroll);

    return pClassSplitter;
}

RscTop * RscTypCont::InitClassSplitWindow( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassSplitWindow;

    // Klasse anlegen
    nId = pHS->getID( "SplitWindow" );
    pClassSplitWindow = new RscClass( nId, RSC_SPLITWINDOW, pSuper );
    pClassSplitWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassSplitWindow );

    INS_WINBIT(pClassSplitWindow,Sizeable)
    INS_WINBIT(pClassSplitWindow,NoSplitDraw)

    return pClassSplitWindow;
}

RscTop * RscTypCont::InitClassTime( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassTime;

    // Klasse anlegen
    nId = pHS->getID( "Time" );
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

RscTop * RscTypCont::InitClassDate( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassDate;

    // Klasse anlegen
    nId = pHS->getID( "Date" );
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

RscTop * RscTypCont::InitClassPatternFormatter( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassPattern;

    // Klasse anlegen
    nId = pHS->getID( "PatternFormatter" );
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

RscTop * RscTypCont::InitClassNumericFormatter( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassNumeric;

    // Klasse anlegen
    nId = pHS->getID( "NumericFormatter" );
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
    nId = aNmTb.Put( "DecimalDigits", VARNAME );
    pClassNumeric->SetVariable( nId, &aUShort, NULL,
                                0, NUMERICFORMATTER_DECIMALDIGITS );
    nId = aNmTb.Put( "Value", VARNAME );
    pClassNumeric->SetVariable( nId, &aIdLong, NULL,
                                0, NUMERICFORMATTER_VALUE );
    nId = aNmTb.Put( "NoThousandSep", VARNAME );
    pClassNumeric->SetVariable( nId, &aBool, NULL,
                                0, NUMERICFORMATTER_NOTHOUSANDSEP );

    return pClassNumeric;
}

RscTop * RscTypCont::InitClassMetricFormatter( RscTop * pSuper,
                                                                                        RscEnum * pFieldUnits )
{
    Atom        nId;
    RscTop *    pClassMetric;

    // Klasse anlegen
    nId = pHS->getID( "MetricFormatter" );
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

RscTop * RscTypCont::InitClassCurrencyFormatter(RscTop * pSuper)
{
    Atom        nId;
    RscTop *    pClassCurrency;

    // Klasse anlegen
    nId = pHS->getID( "CurrencyFormatter" );
    pClassCurrency = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassCurrency->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    return pClassCurrency;
}

RscTop * RscTypCont::InitClassDateFormatter( RscTop * pSuper,
                        RscTop * pClassDate )
{
    Atom        nId;
    RscTop *    pClassDateF;

    // Klasse anlegen
    nId = pHS->getID( "DateFormatter" );
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
    nId = aNmTb.Put( "Value", VARNAME );
    pClassDateF->SetVariable( nId, pClassDate, NULL,
                                                                        0, DATEFORMATTER_VALUE );

        return pClassDateF;
}

RscTop * RscTypCont::InitClassTimeFormatter( RscTop * pSuper,
                        RscTop * pClassTime,
                        RscEnum * pTimeFieldFormat )
{
    Atom        nId;
    RscTop *    pClassTimeF;

    // Klasse anlegen
    nId = pHS->getID( "TimeFormatter" );
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
    nId = aNmTb.Put( "Value", VARNAME );
    pClassTimeF->SetVariable( nId, pClassTime, NULL,
                              0, TIMEFORMATTER_VALUE );

    return pClassTimeF;
}

RscTop * RscTypCont::InitClassSpinField( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassSpinField;

    // Klasse anlegen
    nId = pHS->getID( "SpinField" );
    pClassSpinField = new RscClass( nId, RSC_SPINFIELD, pSuper );
    pClassSpinField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassSpinField );

    INS_WINBIT(pClassSpinField,Repeat)
    INS_WINBIT(pClassSpinField,Spin)

    return pClassSpinField;
}

RscTop * RscTypCont::InitClassPatternField( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassPatternField;

    // Klasse anlegen
    nId = pHS->getID( "PatternField" );
    pClassPatternField = new RscClass( nId, RSC_PATTERNFIELD, pSuper );
    pClassPatternField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassPatternField );

    return pClassPatternField;
}

RscTop * RscTypCont::InitClassNumericField( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassNumericField;

    // Klasse anlegen
    nId = pHS->getID( "NumericField" );
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

RscTop * RscTypCont::InitClassMetricField( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassMetricField;

    // Klasse anlegen
    nId = pHS->getID( "MetricField" );
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

RscTop * RscTypCont::InitClassCurrencyField
(
    const char * pClassName,
    sal_uInt32 nRT,
    RscTop * pSuper
)
{
    Atom        nId;
    RscTop *    pClassCurrencyField;

    // Klasse anlegen
    nId = pHS->getID( pClassName );
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

RscTop * RscTypCont::InitClassDateField( RscTop * pSuper, RscTop * pClassDate )
{
    Atom        nId;
    RscTop *    pClassDateField;

    // Klasse anlegen
    nId = pHS->getID( "DateField" );
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

RscTop * RscTypCont::InitClassTimeField( RscTop * pSuper, RscTop * pClassTime )
{
    Atom        nId;
    RscTop *    pClassTimeField;

    // Klasse anlegen
    nId = pHS->getID( "TimeField" );
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

RscTop * RscTypCont::InitClassPatternBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassPatternBox;

    // Klasse anlegen
    nId = pHS->getID( "PatternBox" );
    pClassPatternBox = new RscClass( nId, RSC_PATTERNBOX, pSuper );
    pClassPatternBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassPatternBox );

    return pClassPatternBox;
}

RscTop * RscTypCont::InitClassNumericBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassNumericBox;

    // Klasse anlegen
    nId = pHS->getID( "NumericBox" );
    pClassNumericBox = new RscClass( nId, RSC_NUMERICBOX, pSuper );
    pClassNumericBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassNumericBox );

    // Variablen anlegen

    return pClassNumericBox;
}

RscTop * RscTypCont::InitClassMetricBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassMetricBox;

    // Klasse anlegen
    nId = pHS->getID( "MetricBox" );
    pClassMetricBox = new RscClass( nId, RSC_METRICBOX, pSuper );
    pClassMetricBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassMetricBox );

    // Variablen anlegen

    return pClassMetricBox;
}

RscTop * RscTypCont::InitClassCurrencyBox
(
    const char * pClassName,
    sal_uInt32 nRT,
    RscTop * pSuper
)
{
    Atom        nId;
    RscTop *    pClassCurrencyBox;

    // Klasse anlegen
    nId = pHS->getID( pClassName );
    pClassCurrencyBox = new RscClass( nId, nRT, pSuper );
    pClassCurrencyBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassCurrencyBox );

    // Variablen anlegen

    return pClassCurrencyBox;
}

RscTop * RscTypCont::InitClassDateBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassDateBox;

    // Klasse anlegen
    nId = pHS->getID( "DateBox" );
    pClassDateBox = new RscClass( nId, RSC_DATEBOX, pSuper );
    pClassDateBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassDateBox );

    // Variablen anlegen

    return pClassDateBox;
}

RscTop * RscTypCont::InitClassTimeBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassTimeBox;

    // Klasse anlegen
    nId = pHS->getID( "TimeBox" );
    pClassTimeBox = new RscClass( nId, RSC_TIMEBOX, pSuper );
    pClassTimeBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassTimeBox );

    // Variablen anlegen

    return pClassTimeBox;
}

RscTop * RscTypCont::InitClassDockingWindow( RscTop * pSuper,
                                             RscEnum * pMapUnit )
{
    Atom        nId;
    RscTop *    pClassDockWindow;

    // Klasse anlegen
    nId = pHS->getID( "DockingWindow" );
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

RscTop * RscTypCont::InitClassToolBoxItem( RscTop * pSuper,
                                           RscTop * pClassBitmap,
                                           RscTop * pClassImage,
                                           RscEnum * pTriState )
{
    Atom        nId;
    RscTop *    pClassToolBoxItem;

    // Klasse anlegen
    nId = pHS->getID( "ToolBoxItem" );
    pClassToolBoxItem = new RscClass( nId, RSC_TOOLBOXITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassToolBoxItem );

    // Variablen anlegen
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aIdNoZeroUShort, NULL, 0,
                                    RSC_TOOLBOXITEM_ID );
    {
        RscEnum   * pEnum;

        aBaseLst.push_back( pEnum = new RscEnum( pHS->getID( "EnumToolBoxItemType" ), RSC_NOTYPE ) );
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
        Atom        l_nVarId, l_nAutoCheckId, l_nRadioCheckId, l_nCheckableId, l_nLeftId, l_nAutoSizeId, l_nDropDownId;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagToolBoxState" ), RSC_NOTYPE ) );

        // Konstanten in Tabelle stellen
        l_nCheckableId = pHS->getID( "TIB_CHECKABLE" );
        SETCONST( pFlag, l_nCheckableId, TIB_CHECKABLE );
        l_nAutoCheckId = pHS->getID( "TIB_AUTOCHECK" );
        SETCONST( pFlag, l_nAutoCheckId, TIB_AUTOCHECK );
        l_nRadioCheckId = pHS->getID( "TIB_RADIOCHECK" );
        SETCONST( pFlag, l_nRadioCheckId, TIB_RADIOCHECK );
        l_nLeftId = pHS->getID( "TIB_LEFT" );
        SETCONST( pFlag, l_nLeftId, TIB_LEFT );
        l_nAutoSizeId = pHS->getID( "TIB_AUTOSIZE" );
        SETCONST( pFlag, l_nAutoSizeId, TIB_AUTOSIZE );
        l_nDropDownId = pHS->getID( "TIB_DROPDOWN" );
        SETCONST( pFlag, l_nDropDownId, TIB_DROPDOWN );

        // Variable einfuegen
        l_nVarId = aNmTb.Put( "_ToolBoxItemFlags", VARNAME );
        pClassToolBoxItem->SetVariable( l_nVarId, pFlag, NULL,
                                     VAR_HIDDEN | VAR_NOENUM,
                                     RSC_TOOLBOXITEM_STATUS );

        // Clientvariablen einfuegen
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nCheckableId )
        );
        nId = aNmTb.Put( "Checkable", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nAutoCheckId )
        );
        nId = aNmTb.Put( "AutoCheck", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nRadioCheckId )
        );
        nId = aNmTb.Put( "RadioCheck", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nLeftId )
        );
        nId = aNmTb.Put( "Left", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nAutoSizeId )
        );
        nId = aNmTb.Put( "AutoSize", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nDropDownId )
        );
        nId = aNmTb.Put( "DropDown", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, NULL, VAR_NODATAINST, 0, l_nVarId );
    }
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aStringLiteral, NULL, 0,
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

RscTop * RscTypCont::InitClassToolBox( RscTop * pSuper,
                                       RscTop * pClassToolBoxItem,
                                       RscTop * pClassImageList )
{
    Atom        nId;
    RscTop *    pClassToolBox;

    // Klasse anlegen
    nId = pHS->getID( "ToolBox" );
    pClassToolBox = new RscClass( nId, RSC_TOOLBOX, pSuper );
    pClassToolBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassToolBox );

    // Variablen anlegen
    {
        RscEnum   * pEnum;

        aBaseLst.push_back( pEnum = new RscEnum( pHS->getID( "EnumButtonType" ), RSC_NOTYPE ) );
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

        aBaseLst.push_back( pEnum = new RscEnum( pHS->getID( "EnumToolBoxAlign" ), RSC_NOTYPE ) );
        SETCONST( pEnum, "BOXALIGN_TOP",                WINDOWALIGN_TOP );
        SETCONST( pEnum, "BOXALIGN_LEFT",               WINDOWALIGN_LEFT );
        SETCONST( pEnum, "BOXALIGN_RIGHT",              WINDOWALIGN_RIGHT );
        SETCONST( pEnum, "BOXALIGN_BOTTOM",             WINDOWALIGN_BOTTOM );

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
        RscLangArray* pLA;
        RscCont * pCont;

        aBaseLst.push_back( pCont = new RscCont( pHS->getID( "ContToolBoxItem" ), RSC_NOTYPE ) );
        pCont->SetTypeClass( pClassToolBoxItem );
        aBaseLst.push_back(
            pLA = new RscLangArray( pHS->getID( "LangContToolBoxItem" )
                                  , RSC_NOTYPE
                                  , pCont
                                  , &aLangType
                                  )
        );
        nId = aNmTb.Put( "ItemList", VARNAME );
        pClassToolBox->SetVariable( nId, pLA, NULL, 0,
                                    RSC_TOOLBOX_ITEMLIST );
    }
    INS_WINBIT(pClassToolBox,Scroll)
    INS_WINBIT(pClassToolBox,LineSpacing)
    INS_WINBIT(pClassToolBox,RectStyle)
    INS_WINBIT(pClassToolBox,Tabstop)

    return pClassToolBox;
}

RscTop * RscTypCont::InitClassStatusBar( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassStatusBar;

    // Klasse anlegen
    nId = pHS->getID( "StatusBar" );
    pClassStatusBar = new RscClass( nId, RSC_STATUSBAR, pSuper );
    pClassStatusBar->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassStatusBar );

    // Variablen anlegen
    INS_WINBIT(pClassStatusBar,Left)
    INS_WINBIT(pClassStatusBar,Right)

    return pClassStatusBar;
}

RscTop * RscTypCont::InitClassMoreButton( RscTop * pSuper, RscEnum * pMapUnit )
{
    Atom        nId;
    RscTop *    pClassMoreButton;

    // Klasse anlegen
    nId = pHS->getID( "MoreButton" );
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

RscTop * RscTypCont::InitClassFloatingWindow( RscTop * pSuper,
                                              RscEnum * pMapUnit )
{
    Atom        nId;
    RscTop *    pClassFloatingWindow;

    // Klasse anlegen
    nId = pHS->getID( "FloatingWindow" );
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
    INS_WINBIT(pClassFloatingWindow,StdPopup)

    return pClassFloatingWindow;
}

RscTop * RscTypCont::InitClassTabControlItem( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassTabControlItem;

    // Klasse anlegen
    nId = pHS->getID( "PageItem" );
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
    pClassTabControlItem->SetVariable( nId, &aIdLong, NULL, 0,
                                       RSC_TABCONTROLITEM_PAGERESID );

    return pClassTabControlItem;
}

RscTop * RscTypCont::InitClassTabControl( RscTop * pSuper,
                                          RscTop * pClassTabControlItem )
{
    Atom        nId;
    RscTop *    pClassTabControl;

    // Klasse anlegen
    nId = pHS->getID( "TabControl" );
    pClassTabControl = new RscClass( nId, RSC_TABCONTROL, pSuper );
    pClassTabControl->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassTabControl );

    // Variablen anlegen
    {
        RscCont * pCont;

        aBaseLst.push_back( pCont = new RscCont( pHS->getID( "ContTabControlItem" ), RSC_NOTYPE ) );
        pCont->SetTypeClass( pClassTabControlItem );
        nId = aNmTb.Put( "PageList", VARNAME );
        pClassTabControl->SetVariable( nId, pCont, NULL, 0,
                                       RSC_TABCONTROL_ITEMLIST );

        INS_WINBIT( pClassTabControl, DropDown );
    }

    return pClassTabControl;
}

RscTop * RscTypCont::InitClassSfxStyleFamilyItem( RscTop * pSuper,
                                                  RscTop * pClassBitmap,
                                                  RscTop * pClassImage,
                                                  RscArray * pStrLst )
{
    Atom        nId;
    RscTop *    pClassSfxFamilyStyleItem;

    // Klasse anlegen
    nId = pHS->getID( "SfxStyleFamilyItem" );
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
        pSfxStyleFamily = new RscEnum( pHS->getID( "StyleFamily" ),
                                    RSC_NOTYPE );

        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_PARA", SFX_STYLE_FAMILY_PARA );
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_CHAR", SFX_STYLE_FAMILY_CHAR );
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_FRAME",SFX_STYLE_FAMILY_FRAME);
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_PAGE", SFX_STYLE_FAMILY_PAGE );
        SETCONST( pSfxStyleFamily, "SFX_STYLE_FAMILY_PSEUDO", SFX_STYLE_FAMILY_PSEUDO );
        aBaseLst.push_back( pSfxStyleFamily );

        nId = aNmTb.Put( "StyleFamily", VARNAME );
        pClassSfxFamilyStyleItem->SetVariable( nId, pSfxStyleFamily, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_STYLEFAMILY );
    }
    nId = aNmTb.Put( "StyleImage", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, pClassImage, NULL, 0,
                                           RSC_SFX_STYLE_ITEM_IMAGE );
    return pClassSfxFamilyStyleItem;
}

RscTop * RscTypCont::InitClassSfxTemplateDialog( RscTop * pSuper,
                                                 RscTop * pClassFamilyStyleItem )
{
    Atom        nId;
    RscTop *    pClassSfxTemplateDialog;

    // Klasse anlegen
    nId = pHS->getID( "SfxStyleFamilies" );
    pClassSfxTemplateDialog = new RscClass( nId, RSC_SFX_STYLE_FAMILIES, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassSfxTemplateDialog );

    // Variablen anlegen
    {
        RscCont * pCont;

        aBaseLst.push_back(
            pCont = new RscCont( pHS->getID( "ContFamilyStyleItem" ), RSC_NOTYPE )
        );
        pCont->SetTypeClass( pClassFamilyStyleItem );
        nId = aNmTb.Put( "StyleFamilyList", VARNAME );
        pClassSfxTemplateDialog->SetVariable( nId, pCont );
    }

    return pClassSfxTemplateDialog;
}

RscTop * RscTypCont::InitClassSfxSlotInfo( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassSfxSlotInfo;

    // Klasse anlegen
    nId = pHS->getID( "SfxSlotInfo" );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
