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


/****************** I N C L U D E S **************************************/
// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>

#include <tools/rc.h>

#include <vclrsc.hxx>

// Programmabhaengige Includes.
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

/****************** M a c r o s ******************************************/
#define INS_WINBIT( pClass, WinBit )        \
    InsWinBit( pClass, #WinBit, n##WinBit##Id );

/*************************************************************************
|*
|*    RscTypCont::Init()
|*
*************************************************************************/
void RscTypCont::Init()
{
    RscEnum *   pFieldUnits;
    RscEnum *   pTimeFieldFormat;
    RscEnum *   pColor;
    RscEnum *   pMapUnit;
    RscEnum *   pKey;
    RscEnum *   pTriState;
    RscEnum *   pMessButtons;
    RscEnum *   pMessDefButton;
    RscTupel *  pGeometry;
    RscArray *  pLangGeometry;
    RscCont  *  pStringList;
    RscArray *  pLangStringList;
    RscTupel *  pStringTupel;
    RscTupel *  pStringLongTupel;
    RscCont  *  pStringTupelList;
    RscCont  *  pStringLongTupelList;
    RscArray *  pLangStringTupelList;
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
    RscTop   *  pClassDialog;
    RscTop   *  pClassModalDialog;
    RscTop   *  pClassModelessDialog;
    RscTop   *  pClassControl;
    RscTop   *  pClassButton;
    RscTop   *  pClassCheckBox;
    RscTop   *  pClassPushButton;
    RscTop   *  pClassOKButton;
    RscTop   *  pClassCancelButton;
    RscTop   *  pClassHelpButton;
    RscTop   *  pClassRadioButton;
    RscTop   *  pClassImageRadioButton;
    RscTop   *  pClassImageButton;
    RscTop   *  pClassTriStateBox;
    RscTop   *  pClassEdit;
    RscTop   *  pClassMultiLineEdit;
    RscTop   *  pClassScrollBar;
    RscTop   *  pClassListBox;
    RscTop   *  pClassMultiListBox;
    RscTop   *  pClassComboBox;
    RscTop   *  pClassFixedText;
    RscTop   *  pClassFixedBitmap;
    RscTop   *  pClassFixedImage;
    RscTop   *  pClassGroupBox;
    RscTop   *  pClassKeyCode;
    RscTop   *  pLangClassKeyCode;
    RscTop   *  pClassAccelItem;
    RscTop   *  pClassAccel;
    RscTop   *  pClassMenuItem;
    RscTop   *  pClassMenu;
    RscTop   *  pClassMenuButton;
    RscTop   *  pClassMessBox;
    RscTop   *  pClassInfoBox;
    RscTop   *  pClassWarningBox;
    RscTop   *  pClassErrorBox;
    RscTop   *  pClassQueryBox;
    RscTop   *  pClassSplitter;
    RscTop   *  pClassSplitWindow;
    RscTop   *  pClassSpinButton;
    RscTop   *  pClassTime;
    RscTop   *  pClassDate;
    RscTop   *  pClassSpinField;
    RscTop   *  pClassPatternField;
    RscTop   *  pClassNumericField;
    RscTop   *  pClassMetricField;
    RscTop   *  pClassCurrencyField;
    RscTop   *  pClassLongCurrencyField;
    RscTop   *  pClassDateField;
    RscTop   *  pClassTimeField;
    RscTop   *  pClassPatternBox;
    RscTop   *  pClassNumericBox;
    RscTop   *  pClassMetricBox;
    RscTop   *  pClassCurrencyBox;
    RscTop   *  pClassLongCurrencyBox;
    RscTop   *  pClassDateBox;
    RscTop   *  pClassTimeBox;
    RscTop   *  pClassDockingWindow;
    RscTop   *  pClassToolBoxItem;
    RscTop   *  pClassToolBox;
    RscTop   *  pClassStatusBar;
    RscTop   *  pClassMoreButton;
    RscTop   *  pClassFloatingWindow;
    RscTop   *  pClassTabPage;
    RscTop   *  pClassTabDialog;
    RscTop   *  pClassTabControlItem;
    RscTop   *  pClassTabControl;
    RscTop   *  pClassFixedLine;
    RscTop   *  pClassScrollBarBox;
    RscTop *    pClassSfxStyleFamilyItem;
    RscTop *    pClassSfxTemplateDialog;
    RscTop *    pClassSfxSlotInfo;

    Atom        nId;

    aNmTb.SetSort( sal_False );
{
    /********** C O M P I L E R   T Y P E N ******************************/
    aNmTb.Put( "LINE",               LINE,           (long)0 );
    aNmTb.Put( "NOT",                NOT,            (long)0 );
    aNmTb.Put( "DEFINE",             DEFINE,         (long)0 );
    aNmTb.Put( "INCLUDE",            INCLUDE,        (long)0 );
    aNmTb.Put( "DEFAULT",            DEFAULT,        (long)0  );
    aNmTb.Put( "class",              CLASS,          (long)0  );
    aNmTb.Put( "extendable",         EXTENDABLE,     (long)0  );
    aNmTb.Put( "writeifset",         WRITEIFSET,     (long)0  );

/* Werte fuer Aufzaehlungstypen */
    aNmTb.Put( "TRUE",               BOOLEAN,        (long)sal_True  );
    aNmTb.Put( "FALSE",              BOOLEAN,        (long)sal_False );

    aNmTb.Put( "XSCALE",             XSCALE ,        (long)0     );
    aNmTb.Put( "YSCALE",             YSCALE ,        (long)0     );
    aNmTb.Put( "RGB",                RGB    ,        (long)0     );
    aNmTb.Put( "POSSIZE",            GEOMETRY,       (long)0     );
    aNmTb.Put( "POS",                POSITION,       (long)0     );
    aNmTb.Put( "SIZE",               DIMENSION,      (long)0     );
    aNmTb.Put( "ZoomInOutputSize",   INZOOMOUTPUTSIZE,(long)0    );
    aNmTb.Put( "FloatingPos",        FLOATINGPOS,    (long)0     );
}
    /********** B A S I S   T Y P E N ************************************/
{
    /********** S H O R T ************************************************/
    aShort.SetRange( -32768, 32767 );

    /********** U S H O R T **********************************************/
    aUShort.SetRange( 0, 0xFFFF );

    /********** L O N G **************************************************/
    aLong.SetRange( SAL_MIN_INT32, SAL_MAX_INT32 );
    aEnumLong.SetRange( SAL_MIN_INT32, SAL_MAX_INT32 );

    /********** I D U S H O R T ******************************************/
    aIdUShort.SetRange( 0, 0xFFFF );

    /********** I D N O Z E R O U S H O R T ******************************/
    aIdNoZeroUShort.SetRange( 1, 0xFFFF );

    /********** N O Z E R O S H O R T ************************************/
    aNoZeroShort.SetRange( -32768, 32767 );
    aNoZeroShort.SetOutRange( 0 );

    /********** R A N G E S H O R T **************************************/
        a1to12Short.SetRange( 1, 12 );
        a0to23Short.SetRange( 0, 23 );
        a1to31Short.SetRange( 1, 31 );
        a0to59Short.SetRange( 0, 59 );
        a0to99Short.SetRange( 0, 99 );
        a0to9999Short.SetRange( 0, 9999 );

    /********** I D R A N G E ********************************************/
    aIdLong.SetRange( SAL_MIN_INT32, SAL_MAX_INT32 );
}
{
    /********** W I N B I T S F L A G ************************************/
    // Variablenname fuer WinBits
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
    /********** I n i t   B a s i c   T y p e s **************************/
    InitLangType();
    aBaseLst.push_back( pFieldUnits      = InitFieldUnitsType() );
    aBaseLst.push_back( pTimeFieldFormat = InitTimeFieldFormat() );
    aBaseLst.push_back( pColor           = InitColor() );
    aBaseLst.push_back( pMapUnit         = InitMapUnit() );
    aBaseLst.push_back( pKey             = InitKey() );
    aBaseLst.push_back( pTriState        = InitTriState() );
    aBaseLst.push_back( pMessButtons     = InitMessButtons() );
    aBaseLst.push_back( pMessDefButton   = InitMessDefButton() );

    aBaseLst.push_back( pGeometry        = InitGeometry() );
    aBaseLst.push_back( pLangGeometry    = InitLangGeometry( pGeometry ) );
    aBaseLst.push_back( pStringList      = InitStringList() );
    aBaseLst.push_back( pLangStringList  = InitLangStringList( pStringList ) );
    aBaseLst.push_back( pStringTupel     = InitStringTupel() );
    aBaseLst.push_back( pStringTupelList = InitStringTupelList( pStringTupel ) );
    aBaseLst.push_back( pLangStringTupelList = InitLangStringTupelList( pStringTupelList ) );
    aBaseLst.push_back( pStringLongTupel = InitStringLongTupel() );
    aBaseLst.push_back( pStringLongTupelList = InitStringLongTupelList( pStringLongTupel ) );
    aBaseLst.push_back( pLangStringLongTupelList = InitLangStringLongTupelList( pStringLongTupelList ) );
}
{
    /********** R E S O U R C E   T Y P E N ******************************/
    /********** R S C M G R **********************************************/
    pRoot = pClassMgr = InitClassMgr();

    /********** V e r s i o n s k o n t r o l l e ************************/
    aVersion.pClass = new RscClass( pHS->getID( "VersionControl" ),
                                    RSC_VERSIONCONTROL, pClassMgr );
    aVersion = aVersion.pClass->Create( NULL, RSCINST() );

    /********** S T R I N G **********************************************/
    pClassString = InitClassString( pClassMgr );
    pRoot->Insert( pClassString );

    // String als Referenzklasse des Basisstrings einsetzen
    aString.SetRefClass( pClassString );

    /********** S T R I N G L I S T **************************************/
    // Klasse anlegen
    nId = pHS->getID( "StringArray" );
    pClassStringArray = new RscClass( nId, RSC_STRINGARRAY, pClassMgr );
    pClassStringArray->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassStringArray );
    pRoot->Insert( pClassStringArray );

    // Variablen anlegen
    nId = aNmTb.Put( "ItemList", VARNAME );
    pClassStringArray->SetVariable( nId, pLangStringLongTupelList );

    /********** B I T M A P **********************************************/
    pClassBitmap = InitClassBitmap( pClassMgr );
    pRoot->Insert( pClassBitmap );

}
{
    /********** C O L O R ************************************************/
    pClassColor = InitClassColor( pClassMgr, pColor );
    pRoot->Insert( pClassColor );

    /********** I M A G E ************************************************/
    pClassImage = InitClassImage( pClassMgr, pClassBitmap, pClassColor );
    pRoot->Insert( pClassImage );

    /********** I M A G E L I S T ****************************************/
     pClassImageList = InitClassImageList( pClassMgr,
                                          pClassColor, pStringLongTupelList );
    pRoot->Insert( pClassImageList );

    /********** W I N D O W **********************************************/
    pClassWindow = InitClassWindow( pClassMgr, pMapUnit,
                                    pLangGeometry );
    pRoot->Insert( pClassWindow );
}
{

    /********** S Y S T E M W I N D O W **********************************/
    pClassSystemWindow = InitClassSystemWindow( pClassWindow );
    //aBaseLst.Insert( pClassSystemWindow, LIST_APPEND );
    pRoot->Insert( pClassSystemWindow );

    /********** W O R K W I N D O W **************************************/
    pClassWorkWindow = InitClassWorkWindow( pClassSystemWindow );
    pRoot->Insert( pClassWorkWindow );

    /********** D I A L O G **********************************************/
    // Klasse anlegen
    pClassDialog = new RscClass( pHS->getID( "Dialog" ),
                                 RSC_DIALOG, pClassSystemWindow );
    pClassDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aBaseLst.push_back( pClassDialog );

    /********** M O D A L D I A L O G ***********************************/
    // Klasse anlegen
    pClassModalDialog = InitClassModalDialog( pClassDialog );
    pRoot->Insert( pClassModalDialog );

    /********** M O D E L E S S D I A L O G ******************************/
    // Klasse anlegen
    pClassModelessDialog = InitClassModelessDialog( pClassDialog );
    pRoot->Insert( pClassModelessDialog );
}
{
    /********** C O N T R O L ********************************************/
    pClassControl = InitClassControl( pClassWindow );
    pRoot->Insert( pClassControl );

    /********** B U T T O N **********************************************/
    // Klasse anlegen
    nId = pHS->getID( "Button" );
    pClassButton = new RscClass( nId, RSC_BUTTON, pClassControl );
    pClassButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassButton );
    pRoot->Insert( pClassButton );

    /********** C H E C K B O X ******************************************/
    pClassCheckBox = InitClassCheckBox( pClassButton );
    pRoot->Insert( pClassCheckBox );

    /********** P U S H B U T T O N **************************************/
    // Klasse anlegen
    pClassPushButton = InitClassPushButton( pClassButton );
    pRoot->Insert( pClassPushButton );

    /********** H E L P B U T T O N **************************************/
    // Klasse anlegen
    nId = pHS->getID( "HelpButton" );
    pClassHelpButton = new RscClass( nId, RSC_HELPBUTTON,
                                        pClassPushButton );
    pClassHelpButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassHelpButton );
    pRoot->Insert( pClassHelpButton );

    /********** O K B U T T O N ******************************************/
    // Klasse anlegen
    nId = pHS->getID( "OKButton" );
    pClassOKButton = new RscClass( nId, RSC_OKBUTTON,
                                        pClassPushButton );
    pClassOKButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassOKButton );
    pRoot->Insert( pClassOKButton );

    /********** C A N C E L B U T T O N **********************************/
    // Klasse anlegen
    nId = pHS->getID( "CancelButton" );
    pClassCancelButton = new RscClass( nId, RSC_CANCELBUTTON,
                                        pClassPushButton );
    pClassCancelButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassCancelButton );
    pRoot->Insert( pClassCancelButton );
}
{
    /********** R A D I O B U T T O N ************************************/
    pClassRadioButton = InitClassRadioButton( pClassButton );
    pRoot->Insert( pClassRadioButton );

    /********** I m a g e R a d i o B u t t o n **************************/
    nId = pHS->getID( "ImageRadioButton" );
    pClassImageRadioButton = InitClassImageRadioButton( pClassRadioButton,
                                                        pClassImage );
    pRoot->Insert( pClassImageRadioButton );

    /********** T R I S T A T E B O X ************************************/
    pClassTriStateBox = InitClassTriStateBox( pClassControl, pTriState );
    pRoot->Insert( pClassTriStateBox );

    /********** I M A G E B U T T O N ************************************/
    pClassImageButton = InitClassImageButton( pClassPushButton,
                                              pClassImage, pTriState );
    pRoot->Insert( pClassImageButton );

    /********** E D I T **************************************************/
    pClassEdit = InitClassEdit( pClassControl );
    pRoot->Insert( pClassEdit );

    /********** M U L T I L I N E E D I T ********************************/
    pClassMultiLineEdit = InitClassMultiLineEdit( pClassEdit );
    pRoot->Insert( pClassMultiLineEdit );

    /********** S C R O L L B A R ****************************************/
    pClassScrollBar = InitClassScrollBar( pClassControl );
    pRoot->Insert( pClassScrollBar );

}
{
    /********** L I S T B O X ********************************************/
    pClassListBox = InitClassListBox( pClassControl, pLangStringLongTupelList );
    pRoot->Insert( pClassListBox );

    /********** M U L T I L I S T B O X **********************************/
    pClassMultiListBox = InitClassMultiListBox( pClassListBox);
    pRoot->Insert( pClassMultiListBox );

    /********** C O M B O B O X ******************************************/
    pClassComboBox = InitClassComboBox( pClassEdit, pLangStringList );
    pRoot->Insert( pClassComboBox );

    /********** F I X E D T E X T ****************************************/
    pClassFixedText = InitClassFixedText( pClassControl );
    pRoot->Insert( pClassFixedText );

    /********** F i x e d B i t m a p ************************************/
    pClassFixedBitmap = InitClassFixedBitmap( pClassControl, pClassBitmap );
    pRoot->Insert( pClassFixedBitmap );

    /********** F i x e d I m a g e **************************************/
    pClassFixedImage = InitClassFixedImage( pClassControl, pClassImage );
    pRoot->Insert( pClassFixedImage );

    /********** G R O U P B O X ******************************************/
    // Klasse anlegen
    nId = pHS->getID( "GroupBox" );
    pClassGroupBox = new RscClass( nId, RSC_GROUPBOX, pClassControl );
    pClassGroupBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassGroupBox );
    pRoot->Insert( pClassGroupBox );

    /********** K E Y C O D E ********************************************/
    pClassKeyCode = InitClassKeyCode( pClassMgr, pKey );
    pRoot->Insert( pClassKeyCode );
    {
    pLangClassKeyCode = new RscClassArray( pHS->getID( "LangKeyCode" ),
                    RSC_KEYCODE, pClassKeyCode, &aLangType );
    aBaseLst.push_back( pLangClassKeyCode );
    }

    /********** A C C E L I T E M  ***************************************/
    pClassAccelItem = InitClassAccelItem( pClassMgr, pLangClassKeyCode );
    pRoot->Insert( pClassAccelItem );
}
{
    /********** A C C E L E R A T O R ************************************/
    pClassAccel = InitClassAccel( pClassMgr, pClassAccelItem );
    pRoot->Insert( pClassAccel );
    nAcceleratorType = pClassAccel->GetId();

    /********** A C C E L I T E M  ***************************************/
    // pClassAccel ist erst hier definiert
    nId = aNmTb.Put( "SubAccelerator", VARNAME );
    pClassAccelItem->SetVariable( nId, pClassAccel, NULL, VAR_SVDYNAMIC,
                               ACCELITEM_ACCEL );

    /********** M E N U I T E M ******************************************/
    pClassMenuItem = InitClassMenuItem( pClassMgr, pClassBitmap,
                                        pLangClassKeyCode );
    pRoot->Insert( pClassMenuItem );

    /********** M E N U **************************************************/
    pClassMenu = InitClassMenu( pClassMgr, pClassMenuItem );
    pRoot->Insert( pClassMenu );

    /********** M E N U I T E M ******************************************/
    // pClassMenu ist erst hier definiert
    nId = aNmTb.Put( "SubMenu", VARNAME );
    pClassMenuItem->SetVariable( nId, pClassMenu, NULL, VAR_SVDYNAMIC,
                                 RSC_MENUITEM_MENU );

    /********** M E N U B U T T O N **************************************/
    pClassMenuButton = InitClassMenuButton( pClassControl, pClassMenu );
    pRoot->Insert( pClassMenuButton );

    /********** M E S S A G E B O X **************************************/
    pClassMessBox = InitClassMessBox( pClassMgr, pMessButtons,
                                         pMessDefButton );
    pRoot->Insert( pClassMessBox );

    /********** I N F O B O X ********************************************/
    // Klasse anlegen
    nId = pHS->getID( "InfoBox" );
    pClassInfoBox = new RscClass( nId, RSC_INFOBOX, pClassMessBox );
    pClassInfoBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassInfoBox );
    pRoot->Insert( pClassInfoBox );

    /********** W A R N I N G B O X **************************************/
    // Klasse anlegen
    nId = pHS->getID( "WarningBox" );
    pClassWarningBox = new RscClass( nId, RSC_WARNINGBOX, pClassMessBox );
    pClassWarningBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassWarningBox );
    pRoot->Insert( pClassWarningBox );

    /********** E R R O R B O X ******************************************/
    // Klasse anlegen
    nId = pHS->getID( "ErrorBox" );
    pClassErrorBox = new RscClass( nId, RSC_ERRORBOX, pClassMessBox );
    pClassErrorBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassErrorBox );
    pRoot->Insert( pClassErrorBox );

    /********** Q U E R Y B O X ******************************************/
    // Klasse anlegen
    nId = pHS->getID( "QueryBox" );
    pClassQueryBox = new RscClass( nId, RSC_QUERYBOX, pClassMessBox );
    pClassQueryBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassQueryBox );
    pRoot->Insert( pClassQueryBox );
}
{
    /********** S P L I T T E R ******************************************/
    pClassSplitter = InitClassSplitter( pClassWindow );
    pRoot->Insert( pClassSplitter );

    /********** S P L I T W I N D O W ************************************/
    pClassSplitWindow = InitClassSplitWindow( pClassWindow );
    pRoot->Insert( pClassSplitWindow );

    /********** S P I N B U T T O N **************************************/
    // Klasse anlegen
    nId = pHS->getID( "SpinButton" );
    pClassSpinButton = new RscClass( nId, RSC_SPINBUTTON, pClassControl );
    pClassSpinButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassSpinButton );
    {
        RscClient * pClient;

        // Clientvariablen einfuegen
        // Sysmodal
        aBaseLst.push_back(
            pClient = new RscClient( pHS->getID( "sal_Bool" ), RSC_NOTYPE, &aWinBits, nRepeatId )
        );
        nId = aNmTb.Put( "Repeat", VARNAME );
        pClassSpinButton->SetVariable( nId, pClient, NULL,
                                      VAR_NODATAINST, 0, nWinBitVarId );
    }
    pRoot->Insert( pClassSpinButton );
}
{
    /********** T I M E **************************************************/
    pClassTime = InitClassTime( pClassMgr );
    pRoot->Insert( pClassTime );

    /********** D A T E **************************************************/
    pClassDate = InitClassDate( pClassMgr );
    pRoot->Insert( pClassDate );
}
{
    /********** S P I N F I E L D ****************************************/
    pClassSpinField = InitClassSpinField( pClassEdit );
    pRoot->Insert( pClassSpinField );
}
{
    /********** P A T T E R N F I E L D **********************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassPatternFormatter( pClassSpinField );
    aBaseLst.push_back( pClassTmp );

    pClassPatternField = InitClassPatternField( pClassTmp );
    pRoot->Insert( pClassPatternField );
    }
    /********** N U M E R I C F I E L D **********************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassSpinField );
    aBaseLst.push_back( pClassTmp );

    pClassNumericField = InitClassNumericField( pClassTmp );
    pRoot->Insert( pClassNumericField );
    }
    /********** M E T R I C F I E L D ************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassSpinField );
    aBaseLst.push_back( pClassTmp );
    pClassTmp = InitClassMetricFormatter( pClassTmp, pFieldUnits );
    aBaseLst.push_back( pClassTmp );

    pClassMetricField = InitClassMetricField( pClassTmp );
    pRoot->Insert( pClassMetricField );
    }
    /********** C U R R E N C Y F I E L D ********************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassSpinField );
    aBaseLst.push_back( pClassTmp );
    pClassTmp = InitClassCurrencyFormatter( pClassTmp );
    aBaseLst.push_back( pClassTmp );

    pClassCurrencyField = InitClassCurrencyField( "CurrencyField", RSC_CURRENCYFIELD, pClassTmp );
    pRoot->Insert( pClassCurrencyField );

    pClassLongCurrencyField = InitClassCurrencyField( "LongCurrencyField", RSC_LONGCURRENCYFIELD, pClassTmp );
    pRoot->Insert( pClassLongCurrencyField );

    }
    /********** D A T E F I E L D ****************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassDateFormatter( pClassSpinField, pClassDate );
    aBaseLst.push_back( pClassTmp );

    pClassDateField = InitClassDateField( pClassTmp, pClassDate );
    pRoot->Insert( pClassDateField );
    }
    /********** T I M E F I E L D ****************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassTimeFormatter( pClassSpinField, pClassTime,
                                                 pTimeFieldFormat );
    aBaseLst.push_back( pClassTmp );

    pClassTimeField = InitClassTimeField( pClassTmp, pClassTime );
    pRoot->Insert( pClassTimeField );
    }
    /********** P A T T E R N B O X **************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassPatternFormatter( pClassComboBox );
    aBaseLst.push_back( pClassTmp );

    pClassPatternBox = InitClassPatternBox( pClassTmp );
    pRoot->Insert( pClassPatternBox );
    }
    /********** N U M E R I C B O X **************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassComboBox );
    aBaseLst.push_back( pClassTmp );

    pClassNumericBox = InitClassNumericBox( pClassTmp );
    pRoot->Insert( pClassNumericBox );
    }
}
{
    /********** M E T R I C B O X ****************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassComboBox );
    aBaseLst.push_back( pClassTmp );
    pClassTmp = InitClassMetricFormatter( pClassTmp, pFieldUnits );
    aBaseLst.push_back( pClassTmp );

    pClassMetricBox = InitClassMetricBox( pClassTmp );
    pRoot->Insert( pClassMetricBox );
    }
    /********** C U R R E N C Y B O X ************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassComboBox );
    aBaseLst.push_back( pClassTmp );
    pClassTmp = InitClassCurrencyFormatter( pClassTmp );
    aBaseLst.push_back( pClassTmp );

    pClassCurrencyBox = InitClassCurrencyBox( "CurrencyBox", RSC_CURRENCYBOX, pClassTmp );
    pRoot->Insert( pClassCurrencyBox );

    pClassLongCurrencyBox = InitClassCurrencyBox( "LongCurrencyBox", RSC_LONGCURRENCYBOX, pClassTmp );
    pRoot->Insert( pClassLongCurrencyBox );
    }
    /********** D A T E B O X ********************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassDateFormatter( pClassComboBox, pClassDate );
    aBaseLst.push_back( pClassTmp );

    pClassDateBox = InitClassDateBox( pClassTmp );
    pRoot->Insert( pClassDateBox );
    }
    /********** T I M E B O X ********************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassTimeFormatter( pClassComboBox, pClassTime,
                                                 pTimeFieldFormat );
    aBaseLst.push_back( pClassTmp );

    pClassTimeBox = InitClassTimeBox( pClassTmp );
    pRoot->Insert( pClassTimeBox );
    }
    /********** D O C K I N G W I N D O W ********************************/
    pClassDockingWindow = InitClassDockingWindow( pClassWindow, pMapUnit );
    pRoot->Insert( pClassDockingWindow );

    /********** T O O L B O X I T E M ************************************/
    pClassToolBoxItem = InitClassToolBoxItem( pClassMgr, pClassBitmap,
                                              pClassImage, pTriState );
    pRoot->Insert( pClassToolBoxItem );

    /********** T O O L B O X ********************************************/
    pClassToolBox = InitClassToolBox( pClassDockingWindow, pClassToolBoxItem,
                                      pClassImageList );
    pRoot->Insert( pClassToolBox );

    /********** S T A T U S B A R ****************************************/
    pClassStatusBar = InitClassStatusBar( pClassWindow );
    pRoot->Insert( pClassStatusBar );

    /********** M O R E B U T T O N **************************************/
    pClassMoreButton = InitClassMoreButton( pClassPushButton, pMapUnit );
    pRoot->Insert( pClassMoreButton );

    /********** F L O A T W I N D O W ************************************/
    pClassFloatingWindow = InitClassFloatingWindow( pClassSystemWindow,
                                                    pMapUnit );
    pRoot->Insert( pClassFloatingWindow );

    /********** T A B P A G E ********************************************/
    // Klasse anlegen
    nId = pHS->getID( "TabPage" );
    pClassTabPage =
          new RscClass( nId, RSC_TABPAGE, pClassWindow );
    pClassTabPage->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassTabPage );
    pRoot->Insert( pClassTabPage );

    /********** T A B D I A L O G ****************************************/
    // Klasse anlegen
    nId = pHS->getID( "TabDialog" );
    pClassTabDialog =
          new RscClass( nId, RSC_TABDIALOG, pClassModalDialog );
    pClassTabDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassTabDialog );
    pRoot->Insert( pClassTabDialog );

    /********** T A B C O N T R O L I T E M *******************************/
    pClassTabControlItem = InitClassTabControlItem( pClassMgr );
    pRoot->Insert( pClassTabControlItem );

    /********** T A B C O N T R O L **************************************/
    pClassTabControl = InitClassTabControl( pClassControl,
                                            pClassTabControlItem );
    pRoot->Insert( pClassTabControl );

    /********** F I X E D L I N E ****************************************/
    // Klasse anlegen
    nId = pHS->getID( "FixedLine" );
    pClassFixedLine =
          new RscClass( nId, RSC_FIXEDLINE, pClassControl );
    pClassFixedLine->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );

    INS_WINBIT(pClassFixedLine,Vert)

    aNmTb.Put( nId, CLASSNAME, pClassFixedLine );
    pRoot->Insert( pClassFixedLine );

    /********** S C R O L L B A R B O X **********************************/
    // Klasse anlegen
    nId = pHS->getID( "ScrollBarBox" );
    pClassScrollBarBox =
          new RscClass( nId, RSC_SCROLLBARBOX, pClassWindow );
    pClassScrollBarBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassScrollBarBox );
    pRoot->Insert( pClassScrollBarBox );
    INS_WINBIT(pClassScrollBarBox,Sizeable)

    /********** S F X S T Y L E F A M I L Y I T E M **********************/
    pClassSfxStyleFamilyItem = InitClassSfxStyleFamilyItem( pClassMgr,
                                                   pClassBitmap,
                                                   pClassImage,
                                                   pLangStringLongTupelList );
    pRoot->Insert( pClassSfxStyleFamilyItem );

    /********** S F X T E M P L A T E D I A L O G ************************/
    pClassSfxTemplateDialog = InitClassSfxTemplateDialog( pClassMgr,
                                                  pClassSfxStyleFamilyItem );
    pRoot->Insert( pClassSfxTemplateDialog );

    /********** S F X I N F O I T E M ************************************/
    pClassSfxSlotInfo = InitClassSfxSlotInfo( pClassMgr );
    pRoot->Insert( pClassSfxSlotInfo );
}

    aNmTb.SetSort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
