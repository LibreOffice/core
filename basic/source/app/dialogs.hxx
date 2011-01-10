/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DIALOGS_HXX
#define _DIALOGS_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <tools/config.hxx>
#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/ctrlbox.hxx>

class SbxVariable;

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

class AboutDialog : public ModalDialog {
    FixedText a1,a4,aVersionString;
    OKButton  aOk;
public:
    AboutDialog (Window*, const ResId&);
};

class FindDialog : public ModalDialog {
    FixedText       aFT1;
    Edit            aFind;
    OKButton        aOk;
    CancelButton    aCancel;
    String*         pFind;
    DECL_LINK( ButtonClick, Button * );
public:
    FindDialog (Window*, const ResId&, String&);
};

class ReplaceDialog : public ModalDialog {
    FixedText       aFT1;
    FixedText       aFT2;
    Edit            aFind;
    Edit            aReplace;
    OKButton        aOk;
    CancelButton    aCancel;
    String*         pFind;
    String*         pReplace;
    DECL_LINK( ButtonClick, Button * );
public:
    ReplaceDialog (Window*, const ResId&, String&, String&);
};

////////////////////////////////////////////////////////////////////

class ConfEdit : public PushButton
{
protected:
    FixedText aText;
    Edit aEdit;
    ByteString aKeyName;

    void Init( Config &aConf );

public:
    ConfEdit( Window* pParent, sal_uInt16 nResText, sal_uInt16 nResEdit, sal_uInt16 nResButton, const ByteString& aKN, Config &aConf );
    ConfEdit( Window* pParent, sal_uInt16 nResEdit, sal_uInt16 nResButton, const ByteString& aKN, Config &aConf );
    void Save( Config &aConf );
    void Reload( Config &aConf );
    void Click();
    String GetValue() { return aEdit.GetText(); };
    void SetModifyHdl( Link aLink ) { aEdit.SetModifyHdl( aLink ); };
};


class OptConfEdit : public ConfEdit
{
protected:
    CheckBox aCheck;
    ConfEdit& rBase;
    DECL_LINK( ToggleHdl, CheckBox* );
public:
    OptConfEdit( Window* pParent, sal_uInt16 nResCheck, sal_uInt16 nResEdit, sal_uInt16 nResButton, const ByteString& aKN, ConfEdit& rBaseEdit, Config& aConf );
    void Reload( Config &aConf );
    DECL_LINK( BaseModifyHdl, Edit* );
};


class OptionsDialog : public TabDialog
{
private:
    TabControl      aTabCtrl;

    OKButton aOK;
    CancelButton aCancel;
    DECL_LINK( OKClick, Button * );

    Config aConfig;

public:
    OptionsDialog( Window* pParent, const ResId& );
    ~OptionsDialog();
    virtual sal_Bool    Close();


    DECL_LINK( ActivatePageHdl, TabControl * );
};

class ProfileOptions : public TabPage
{
    Config &rConf;

    FixedLine aFlProfile;
    ComboBox aCbProfile;
    PushButton aPbNewProfile;
    PushButton aPbDelProfile;

    FixedLine aDirs;
    ConfEdit aLog;
    ConfEdit aBasis;
    OptConfEdit aHID;

    CheckBox aAutoReload;
    CheckBox aAutoSave;
    CheckBox aStopOnSyntaxError;

    void LoadData();

    DECL_LINK( Select, ComboBox* );
    DECL_LINK( DelProfile, Button* );
    DECL_LINK( NewProfile, Button* );
    DECL_LINK( CheckButtonsHdl, ComboBox* );

    void ReloadProfile();
    void Save();

public:
    ProfileOptions( Window*, Config &rConfig );
    void Save( Config &rConfig );
};


class CrashreportOptions : public TabPage
{
    FixedLine aFLCrashreport;
    CheckBox aCBUseProxy;
    FixedText aFTCRHost;
    Edit aEDCRHost;
    FixedText aFTCRPort;
    NumericField aNFCRPort;

    CheckBox aCBAllowContact;
    FixedText aFTEMail;
    Edit aEDEMail;

    DECL_LINK( CheckProxy, void*);
    DECL_LINK( CheckResponse, void*);

public:
    CrashreportOptions( Window*, Config &aConfig );
    void Save( Config &aConfig );
};

class MiscOptions : public TabPage
{
    FixedLine aFLCommunication;
    FixedText aFTHost;
    Edit aEDHost;
    FixedText aFTTTPort;
    NumericField aNFTTPort;
    FixedText aFTUNOPort;
    NumericField aNFUNOPort;
    FixedLine aOther;
    FixedText aTimeoutText;
    TimeField aServerTimeout;
    FixedText aFTLRU;
    NumericField aTFMaxLRU;
    FixedText aFTProgDir;
    Edit aEDProgDir;
    PushButton aPBProgDir;

