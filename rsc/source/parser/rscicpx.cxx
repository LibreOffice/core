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

RscTop * RscTypCont::InitClassColor( RscTop * pSuper, RscEnum * pColor )
{
    Atom        nId;
    RscTop *    pClassColor;

    // initialize class
    nId = pHS->getID( "Color" );
    pClassColor = new RscClass( nId, RSC_COLOR, pSuper );
    pClassColor->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassColor );

    // initialize variables
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

    // initialize class
    nId = pHS->getID( "Image" );
    pClassImage = new RscClass( nId, RSC_IMAGE, pSuper );
    pClassImage->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassImage );

    // initialize variables
    nId = aNmTb.Put( "ImageBitmap", VARNAME );
    pClassImage->SetVariable( nId, pClassBitmap, nullptr, 0, (sal_uInt32)RscImageFlags::ImageBitmap );
    nId = aNmTb.Put( "MaskColor", VARNAME );
    pClassImage->SetVariable( nId, pClassColor, nullptr,
                              VAR_SVDYNAMIC, (sal_uInt32)RscImageFlags::MaskColor );

    return pClassImage;
}

RscTop * RscTypCont::InitClassImageList( RscTop * pSuper,
                                         RscTop * pClassColor, RscCont * pStrLst )
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

    nId = aNmTb.Put( "MaskColor", VARNAME );
    pClassImageList->SetVariable( nId, pClassColor, nullptr,
                                  VAR_SVDYNAMIC, (sal_uInt32)RscImageListFlags::MaskColor );

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

RscTop * RscTypCont::InitClassCheckBox( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassCheckBox;

    // initialize class
    nId = pHS->getID( "CheckBox" );
    pClassCheckBox = new RscClass( nId, RSC_CHECKBOX, pSuper );
    pClassCheckBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassCheckBox );

    // initialize variables
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

RscTop * RscTypCont::InitClassImageButton( RscTop * pSuper,
                                           RscTop * pClassImage,
                                           RscEnum * pTriState )
{
    Atom        nId;
    RscTop *    pClassImageButton;

    // initialize class
    nId = pHS->getID( "ImageButton" );
    pClassImageButton = new RscClass( nId, RSC_IMAGEBUTTON, pSuper );
    pClassImageButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassImageButton );

    // initialize variables
    {
        nId = aNmTb.Put( "ButtonImage", VARNAME );
        pClassImageButton->SetVariable( nId, pClassImage, nullptr, 0,
                                         (sal_uInt32)RscImageButtonFlags::Image );
    }
    // initialize variables
    {
        Atom        nVarId;
        RscEnum   * pSymbol;

        aBaseLst.push_back( pSymbol = new RscEnum( pHS->getID( "EnumSymbolButton" ), RSC_NOTYPE ) );

        SETCONST( pSymbol, "IMAGEBUTTON_DONTKNOW",      SymbolType::DONTKNOW );
        SETCONST( pSymbol, "IMAGEBUTTON_IMAGE",         SymbolType::IMAGE );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_UP",      SymbolType::ARROW_UP );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_DOWN",    SymbolType::ARROW_DOWN );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_LEFT",    SymbolType::ARROW_LEFT );
        SETCONST( pSymbol, "IMAGEBUTTON_ARROW_RIGHT",   SymbolType::ARROW_RIGHT );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_UP",       SymbolType::SPIN_UP );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_DOWN",     SymbolType::SPIN_DOWN );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_LEFT",     SymbolType::SPIN_LEFT );
        SETCONST( pSymbol, "IMAGEBUTTON_SPIN_RIGHT",    SymbolType::SPIN_RIGHT );
        SETCONST( pSymbol, "IMAGEBUTTON_FIRST",         SymbolType::FIRST );
        SETCONST( pSymbol, "IMAGEBUTTON_LAST",          SymbolType::LAST );
        SETCONST( pSymbol, "IMAGEBUTTON_PREV",          SymbolType::PREV );
        SETCONST( pSymbol, "IMAGEBUTTON_NEXT",          SymbolType::NEXT );
        SETCONST( pSymbol, "IMAGEBUTTON_PAGEUP",        SymbolType::PAGEUP );
        SETCONST( pSymbol, "IMAGEBUTTON_PAGEDOWN",      SymbolType::PAGEDOWN );
        SETCONST( pSymbol, "IMAGEBUTTON_PLAY",          SymbolType::PLAY );
        SETCONST( pSymbol, "IMAGEBUTTON_REVERSEPLAY",   SymbolType::REVERSEPLAY );
        SETCONST( pSymbol, "IMAGEBUTTON_STOP",          SymbolType::STOP );
        SETCONST( pSymbol, "IMAGEBUTTON_PAUSE",         SymbolType::PAUSE );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDSTART",     SymbolType::WINDSTART );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDEND",       SymbolType::WINDEND );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDBACKWARD",  SymbolType::WINDBACKWARD );
        SETCONST( pSymbol, "IMAGEBUTTON_WINDFORWARD",   SymbolType::WINDFORWARD );

        // add variable
        nVarId = aNmTb.Put( "Symbol", VARNAME );
        pClassImageButton->SetVariable( nVarId, pSymbol, nullptr, 0,
                                        (sal_uInt32)RscImageButtonFlags::Symbol );
    }
    nId = aNmTb.Put( "State", VARNAME );
    pClassImageButton->SetVariable( nId, pTriState, nullptr, 0,
                                    (sal_uInt32)RscImageButtonFlags::State );

    INS_WINBIT(pClassImageButton,Repeat)
    INS_WINBIT(pClassImageButton,SmallStyle)
    INS_WINBIT(pClassImageButton,RectStyle)

    return pClassImageButton;
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
    INS_WINBIT(pClassListBox,DDExtraWidth)

    {
        RSCINST aDflt = aUShort.Create( nullptr, RSCINST(), false );
        aDflt.pClass->SetNumber( aDflt, (sal_uInt16)0xFFFF );
        nId = aNmTb.Put( "CurPos", VARNAME );
        pClassListBox->SetVariable( nId, &aUShort, &aDflt );
    }
    nId = aNmTb.Put( "StringList", VARNAME );
    pClassListBox->SetVariable( nId, pStrLst );

    return pClassListBox;
}

