/*************************************************************************
 *
 *  $RCSfile: rcontrol.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:48:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to work inside and outside the StarOffice environment.
 *    Only adaption of file commtypes.hxx should be necessary. Else it is a bug!
 *
 ************************************************************************/

#ifndef _RCONTROL_HXX
#define _RCONTROL_HXX


#define UID_ACTIVE          0


#define SI_IPCCommandBlock  1
#define SI_SocketCommandBlock   SI_IPCCommandBlock  // Zumindest erstmal
#define SI_DirectCommandBlock   2
#define SIControl           3
#define SISlot              4
#define SIFlow              5
#define SICommand           6
#define SIUnoSlot           7
#define SIStringControl     8

#define SIReturnBlock       11
#define SIReturn            12
#define SIReturnError       13

// Typisierung im Stream
#define BinUSHORT           11
#define BinULONG            14
#define BinString           12
#define BinBool             13
#define BinSbxValue         15


// Classes
// !!!Diese Defines duerfen niemals geaendert werden!!!
#define C_NoType            -1
// Maximale 32 einfache Controls
#define C_TabControl        0
#define C_RadioButton       1
#define C_CheckBox          2
#define C_TriStateBox       3
#define C_Edit              4
#define C_MultiLineEdit     5
#define C_MultiListBox      6
#define C_ListBox           7
#define C_ComboBox          8
#define C_PushButton        9

#define C_SpinField         10
#define C_PatternField      11
#define C_NumericField      12
#define C_MetricField       13
#define C_CurrencyField     14
#define C_DateField         15
#define C_TimeField         16

#define C_ImageRadioButton  17
#define C_NumericBox        18
#define C_MetricBox         19
#define C_CurrencyBox       20
#define C_DateBox           21
#define C_TimeBox           22

#define C_ImageButton       23
#define C_MenuButton        24
#define C_MoreButton        25

// Maximale 7 Container
#define C_TabPage           32
#define C_Dlg               33
#define C_FloatWin          34
#define C_ModelessDlg       35
#define C_WorkWin           36
#define C_DockingWin        37

// Diese Defines koennen geaendert werden
#define C_MessBox           40
#define C_InfoBox           41
#define C_WarningBox        42
#define C_ErrorBox          43
#define C_QueryBox          44

#define C_TabDlg            45
#define C_SingleTabDlg      46

#define C_Window            47


#define C_PatternBox        60
#define C_ToolBox           61
#define C_ValueSet          62
#define C_Control           63
#define C_TreeListBox       64  // Hurray the TreeListBox finally got its own Window Type

#define C_OkButton          65
#define C_CancelButton      66
#define C_ButtonDialog      67


// Some new Elements for Mozilla
#define C_Anchor            68
#define C_FormElement       69
#define C_DOMText           70
#define C_Image             71



#define M_WITH_RETURN       0x0200  // Die Variable wird zum Aufnehmen des Wertes gespeichert
#define M_KEY_STRING        0x0400  // Key Befehle werden umgewandelt i.e. "<return><up>"
#define M_SOFFICE           0x0800  // Command valid for Star/Open Office
#define M_MOZILLA           0x1000  // Command valid for Mozilla
// for MacroRecorder
#define M_RET_NUM_CONTROL   0x2000  // decode ULong as Control (For Tabpages, Toolboxes, ... )

// Methoden
#define M_Select            21
#define M_SetNoSelection    22
#define M_SetText           23
#define M_More              24
#define M_Less              25
#define M_ToMin             26
#define M_ToMax             27
#define M_Check             28
#define M_UnCheck           29
#define M_TriState          30
#define M_SetPage           31
#define M_Click             32

#define M_Close             33      // Push Buttons on Dialog (Auch More Button)
#define M_Cancel            34
#define M_OK                35
#define M_Help              36
#define M_Default           37      // Push defaultbutton on Dialog

#define M_Yes               38
#define M_No                39
#define M_Repeat            40

// Diese Defines fallen nach einiger Zeit weg!!
//#define M_Schliessen        M_Close
//#define M_Abbrechen         M_Cancel
//#define M_Hilfe             M_Help

//#define M_Ja              M_Yes
//#define M_Nein                M_No
//#define M_Wiederholen     M_Repeat
// Ende der wegfallenden Defines

#define M_Open              41
#define M_Pick              42
#define M_Move              43
#define M_Size              44
#define M_Minimize          45
#define M_Maximize          46
#define M_Dock              47
#define M_Undock            48



#define M_TypeKeys          ( M_KEY_STRING | 50 )
#define M_MouseDown         51
#define M_MouseUp           52
#define M_MouseMove         53
#define M_MouseDoubleClick  54
#define M_SnapShot          55
#define M_SetNextToolBox    56
#define M_OpenContextMenu   57
#define M_MultiSelect       58

