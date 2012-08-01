/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.test.vcl.client;

/**
 * Define all constant variables
 *
 */
public interface Constant {

    public final static int CH_NoHeader = 0x0000;

    public final static int CH_SimpleMultiChannel = 0x0001;

    public final static int CH_Handshake = 0x0002;

    public final static int CH_REQUEST_HandshakeAlive = 0x0101;

    public final static int CH_RESPONSE_HandshakeAlive = 0x0102;

    public final static int CH_REQUEST_ShutdownLink = 0x0104;

    public final static int CH_ShutdownLink = 0x0105;

    public final static int CH_SUPPORT_OPTIONS = 0x0103;

    public final static int CH_SetApplication = 0x0106;

    public final static int CM_PROTOCOL_OLDSTYLE = 0x0001;

    public final static int CM_PROTOCOL_MARS = 0x0001;

    public final static int CM_PROTOCOL_BROADCASTER = 0x0002;

    public final static int CM_PROTOCOL_USER_START = 0x0100;

    public static final char SIControl = 3;

    public static final char SISlot = 4;

    public static final char SIFlow = 5;

    public static final char SICommand = 6;

    public static final char SIUnoSlot = 7;

    public static final char SIStringControl = 8;

    public static final char SIReturnBlock = 11;

    public static final char SIReturn = 12;

    public static final char SIReturnError = 13;

    public static final char RET_Sequence = 132;

    public static final char RET_Value = 133;

    public static final char RET_WinInfo = 134;

    public static final char RET_ProfileInfo = 135;

    public static final char RET_DirectLoging = 136;

    public static final char RET_MacroRecorder = 137;

    public static final char BinUSHORT = 11;

    public static final char BinULONG = 14;

    public static final char BinString = 12;

    public static final char BinBool = 13;

    public static final char BinSbxValue = 15;

    public static final char PARAM_NONE = 0x0000;

    public static final char PARAM_USHORT_1 = 0x0001;

    public static final char PARAM_USHORT_2 = 0x0002;

    public static final char PARAM_USHORT_3 = 0x0100;

    public static final char PARAM_USHORT_4 = 0x0200;

    public static final char PARAM_ULONG_1 = 0x0004;

    public static final char PARAM_ULONG_2 = 0x0008;

    public static final char PARAM_STR_1 = 0x0010;

    public static final char PARAM_STR_2 = 0x0020;

    public static final char PARAM_BOOL_1 = 0x0040;

    public static final char PARAM_BOOL_2 = 0x0080;

    public static final char PARAM_SBXVALUE_1 = 0x0400;

    public static final char F_EndCommandBlock = 101;

    public static final char F_Sequence = 102;

    public static final char M_WITH_RETURN = 0x0200;

    public static final char M_KEY_STRING = 0x0400;

    public static final char M_SOFFICE = 0x0800;

    public static final char M_MOZILLA = 0x1000;

    // for MacroRecorder
    public static final char M_RET_NUM_CONTROL = 0x2000; // decode ULong as
    // Control (For
    // Tabpages,
    // Toolboxes, ... )

    public static final char M_Select = 21;

    public static final char M_SetNoSelection = 22;

    public static final char M_SetText = 23;

    public static final char M_More = 24;

    public static final char M_Less = 25;

    public static final char M_ToMin = 26;

    public static final char M_ToMax = 27;

    public static final char M_Check = 28;

    public static final char M_UnCheck = 29;

    public static final char M_TriState = 30;

    public static final char M_SetPage = 31;

    public static final char M_Click = 32;

    public static final char M_Close = 33; // Push Buttons on Dialog (Auch More
    // Button)

    public static final char M_Cancel = 34;

    public static final char M_OK = 35;

    public static final char M_Help = 36;

    public static final char M_Default = 37; // Push defaultbutton on Dialog

    public static final char M_Yes = 38;

    public static final char M_No = 39;

    public static final char M_Repeat = 40;

    public static final char M_Open = 41;

    public static final char M_Pick = 42;

    public static final char M_Move = 43;

    public static final char M_Size = 44;

    public static final char M_Minimize = 45;