RscTop * RscTypCont::InitClassComboBox( RscTop * pSuper, RscArray * pStrLst )
{
    Atom        nId;
    RscTop *    pClassComboBox;

    // initialize class
    nId = pHS->getID( "ComboBox" );
    pClassComboBox = new RscClass( nId, RSC_COMBOBOX, pSuper );
    pClassComboBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassComboBox );

    // initialize variables
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

RscTop * RscTypCont::InitClassRadioButton( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassRadioButton;

    // initialize class
    nId = pHS->getID( "RadioButton" );
    pClassRadioButton = new RscClass( nId, RSC_RADIOBUTTON, pSuper );
    pClassRadioButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassRadioButton );

    // initialize variables
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

RscTop * RscTypCont::InitClassMenuItem( RscTop * pSuper,
                                        RscTop * pClassBitmap )
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
    nId = aNmTb.Put( "ItemBitmap", VARNAME );
    pClassMenuItem->SetVariable( nId, pClassBitmap, nullptr, 0,
                                 (sal_uInt32)RscMenuItem::Bitmap );
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

RscTop * RscTypCont::InitClassNumericFormatter( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassNumeric;

    // initialize class
    nId = pHS->getID( "NumericFormatter" );
    pClassNumeric = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassNumeric->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // initialize variables
    nId = aNmTb.Put( "Minimum", VARNAME );
    pClassNumeric->SetVariable( nId, &aIdLong, nullptr,
                                0, (sal_uInt32)RscNumFormatterFlags::Min );
    nId = aNmTb.Put( "Maximum", VARNAME );
    pClassNumeric->SetVariable( nId, &aIdLong, nullptr,
                                0, (sal_uInt32)RscNumFormatterFlags::Max );
    nId = aNmTb.Put( "StrictFormat", VARNAME );
    pClassNumeric->SetVariable( nId, &aBool, nullptr,
                                0, (sal_uInt32)RscNumFormatterFlags::StrictFormat );
    nId = aNmTb.Put( "DecimalDigits", VARNAME );
    pClassNumeric->SetVariable( nId, &aUShort, nullptr,
                                0, (sal_uInt32)RscNumFormatterFlags::DecimalDigits );
    nId = aNmTb.Put( "Value", VARNAME );
    pClassNumeric->SetVariable( nId, &aIdLong, nullptr,
                                0, (sal_uInt32)RscNumFormatterFlags::Value );

    return pClassNumeric;
}

