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

    // add client variables
    aBaseLst.push_back(
        pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, &aWinBits, nVal )
    );
    Atom nId = aNmTb.Put( rName.getStr(), VARNAME );
    pClass->SetVariable( nId, pClient, nullptr,
                         VAR_NODATAINST, 0, nWinBitVarId );
}

#define INS_WINBIT( pClass, WinBit )        \
    InsWinBit( pClass, #WinBit, n##WinBit##Id );

RscTop * RscTypCont::InitClassMgr()
{
    RscTop      *   pClassMgr;
    RscBaseCont *   pClass;
    Atom            nId;

    aBaseLst.push_back( pClass = new RscBaseCont( InvalidAtom, RSC_NOTYPE, nullptr, false ) );

    nId = pHS->getID( "Resource" );
    pClassMgr = new RscMgr( nId, RSC_RESOURCE, pClass );
    aNmTb.Put( nId, CLASSNAME, pClassMgr );
    pClassMgr->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // initialize variables
    {
        RscContWriteSrc *   pCont;

        // initialize variables
        aBaseLst.push_back( pCont = new RscContExtraData( pHS->getID( "ContExtradata" ), RSC_NOTYPE ) );
        pCont->SetTypeClass( &aShort, &aString );
        nRsc_EXTRADATA = nId = aNmTb.Put( "ExtraData", VARNAME );
        pClassMgr->SetVariable( nId, pCont );
    };
    nId = aNmTb.Put( "Comment", VARNAME );
    pClassMgr->SetVariable( nId, &aString, nullptr, VAR_NORC );

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

    // initialize variables
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

    // the class RscSysDepend handles variables "FILE" in a specific way
    nId = aNmTb.Put( "File", VARNAME );
    pClassBitmap->SetVariable( nId, &aLangString, nullptr, VAR_NORC );

    return pClassBitmap;
}

RscTop * RscTypCont::InitClassImage( RscTop * pSuper, RscTop * pClassBitmap )
{
    Atom        nId;
    RscTop *    pClassImage;

    // initialize class
    nId = pHS->getID( "Image" );
    pClassImage = new RscClass( nId, RSC_IMAGE, pSuper );
    pClassImage->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassImage );

    // initialize variables
    nId = aNmTb.Put( "ImageBitmap", VARNAME );
    pClassImage->SetVariable( nId, pClassBitmap, nullptr, 0, (sal_uInt32)RscImageFlags::ImageBitmap );

    return pClassImage;
}

RscTop * RscTypCont::InitClassImageList( RscTop * pSuper, RscCont * pStrLst )
{
    Atom        nId;
    RscTop *    pClassImageList;

    // initialize class
    nId = pHS->getID( "ImageList" );
    pClassImageList = new RscClass( nId, RSC_IMAGELIST, pSuper );
    pClassImageList->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassImageList );

    nId = aNmTb.Put( "Prefix", VARNAME );
    pClassImageList->SetVariable( nId, &aString );

    RscCont * pCont = new RscCont( pHS->getID( "sal_uInt16 *" ), RSC_NOTYPE );
    pCont->SetTypeClass( &aIdUShort );
    aBaseLst.push_back( pCont );
    nId = aNmTb.Put( "IdList", VARNAME );
    pClassImageList->SetVariable( nId, pCont, nullptr, 0,
                                      (sal_uInt32)RscImageListFlags::IdList );

    nId = aNmTb.Put( "FileList", VARNAME );
    pClassImageList->SetVariable( nId, pStrLst );

    nId = aNmTb.Put( "IdCount", VARNAME );
    pClassImageList->SetVariable( nId, &aUShort, nullptr, 0,
                                  (sal_uInt32)RscImageListFlags::IdCount );
    return pClassImageList;
}