    public static final char M_Maximize = 46;

    public static final char M_Dock = 47;

    public static final char M_Undock = 48;

    public static final char M_TypeKeys = (M_KEY_STRING | 50);

    public static final char M_MouseDown = 51;

    public static final char M_MouseUp = 52;

    public static final char M_MouseMove = 53;

    public static final char M_MouseDoubleClick = 54;

    public static final char M_SnapShot = 55;

    public static final char M_SetNextToolBox = 56;

    public static final char M_OpenContextMenu = 57;

    public static final char M_MultiSelect = 58;

    // Filedialog
    public static final char M_SetPath = 60;

    public static final char M_SetCurFilter = 61;

    // Printdialog
    public static final char M_SetPrinter = 70;

    public static final char M_CheckRange = 71;

    public static final char M_SetRangeText = 72;

    public static final char M_SetFirstPage = 73;

    public static final char M_SetLastPage = 74;

    public static final char M_CheckCollate = 75;

    public static final char M_SetPageId = 76;

    public static final char M_SetPageNr = 77;

    public static final char M_AnimateMouse = 78;

    public static final char M_TearOff = 79;

    public static final char M_FadeIn = 80;

    public static final char M_FadeOut = 81;

    public static final char M_Pin = 82;

    public static final char M_UseMenu = 83; // Use the menu of the next
    // possible parent of given
    // Window

    public static final char M_OpenMenu = 84; // MenuButtons and Menus in
    // ToolBoxes

    public static final char M_Restore = 85; // Window Control together with
    // M_Maximize and M_Minimize

    public static final char M_DisplayPercent = 200;

    public static final char M_LAST_NO_RETURN = 200;

    public static final char M_Exists = (M_WITH_RETURN | 1);

    public static final char M_NotExists = (M_WITH_RETURN | 2);

    public static final char M_IsEnabled = (M_WITH_RETURN | 3);

    public static final char M_IsVisible = (M_WITH_RETURN | 4);

    public static final char M_IsWritable = (M_WITH_RETURN | 5);

    public static final char M_GetPage = (M_WITH_RETURN | 6);

    public static final char M_IsChecked = (M_WITH_RETURN | 7);

    public static final char M_IsTristate = (M_WITH_RETURN | 8);

    public static final char M_GetState = (M_WITH_RETURN | 9);

    public static final char M_GetText = (M_WITH_RETURN | 10);

    public static final char M_GetSelCount = (M_WITH_RETURN | 11);

    public static final char M_GetSelIndex = (M_WITH_RETURN | 12);

    public static final char M_GetSelText = (M_WITH_RETURN | 13);

    public static final char M_GetItemCount = (M_WITH_RETURN | 14);

    public static final char M_GetItemText = (M_WITH_RETURN | 15);

    public static final char M_IsOpen = (M_WITH_RETURN | 16);

    public static final char M_Caption = (M_WITH_RETURN | 17);

    public static final char M_IsMax = (M_WITH_RETURN | 18);

    public static final char M_IsDocked = (M_WITH_RETURN | 19);

    public static final char M_GetRT = (M_WITH_RETURN | 20);

    public static final char M_GetPageId = (M_WITH_RETURN | 21);

    public static final char M_GetPageCount = (M_WITH_RETURN | 22);

    public static final char M_GetPosX = (M_WITH_RETURN | 23);

    public static final char M_GetPosY = (M_WITH_RETURN | 24);

    public static final char M_GetSizeX = (M_WITH_RETURN | 25);

    public static final char M_GetSizeY = (M_WITH_RETURN | 26);

    public static final char M_GetNextToolBox = (M_WITH_RETURN | 27);

    public static final char M_GetButtonCount = (M_WITH_RETURN | 28);

    public static final char M_GetButtonId = (M_WITH_RETURN | 29);

    public static final char M_IsFadeIn = (M_WITH_RETURN | 30);

    public static final char M_IsPin = (M_WITH_RETURN | 31);

    // Statusbar
    public static final char M_StatusGetText = (M_WITH_RETURN | 32);

    public static final char M_StatusIsProgress = (M_WITH_RETURN | 33);

