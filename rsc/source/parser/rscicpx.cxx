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

RscTop * RscTypCont::InitClassMgr()
{
    RscTop      *   pClassMgr;
    RscBaseCont *   pClass;
    Atom            nId;

    aBaseLst.push_back( pClass = new RscBaseCont( InvalidAtom, RSC_NOTYPE, false ) );

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
    pClassMgr->SetVariable( nId, &aString, nullptr, RSCVAR::NoRc );

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
    pClassBitmap->SetVariable( nId, &aLangString, nullptr, RSCVAR::NoRc );

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
    pClassImage->SetVariable( nId, pClassBitmap, nullptr, RSCVAR::NONE, (sal_uInt32)RscImageFlags::ImageBitmap );

    return pClassImage;
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
                                                                        RSCVAR::Hidden | RSCVAR::NoEnum );

        // add client variables
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nShiftId )
        );
        nId = aNmTb.Put( "Shift", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, nullptr,
                                   RSCVAR::NoDataInst, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nMod1Id )
        );
        nId = aNmTb.Put( "Modifier1", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, nullptr,
                                   RSCVAR::NoDataInst, 0, nVarId );


        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nMod2Id )
        );
        nId = aNmTb.Put( "Modifier2", VARNAME );
        pClassKeyCode->SetVariable( nId, pClient, nullptr,
                                   RSCVAR::NoDataInst, 0, nVarId );
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
    pClassMenuItem->SetVariable( nId, &aBool, nullptr, RSCVAR::NONE,
                                 (sal_uInt32)RscMenuItem::Separator );
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassMenuItem->SetVariable( nId, &aIdNoZeroUShort, nullptr, RSCVAR::NONE,
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
                                     RSCVAR::Hidden | RSCVAR::NoEnum,
                                     (sal_uInt32)RscMenuItem::Status );

        // add client variables
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nCheckableId )
        );
        nId = aNmTb.Put( "Checkable", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     RSCVAR::NoDataInst, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nAutoCheckId )
        );
        nId = aNmTb.Put( "AutoCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     RSCVAR::NoDataInst, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nRadioCheckId )
        );
        nId = aNmTb.Put( "RadioCheck", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     RSCVAR::NoDataInst, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nAboutId )
        );
        nId = aNmTb.Put( "About", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     RSCVAR::NoDataInst, 0, nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, nHelpId )
        );
        nId = aNmTb.Put( "Help", VARNAME );
        pClassMenuItem->SetVariable( nId, pClient, nullptr,
                                     RSCVAR::NoDataInst, 0, nVarId );

    }
    nId = aNmTb.Put( "Text", VARNAME );
    pClassMenuItem->SetVariable( nId, &aLangString, nullptr, RSCVAR::NONE,
                                 (sal_uInt32)RscMenuItem::Text );
    nId = aNmTb.Put( "HelpText", VARNAME );
    pClassMenuItem->SetVariable( nId, &aLangString, nullptr, RSCVAR::NONE,
                                 (sal_uInt32)RscMenuItem::HelpText );
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassMenuItem->SetVariable( nId, &aStringLiteral, nullptr, RSCVAR::NONE,
                                 (sal_uInt32)RscMenuItem::HelpId );
    nId = aNmTb.Put( "Disable", VARNAME );
    pClassMenuItem->SetVariable( nId, &aBool, nullptr, RSCVAR::NONE,
                                 (sal_uInt32)RscMenuItem::Disable );
    nId = aNmTb.Put( "Command", VARNAME );
    pClassMenuItem->SetVariable( nId, &aString, nullptr, RSCVAR::NONE,
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
        pClassMenu->SetVariable( nId, pCont, nullptr, RSCVAR::NONE, (sal_uInt32)RscMenu::Items );
    }
    nId = aNmTb.Put( "Text", VARNAME );
    pClassMenu->SetVariable( nId, &aLangString, nullptr, RSCVAR::NONE, (sal_uInt32)RscMenu::Text );

    return pClassMenu;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