RscTop * RscTypCont::InitClassWindow( RscTop * pSuper, RscEnum * pMapUnit,
                                      RscArray * pLangGeo )
{
    Atom        nId;
    RscTop *    pClassWindow;

    // initialize class
    nId = pHS->getID( "Window" );
    pClassWindow = new RscClass( nId, RSC_WINDOW, pSuper );
    pClassWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassWindow );

    // initialize variables
    {
        RscFlag *   pFlag;
        RscClient * pClient;
        Atom        nVarId, nDisableId, nOutputSizeId;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagWndExtra" ), RSC_NOTYPE ) );

        // set the constants in table
        nDisableId = pHS->getID( "RSWND_DISABLE" );
        SETCONST( pFlag, nDisableId, RSWND::DISABLED );
        nOutputSizeId = pHS->getID( "RSWND_OUTPUTSIZE" );
        SETCONST( pFlag, nOutputSizeId, RSWND::CLIENTSIZE );

        // add variable
        nVarId = aNmTb.Put( "_RscExtraFlags", VARNAME );
        pClassWindow->SetVariable( nVarId, pFlag, nullptr,
                                                                        VAR_HIDDEN | VAR_NOENUM );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nDisableId )
        );
        nId = aNmTb.Put( "Disable", VARNAME );
        pClassWindow->SetVariable( nId, pClient, nullptr,
                                   VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nOutputSizeId )
        );
        nId = aNmTb.Put( "OutputSize", VARNAME );
        pClassWindow->SetVariable( nId, pClient, nullptr,
                                   VAR_NODATAINST, 0, nVarId );
    }

    pClassWindow->SetVariable( nWinBitVarId, &aWinBits, nullptr,
                               VAR_HIDDEN | VAR_NOENUM );

    INS_WINBIT(pClassWindow,Border)
    INS_WINBIT(pClassWindow,Hide)
    INS_WINBIT(pClassWindow,ClipChildren)
    INS_WINBIT(pClassWindow,SVLook)
    InsWinBit( pClassWindow, "DialogControl", nTabControlId );

    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassWindow->SetVariable( nId, &aStringLiteral, nullptr, 0, (sal_uInt32)RscWindowFlags::HelpId );


    nRsc_XYMAPMODEId = nId = aNmTb.Put( "_XYMapMode", VARNAME );
    pClassWindow->SetVariable( nId, pMapUnit, nullptr, 0, (sal_uInt32)RscWindowFlags::XYMapMode  );
    nRsc_X = nId = aNmTb.Put( "_X", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, nullptr, 0, (sal_uInt32)RscWindowFlags::X  );
    nRsc_Y = nId = aNmTb.Put( "_Y", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, nullptr, 0, (sal_uInt32)RscWindowFlags::Y  );

    nRsc_WHMAPMODEId = nId = aNmTb.Put( "_WHMapMode", VARNAME );
    pClassWindow->SetVariable( nId, pMapUnit, nullptr, 0, (sal_uInt32)RscWindowFlags::WHMapMode  );
    nRsc_WIDTH = nId = aNmTb.Put( "_Width", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, nullptr, 0, (sal_uInt32)RscWindowFlags::Width  );
    nRsc_HEIGHT = nId = aNmTb.Put( "_Height", VARNAME );
    pClassWindow->SetVariable( nId, &aLong, nullptr, 0, (sal_uInt32)RscWindowFlags::Height  );

    nRsc_DELTALANG = nId = aNmTb.Put( "DeltaLang", VARNAME );
    pClassWindow->SetVariable( nId, pLangGeo, nullptr, VAR_NORC | VAR_NOENUM);
    nId = aNmTb.Put( "Text", VARNAME );
    pClassWindow->SetVariable( nId, &aLangString, nullptr, 0, (sal_uInt32)RscWindowFlags::Text );
    nId = aNmTb.Put( "QuickHelpText", VARNAME );
    pClassWindow->SetVariable( nId, &aLangString, nullptr, 0, (sal_uInt32)RscWindowFlags::QuickText );

    return pClassWindow;
}