RscTop * RscTypCont::InitClassMetricFormatter( RscTop * pSuper,
                                                                                        RscEnum * pFieldUnits )
{
    Atom        nId;
    RscTop *    pClassMetric;

    // initialize class
    nId = pHS->getID( "MetricFormatter" );
    pClassMetric = new RscClass( nId, RSC_NOTYPE, pSuper );
    pClassMetric->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // initialize variables
    nId = aNmTb.Put( "Unit", VARNAME );
    pClassMetric->SetVariable( nId, pFieldUnits, nullptr,
                               0, (sal_uInt32)RscMetricFormatterFlags::Unit );
    nId = aNmTb.Put( "CustomUnitText", VARNAME );
    pClassMetric->SetVariable( nId, &aLangString, nullptr,
                               0, (sal_uInt32)RscMetricFormatterFlags::CustomUnitText );

    return pClassMetric;
}

RscTop * RscTypCont::InitClassSpinField( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassSpinField;

    // initialize class
    nId = pHS->getID( "SpinField" );
    pClassSpinField = new RscClass( nId, RSC_SPINFIELD, pSuper );
    pClassSpinField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassSpinField );

    INS_WINBIT(pClassSpinField,Repeat)
    INS_WINBIT(pClassSpinField,Spin)

    return pClassSpinField;
}

RscTop * RscTypCont::InitClassNumericField( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassNumericField;

    // initialize class
    nId = pHS->getID( "NumericField" );
    pClassNumericField = new RscClass( nId, RSC_NUMERICFIELD, pSuper );
    pClassNumericField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassNumericField );

    // initialize variables
    nId = aNmTb.Put( "First", VARNAME );
    pClassNumericField->SetVariable( nId, &aIdLong, nullptr,
                                                                        0, NUMERICFIELD_FIRST );
    nId = aNmTb.Put( "Last", VARNAME );
    pClassNumericField->SetVariable( nId, &aIdLong, nullptr,
                                                                        0, NUMERICFIELD_LAST );
    nId = aNmTb.Put( "SpinSize", VARNAME );
    pClassNumericField->SetVariable( nId, &aIdLong, nullptr,
                                                                        0, NUMERICFIELD_SPINSIZE  );
    return pClassNumericField;
}

RscTop * RscTypCont::InitClassMetricField( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassMetricField;

    // initialize class
    nId = pHS->getID( "MetricField" );
    pClassMetricField = new RscClass( nId, RSC_METRICFIELD, pSuper );
    pClassMetricField->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    aNmTb.Put( nId, CLASSNAME, pClassMetricField );

    // initialize variables
    nId = aNmTb.Put( "First", VARNAME );
    pClassMetricField->SetVariable( nId, &aIdLong, nullptr,
                                    0, (sal_uInt32)RscMetricFieldFlags::First );
    nId = aNmTb.Put( "Last", VARNAME );
    pClassMetricField->SetVariable( nId, &aIdLong, nullptr,
                                    0, (sal_uInt32)RscMetricFieldFlags::Last );
    nId = aNmTb.Put( "SpinSize", VARNAME );
    pClassMetricField->SetVariable( nId, &aIdLong, nullptr,
                                    0, (sal_uInt32)RscMetricFieldFlags::SpinSize  );

    return pClassMetricField;
}