// Filedialog
#define M_SetPath           60
#define M_SetCurFilter      61

// Printdialog
#define M_SetPrinter        70
#define M_CheckRange        71
#define M_SetRangeText      72
#define M_SetFirstPage      73
#define M_SetLastPage       74
#define M_CheckCollate      75
#define M_SetPageId         76
#define M_SetPageNr         77

#define M_AnimateMouse      78
#define M_TearOff           79

#define M_FadeIn            80
#define M_FadeOut           81
#define M_Pin               82

#define M_UseMenu           83      // Use the menu of the next possible parent of given Window

#define M_OpenMenu          84      // MenuButtons and Menus in ToolBoxes

#define M_Restore           85      // Window Control together with M_Maximize and M_Minimize

#define M_DisplayPercent    200     // Zum Anzeigen der Prozente des Windows

#define M_LAST_NO_RETURN    200

#if ( M_LAST_NO_RETURN >= M_WITH_RETURN )
#error "Bereich überschritten"
#endif

#define M_Exists            ( M_WITH_RETURN |  1 )
#define M_NotExists         ( M_WITH_RETURN |  2 )
#define M_IsEnabled         ( M_WITH_RETURN |  3 )
#define M_IsVisible         ( M_WITH_RETURN |  4 )
#define M_IsWritable        ( M_WITH_RETURN |  5 )

#define M_GetPage           ( M_WITH_RETURN |  6 )
#define M_IsChecked         ( M_WITH_RETURN |  7 )
#define M_IsTristate        ( M_WITH_RETURN |  8 )
#define M_GetState          ( M_WITH_RETURN |  9 )
#define M_GetText           ( M_WITH_RETURN | 10 )
#define M_GetSelCount       ( M_WITH_RETURN | 11 )
#define M_GetSelIndex       ( M_WITH_RETURN | 12 )
#define M_GetSelText        ( M_WITH_RETURN | 13 )
#define M_GetItemCount      ( M_WITH_RETURN | 14 )
#define M_GetItemText       ( M_WITH_RETURN | 15 )
#define M_IsOpen            ( M_WITH_RETURN | 16 )
#define M_Caption           ( M_WITH_RETURN | 17 )
#define M_IsMax             ( M_WITH_RETURN | 18 )
#define M_IsDocked          ( M_WITH_RETURN | 19 )
#define M_GetRT             ( M_WITH_RETURN | 20 )
#define M_GetPageId         ( M_WITH_RETURN | 21 )
#define M_GetPageCount      ( M_WITH_RETURN | 22 )
#define M_GetPosX           ( M_WITH_RETURN | 23 )
#define M_GetPosY           ( M_WITH_RETURN | 24 )
#define M_GetSizeX          ( M_WITH_RETURN | 25 )
#define M_GetSizeY          ( M_WITH_RETURN | 26 )
#define M_GetNextToolBox    ( M_WITH_RETURN | 27 )
#define M_GetButtonCount    ( M_WITH_RETURN | 28 )
#define M_GetButtonId       ( M_WITH_RETURN | 29 )

#define M_IsFadeIn          ( M_WITH_RETURN | 30 )
#define M_IsPin             ( M_WITH_RETURN | 31 )

// Statusbar
#define M_StatusGetText     ( M_WITH_RETURN | 32 )
#define M_StatusIsProgress  ( M_WITH_RETURN | 33 )
#define M_StatusGetItemCount ( M_WITH_RETURN | 34 )
#define M_StatusGetItemId   ( M_WITH_RETURN | 35 )

//
#define M_GetMouseStyle     ( M_WITH_RETURN | 36 )

// support for Messagebox with checkbox
#define M_GetCheckBoxText   ( M_WITH_RETURN | 37 )

// Scrollbars
#define M_HasScrollBar      ( M_WITH_RETURN | 38 )
#define M_IsScrollBarEnabled ( M_WITH_RETURN | 39 )

// Diese befehle werden nur intern im Controller verwendet. Sie tauchen nicht im Testtool auf!
#define _M_IsEnabled        ( M_WITH_RETURN | 50 )


#define M_GetFixedTextCount ( M_WITH_RETURN | 51 )
#define M_GetFixedText      ( M_WITH_RETURN | 52 )


#define M_IsMin             ( M_WITH_RETURN | 53 )
#define M_IsRestore         ( M_WITH_RETURN | 54 )



//#define M_SOFFICE           0x0800  // Command valid for Star/Open Office
//#define M_MOZILLA           0x1000  // Command valid for Mozilla