RscTop * RscTypCont::InitClassControl( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassControl;

    // initialize class
    nId = pHS->getID( "Control" );
    pClassControl = new RscClass( nId, RSC_CONTROL, pSuper );
    pClassControl->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassControl );

    InsWinBit( pClassControl, "TabStop", nTabstopId );
    INS_WINBIT(pClassControl,Group)

    return pClassControl;
}

RscTop * RscTypCont::InitClassPushButton( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassPushButton;

    // initialize class
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

RscTop * RscTypCont::InitClassEdit( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassEdit;

    // initialize class
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

RscTop * RscTypCont::InitClassListBox( RscTop * pSuper, RscArray * pStrLst )
{
    Atom        nId;
    RscTop *    pClassListBox;

    // initialize class
    nId = pHS->getID( "ListBox" );
    pClassListBox = new RscClass( nId, RSC_LISTBOX, pSuper );
    pClassListBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassListBox );

    // initialize variables
    INS_WINBIT(pClassListBox,Sort)
    INS_WINBIT(pClassListBox,DropDown)
    INS_WINBIT(pClassListBox,HScroll);
    INS_WINBIT(pClassListBox,VScroll);
    INS_WINBIT(pClassListBox,AutoSize)
    INS_WINBIT(pClassListBox,AutoHScroll)

    {
        RSCINST aDflt = aUShort.Create( nullptr, RSCINST() );
        aDflt.pClass->SetNumber( aDflt, (sal_uInt16)0xFFFF );
        nId = aNmTb.Put( "CurPos", VARNAME );
        pClassListBox->SetVariable( nId, &aUShort, &aDflt );
    }
    nId = aNmTb.Put( "StringList", VARNAME );
    pClassListBox->SetVariable( nId, pStrLst );

    return pClassListBox;
}

RscTop * RscTypCont::InitClassFixedText( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassFixedText;

    // initialize class
    nId = pHS->getID( "FixedText" );
    pClassFixedText = new RscClass( nId, RSC_TEXT, pSuper );
    pClassFixedText->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedText );

    // initialize variables
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

RscTop * RscTypCont::InitClassFixedImage( RscTop * pSuper, RscTop * pClassImage )
{
    Atom        nId;
    RscTop *    pClassFixedImage;

    // initialize class
    nId = pHS->getID( "FixedImage" );
    pClassFixedImage = new RscClass( nId, RSC_FIXEDIMAGE, pSuper );
    pClassFixedImage->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedImage );

    // initialize variables
    nId = aNmTb.Put( "Fixed", VARNAME );
    pClassFixedImage->SetVariable( nId, pClassImage, nullptr, 0, RSC_FIXEDIMAGE_IMAGE );

    return pClassFixedImage;
}