    public static final char M_StatusGetItemCount = (M_WITH_RETURN | 34);

    public static final char M_StatusGetItemId = (M_WITH_RETURN | 35);

    //
    public static final char M_GetMouseStyle = (M_WITH_RETURN | 36);

    // support for Messagebox with checkbox
    public static final char M_GetCheckBoxText = (M_WITH_RETURN | 37);

    // Scrollbars
    public static final char M_HasScrollBar = (M_WITH_RETURN | 38);

    public static final char M_IsScrollBarEnabled = (M_WITH_RETURN | 39);

    // Dieser befehl wird nur intern im Controller (sts library) verwendet. Sie
    // tauchen nicht im Testtool auf!
    public static final char _M_IsEnabled = (M_WITH_RETURN | 50);

    public static final char M_GetFixedTextCount = (M_WITH_RETURN | 51);

    public static final char M_GetFixedText = (M_WITH_RETURN | 52);

    public static final char M_IsMin = (M_WITH_RETURN | 53);

    public static final char M_IsRestore = (M_WITH_RETURN | 54);

    public static final char M_GetItemType = (M_WITH_RETURN | 55);

    // Commands for (Edit)BrowseBox
    public static final char M_GetColumnCount = (M_WITH_RETURN | 56);

    public static final char M_GetRowCount = (M_WITH_RETURN | 57);

    public static final char M_IsEditing = (M_WITH_RETURN | 58);

    public static final char M_IsItemEnabled = (M_WITH_RETURN | 59);

    // For TreeListBox
    public static final char M_Collapse = (M_WITH_RETURN | 60);
    public static final char M_Expand = (M_WITH_RETURN | 61);

    // Symphony Special
    public static final char M_GetHelpText = (M_WITH_RETURN | 90);
    public static final char M_GetQuickHelpText = (M_WITH_RETURN | 91);
    public static final char M_GetScreenRectangle = (M_WITH_RETURN | 92);
    public static final char M_HasFocus = (M_WITH_RETURN | 93);
    public static final char M_GetItemHelpText = (M_WITH_RETURN | 94);
    public static final char M_GetItemQuickHelpText = (M_WITH_RETURN | 95);
    public static final char M_GetItemText2 = (M_WITH_RETURN | 96);
    // Symphony Special End

    public static final char RC_AppAbort = (M_SOFFICE | M_MOZILLA | 1);

    public static final char RC_SetClipboard = (M_SOFFICE | M_MOZILLA | 2);

    public static final char RC_NoDebug = (M_SOFFICE | M_MOZILLA | 3);

    public static final char RC_Debug = (M_SOFFICE | M_MOZILLA | 4);

    public static final char RC_GPF = (M_SOFFICE | M_MOZILLA | 5);

    public static final char RC_DisplayHid = (M_SOFFICE | M_MOZILLA | 6);

    public static final char RC_AppDelay = (M_SOFFICE | M_MOZILLA | 7);

    public static final char RC_UseBindings = (M_SOFFICE | 8);

    public static final char RC_Profile = (M_SOFFICE | M_MOZILLA | 9);

    // =(Popup);Menu
    public static final char RC_MenuSelect = (M_SOFFICE | M_MOZILLA | 10);

    public static final char RC_SetControlType = (M_SOFFICE | 11);

    // RemoteFileAccess
    public static final char RC_Kill = (M_SOFFICE | 12);

    public static final char RC_RmDir = (M_SOFFICE | 13);

    public static final char RC_MkDir = (M_SOFFICE | 14);

    public static final char RC_FileCopy = (M_SOFFICE | 15);

    public static final char RC_Name = (M_SOFFICE | 16);

    public static final char RC_CaptureAssertions = (M_SOFFICE | M_MOZILLA | 17);

    public static final char RC_Assert = (M_SOFFICE | M_MOZILLA | 18);

    public static final char RC_MenuOpen = (M_SOFFICE | M_MOZILLA | 19);

    public static final char RC_TypeKeysDelay = (M_SOFFICE | M_MOZILLA | 20);

