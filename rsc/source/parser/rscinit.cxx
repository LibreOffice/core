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

#include <rsctree.hxx>
#include <rsctop.hxx>
#include <rscrange.hxx>
#include <rscconst.hxx>
#include <rscflag.hxx>
#include <rscstr.hxx>
#include <rsccont.hxx>
#include <rscmgr.hxx>
#include <rscclass.hxx>
#include <rsckey.hxx>
#include <rscdb.hxx>
#include <rsclex.hxx>
#include <rscyacc.hxx>

#define INS_WINBIT( pClass, WinBit )        \
    InsWinBit( pClass, #WinBit, n##WinBit##Id );

void RscTypCont::Init()
{
    RscEnum *   pFieldUnits;
    RscEnum *   pColor;
    RscEnum *   pMapUnit;
    RscEnum *   pKey;
    RscEnum *   pTriState;
    RscTupel *  pGeometry;
    RscArray *  pLangGeometry;
    RscCont  *  pStringList;
    RscArray *  pLangStringList;
    RscTupel *  pStringTupel;
    RscTupel *  pStringLongTupel;
    RscCont  *  pStringTupelList;
    RscCont  *  pStringLongTupelList;
    RscArray *  pLangStringLongTupelList;

    RscTop   *  pClassMgr;
    RscTop   *  pClassString;
    RscTop   *  pClassStringArray;
    RscTop   *  pClassBitmap;
    RscTop   *  pClassColor;
    RscTop   *  pClassImage;
    RscTop   *  pClassImageList;
    RscTop   *  pClassWindow;
    RscTop   *  pClassSystemWindow;
    RscTop   *  pClassWorkWindow;
    RscTop   *  pClassControl;
    RscTop   *  pClassButton;
    RscTop   *  pClassCheckBox;
    RscTop   *  pClassPushButton;
    RscTop   *  pClassRadioButton;
    RscTop   *  pClassImageButton;
    RscTop   *  pClassEdit;
    RscTop   *  pClassListBox;
    RscTop   *  pClassComboBox;
    RscTop   *  pClassFixedText;
    RscTop   *  pClassFixedImage;
    RscTop   *  pClassKeyCode;
    RscTop   *  pLangClassKeyCode;
    RscTop   *  pClassAccelItem;
    RscTop   *  pClassSpinField;
    RscTop   *  pClassAccel;

    Atom        nId;

    aNmTb.SetSort( false );
{
    aNmTb.Put( "LINE",               LINE,           (sal_IntPtr)0 );
    aNmTb.Put( "NOT",                NOT,            (sal_IntPtr)0 );
    aNmTb.Put( "DEFINE",             DEFINE,         (sal_IntPtr)0 );
    aNmTb.Put( "INCLUDE",            INCLUDE,        (sal_IntPtr)0 );
    aNmTb.Put( "DEFAULT",            DEFAULT,        (sal_IntPtr)0  );
    aNmTb.Put( "class",              CLASS,          (sal_IntPtr)0  );
    aNmTb.Put( "extendable",         EXTENDABLE,     (sal_IntPtr)0  );
    aNmTb.Put( "writeifset",         WRITEIFSET,     (sal_IntPtr)0  );

/* values for integer types */
    aNmTb.Put( "TRUE",               BOOLEAN,        (sal_IntPtr)sal_True  );
    aNmTb.Put( "FALSE",              BOOLEAN,        (sal_IntPtr)sal_False );

    aNmTb.Put( "XSCALE",             XSCALE ,        (sal_IntPtr)0     );
    aNmTb.Put( "YSCALE",             YSCALE ,        (sal_IntPtr)0     );
    aNmTb.Put( "RGB",                RGB    ,        (sal_IntPtr)0     );
    aNmTb.Put( "POSSIZE",            GEOMETRY,       (sal_IntPtr)0     );
    aNmTb.Put( "POS",                POSITION,       (sal_IntPtr)0     );
    aNmTb.Put( "SIZE",               DIMENSION,      (sal_IntPtr)0     );
    aNmTb.Put( "ZoomInOutputSize",   INZOOMOUTPUTSIZE,(sal_IntPtr)0    );
    aNmTb.Put( "FloatingPos",        FLOATINGPOS,    (sal_IntPtr)0     );
}
{
    aShort.SetRange( -32768, 32767 );

    aUShort.SetRange( 0, 0xFFFF );

    aLong.SetRange( SAL_MIN_INT32, SAL_MAX_INT32 );
    aEnumLong.SetRange( SAL_MIN_INT32, SAL_MAX_INT32 );

    aIdUShort.SetRange( 0, 0xFFFF );

    aIdNoZeroUShort.SetRange( 1, 0xFFFF );

    aNoZeroShort.SetRange( -32768, 32767 );
    aNoZeroShort.SetOutRange( 0 );

    aIdLong.SetRange( SAL_MIN_INT32, SAL_MAX_INT32 );
}
{
    // variable name for WinBits
    nWinBitVarId = aNmTb.Put( "_WinBits", VARNAME );

    // Windows
    nBorderId       = pHS->getID( "WB_BORDER" );
    aWinBits.SetConstant( nBorderId, sal::static_int_cast<sal_Int32>(WB_BORDER) );
    nHideId         = pHS->getID( "WB_HIDE" );
    aWinBits.SetConstant( nHideId, sal::static_int_cast<sal_Int32>(WB_HIDE) );
    nClipChildrenId = pHS->getID( "WB_CLIPCHILDREN" );
    aWinBits.SetConstant( nClipChildrenId, sal::static_int_cast<sal_Int32>(WB_CLIPCHILDREN) );
    nSizeableId     = pHS->getID( "WB_SIZEABLE" );
    aWinBits.SetConstant( nSizeableId, sal::static_int_cast<sal_Int32>(WB_SIZEABLE) );
    nMoveableId     = pHS->getID( "WB_MOVEABLE" );
    aWinBits.SetConstant( nMoveableId, sal::static_int_cast<sal_Int32>(WB_MOVEABLE) );
    nMinimizeId     = pHS->getID( "WB_MINABLE" );
    aWinBits.SetConstant( nMinimizeId, 0 /*WB_MINABLE*/ );
    nMaximizeId     = pHS->getID( "WB_MAXABLE" );
    aWinBits.SetConstant( nMaximizeId, 0 /*WB_MAXABLE*/ );
    nCloseableId    = pHS->getID( "WB_CLOSEABLE" );
    aWinBits.SetConstant( nCloseableId, sal::static_int_cast<sal_Int32>(WB_CLOSEABLE) );
    nAppId          = pHS->getID( "WB_APP" );
    aWinBits.SetConstant( nAppId, sal::static_int_cast<sal_Int32>(WB_APP) );
    nTabstopId      = pHS->getID( "WB_TABSTOP" );
    aWinBits.SetConstant( nTabstopId, sal::static_int_cast<sal_Int32>(WB_TABSTOP) );
    nGroupId        = pHS->getID( "WB_GROUP" );
    aWinBits.SetConstant( nGroupId, sal::static_int_cast<sal_Int32>(WB_GROUP) );
    nSysmodalId     = pHS->getID( "WB_SYSMODAL" );
    aWinBits.SetConstant( nSysmodalId, 0 /*WB_SYSMODAL*/ );
}
{
    nLeftId         = pHS->getID( "WB_LEFT" );
    aWinBits.SetConstant( nLeftId, sal::static_int_cast<sal_Int32>(WB_LEFT) );
    nCenterId       = pHS->getID( "WB_CENTER" );
    aWinBits.SetConstant( nCenterId, sal::static_int_cast<sal_Int32>(WB_CENTER) );
    nRightId        = pHS->getID( "WB_RIGHT" );
    aWinBits.SetConstant( nRightId, sal::static_int_cast<sal_Int32>(WB_RIGHT) );
    nTopId          = pHS->getID( "WB_TOP" );
    aWinBits.SetConstant( nTopId, sal::static_int_cast<sal_Int32>(WB_TOP) );
    nVCenterId      = pHS->getID( "WB_VCENTER" );
    aWinBits.SetConstant( nVCenterId, sal::static_int_cast<sal_Int32>(WB_VCENTER) );
    nBottomId       = pHS->getID( "WB_BOTTOM" );
    aWinBits.SetConstant( nBottomId, sal::static_int_cast<sal_Int32>(WB_BOTTOM) );
    nHScrollId      = pHS->getID( "WB_HSCROLL" );
    aWinBits.SetConstant( nHScrollId, sal::static_int_cast<sal_Int32>(WB_HSCROLL) );
    nVScrollId      = pHS->getID( "WB_VSCROLL" );
    aWinBits.SetConstant( nVScrollId, sal::static_int_cast<sal_Int32>(WB_VSCROLL) );
    nSortId         = pHS->getID( "WB_SORT" );
    aWinBits.SetConstant( nSortId, sal::static_int_cast<sal_Int32>(WB_SORT) );
    nDefaultId          = pHS->getID( "WB_DEFBUTTON" );
    aWinBits.SetConstant( nDefaultId, sal::static_int_cast<sal_Int32>(WB_DEFBUTTON) );
    nRepeatId           = pHS->getID( "WB_REPEAT" );
    aWinBits.SetConstant( nRepeatId, sal::static_int_cast<sal_Int32>(WB_REPEAT) );
    nSVLookId           = pHS->getID( "WB_SVLOOK" );
    aWinBits.SetConstant( nSVLookId, sal::static_int_cast<sal_Int32>(WB_3DLOOK) );
    nDropDownId         = pHS->getID( "WB_DROPDOWN" );
    aWinBits.SetConstant( nDropDownId, sal::static_int_cast<sal_Int32>(WB_DROPDOWN) );
    nPassWordId         = pHS->getID( "WB_PASSWORD" );
    aWinBits.SetConstant( nPassWordId, sal::static_int_cast<sal_Int32>(WB_PASSWORD) );
    nReadOnlyId         = pHS->getID( "WB_READONLY" );
    aWinBits.SetConstant( nReadOnlyId, sal::static_int_cast<sal_Int32>(WB_READONLY) );
    nAutoSizeId         = pHS->getID( "WB_AUTOSIZE" );
    aWinBits.SetConstant( nAutoSizeId, sal::static_int_cast<sal_Int32>(WB_AUTOSIZE) );
    nSpinId             = pHS->getID( "WB_SPIN" );
    aWinBits.SetConstant( nSpinId, sal::static_int_cast<sal_Int32>(WB_SPIN) );
    nTabControlId       = pHS->getID( "WB_DIALOGCONTROL" );
    aWinBits.SetConstant( nTabControlId, sal::static_int_cast<sal_Int32>(WB_DIALOGCONTROL) );
    nSimpleModeId       = pHS->getID( "WB_SIMPLEMODE" );
    aWinBits.SetConstant( nSimpleModeId, sal::static_int_cast<sal_Int32>(WB_SIMPLEMODE) );
    nDragId             = pHS->getID( "WB_DRAG" );
    aWinBits.SetConstant( nDragId, sal::static_int_cast<sal_Int32>(WB_DRAG) );
    nScrollId           = pHS->getID( "WB_SCROLL" );
    aWinBits.SetConstant( nScrollId, sal::static_int_cast<sal_Int32>(WB_SCROLL) );
    nZoomableId         = pHS->getID( "WB_ZOOMABLE" );
    aWinBits.SetConstant( nZoomableId, sal::static_int_cast<sal_Int32>(WB_ROLLABLE) );
    nHideWhenDeactivateId = pHS->getID( "WB_HIDEWHENDEACTIVATE" );
    aWinBits.SetConstant( nHideWhenDeactivateId, 0 /*WB_HIDEWHENDEACTIVATE*/ );
    nAutoHScrollId      = pHS->getID( "WB_AUTOHSCROLL" );
    aWinBits.SetConstant( nAutoHScrollId, sal::static_int_cast<sal_Int32>(WB_AUTOHSCROLL) );
    nAutoVScrollId      = pHS->getID( "WB_AUTOVSCROLL" );
    aWinBits.SetConstant( nAutoVScrollId, sal::static_int_cast<sal_Int32>(WB_AUTOVSCROLL) );
    nDDExtraWidthId     = pHS->getID( "WB_DDEXTRAWIDTH" );
    aWinBits.SetConstant( nDDExtraWidthId, 0 /*WB_DDEXTRAWIDTH*/ );
    nWordBreakId        = pHS->getID( "WB_WORDBREAK" );
    aWinBits.SetConstant( nWordBreakId, sal::static_int_cast<sal_Int32>(WB_WORDBREAK) );
    nLeftLabelId        = pHS->getID( "WB_LEFTLABEL" );
    aWinBits.SetConstant( nLeftLabelId, 0 /*WB_LEFTLABEL*/ );
    nHasLinesId         = pHS->getID( "WB_HASLINES" );
    aWinBits.SetConstant( nHasLinesId, sal::static_int_cast<sal_Int32>(WB_HASLINES) );
    nHasButtonsId       = pHS->getID( "WB_HASBUTTONS" );
    aWinBits.SetConstant( nHasButtonsId, sal::static_int_cast<sal_Int32>(WB_HASBUTTONS) );
    nRectStyleId        = pHS->getID( "WB_RECTSTYLE" );
    aWinBits.SetConstant( nRectStyleId, sal::static_int_cast<sal_Int32>(WB_RECTSTYLE) );
    nLineSpacingId      = pHS->getID( "WB_LINESPACING" );
    aWinBits.SetConstant( nLineSpacingId, sal::static_int_cast<sal_Int32>(WB_LINESPACING) );
    nSmallStyleId       = pHS->getID( "WB_SMALLSTYLE" );
    aWinBits.SetConstant( nSmallStyleId, sal::static_int_cast<sal_Int32>(WB_SMALLSTYLE) );
    nEnableResizingId   = pHS->getID( "WB_ENABLERESIZING" );
    aWinBits.SetConstant( nEnableResizingId, 0 /*WB_ENABLERESIZING*/ );
    nDockableId         = pHS->getID( "WB_DOCKABLE" );
    aWinBits.SetConstant( nDockableId, sal::static_int_cast<sal_Int32>(WB_DOCKABLE) );
    nScaleId            = pHS->getID( "WB_SCALE" );
    aWinBits.SetConstant( nScaleId, sal::static_int_cast<sal_Int32>(WB_SCALE) );
    nIgnoreTabId        = pHS->getID( "WB_IGNORETAB" );
    aWinBits.SetConstant( nIgnoreTabId, sal::static_int_cast<sal_Int32>(WB_IGNORETAB) );
    nNoSplitDrawId      = pHS->getID( "WB_NOSPLITDRAW" );
    aWinBits.SetConstant( nNoSplitDrawId, sal::static_int_cast<sal_Int32>(WB_NOSPLITDRAW) );
    nTopImageId         = pHS->getID( "WB_TOPIMAGE" );
    aWinBits.SetConstant( nTopImageId, sal::static_int_cast<sal_Int32>(WB_TOPIMAGE) );
    nNoLabelId          = pHS->getID( "WB_NOLABEL" );
    aWinBits.SetConstant( nNoLabelId, sal::static_int_cast<sal_Int32>(WB_NOLABEL) );
    nVertId             = pHS->getID( "WB_VERT" );
    aWinBits.SetConstant( nVertId, sal::static_int_cast<sal_Int32>(WB_VERT) );
    nSysWinId           = pHS->getID( "WB_SYSTEMWINDOW" );
    aWinBits.SetConstant( nSysWinId, sal::static_int_cast<sal_Int32>(WB_SYSTEMWINDOW) );
    nStdPopupId         = pHS->getID( "WB_STDPOPUP" );
    aWinBits.SetConstant( nStdPopupId, sal::static_int_cast<sal_Int32>(WB_STDPOPUP) );
}
{
    InitLangType();
    aBaseLst.push_back( pFieldUnits      = InitFieldUnitsType() );
    aBaseLst.push_back( pColor           = InitColor() );
    aBaseLst.push_back( pMapUnit         = InitMapUnit() );
    aBaseLst.push_back( pKey             = InitKey() );
    aBaseLst.push_back( pTriState        = InitTriState() );

    aBaseLst.push_back( pGeometry        = InitGeometry() );
    aBaseLst.push_back( pLangGeometry    = InitLangGeometry( pGeometry ) );
    aBaseLst.push_back( pStringList      = InitStringList() );
    aBaseLst.push_back( pLangStringList  = InitLangStringList( pStringList ) );
    aBaseLst.push_back( pStringTupel     = InitStringTupel() );
    aBaseLst.push_back( pStringTupelList = InitStringTupelList( pStringTupel ) );
    aBaseLst.push_back( InitLangStringTupelList( pStringTupelList ) );
    aBaseLst.push_back( pStringLongTupel = InitStringLongTupel() );
    aBaseLst.push_back( pStringLongTupelList = InitStringLongTupelList( pStringLongTupel ) );
    aBaseLst.push_back( pLangStringLongTupelList = InitLangStringLongTupelList( pStringLongTupelList ) );
}
{
    pRoot = pClassMgr = InitClassMgr();

    aVersion.pClass = new RscClass( pHS->getID( "VersionControl" ),
                                    RSC_VERSIONCONTROL, pClassMgr );
    aVersion = aVersion.pClass->Create( nullptr, RSCINST() );

    pClassString = InitClassString( pClassMgr );
    pRoot->Insert( pClassString );

    // set String as reference class of the base strings
    aString.SetRefClass( pClassString );

    // initialize class
    nId = pHS->getID( "StringArray" );
    pClassStringArray = new RscClass( nId, RSC_STRINGARRAY, pClassMgr );
    pClassStringArray->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassStringArray );
    pRoot->Insert( pClassStringArray );

    // initialize variables
    nId = aNmTb.Put( "ItemList", VARNAME );
    pClassStringArray->SetVariable( nId, pLangStringLongTupelList );

    pClassBitmap = InitClassBitmap( pClassMgr );
    pRoot->Insert( pClassBitmap );

}
{
    pClassColor = InitClassColor( pClassMgr, pColor );
    pRoot->Insert( pClassColor );

    pClassImage = InitClassImage( pClassMgr, pClassBitmap, pClassColor );
    pRoot->Insert( pClassImage );

     pClassImageList = InitClassImageList( pClassMgr,
                                          pClassColor, pStringLongTupelList );
    pRoot->Insert( pClassImageList );

    pClassWindow = InitClassWindow( pClassMgr, pMapUnit,
                                    pLangGeometry );
    pRoot->Insert( pClassWindow );
}
{

    pClassSystemWindow = InitClassSystemWindow( pClassWindow );
    //aBaseLst.Insert( pClassSystemWindow, LIST_APPEND );
    pRoot->Insert( pClassSystemWindow );

    pClassWorkWindow = InitClassWorkWindow( pClassSystemWindow );
    pRoot->Insert( pClassWorkWindow );
}
{
    pClassControl = InitClassControl( pClassWindow );
    pRoot->Insert( pClassControl );

    // initialize class
    nId = pHS->getID( "Button" );
    pClassButton = new RscClass( nId, RSC_BUTTON, pClassControl );
    pClassButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassButton );
    pRoot->Insert( pClassButton );

    pClassCheckBox = InitClassCheckBox( pClassButton );
    pRoot->Insert( pClassCheckBox );

    // initialize class
    pClassPushButton = InitClassPushButton( pClassButton );
    pRoot->Insert( pClassPushButton );
}
{
    pClassRadioButton = InitClassRadioButton( pClassButton );
    pRoot->Insert( pClassRadioButton );

    pClassImageButton = InitClassImageButton( pClassPushButton,
                                              pClassImage, pTriState );
    pRoot->Insert( pClassImageButton );

    pClassEdit = InitClassEdit( pClassControl );
    pRoot->Insert( pClassEdit );
}
{
    pClassListBox = InitClassListBox( pClassControl, pLangStringLongTupelList );
    pRoot->Insert( pClassListBox );

    pClassComboBox = InitClassComboBox( pClassEdit, pLangStringList );
    pRoot->Insert( pClassComboBox );

    pClassFixedText = InitClassFixedText( pClassControl );
    pRoot->Insert( pClassFixedText );

    pClassFixedImage = InitClassFixedImage( pClassControl, pClassImage );
    pRoot->Insert( pClassFixedImage );

    pClassKeyCode = InitClassKeyCode( pClassMgr, pKey );
    pRoot->Insert( pClassKeyCode );
    {
    pLangClassKeyCode = new RscClassArray( pHS->getID( "LangKeyCode" ),
                    RSC_KEYCODE, pClassKeyCode, &aLangType );
    aBaseLst.push_back( pLangClassKeyCode );
    }

    pClassAccelItem = InitClassAccelItem( pClassMgr, pLangClassKeyCode );
    pRoot->Insert( pClassAccelItem );
}
{
    pClassAccel = InitClassAccel( pClassMgr, pClassAccelItem );
    pRoot->Insert( pClassAccel );
    nAcceleratorType = pClassAccel->GetId();

    // pClassAccel is only completely defined here
    nId = aNmTb.Put( "SubAccelerator", VARNAME );
    pClassAccelItem->SetVariable( nId, pClassAccel, nullptr, VAR_SVDYNAMIC,
                               ACCELITEM_ACCEL );

    RscTop* pClassMenuItem = InitClassMenuItem( pClassMgr, pClassBitmap,
                                        pLangClassKeyCode );
    pRoot->Insert( pClassMenuItem );

    RscTop* pClassMenu = InitClassMenu( pClassMgr, pClassMenuItem );
    pRoot->Insert( pClassMenu );

    // pClassMenu is only completely defined here
    nId = aNmTb.Put( "SubMenu", VARNAME );
    pClassMenuItem->SetVariable( nId, pClassMenu, nullptr, VAR_SVDYNAMIC,
                                 RSC_MENUITEM_MENU );
}
{
    RscTop* pClassSplitWindow = InitClassSplitWindow( pClassWindow );
    pRoot->Insert( pClassSplitWindow );

    // initialize class
    nId = pHS->getID( "SpinButton" );
    RscTop*  pClassSpinButton = new RscClass( nId, RSC_SPINBUTTON, pClassControl );
    pClassSpinButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassSpinButton );
    {
        RscClient * pClient;

        // add client variables
        // Sysmodal
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, &aWinBits, nRepeatId )
        );
        nId = aNmTb.Put( "Repeat", VARNAME );
        pClassSpinButton->SetVariable( nId, pClient, nullptr,
                                      VAR_NODATAINST, 0, nWinBitVarId );
    }
    pRoot->Insert( pClassSpinButton );
}
{
    pClassSpinField = InitClassSpinField( pClassEdit );
    pRoot->Insert( pClassSpinField );
}
{
    { // hand-made multiple inheritance
        RscTop* pClassTmp = InitClassNumericFormatter( pClassSpinField );
        aBaseLst.push_back( pClassTmp );

        RscTop* pClassNumericField = InitClassNumericField( pClassTmp );
        pRoot->Insert( pClassNumericField );
    }
    { // hand-made multiple inheritance
        RscTop* pClassTmp = InitClassNumericFormatter( pClassSpinField );
        aBaseLst.push_back( pClassTmp );
        pClassTmp = InitClassMetricFormatter( pClassTmp, pFieldUnits );
        aBaseLst.push_back( pClassTmp );

        RscTop* pClassMetricField = InitClassMetricField( pClassTmp );
        pRoot->Insert( pClassMetricField );
    }
}
{
    RscTop* pClassDockingWindow = InitClassDockingWindow( pClassWindow, pMapUnit );
    pRoot->Insert( pClassDockingWindow );

    RscTop* pClassToolBoxItem = InitClassToolBoxItem( pClassMgr, pClassBitmap,
                                              pClassImage, pTriState );
    pRoot->Insert( pClassToolBoxItem );

    RscTop* pClassToolBox = InitClassToolBox( pClassDockingWindow, pClassToolBoxItem,
                                      pClassImageList );
    pRoot->Insert( pClassToolBox );

    // initialize class
    nId = pHS->getID( "FixedLine" );
    RscTop* pClassFixedLine = new RscClass( nId, RSC_FIXEDLINE, pClassControl );
    pClassFixedLine->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    INS_WINBIT(pClassFixedLine,Vert)

    aNmTb.Put( nId, CLASSNAME, pClassFixedLine );
    pRoot->Insert( pClassFixedLine );

    RscTop* pClassSfxStyleFamilyItem = InitClassSfxStyleFamilyItem( pClassMgr,
                                                   pClassBitmap,
                                                   pClassImage,
                                                   pLangStringLongTupelList );
    pRoot->Insert( pClassSfxStyleFamilyItem );

    RscTop* pClassSfxTemplateDialog = InitClassSfxTemplateDialog( pClassMgr,
                                                  pClassSfxStyleFamilyItem );
    pRoot->Insert( pClassSfxTemplateDialog );

    RscTop* pClassSfxSlotInfo = InitClassSfxSlotInfo( pClassMgr );
    pRoot->Insert( pClassSfxSlotInfo );
}

    aNmTb.SetSort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