RscTop * RscTypCont::InitClassDockingWindow( RscTop * pSuper,
                                             RscEnum * pMapUnit )
{
    Atom        nId;
    RscTop *    pClassDockWindow;

    // initialize class
    nId = pHS->getID( "DockingWindow" );
    pClassDockWindow = new RscClass( nId, RSC_DOCKINGWINDOW, pSuper );
    pClassDockWindow->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassDockWindow );

    // initialize variables
    nId = aNmTb.Put( "_FloatingPosMapMode", VARNAME );
    pClassDockWindow->SetVariable( nId, pMapUnit, nullptr, 0,
                                   RSC_DOCKINGWINDOW_XYMAPMODE  );
    nId = aNmTb.Put( "_FloatingPosX", VARNAME );
    pClassDockWindow->SetVariable( nId, &aShort, nullptr, 0,
                                   RSC_DOCKINGWINDOW_X );
    nId = aNmTb.Put( "_FloatingPosY", VARNAME );
    pClassDockWindow->SetVariable( nId, &aShort, nullptr, 0,
                                   RSC_DOCKINGWINDOW_Y );
    nId = aNmTb.Put( "FloatingMode", VARNAME );
    pClassDockWindow->SetVariable( nId, &aBool, nullptr, 0,
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
                                           RscTop * pClassBitmap )
{
    Atom        nId;
    RscTop *    pClassToolBoxItem;

    // initialize class
    nId = pHS->getID( "ToolBoxItem" );
    pClassToolBoxItem = new RscClass( nId, RSC_TOOLBOXITEM, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassToolBoxItem );

    // initialize variables
    nId = aNmTb.Put( "Identifier", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aIdNoZeroUShort, nullptr, 0,
                                    (sal_uInt32)RscToolboxItemFlags::Id );
    {
        RscEnum   * pEnum;

        aBaseLst.push_back( pEnum = new RscEnum( pHS->getID( "EnumToolBoxItemType" ), RSC_NOTYPE ) );
        SETCONST( pEnum, "TOOLBOXITEM_BUTTON", ToolBoxItemType::BUTTON );
        SETCONST( pEnum, "TOOLBOXITEM_SPACE", ToolBoxItemType::SPACE );
        SETCONST( pEnum, "TOOLBOXITEM_SEPARATOR", ToolBoxItemType::SEPARATOR );
        SETCONST( pEnum, "TOOLBOXITEM_BREAK", ToolBoxItemType::BREAK );

        // add variable
        nId = aNmTb.Put( "Type", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pEnum, nullptr, 0,
                                  (sal_uInt32)RscToolboxItemFlags::Type  );
    }
    {
        RscFlag *   pFlag;
        RscClient * pClient;
        Atom        l_nVarId, l_nAutoCheckId, l_nRadioCheckId, l_nCheckableId, l_nLeftId, l_nAutoSizeId, l_nDropDownId;

        aBaseLst.push_back( pFlag = new RscFlag( pHS->getID( "FlagToolBoxState" ), RSC_NOTYPE ) );

        // set constants in table
        l_nCheckableId = pHS->getID( "ToolBoxItemBits::CHECKABLE" );
        SETCONST( pFlag, l_nCheckableId, ToolBoxItemBits::CHECKABLE );
        l_nAutoCheckId = pHS->getID( "ToolBoxItemBits::AUTOCHECK" );
        SETCONST( pFlag, l_nAutoCheckId, ToolBoxItemBits::AUTOCHECK );
        l_nRadioCheckId = pHS->getID( "ToolBoxItemBits::RADIOCHECK" );
        SETCONST( pFlag, l_nRadioCheckId, ToolBoxItemBits::RADIOCHECK );
        l_nLeftId = pHS->getID( "ToolBoxItemBits::LEFT" );
        SETCONST( pFlag, l_nLeftId, ToolBoxItemBits::LEFT );
        l_nAutoSizeId = pHS->getID( "ToolBoxItemBits::AUTOSIZE" );
        SETCONST( pFlag, l_nAutoSizeId, ToolBoxItemBits::AUTOSIZE );
        l_nDropDownId = pHS->getID( "ToolBoxItemBits::DROPDOWN" );
        SETCONST( pFlag, l_nDropDownId, ToolBoxItemBits::DROPDOWN );

        // add variable
        l_nVarId = aNmTb.Put( "_ToolBoxItemFlags", VARNAME );
        pClassToolBoxItem->SetVariable( l_nVarId, pFlag, nullptr,
                                     VAR_HIDDEN | VAR_NOENUM,
                                     (sal_uInt32)RscToolboxItemFlags::Status );

        // add client variables
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nCheckableId )
        );
        nId = aNmTb.Put( "Checkable", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, nullptr, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nAutoCheckId )
        );
        nId = aNmTb.Put( "AutoCheck", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, nullptr, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nRadioCheckId )
        );
        nId = aNmTb.Put( "RadioCheck", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, nullptr, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nLeftId )
        );
        nId = aNmTb.Put( "Left", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, nullptr, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nAutoSizeId )
        );
        nId = aNmTb.Put( "AutoSize", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, nullptr, VAR_NODATAINST, 0, l_nVarId );

        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, pFlag, l_nDropDownId )
        );
        nId = aNmTb.Put( "DropDown", VARNAME );
        pClassToolBoxItem->SetVariable( nId, pClient, nullptr, VAR_NODATAINST, 0, l_nVarId );
    }
    nId = aNmTb.Put( "HelpID", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aStringLiteral, nullptr, 0,
                                    (sal_uInt32)RscToolboxItemFlags::HelpId  );
    nId = aNmTb.Put( "Text", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aLangString, nullptr, 0,
                                    (sal_uInt32)RscToolboxItemFlags::Text );
    nId = aNmTb.Put( "ItemBitmap", VARNAME );
    pClassToolBoxItem->SetVariable( nId, pClassBitmap, nullptr, 0,
                                    (sal_uInt32)RscToolboxItemFlags::Bitmap );
    nId = aNmTb.Put( "Disable", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aBool, nullptr, 0,
                                    (sal_uInt32)RscToolboxItemFlags::Disable );

    nId = aNmTb.Put( "Hide", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aBool, nullptr, 0,
                                    (sal_uInt32)RscToolboxItemFlags::Hide );
    nId = aNmTb.Put( "Command", VARNAME );
    pClassToolBoxItem->SetVariable( nId, &aString, nullptr, 0,
                                    (sal_uInt32)RscToolboxItemFlags::Command );

    return pClassToolBoxItem;
}