    DECL_LINK( Click, void*);

public:
    MiscOptions( Window*, Config &aConfig );
    void Save( Config &aConfig );
};

class FontOptions : public TabPage
{
    FixedText aFTFontName;
    FontNameBox aFontName;
    FixedText aFTStyle;
    FontStyleBox aFontStyle;
    FixedText aFTSize;
    FontSizeBox aFontSize;
    FixedText aFTPreview;

    FontList aFontList;

    DECL_LINK( FontNameChanged, void* );
    DECL_LINK( FontStyleChanged, void* );
    DECL_LINK( FontSizeChanged, void* );

    void UpdatePreview();

public:
    FontOptions( Window*, Config &aConfig );
    void Save( Config &aConfig );
};


DECLARE_LIST( StringList, String * )
#define C_KEY_ALLE          CByteString("All")
#define C_KEY_AKTUELL       CByteString("Current")
#define C_KEY_TYPE          CByteString("Type")
#define C_KEY_DELETE        CByteString("Deleted Groups")

class GenericOptions : public TabPage
{
    Config &aConf;

    FixedLine aFlArea;
    ComboBox aCbArea;
    PushButton aPbNewArea;
    PushButton aPbDelArea;

    FixedLine aFlValue;
    ComboBox aCbValue;
    PushButton aPbSelectPath;
    PushButton aPbNewValue;
    PushButton aPbDelValue;

    int nMoveButtons;
    sal_Bool bShowSelectPath;
    AutoTimer aMoveTimer;
    DECL_LINK( MoveButtons, AutoTimer* );

    ByteString aLastGroupName;

    String ReadKey( const ByteString &aGroup, const ByteString &aKey );

    StringList* GetAllGroups();
    void LoadData();

    void ShowSelectPath( const String aType );

    DECL_LINK( LoadGroup, ComboBox* );
    DECL_LINK( DelGroup, Button* );
    DECL_LINK( NewGroup, Button* );
    DECL_LINK( SelectPath, Button* );
    DECL_LINK( DelValue, Button* );
    DECL_LINK( NewValue, Button* );
    DECL_LINK( CheckButtonsHdl, ComboBox* );

public:
    GenericOptions( Window*, Config &aConfig );
    ~GenericOptions();
    void Save( Config &aConfig );
};


struct WinInfoRec;
class SplitWindow;

class DisplayHidDlg : public FloatingWindow
{
protected:
    ToolBox aTbConf;
    FixedText aFtControls;
    MultiListBox aMlbControls;
    FixedText aFtSlots;
    MultiListBox aMlbSlots;
    PushButton aPbKopieren;
    PushButton aPbBenennen;
    PushButton aPbSelectAll;
    OKButton aOKClose;

    DockingWindow* pControls;
    DockingWindow* pSlots;
    SplitWindow *pSplit;

    sal_uIntPtr nDisplayMode;

    DECL_LINK( Select, void* );
    DECL_LINK( SelectAll, PushButton* );
    DECL_LINK( CopyToClipboard, void* );

public:
    DisplayHidDlg( Window * pParent );
    virtual ~DisplayHidDlg();

    virtual void Resize();

    void AddData( WinInfoRec* pWinInfo );

};


class VarEditDialog : public ModelessDialog
{
protected:
    FixedText aFixedTextRID_FT_NAME;
    FixedText aFixedTextRID_FT_CONTENT;
    FixedText aFixedTextRID_FT_NEW_CONTENT;
    FixedText aFixedTextRID_FT_NAME_VALUE;
    FixedText aFixedTextRID_FT_CONTENT_VALUE;

    RadioButton aRadioButtonRID_RB_NEW_BOOL_T;
    RadioButton aRadioButtonRID_RB_NEW_BOOL_F;
    NumericField aNumericFieldRID_NF_NEW_INTEGER;
    NumericField aNumericFieldRID_NF_NEW_LONG;
    Edit aEditRID_ED_NEW_STRING;

    OKButton aOKButtonRID_OK;
    CancelButton aCancelButtonRID_CANCEL;

    SbxVariable *pVar;

    DECL_LINK( OKClick, Button * );

//  sal_Bool bCompare = sal_False;
//  String aCompareString;

public:
    VarEditDialog( Window * pParent, SbxVariable *pPVar );
};


#endif