RscTop * RscTypCont::InitClassKeyCode( RscTop * pSuper, RscEnum * pKey )
{
    Atom        nId;
    RscTop *    pClassKeyCode;

    // initialize class
    nId = pHS->getID( "KeyCode" );
    pClassKeyCode = new RscClass( nId, RSC_KEYCODE, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassKeyCode );

    // initialize variables
    nId = aNmTb.Put( "Code", VARNAME );
    pClassKeyCode->SetVariable( nId, pKey );

    {
        RscFlag *   pFlag;
        RscClient * pClient;
        Atom        nVarId, nShiftId, nMod1Id, nMod2Id;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagKeyModifier" ), RSC_NOTYPE ) );

        // set constants in table
        nShiftId = pHS->getID( "KEY_SHIFT" );
        SETCONST( pFlag, nShiftId, KEY_SHIFT );
        nMod1Id = pHS->getID( "KEY_MOD1" );
        SETCONST( pFlag, nMod1Id, KEY_MOD1 );
        nMod2Id = pHS->getID( "KEY_MOD2" );
        SETCONST( pFlag, nMod2Id, KEY_MOD2 );

        // add variable
        nVarId = aNmTb.Put( "_ModifierFlags", VARNAME );
        pClassKeyCode->SetVariable( nVarId, pFlag, nullptr,
                                                                        VAR_HIDDEN | VAR_NOENUM );

        // add client variables
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nShiftId )
        );
        nId = aNmTb.Put( "Shift", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, nullptr,
                                   VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nMod1Id )
        );
        nId = aNmTb.Put( "Modifier1", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, nullptr,
                                   VAR_NODATAINST, 0, nVarId );


        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nMod2Id )
        );
        nId = aNmTb.Put( "Modifier2", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, nullptr,
                                   VAR_NODATAINST, 0, nVarId );
    }
    {
        Atom        nVarId;
        RscEnum   * pKeyFunc;

        aBaseLst.push_back( pKeyFunc = new RscEnum( pHS->getID( "EnumKeyFunc" ), RSC_NOTYPE ) );

        SETCONST( pKeyFunc, "KEYFUNC_DONTKNOW",         KeyFuncType::DONTKNOW        );
        SETCONST( pKeyFunc, "KEYFUNC_NEW",              KeyFuncType::NEW             );
        SETCONST( pKeyFunc, "KEYFUNC_OPEN",             KeyFuncType::OPEN            );
        SETCONST( pKeyFunc, "KEYFUNC_SAVE",             KeyFuncType::SAVE            );
        SETCONST( pKeyFunc, "KEYFUNC_SAVEAS",           KeyFuncType::SAVEAS          );
        SETCONST( pKeyFunc, "KEYFUNC_PRINT",            KeyFuncType::PRINT           );
        SETCONST( pKeyFunc, "KEYFUNC_CLOSE",            KeyFuncType::CLOSE           );
        SETCONST( pKeyFunc, "KEYFUNC_QUIT",             KeyFuncType::QUIT            );
        SETCONST( pKeyFunc, "KEYFUNC_CUT",              KeyFuncType::CUT             );
        SETCONST( pKeyFunc, "KEYFUNC_COPY",             KeyFuncType::COPY            );
        SETCONST( pKeyFunc, "KEYFUNC_PASTE",            KeyFuncType::PASTE           );
        SETCONST( pKeyFunc, "KEYFUNC_UNDO",             KeyFuncType::UNDO            );
        SETCONST( pKeyFunc, "KEYFUNC_REDO",             KeyFuncType::REDO            );
        SETCONST( pKeyFunc, "KEYFUNC_DELETE",           KeyFuncType::DELETE          );
        SETCONST( pKeyFunc, "KEYFUNC_REPEAT",           KeyFuncType::REPEAT          );
        SETCONST( pKeyFunc, "KEYFUNC_FIND",             KeyFuncType::FIND            );
        SETCONST( pKeyFunc, "KEYFUNC_PROPERTIES",       KeyFuncType::PROPERTIES      );
        SETCONST( pKeyFunc, "KEYFUNC_FRONT",            KeyFuncType::FRONT           );
        SETCONST( pKeyFunc, "KEYFUNC_FINDBACKWARD",     KeyFuncType::FINDBACKWARD );
        // add variable
        nVarId = aNmTb.Put( "Function", VARNAME );
        pClassKeyCode->SetVariable( nVarId, pKeyFunc );
    }

    return pClassKeyCode;
}