// RemoteCommands
#define RC_AppAbort         ( M_SOFFICE | M_MOZILLA | 1 )
#define RC_SetClipboard     ( M_SOFFICE | M_MOZILLA | 2 )
#define RC_NoDebug          ( M_SOFFICE | M_MOZILLA | 3 )
#define RC_Debug            ( M_SOFFICE | M_MOZILLA | 4 )
#define RC_GPF              ( M_SOFFICE | M_MOZILLA | 5 )
#define RC_DisplayHid       ( M_SOFFICE | M_MOZILLA | 6 )
#define RC_AppDelay         ( M_SOFFICE | M_MOZILLA | 7 )
#define RC_UseBindings      ( M_SOFFICE             | 8 )
#define RC_Profile          ( M_SOFFICE | M_MOZILLA | 9 )
// (Popup)Menu
#define RC_MenuSelect       ( M_SOFFICE | M_MOZILLA | 10 )
#define RC_SetControlType   ( M_SOFFICE             | 11 )
// RemoteFileAccess
#define RC_Kill             ( M_SOFFICE             | 12 )
#define RC_RmDir            ( M_SOFFICE             | 13 )
#define RC_MkDir            ( M_SOFFICE             | 14 )
#define RC_FileCopy         ( M_SOFFICE             | 15 )
#define RC_Name             ( M_SOFFICE             | 16 )

#define RC_CaptureAssertions (M_SOFFICE | M_MOZILLA | 17 )
#define RC_Assert           ( M_SOFFICE | M_MOZILLA | 18 )

#define RC_MenuOpen         ( M_SOFFICE | M_MOZILLA | 19 )

#define RC_TypeKeysDelay    ( M_SOFFICE | M_MOZILLA | 20 )

#define RC_ShowBar          (             M_MOZILLA | 21 )

#define RC_LoadURL          (             M_MOZILLA | 22 )

#define RC_CloseSysDialog   ( M_SOFFICE             | 23 )

#define RC_SAXRelease       ( M_SOFFICE             | 24 )

#define RC_RecordMacro      ( M_SOFFICE             | 25 )

#define RC_ActivateDocument ( M_SOFFICE             | 26 )

#define RC_CatchGPF         ( M_SOFFICE             | 27 )

#define _RC_LAST_NO_RETURN                            27

#if ( _RC_LAST_NO_RETURN >= M_WITH_RETURN )
#error "Bereich überschritten"
#endif

// Befehle mit Returnwert
#define RC_GetClipboard     ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 1 )
#define RC_WinTree          ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 2 )
#define RC_ResetApplication ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 3 )
#define RC_GetNextCloseWindow ( M_SOFFICE             | M_WITH_RETURN | 4 )
#define RC_ApplicationBusy  ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 5 )
// (Popup)Menu
#define RC_MenuGetItemCount ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 6 )
#define RC_MenuGetItemId    ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 7 )
#define RC_MenuGetItemPos   ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 8 )
#define RC_MenuIsSeperator  ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 9 )
#define RC_MenuIsItemChecked ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 10 )
#define RC_MenuIsItemEnabled ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 11 )
#define RC_MenuGetItemText  ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 12 )
// RemoteFileAccess
#define RC_Dir              ( M_SOFFICE             | M_WITH_RETURN | 18 )
#define RC_FileLen          ( M_SOFFICE             | M_WITH_RETURN | 19 )
#define RC_FileDateTime     ( M_SOFFICE             | M_WITH_RETURN | 20 )

#define RC_Translate        ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 21 )
#define RC_GetMouseStyle    ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 22 )
#define RC_UnpackStorage    ( M_SOFFICE             | M_WITH_RETURN | 23 )

#define RC_IsBarVisible     (             M_MOZILLA | M_WITH_RETURN | 24 )

#define RC_MenuGetItemCommand ( M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 25 )

#define RC_ExistsSysDialog  ( M_SOFFICE             | M_WITH_RETURN | 26 )

#define RC_SAXCheckWellformed ( M_SOFFICE             | M_WITH_RETURN | 27 )
#define RC_SAXReadFile      ( M_SOFFICE             | M_WITH_RETURN | 28 )

#define RC_SAXGetNodeType   ( M_SOFFICE             | M_WITH_RETURN | 29 )
#define RC_SAXGetElementName ( M_SOFFICE             | M_WITH_RETURN | 30 )
#define RC_SAXGetChars      ( M_SOFFICE             | M_WITH_RETURN | 31 )
#define RC_SAXGetChildCount ( M_SOFFICE             | M_WITH_RETURN | 32 )
#define RC_SAXGetAttributeCount ( M_SOFFICE             | M_WITH_RETURN | 33 )
#define RC_SAXGetAttributeName ( M_SOFFICE             | M_WITH_RETURN | 34 )
#define RC_SAXGetAttributeValue ( M_SOFFICE             | M_WITH_RETURN | 35 )
#define RC_SAXSeekElement   ( M_SOFFICE             | M_WITH_RETURN | 36 )
#define RC_SAXHasElement    ( M_SOFFICE             | M_WITH_RETURN | 37 )
#define RC_SAXGetElementPath ( M_SOFFICE             | M_WITH_RETURN | 38 )