RscTop * RscTypCont::InitClassToolBox( RscTop * pSuper,
                                       RscTop * pClassToolBoxItem )
{
    Atom        nId;
    RscTop *    pClassToolBox;

    // initialize class
    nId = pHS->getID( "ToolBox" );
    pClassToolBox = new RscClass( nId, RSC_TOOLBOX, pSuper );
    pClassToolBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassToolBox );

    // initialize variables
    {
        RscEnum   * pEnum;

        aBaseLst.push_back( pEnum = new RscEnum( pHS->getID( "EnumButtonType" ), RSC_NOTYPE ) );
        SETCONST( pEnum, "BUTTON_SYMBOL",               ButtonType::SYMBOLONLY );
        SETCONST( pEnum, "BUTTON_TEXT",                 ButtonType::TEXT );
        SETCONST( pEnum, "BUTTON_SYMBOLTEXT",   ButtonType::SYMBOLTEXT );

        // add variable
        nId = aNmTb.Put( "ButtonType", VARNAME );
        pClassToolBox->SetVariable( nId, pEnum, nullptr, 0,
                                  (sal_uInt32)RscToolboxFlags::ButtonType  );
    }
    {
        RscEnum   * pEnum;

        aBaseLst.push_back( pEnum = new RscEnum( pHS->getID( "EnumToolBoxAlign" ), RSC_NOTYPE ) );
        SETCONST( pEnum, "BOXALIGN_TOP",                WindowAlign::Top );
        SETCONST( pEnum, "BOXALIGN_LEFT",               WindowAlign::Left );
        SETCONST( pEnum, "BOXALIGN_RIGHT",              WindowAlign::Right );
        SETCONST( pEnum, "BOXALIGN_BOTTOM",             WindowAlign::Bottom );

        // add variable
        nId = aNmTb.Put( "Align", VARNAME );
        pClassToolBox->SetVariable( nId, pEnum, nullptr, 0,
                                  (sal_uInt32)RscToolboxFlags::Align  );
    }
    nId = aNmTb.Put( "LineCount", VARNAME );
    pClassToolBox->SetVariable( nId, &aIdNoZeroUShort, nullptr, 0,
                                 (sal_uInt32)RscToolboxFlags::LineCount );
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
        pClassToolBox->SetVariable( nId, pLA, nullptr, 0,
                                    (sal_uInt32)RscToolboxFlags::ItemList );
    }
    INS_WINBIT(pClassToolBox,Scroll)
    INS_WINBIT(pClassToolBox,LineSpacing)
    INS_WINBIT(pClassToolBox,RectStyle)
    INS_WINBIT(pClassToolBox,Tabstop)

    return pClassToolBox;
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