RscTop * RscTypCont::InitClassMenuItem( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassMenuItem;

    // initialize class
    nId = pHS->getID( "MenuItem" );
    pClassMenuItem = new RscClass( nId, RSC_MENUITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassMenuItem );

    // initialize variables
    nId = aNmTb.Put( "Separator", VARNAME );
    pClassMenuItem->SetVariable( nId, &aBool, nullptr, 0,
                                 (sal_uInt32)RscMenuItem::Separator );
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassMenuItem->SetVariable( nId, &aIdNoZeroUShort, nullptr, 0,
                                 (sal_uInt32)RscMenuItem::Id );
    {
        RscFlag *   pFlag;
        RscClient * pClient;
        Atom        nVarId, nAutoCheckId, nRadioCheckId;
        Atom        nCheckableId, nAboutId, nHelpId;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagMenuState" ), RSC_NOTYPE ) );

        // set constants in table
        nCheckableId = pHS->getID( "MIB_CHECKABLE" );
        SETCONST( pFlag, nCheckableId, MenuItemBits::CHECKABLE );
        nAutoCheckId = pHS->getID( "MIB_AUTOCHECK" );
        SETCONST( pFlag, nAutoCheckId, MenuItemBits::AUTOCHECK );
        nRadioCheckId = pHS->getID( "MIB_RADIOCHECK" );
        SETCONST( pFlag, nRadioCheckId, MenuItemBits::RADIOCHECK );
        nAboutId = pHS->getID( "MIB_ABOUT" );
        SETCONST( pFlag, nAboutId, MenuItemBits::ABOUT );
        nHelpId = pHS->getID( "MIB_HELP" );
        SETCONST( pFlag, nHelpId, MenuItemBits::HELP );

        // add variable
        nVarId = aNmTb.Put( "_MenuItemFlags", VARNAME );
        pClassMenuItem->SetVariable( nVarId, pFlag, nullptr,
                                     VAR_HIDDEN | VAR_NOENUM,
                                     (sal_uInt32)RscMenuItem::Status );

        // add client variables
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nCheckableId )
        );
        nId = aNmTb.Put( "Checkable", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nAutoCheckId )
        );
        nId = aNmTb.Put( "AutoCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nRadioCheckId )
        );
        nId = aNmTb.Put( "RadioCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nAboutId )
        );
        nId = aNmTb.Put( "About", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     VAR_NODATAINST, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nHelpId )
        );
        nId = aNmTb.Put( "Help", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     VAR_NODATAINST, 0, nVarId );

    }
    nId = aNmTb.Put( "Text", VARNAME );
    pClassMenuItem->SetVariable( nId, &aLangString, nullptr, 0,
                                 (sal_uInt32)RscMenuItem::Text );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassMenuItem->SetVariable( nId, &aLangString, nullptr, 0,
                                 (sal_uInt32)RscMenuItem::HelpText );
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassMenuItem->SetVariable( nId, &aStringLiteral, nullptr, 0,
                                 (sal_uInt32)RscMenuItem::HelpId );
    nId = aNmTb.Put( "Disable", VARNAME );
    pClassMenuItem->SetVariable( nId, &aBool, nullptr, 0,
                                 (sal_uInt32)RscMenuItem::Disable );
    nId = aNmTb.Put( "Command", VARNAME );
    pClassMenuItem->SetVariable( nId, &aString, nullptr, 0,
                                    (sal_uInt32)RscMenuItem::Command );

    return pClassMenuItem;
}

RscTop * RscTypCont::InitClassMenu( RscTop * pSuper,
                                    RscTop * pClassMenuItem )
{
    Atom        nId;
    RscTop *    pClassMenu;

    // initialize class
    nId = pHS->getID( "Menu" );
    pClassMenu = new RscClass( nId, RSC_MENU, pSuper );
    pClassMenu->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassMenu );

    // initialize variables
    {
        RscCont * pCont;

        aBaseLst.push_back( pCont = new RscCont( pHS->getID( "ContMenuItem" ), RSC_NOTYPE ) );
        pCont->SetTypeClass( pClassMenuItem );
        nId = aNmTb.Put( "ItemList", VARNAME );
        pClassMenu->SetVariable( nId, pCont, nullptr, 0, (sal_uInt32)RscMenu::Items );
    }
    nId = aNmTb.Put( "Text", VARNAME );
    pClassMenu->SetVariable( nId, &aLangString, nullptr, 0, (sal_uInt32)RscMenu::Text );
    nId = aNmTb.Put( "DefaultItemId", VARNAME );
    pClassMenu->SetVariable( nId, &aIdUShort, nullptr, 0,
                                 (sal_uInt32)RscMenu::DefaultItemId );

    return pClassMenu;
}