#define RC_GetDocumentCount ( M_SOFFICE             | M_WITH_RETURN | 39 )

#define RC_GetSystemLanguage ( M_SOFFICE             | M_WITH_RETURN | 40 )

// Flow Control
#define F_EndCommandBlock   101         // Initiiert Rückmeldung des Status
#define F_Sequence          102         // Übergibt Sequence Nummer (1. in jedem Stream)

// Return codes
#define RET_Sequence        132         // Übergibt Sequence Nummer (1. in jedem Stream)
#define RET_Value           133         // Übergibt Return-wert
#define RET_WinInfo         134         // Information über aktuelles Fenster/Control
#define RET_ProfileInfo     135         // Profile Information
#define RET_DirectLoging    136         // Direktes Übertragen von Informationen in das Log
#define RET_MacroRecorder   137         // MakroRecorder Befehl übertragen



// Subcodes die in nUId geliefert werden
// für F_ProfileInfo
#define S_ProfileReset      201         // nNr1 = Anzahl Borders
    // Achtung!! Diese Defines müssen aufeinanderfolgende Nummern haben!!
#define S_ProfileBorder1    202         // nNr1 = Border1 in ms
#define S_ProfileBorder2    203         // nNr1 = Border2 in ms
#define S_ProfileBorder3    204         // nNr1 = Border3 in ms
#define S_ProfileBorder4    205         // nNr1 = Border4 in ms
    // Achtung Ende
#define S_ProfileTime       210         // nNr1 = remote Zeit des Befehls
#define S_ProfileDump       211         // Gibt die daten aus.

// für F_DirectLoging
#define S_AssertError       220
#define S_AssertWarning     221
#define S_AssertTrace       222
#define S_QAError           223



// Konstanten die im Basic zur Verfügung stehen sollen

// Verschiedene Typen von Controls für den Befehl SetControlType
// !!!!!!!!!!!  Müssen alle bei SetControlType eingetragen werden  !!!!!!!!!!!!
#define CONST_CTBrowseBox       100
//#define CONST_CTTreeListBox       101
#define CONST_CTProgressBar     102     // Eigentlich ein Window, aber der Einheitlichen Namensgebung wegen
#define CONST_CTValueSet        103

// Konstanten für das ALignment des gesuchten Splitters
#define CONST_ALIGN_LEFT        120
#define CONST_ALIGN_TOP         121
#define CONST_ALIGN_RIGHT       122
#define CONST_ALIGN_BOTTOM      123

/// What dialog to use in RC_CloseSysDialog or RC_ExistsSysDialog
#define CONST_FilePicker        301
#define CONST_FolderPicker      302

/// NodeTypes of the SAX Parser
#define CONST_NodeTypeCharacter 555
#define CONST_NodeTypeElement   556
#define CONST_NodeTypeComment   557


// Beschreibt die Parametertypen als Bitfeld  Reihenfolge immer!
// wie hier Aufgelistet
#define PARAM_NONE              0x0000
#define PARAM_USHORT_1          0x0001
#define PARAM_USHORT_2          0x0002
#define PARAM_USHORT_3          0x0100      // Nicht in der Reihe!!
#define PARAM_USHORT_4          0x0200      // Nicht in der Reihe!!
#define PARAM_ULONG_1           0x0004
#define PARAM_ULONG_2           0x0008
#define PARAM_STR_1             0x0010
#define PARAM_STR_2             0x0020
#define PARAM_BOOL_1            0x0040
#define PARAM_BOOL_2            0x0080
#define PARAM_SBXVALUE_1        0x0400      // hier mit 0x0400 Weiter!!! Siehe Oben!

// Zusätzliche Beschreibung!! wird auch mit dem Rest verodert
//#define PARAM_STR_RAW           0x8000        // Der Zeichensatz der Strings wird nicht konvertiert(für Fareastern)


#define ERR_SEND_TIMEOUT        100
#define ERR_EXEC_TIMEOUT        101
#define ERR_RESTART_FAIL        102
#define ERR_RESTART             103
#define ERR_NO_WIN              104     // Keine *.Win Dateien gefunden
#define ERR_NO_SID              105     // Keine *.Sid Dateien gefunden
#define ERR_NO_FILE             106     // Datei nicht gefunden

#endif
