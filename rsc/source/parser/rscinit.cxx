/*************************************************************************
 *
 *  $RCSfile: rscinit.cxx,v $
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

// Programmabhaengige Includes.
#ifndef _RSCTREE_HXX
#include <rsctree.hxx>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif
#ifndef _RSCRANGE_HXX
#include <rscrange.hxx>
#endif
#ifndef _RSCCONST_HXX
#include <rscconst.hxx>
#endif
#ifndef _RSCFLAG_HXX
#include <rscflag.hxx>
#endif
#ifndef _RSCSTR_HXX
#include <rscstr.hxx>
#endif
#ifndef _RSCCONT_HXX
#include <rsccont.hxx>
#endif
#ifndef _RSCMGR_HXX
#include <rscmgr.hxx>
#endif
#ifndef _RSCCLASS_HXX
#include <rscclass.hxx>
#endif
#ifndef _RSCKEY_HXX
#include <rsckey.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif

#include "rsclex.hxx"
#include <rscyacc.yxx.h>

/****************** M a c r o s ******************************************/
#define INS_WINBIT( pClass, WinBit )        \
    InsWinBit( pClass, #WinBit, n##WinBit##Id );

/****************** C O D E **********************************************/
void NameToVerCtrl( RSCINST & aVersion, RscTop * pClass,
                    RscTop * pClassString )
{
    if( pClass )
    {
        NameToVerCtrl( aVersion, (RscTop *)pClass->Left(), pClassString );
        {
            RSCINST aVI;
            RSCINST aStr;

            // Namen in Versionskontrolle einsetzen
            aVersion.pClass->
                  GetElement( aVersion, RscId( pClass->GetTypId() ),
                              pClassString, RSCINST(), &aVI );
            aStr = aVI.pClass->GetVariable( aVI, pHS->Insert( "TEXT" ),
                                            RSCINST() );
            aStr.pClass->SetString( aStr, pHS->Get( pClass->GetId() ) );
        }
        NameToVerCtrl( aVersion, (RscTop *)pClass->Right(), pClassString );
    }
}

/*************************************************************************
|*
|*    RscTypCont::Init()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.03.91
|*    Letzte Aenderung  MM 27.06.91
|*
*************************************************************************/
void RscTypCont::Init()
{
    RscEnum *   pDateFormat;
    RscEnum *   pTimeFormat;
    RscEnum *   pWeekDayFormat;
    RscEnum *   pMonthFormat;
    RscEnum *   pFieldUnits;
    RscEnum *   pDayOfWeek;
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
    RscTop   *  pClassI12;
    RscTop   *  pLangClassI12;
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

    HASHID      nId;

    aNmTb.SetSort( FALSE );
{
    /********** C O M P I L E R   T Y P E N ******************************/
    aNmTb.Put( "LINE",               (USHORT)LINE,           (long)0 );
    aNmTb.Put( "NOT",                (USHORT)NOT,            (long)0 );
    aNmTb.Put( "DEFINE",             (USHORT)DEFINE,         (long)0 );
    aNmTb.Put( "INCLUDE",            (USHORT)INCLUDE,        (long)0 );
    aNmTb.Put( "DEFAULT",           (USHORT)DEFAULT,         (long)0  );
    aNmTb.Put( "class",             (USHORT)CLASS,           (long)0  );
    aNmTb.Put( "extendable",        (USHORT)EXTENDABLE,      (long)0  );
    aNmTb.Put( "writeifset",        (USHORT)WRITEIFSET,      (long)0  );

/* Werte fuer Aufzaehlungstypen */
    aNmTb.Put( "TRUE",               (USHORT)BOOLEAN,        (long)TRUE  );
    aNmTb.Put( "FALSE",              (USHORT)BOOLEAN,        (long)FALSE );

/* Vordefinierte HilfeId's */
    aNmTb.Put( "HELP_INDEX",         (USHORT)NUMBER,     HELP_INDEX      );
    aNmTb.Put( "HELP_HELPONHELP",    (USHORT)NUMBER,     HELP_HELPONHELP );

    aNmTb.Put( "XSCALE",             (USHORT)XSCALE ,        (long)0     );
    aNmTb.Put( "YSCALE",             (USHORT)YSCALE ,        (long)0     );
    aNmTb.Put( "RGB",                (USHORT)RGB    ,        (long)0     );
    aNmTb.Put( "POSSIZE",            (USHORT)GEOMETRY,       (long)0     );
    aNmTb.Put( "POS",                (USHORT)POSITION,       (long)0     );
    aNmTb.Put( "SIZE",               (USHORT)DIMENSION,      (long)0     );
    aNmTb.Put( "ZoomInOutputSize",   (USHORT)INZOOMOUTPUTSIZE,(long)0    );
    aNmTb.Put( "FloatingPos",        (USHORT)FLOATINGPOS,    (long)0     );
}
    /********** B A S I S   T Y P E N ************************************/
{
    /********** S H O R T ************************************************/
    aShort.SetRange( -32768, 32767 );

    /********** U S H O R T **********************************************/
    aUShort.SetRange( 0, 0xFFFF );

    /********** L O N G **************************************************/
    aLong.SetRange( -0x80000000, 0x7FFFFFFF );
    aEnumLong.SetRange( -0x80000000, 0x7FFFFFFF );

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
    aIdLong.SetRange( -0x80000000, 0x7FFFFFFF );
}
{
    /********** W I N B I T S F L A G ************************************/
    // Variablenname fuer WinBits
    nWinBitVarId = aNmTb.Put( "_WinBits", VARNAME );

    // Windows
    nBorderId       = pHS->Insert( "WB_BORDER" );
    aWinBits.SetConstant( nBorderId, WB_BORDER );
    nHideId         = pHS->Insert( "WB_HIDE" );
    aWinBits.SetConstant( nHideId, WB_HIDE );
    nClipChildrenId = pHS->Insert( "WB_CLIPCHILDREN" );
    aWinBits.SetConstant( nClipChildrenId, WB_CLIPCHILDREN );
    nSizeableId     = pHS->Insert( "WB_SIZEABLE" );
    aWinBits.SetConstant( nSizeableId, WB_SIZEABLE );
    nMoveableId     = pHS->Insert( "WB_MOVEABLE" );
    aWinBits.SetConstant( nMoveableId, WB_MOVEABLE );
    nMinimizeId     = pHS->Insert( "WB_MINABLE" );
    aWinBits.SetConstant( nMinimizeId, WB_MINABLE );
    nMaximizeId     = pHS->Insert( "WB_MAXABLE" );
    aWinBits.SetConstant( nMaximizeId, WB_MAXABLE );
    nCloseableId    = pHS->Insert( "WB_CLOSEABLE" );
    aWinBits.SetConstant( nCloseableId, WB_CLOSEABLE );
    nAppId          = pHS->Insert( "WB_APP" );
    aWinBits.SetConstant( nAppId, WB_APP );
    nTabstopId      = pHS->Insert( "WB_TABSTOP" );
    aWinBits.SetConstant( nTabstopId, WB_TABSTOP );
    nGroupId        = pHS->Insert( "WB_GROUP" );
    aWinBits.SetConstant( nGroupId, WB_GROUP );
    nSysmodalId     = pHS->Insert( "WB_SYSMODAL" );
    aWinBits.SetConstant( nSysmodalId, WB_SYSMODAL );
}
{
    nLeftId         = pHS->Insert( "WB_LEFT" );
    aWinBits.SetConstant( nLeftId, WB_LEFT );
    nCenterId       = pHS->Insert( "WB_CENTER" );
    aWinBits.SetConstant( nCenterId, WB_CENTER );
    nRightId        = pHS->Insert( "WB_RIGHT" );
    aWinBits.SetConstant( nRightId, WB_RIGHT );
    nHscrollId      = pHS->Insert( "WB_HSCROLL" );
    aWinBits.SetConstant( nHscrollId, WB_HSCROLL );
    nVscrollId      = pHS->Insert( "WB_VSCROLL" );
    aWinBits.SetConstant( nVscrollId, WB_VSCROLL );
    nSortId         = pHS->Insert( "WB_SORT" );
    aWinBits.SetConstant( nSortId, WB_SORT );
    nDefaultId          = pHS->Insert( "WB_DEFBUTTON" );
    aWinBits.SetConstant( nDefaultId, WB_DEFBUTTON );
    nRepeatId           = pHS->Insert( "WB_REPEAT" );
    aWinBits.SetConstant( nRepeatId, WB_REPEAT );
    nSVLookId           = pHS->Insert( "WB_SVLOOK" );
    aWinBits.SetConstant( nSVLookId, WB_SVLOOK );
    nDropDownId         = pHS->Insert( "WB_DROPDOWN" );
    aWinBits.SetConstant( nDropDownId, WB_DROPDOWN );
    nPassWordId         = pHS->Insert( "WB_PASSWORD" );
    aWinBits.SetConstant( nPassWordId, WB_PASSWORD );
    nReadOnlyId         = pHS->Insert( "WB_READONLY" );
    aWinBits.SetConstant( nReadOnlyId, WB_READONLY );
    nAutoSizeId         = pHS->Insert( "WB_AUTOSIZE" );
    aWinBits.SetConstant( nAutoSizeId, WB_AUTOSIZE );
    nSpinId             = pHS->Insert( "WB_SPIN" );
    aWinBits.SetConstant( nSpinId, WB_SPIN );
    nTabControlId       = pHS->Insert( "WB_DIALOGCONTROL" );
    aWinBits.SetConstant( nTabControlId, WB_DIALOGCONTROL );
    nSimpleModeId       = pHS->Insert( "WB_SIMPLEMODE" );
    aWinBits.SetConstant( nSimpleModeId, WB_SIMPLEMODE );
    nDragId             = pHS->Insert( "WB_DRAG" );
    aWinBits.SetConstant( nDragId, WB_DRAG );
    nSaveAsId           = pHS->Insert( "WB_SAVEAS" );
    aWinBits.SetConstant( nSaveAsId, WB_SAVEAS );
    nOpenId             = pHS->Insert( "WB_OPEN" );
    aWinBits.SetConstant( nOpenId, WB_OPEN );
    nScrollId           = pHS->Insert( "WB_SCROLL" );
    aWinBits.SetConstant( nScrollId, WB_SCROLL );
    nZoomableId         = pHS->Insert( "WB_ZOOMABLE" );
    aWinBits.SetConstant( nZoomableId, WB_ROLLABLE );
    nHideWhenDeactivateId = pHS->Insert( "WB_HIDEWHENDEACTIVATE" );
    aWinBits.SetConstant( nHideWhenDeactivateId, WB_HIDEWHENDEACTIVATE );
    nAutoHScrollId      = pHS->Insert( "WB_AUTOHSCROLL" );
    aWinBits.SetConstant( nAutoHScrollId, WB_AUTOHSCROLL );
    nDDExtraWidthId     = pHS->Insert( "WB_DDEXTRAWIDTH" );
    aWinBits.SetConstant( nDDExtraWidthId, WB_DDEXTRAWIDTH );
    nWordBreakId        = pHS->Insert( "WB_WORDBREAK" );
    aWinBits.SetConstant( nWordBreakId, WB_WORDBREAK );
    nLeftLabelId        = pHS->Insert( "WB_LEFTLABEL" );
    aWinBits.SetConstant( nLeftLabelId, WB_LEFTLABEL );
    nHasLinesId         = pHS->Insert( "WB_HASLINES" );
    aWinBits.SetConstant( nHasLinesId, WB_HASLINES );
    nHasButtonsId       = pHS->Insert( "WB_HASBUTTONS" );
    aWinBits.SetConstant( nHasButtonsId, WB_HASBUTTONS );
    nRectStyleId        = pHS->Insert( "WB_RECTSTYLE" );
    aWinBits.SetConstant( nRectStyleId, WB_RECTSTYLE );
    nLineSpacingId      = pHS->Insert( "WB_LINESPACING" );
    aWinBits.SetConstant( nLineSpacingId, WB_LINESPACING );
    nSmallStyleId       = pHS->Insert( "WB_SMALLSTYLE" );
    aWinBits.SetConstant( nSmallStyleId, WB_SMALLSTYLE );
    nEnableResizingId   = pHS->Insert( "WB_ENABLERESIZING" );
    aWinBits.SetConstant( nEnableResizingId, WB_ENABLERESIZING );
    nDockableId         = pHS->Insert( "WB_DOCKABLE" );
    aWinBits.SetConstant( nDockableId, WB_DOCKABLE );
    nScaleId            = pHS->Insert( "WB_SCALE" );
    aWinBits.SetConstant( nScaleId, WB_SCALE );
    nIgnoreTabId        = pHS->Insert( "WB_IGNORETAB" );
    aWinBits.SetConstant( nIgnoreTabId, WB_IGNORETAB );
    nNoSplitDrawId      = pHS->Insert( "WB_NOSPLITDRAW" );
    aWinBits.SetConstant( nNoSplitDrawId, WB_NOSPLITDRAW );
    nTopImageId         = pHS->Insert( "WB_TOPIMAGE" );
    aWinBits.SetConstant( nTopImageId, WB_TOPIMAGE );
    nNoLabelId          = pHS->Insert( "WB_NOLABEL" );
    aWinBits.SetConstant( nNoLabelId, WB_NOLABEL );
}
{
    /********** I n i t   B a s i c   T y p e s **************************/
    InitLangType();
    aBaseLst.Insert( pDateFormat = InitDateFormatType(), LIST_APPEND );
    aBaseLst.Insert( pTimeFormat = InitTimeFormatType(), LIST_APPEND );
    aBaseLst.Insert( pWeekDayFormat = InitWeekDayFormatType(), LIST_APPEND );
    aBaseLst.Insert( pMonthFormat = InitMonthFormatType(), LIST_APPEND );
    aBaseLst.Insert( pFieldUnits = InitFieldUnitsType(), LIST_APPEND );
    aBaseLst.Insert( pDayOfWeek = InitDayOfWeekType(), LIST_APPEND );
    aBaseLst.Insert( pTimeFieldFormat = InitTimeFieldFormat(), LIST_APPEND );
    aBaseLst.Insert( pColor   = InitColor(), LIST_APPEND             );
    aBaseLst.Insert( pMapUnit       = InitMapUnit(),       LIST_APPEND );
    aBaseLst.Insert( pKey           = InitKey(),           LIST_APPEND );
    aBaseLst.Insert( pTriState      = InitTriState(),      LIST_APPEND );
    aBaseLst.Insert( pMessButtons   = InitMessButtons(),   LIST_APPEND );
    aBaseLst.Insert( pMessDefButton = InitMessDefButton(), LIST_APPEND );

    aBaseLst.Insert( pGeometry      = InitGeometry(),           LIST_APPEND );
    aBaseLst.Insert( pLangGeometry = InitLangGeometry( pGeometry ),
                     LIST_APPEND );
    aBaseLst.Insert( pStringList = InitStringList(), LIST_APPEND );
    aBaseLst.Insert( pLangStringList = InitLangStringList( pStringList ),
                     LIST_APPEND );
    aBaseLst.Insert( pStringTupel = InitStringTupel(), LIST_APPEND );
    aBaseLst.Insert( pStringTupelList = InitStringTupelList( pStringTupel ),
                     LIST_APPEND );
    aBaseLst.Insert( pLangStringTupelList =
                  InitLangStringTupelList( pStringTupelList ), LIST_APPEND );
    aBaseLst.Insert( pStringLongTupel = InitStringLongTupel(), LIST_APPEND );
    aBaseLst.Insert( pStringLongTupelList = InitStringLongTupelList( pStringLongTupel ),
                     LIST_APPEND );
    aBaseLst.Insert( pLangStringLongTupelList =
                  InitLangStringLongTupelList( pStringLongTupelList ), LIST_APPEND );
}
{
    /********** R E S O U R C E   T Y P E N ******************************/
    /********** R S C M G R **********************************************/
    pRoot = pClassMgr = InitClassMgr();

    /********** V e r s i o n s k o n t r o l l e ************************/
    aVersion.pClass = new RscClass( pHS->Insert( "VersionControl" ),
                                    RSC_VERSIONCONTROL, pClassMgr );
    aVersion = aVersion.pClass->Create( NULL, RSCINST() );

    /********** S T R I N G **********************************************/
    pClassString = InitClassString( pClassMgr );
    pRoot->Insert( pClassString );

    // String als Referenzklasse des Basisstrings einsetzen
    aString.SetRefClass( pClassString );

    /********** S T R I N G L I S T **************************************/
    // Klasse anlegen
    nId = pHS->Insert( "StringArray" );
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
    pClassImageList = InitClassImageList( pClassMgr, pClassBitmap, pClassColor );
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
    pClassDialog = new RscClass( pHS->Insert( "Dialog" ),
                                 RSC_DIALOG, pClassSystemWindow );
    pClassDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aBaseLst.Insert( pClassDialog, LIST_APPEND );

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
    nId = pHS->Insert( "Button" );
    pClassButton = new RscClass( nId, RSC_BUTTON, pClassControl );
    pClassButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassButton );
    pRoot->Insert( pClassButton );

    /********** C H E C K B O X ******************************************/
    // Klasse anlegen
    nId = pHS->Insert( "CheckBox" );
    pClassCheckBox = new RscClass( nId, RSC_CHECKBOX, pClassButton );
    pClassCheckBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassCheckBox );
    pRoot->Insert( pClassCheckBox );

    // Variablen anlegen
    nId = aNmTb.Put( "Check", VARNAME );
    pClassCheckBox->SetVariable( nId, &aBool );

    /********** P U S H B U T T O N **************************************/
    // Klasse anlegen
    pClassPushButton = InitClassPushButton( pClassButton );
    pRoot->Insert( pClassPushButton );

    /********** H E L P B U T T O N **************************************/
    // Klasse anlegen
    nId = pHS->Insert( "HelpButton" );
    pClassHelpButton = new RscClass( nId, RSC_HELPBUTTON,
                                        pClassPushButton );
    pClassHelpButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassHelpButton );
    pRoot->Insert( pClassHelpButton );

    /********** O K B U T T O N ******************************************/
    // Klasse anlegen
    nId = pHS->Insert( "OKButton" );
    pClassOKButton = new RscClass( nId, RSC_OKBUTTON,
                                        pClassPushButton );
    pClassOKButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassOKButton );
    pRoot->Insert( pClassOKButton );

    /********** C A N C E L B U T T O N **********************************/
    // Klasse anlegen
    nId = pHS->Insert( "CancelButton" );
    pClassCancelButton = new RscClass( nId, RSC_CANCELBUTTON,
                                        pClassPushButton );
    pClassCancelButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassCancelButton );
    pRoot->Insert( pClassCancelButton );
}
{
    /********** R A D I O B U T T O N ************************************/
    // Klasse anlegen
    nId = pHS->Insert( "RadioButton" );
    pClassRadioButton = new RscClass( nId, RSC_RADIOBUTTON, pClassButton );
    pClassRadioButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassRadioButton );
    pRoot->Insert( pClassRadioButton );

    // Variablen anlegen
    nId = aNmTb.Put( "Check", VARNAME );
    pClassRadioButton->SetVariable( nId, &aBool );

    /********** I m a g e R a d i o B u t t o n **************************/
    nId = pHS->Insert( "ImageRadioButton" );
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
    nId = pHS->Insert( "GroupBox" );
    pClassGroupBox = new RscClass( nId, RSC_GROUPBOX, pClassControl );
    pClassGroupBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassGroupBox );
    pRoot->Insert( pClassGroupBox );

    /********** K E Y C O D E ********************************************/
    pClassKeyCode = InitClassKeyCode( pClassMgr, pKey );
    pRoot->Insert( pClassKeyCode );
    {
    pLangClassKeyCode = new RscClassArray( pHS->Insert( "LangKeyCode" ),
                    RSC_KEYCODE, pClassKeyCode, &aLangType,
                    &nLangTypeId, &nDfltLangTypeId );
    aBaseLst.Insert( pLangClassKeyCode );
    }

    /********** A C C E L I T E M  ***************************************/
    pClassAccelItem = InitClassAccelItem( pClassMgr, pLangClassKeyCode );
    pRoot->Insert( pClassAccelItem );
}
{
    /********** A C C E L E R A T O R ************************************/
    pClassAccel = InitClassAccel( pClassMgr, pClassAccelItem );
    pRoot->Insert( pClassAccel );

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
    nId = pHS->Insert( "InfoBox" );
    pClassInfoBox = new RscClass( nId, RSC_INFOBOX, pClassMessBox );
    pClassInfoBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassInfoBox );
    pRoot->Insert( pClassInfoBox );

    /********** W A R N I N G B O X **************************************/
    // Klasse anlegen
    nId = pHS->Insert( "WarningBox" );
    pClassWarningBox = new RscClass( nId, RSC_WARNINGBOX, pClassMessBox );
    pClassWarningBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassWarningBox );
    pRoot->Insert( pClassWarningBox );

    /********** E R R O R B O X ******************************************/
    // Klasse anlegen
    nId = pHS->Insert( "ErrorBox" );
    pClassErrorBox = new RscClass( nId, RSC_ERRORBOX, pClassMessBox );
    pClassErrorBox->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassErrorBox );
    pRoot->Insert( pClassErrorBox );

    /********** Q U E R Y B O X ******************************************/
    // Klasse anlegen
    nId = pHS->Insert( "QueryBox" );
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
    nId = pHS->Insert( "SpinButton" );
    pClassSpinButton = new RscClass( nId, RSC_SPINBUTTON, pClassControl );
    pClassSpinButton->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassSpinButton );
    {
        RscClient * pClient;

        // Clientvariablen einfuegen
        // Sysmodal
        aBaseLst.Insert(
            pClient = new RscClient( pHS->Insert( "BOOL" ), RSC_NOTYPE,
                                     &aWinBits, nRepeatId ),
            LIST_APPEND );
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
    pClassDate = InitClassDate( pClassMgr, pDayOfWeek );
    pRoot->Insert( pClassDate );
}
{
    /********** I N T E R N A T I O N A L ********************************/
    {
    RscTop * pClassInt1;

    pClassInt1 = InitClassInt1( pClassMgr,
                                pDateFormat, pTimeFormat,
                                pWeekDayFormat, pMonthFormat );
    aBaseLst.Insert( pClassInt1, LIST_APPEND );
    pClassI12 = InitClassInternational( pClassInt1,
                                        pDateFormat, pTimeFormat,
                                        pWeekDayFormat, pMonthFormat );
    pRoot->Insert( pClassI12 );
    }
    {
    pLangClassI12 = new RscClassArray( pHS->Insert( "LangInternational" ),
                    RSC_INTERNATIONAL, pClassI12, &aLangType,
                    &nLangTypeId, &nDfltLangTypeId );
    aBaseLst.Insert( pLangClassI12 );
    }

    /********** S P I N F I E L D ****************************************/
    pClassSpinField = InitClassSpinField( pClassEdit );
    pRoot->Insert( pClassSpinField );
}
{
    /********** P A T T E R N F I E L D **********************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassPatternFormatter( pClassSpinField );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassPatternField = InitClassPatternField( pClassTmp );
    pRoot->Insert( pClassPatternField );
    }
    /********** N U M E R I C F I E L D **********************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassSpinField,
                                                    pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassNumericField = InitClassNumericField( pClassTmp );
    pRoot->Insert( pClassNumericField );
    }
    /********** M E T R I C F I E L D ************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassSpinField,
                                                    pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );
    pClassTmp = InitClassMetricFormatter( pClassTmp, pFieldUnits );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassMetricField = InitClassMetricField( pClassTmp );
    pRoot->Insert( pClassMetricField );
    }
    /********** C U R R E N C Y F I E L D ********************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassSpinField,
                                                    pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );
    pClassTmp = InitClassCurrencyFormatter( pClassTmp, pFieldUnits );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassCurrencyField = InitClassCurrencyField( "CurrencyField", RSC_CURRENCYFIELD, pClassTmp );
    pRoot->Insert( pClassCurrencyField );

    pClassLongCurrencyField = InitClassCurrencyField( "LongCurrencyField", RSC_LONGCURRENCYFIELD, pClassTmp );
    pRoot->Insert( pClassLongCurrencyField );

    }
    /********** D A T E F I E L D ****************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassDateFormatter( pClassSpinField, pClassDate,
                                                     pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassDateField = InitClassDateField( pClassTmp, pClassDate );
    pRoot->Insert( pClassDateField );
    }
    /********** T I M E F I E L D ****************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassTimeFormatter( pClassSpinField, pClassTime,
                                                 pClassI12, pTimeFieldFormat );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassTimeField = InitClassTimeField( pClassTmp, pClassTime );
    pRoot->Insert( pClassTimeField );
    }
    /********** P A T T E R N B O X **************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassPatternFormatter( pClassComboBox );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassPatternBox = InitClassPatternBox( pClassTmp );
    pRoot->Insert( pClassPatternBox );
    }
    /********** N U M E R I C B O X **************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassComboBox,
                                                    pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassNumericBox = InitClassNumericBox( pClassTmp );
    pRoot->Insert( pClassNumericBox );
    }
}
{
    /********** M E T R I C B O X ****************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassComboBox,
                                                    pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );
    pClassTmp = InitClassMetricFormatter( pClassTmp, pFieldUnits );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassMetricBox = InitClassMetricBox( pClassTmp );
    pRoot->Insert( pClassMetricBox );
    }
    /********** C U R R E N C Y B O X ************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassNumericFormatter( pClassComboBox,
                                                    pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );
    pClassTmp = InitClassCurrencyFormatter( pClassTmp, pFieldUnits );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassCurrencyBox = InitClassCurrencyBox( "CurrencyBox", RSC_CURRENCYBOX, pClassTmp );
    pRoot->Insert( pClassCurrencyBox );

    pClassLongCurrencyBox = InitClassCurrencyBox( "LongCurrencyBox", RSC_LONGCURRENCYBOX, pClassTmp );
    pRoot->Insert( pClassLongCurrencyBox );
    }
    /********** D A T E B O X ********************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassDateFormatter( pClassComboBox, pClassDate,
                                                 pClassI12 );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassDateBox = InitClassDateBox( pClassTmp, pClassDate );
    pRoot->Insert( pClassDateBox );
    }
    /********** T I M E B O X ********************************************/
    { // Mehrfachvererbung von Hand
    RscTop * pClassTmp = InitClassTimeFormatter( pClassComboBox, pClassTime,
                                                 pClassI12, pTimeFieldFormat );
    aBaseLst.Insert( pClassTmp, LIST_APPEND );

    pClassTimeBox = InitClassTimeBox( pClassTmp, pClassTime );
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
    nId = pHS->Insert( "TabPage" );
    pClassTabPage =
          new RscClass( nId, RSC_TABPAGE, pClassWindow );
    pClassTabPage->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassTabPage );
    pRoot->Insert( pClassTabPage );

    /********** T A B D I A L O G ****************************************/
    // Klasse anlegen
    nId = pHS->Insert( "TabDialog" );
    pClassTabDialog =
          new RscClass( nId, RSC_TABDIALOG, pClassModalDialog );
    pClassTabDialog->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassTabDialog );
    pRoot->Insert( pClassTabDialog );

    /********** T A B C O N T R O L I T E M *******************************/
    pClassTabControlItem = InitClassTabControlItem( pClassMgr,
                                                    pClassTabPage );
    pRoot->Insert( pClassTabControlItem );

    /********** T A B C O N T R O L **************************************/
    pClassTabControl = InitClassTabControl( pClassControl,
                                            pClassTabControlItem );
    pRoot->Insert( pClassTabControl );

    /********** F I X E D L I N E ****************************************/
    // Klasse anlegen
    nId = pHS->Insert( "FixedLine" );
    pClassFixedLine =
          new RscClass( nId, RSC_FIXEDLINE, pClassControl );
    pClassFixedLine->SetCallPar( *pWinPar1, *pWinPar2, *pWinParType );
    aNmTb.Put( nId, CLASSNAME, pClassFixedLine );
    pRoot->Insert( pClassFixedLine );

    /********** S C R O L L B A R B O X **********************************/
    // Klasse anlegen
    nId = pHS->Insert( "ScrollBarBox" );
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
    NameToVerCtrl( aVersion, pRoot, pClassString );

    aNmTb.SetSort();
}