RscTop * RscTypCont::InitClassSfxStyleFamilyItem( RscTop * pSuper,
                                                  RscTop * pClassBitmap,
                                                  RscTop * pClassImage,
                                                  RscArray * pStrLst )
{
    Atom        nId;
    RscTop *    pClassSfxFamilyStyleItem;

    // initialize class
    nId = pHS->getID( "SfxStyleFamilyItem" );
    pClassSfxFamilyStyleItem = new RscClass( nId, RSC_SFX_STYLE_FAMILY_ITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassSfxFamilyStyleItem );

    nId = aNmTb.Put( "FilterList", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, pStrLst, nullptr, 0,
                                           SfxStyleItem::List );
    nId = aNmTb.Put( "StyleBitmap", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, pClassBitmap, nullptr, 0,
                                           SfxStyleItem::Bitmap );
    nId = aNmTb.Put( "Text", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, &aLangString, nullptr, 0,
                                           SfxStyleItem::Text );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, &aLangString, nullptr, 0,
                                           SfxStyleItem::HelpText );
    {
        RscEnum * pSfxStyleFamily;
        pSfxStyleFamily = new RscEnum( pHS->getID( "StyleFamily" ),
                                    RSC_NOTYPE );

        SETCONST( pSfxStyleFamily, "SfxStyleFamily::Para", SfxStyleFamily::Para );
        SETCONST( pSfxStyleFamily, "SfxStyleFamily::Char", SfxStyleFamily::Char );
        SETCONST( pSfxStyleFamily, "SfxStyleFamily::Frame",SfxStyleFamily::Frame);
        SETCONST( pSfxStyleFamily, "SfxStyleFamily::Page", SfxStyleFamily::Page );
        SETCONST( pSfxStyleFamily, "SfxStyleFamily::Pseudo", SfxStyleFamily::Pseudo );
        SETCONST( pSfxStyleFamily, "SfxStyleFamily::Table", SfxStyleFamily::Table );
        aBaseLst.push_back( pSfxStyleFamily );

        nId = aNmTb.Put( "StyleFamily", VARNAME );
        pClassSfxFamilyStyleItem->SetVariable( nId, pSfxStyleFamily, nullptr, 0,
                                           SfxStyleItem::StyleFamily );
    }
    nId = aNmTb.Put( "StyleImage", VARNAME );
    pClassSfxFamilyStyleItem->SetVariable( nId, pClassImage, nullptr, 0,
                                           SfxStyleItem::Image );
    return pClassSfxFamilyStyleItem;
}

RscTop * RscTypCont::InitClassSfxTemplateDialog( RscTop * pSuper,
                                                 RscTop * pClassFamilyStyleItem )
{
    Atom        nId;
    RscTop *    pClassSfxTemplateDialog;

    // initialize class
    nId = pHS->getID( "SfxStyleFamilies" );
    pClassSfxTemplateDialog = new RscClass( nId, RSC_SFX_STYLE_FAMILIES, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassSfxTemplateDialog );

    // initialize variables
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

    // initialize class
    nId = pHS->getID( "SfxSlotInfo" );
    pClassSfxSlotInfo = new RscClass( nId, RSC_SFX_SLOT_INFO, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassSfxSlotInfo );

    nId = aNmTb.Put( "SlotName", VARNAME );
    pClassSfxSlotInfo->SetVariable( nId, &aLangString, nullptr, 0,
                                    SfxSlotInfo::SlotName );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassSfxSlotInfo->SetVariable( nId, &aLangString, nullptr, 0,
                                    SfxSlotInfo::HelpText );
    return pClassSfxSlotInfo;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