    public static final char RC_ShowBar = (M_MOZILLA | 21);

    public static final char RC_LoadURL = (M_MOZILLA | 22);

    public static final char RC_CloseSysDialog = (M_SOFFICE | 23);

    public static final char RC_SAXRelease = (M_SOFFICE | 24);

    public static final char RC_RecordMacro = (M_SOFFICE | 25);

    public static final char RC_ActivateDocument = (M_SOFFICE | 26);

    public static final char RC_CatchGPF = (M_SOFFICE | 27);

    // Befehle mit Returnwert
    public static final char RC_GetClipboard = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 1);

    public static final char RC_WinTree = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 2);

    public static final char RC_ResetApplication = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 3);

    public static final char RC_GetNextCloseWindow = (M_SOFFICE | M_WITH_RETURN | 4);

    public static final char RC_ApplicationBusy = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 5);

    // =(Popup);Menu
    public static final char RC_MenuGetItemCount = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 6);

    public static final char RC_MenuGetItemId = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 7);

    public static final char RC_MenuGetItemPos = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 8);

    public static final char RC_MenuIsSeperator = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 9);

    public static final char RC_MenuIsItemChecked = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 10);

    public static final char RC_MenuIsItemEnabled = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 11);

    public static final char RC_MenuGetItemText = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 12);

    // RemoteFileAccess
    public static final char RC_Dir = (M_SOFFICE | M_WITH_RETURN | 18);

    public static final char RC_FileLen = (M_SOFFICE | M_WITH_RETURN | 19);

    public static final char RC_FileDateTime = (M_SOFFICE | M_WITH_RETURN | 20);

    public static final char RC_Translate = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 21);

    public static final char RC_GetMouseStyle = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 22);

    public static final char RC_UnpackStorage = (M_SOFFICE | M_WITH_RETURN | 23);

    public static final char RC_IsBarVisible = (M_MOZILLA | M_WITH_RETURN | 24);

    public static final char RC_MenuGetItemCommand = (M_SOFFICE | M_MOZILLA | M_WITH_RETURN | 25);

    public static final char RC_ExistsSysDialog = (M_SOFFICE | M_WITH_RETURN | 26);

    public static final char RC_SAXCheckWellformed = (M_SOFFICE | M_WITH_RETURN | 27);

    public static final char RC_SAXReadFile = (M_SOFFICE | M_WITH_RETURN | 28);

    public static final char RC_SAXGetNodeType = (M_SOFFICE | M_WITH_RETURN | 29);

    public static final char RC_SAXGetElementName = (M_SOFFICE | M_WITH_RETURN | 30);

    public static final char RC_SAXGetChars = (M_SOFFICE | M_WITH_RETURN | 31);

    public static final char RC_SAXGetChildCount = (M_SOFFICE | M_WITH_RETURN | 32);

    public static final char RC_SAXGetAttributeCount = (M_SOFFICE | M_WITH_RETURN | 33);

    public static final char RC_SAXGetAttributeName = (M_SOFFICE | M_WITH_RETURN | 34);

    public static final char RC_SAXGetAttributeValue = (M_SOFFICE | M_WITH_RETURN | 35);

    public static final char RC_SAXSeekElement = (M_SOFFICE | M_WITH_RETURN | 36);

    public static final char RC_SAXHasElement = (M_SOFFICE | M_WITH_RETURN | 37);

    public static final char RC_SAXGetElementPath = (M_SOFFICE | M_WITH_RETURN | 38);

    public static final char RC_GetDocumentCount = (M_SOFFICE | M_WITH_RETURN | 39);

    public static final char RC_GetSystemLanguage = (M_SOFFICE | M_WITH_RETURN | 40);

    public static final char RC_IsProduct = (M_SOFFICE | M_WITH_RETURN | 41);

    public static final char RC_MenuHasSubMenu = (M_SOFFICE | M_WITH_RETURN | 42);

    public static final char RC_UsePostEvents = (M_SOFFICE | M_WITH_RETURN | 43);

    public static final char RC_WaitSlot = (M_SOFFICE | M_WITH_RETURN | 44);


    public final static int WINDOW_BASE = 0x0100;

    public final static int WINDOW_FIRST = (WINDOW_BASE + 0x30);

    public final static int WINDOW_MESSBOX = (WINDOW_FIRST);

    public final static int WINDOW_INFOBOX = (WINDOW_FIRST + 0x01);

    public final static int WINDOW_WARNINGBOX = (WINDOW_FIRST + 0x02);

    public final static int WINDOW_ERRORBOX = (WINDOW_FIRST + 0x03);

    public final static int WINDOW_QUERYBOX = (WINDOW_FIRST + 0x04);

    public final static int WINDOW_WINDOW = (WINDOW_FIRST + 0x05);

    public final static int WINDOW_SYSWINDOW = (WINDOW_FIRST + 0x06);

    public final static int WINDOW_WORKWINDOW = (WINDOW_FIRST + 0x07);

    // public final static int WINDOW_MDIWINDOW = (WINDOW_FIRST + 0x08);
    public final static int WINDOW_FLOATINGWINDOW = (WINDOW_FIRST + 0x09);

    public final static int WINDOW_DIALOG = (WINDOW_FIRST + 0x0a);

    public final static int WINDOW_MODELESSDIALOG = (WINDOW_FIRST + 0x0b);

    public final static int WINDOW_MODALDIALOG = (WINDOW_FIRST + 0x0c);

    public final static int WINDOW_SYSTEMDIALOG = (WINDOW_FIRST + 0x0d);

    public final static int WINDOW_PATHDIALOG = (WINDOW_FIRST + 0x0e);

    public final static int WINDOW_FILEDIALOG = (WINDOW_FIRST + 0x0f);

    public final static int WINDOW_PRINTERSETUPDIALOG = (WINDOW_FIRST + 0x10);

    public final static int WINDOW_PRINTDIALOG = (WINDOW_FIRST + 0x11);

    public final static int WINDOW_COLORDIALOG = (WINDOW_FIRST + 0x12);

    public final static int WINDOW_FONTDIALOG = (WINDOW_FIRST + 0x13);

    public final static int WINDOW_CONTROL = (WINDOW_FIRST + 0x14);

    public final static int WINDOW_BUTTON = (WINDOW_FIRST + 0x15);

    public final static int WINDOW_PUSHBUTTON = (WINDOW_FIRST + 0x16);

    public final static int WINDOW_OKBUTTON = (WINDOW_FIRST + 0x17);

    public final static int WINDOW_CANCELBUTTON = (WINDOW_FIRST + 0x18);

    public final static int WINDOW_HELPBUTTON = (WINDOW_FIRST + 0x19);

    public final static int WINDOW_IMAGEBUTTON = (WINDOW_FIRST + 0x1a);

    public final static int WINDOW_MENUBUTTON = (WINDOW_FIRST + 0x1b);

    public final static int WINDOW_MOREBUTTON = (WINDOW_FIRST + 0x1c);

    public final static int WINDOW_SPINBUTTON = (WINDOW_FIRST + 0x1d);

    public final static int WINDOW_RADIOBUTTON = (WINDOW_FIRST + 0x1e);

    public final static int WINDOW_IMAGERADIOBUTTON = (WINDOW_FIRST + 0x1f);

    public final static int WINDOW_CHECKBOX = (WINDOW_FIRST + 0x20);

    public final static int WINDOW_TRISTATEBOX = (WINDOW_FIRST + 0x21);

    public final static int WINDOW_EDIT = (WINDOW_FIRST + 0x22);

    public final static int WINDOW_MULTILINEEDIT = (WINDOW_FIRST + 0x23);

    public final static int WINDOW_COMBOBOX = (WINDOW_FIRST + 0x24);

    public final static int WINDOW_LISTBOX = (WINDOW_FIRST + 0x25);

    public final static int WINDOW_MULTILISTBOX = (WINDOW_FIRST + 0x26);

    public final static int WINDOW_FIXEDTEXT = (WINDOW_FIRST + 0x27);

    public final static int WINDOW_FIXEDLINE = (WINDOW_FIRST + 0x28);

    public final static int WINDOW_FIXEDBITMAP = (WINDOW_FIRST + 0x29);

    public final static int WINDOW_FIXEDIMAGE = (WINDOW_FIRST + 0x2a);

    public final static int WINDOW_GROUPBOX = (WINDOW_FIRST + 0x2c);

    public final static int WINDOW_SCROLLBAR = (WINDOW_FIRST + 0x2d);

    public final static int WINDOW_SCROLLBARBOX = (WINDOW_FIRST + 0x2e);

    public final static int WINDOW_SPLITTER = (WINDOW_FIRST + 0x2f);

    public final static int WINDOW_SPLITWINDOW = (WINDOW_FIRST + 0x30);

    public final static int WINDOW_SPINFIELD = (WINDOW_FIRST + 0x31);

    public final static int WINDOW_PATTERNFIELD = (WINDOW_FIRST + 0x32);

    public final static int WINDOW_NUMERICFIELD = (WINDOW_FIRST + 0x33);

    public final static int WINDOW_METRICFIELD = (WINDOW_FIRST + 0x34);

    public final static int WINDOW_CURRENCYFIELD = (WINDOW_FIRST + 0x35);

    public final static int WINDOW_DATEFIELD = (WINDOW_FIRST + 0x36);

    public final static int WINDOW_TIMEFIELD = (WINDOW_FIRST + 0x37);

    public final static int WINDOW_PATTERNBOX = (WINDOW_FIRST + 0x38);

    public final static int WINDOW_NUMERICBOX = (WINDOW_FIRST + 0x39);

    public final static int WINDOW_METRICBOX = (WINDOW_FIRST + 0x3a);

    public final static int WINDOW_CURRENCYBOX = (WINDOW_FIRST + 0x3b);

    public final static int WINDOW_DATEBOX = (WINDOW_FIRST + 0x3c);

    public final static int WINDOW_TIMEBOX = (WINDOW_FIRST + 0x3d);

    public final static int WINDOW_LONGCURRENCYFIELD = (WINDOW_FIRST + 0x3e);

    public final static int WINDOW_LONGCURRENCYBOX = (WINDOW_FIRST + 0x3f);

    public final static int WINDOW_TOOLBOX = (WINDOW_FIRST + 0x41);

    public final static int WINDOW_DOCKINGWINDOW = (WINDOW_FIRST + 0x42);

    public final static int WINDOW_STATUSBAR = (WINDOW_FIRST + 0x43);

    public final static int WINDOW_TABPAGE = (WINDOW_FIRST + 0x44);

    public final static int WINDOW_TABCONTROL = (WINDOW_FIRST + 0x45);

    public final static int WINDOW_TABDIALOG = (WINDOW_FIRST + 0x46);

    public final static int WINDOW_BORDERWINDOW = (WINDOW_FIRST + 0x47);

    public final static int WINDOW_BUTTONDIALOG = (WINDOW_FIRST + 0x48);

    public final static int WINDOW_SYSTEMCHILDWINDOW = (WINDOW_FIRST + 0x49);

    public final static int WINDOW_FIXEDBORDER = (WINDOW_FIRST + 0x4a);

    public final static int WINDOW_SLIDER = (WINDOW_FIRST + 0x4b);

    public final static int WINDOW_MENUBARWINDOW = (WINDOW_FIRST + 0x4c);

    public final static int WINDOW_TREELISTBOX = (WINDOW_FIRST + 0x4d);

    public final static int WINDOW_HELPTEXTWINDOW = (WINDOW_FIRST + 0x4e);

    public final static int WINDOW_INTROWINDOW = (WINDOW_FIRST + 0x4f);

    public final static int WINDOW_LISTBOXWINDOW = (WINDOW_FIRST + 0x50);

    public final static int WINDOW_DOCKINGAREA = (WINDOW_FIRST + 0x51);

    public final static int WINDOW_VALUESETLISTBOX = (WINDOW_FIRST + 0x55);

    public final static int WINDOW_LAST = (WINDOW_DOCKINGAREA);

    public final static String UID_ACTIVE = "UID_ACTIVE";
}
