/*************************************************************************
 *
 *  $RCSfile: dialogs.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
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
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_TABDLG_HXX //autogen
#include <vcl/tabdlg.hxx>
#endif
#ifndef _SV_TABCTRL_HXX //autogen
#include <vcl/tabctrl.hxx>
#endif
#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

class SbxVariable;

#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif

class AboutDialog : public ModalDialog {
    FixedText a1,a3,a4,aVersionString;
    OKButton  aOk;
//  FixedIcon aIcon;
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

class ConfEdit : public PushButton
{
    FixedText aText;
    ByteString aKeyName;
    Edit aEdit;
public:
    ConfEdit( Window* pParent, USHORT nResText, USHORT nResEdit, USHORT nResButton, const ByteString& aKN );
    void Save();
    void Click();
};


class OptionsDialog : public ModelessDialog
{
private:
    TabControl      aTabCtrl;

    OKButton aOK;
    CancelButton aCancel;
    DECL_LINK( OKClick, Button * );

public:
    OptionsDialog( Window* pParent, const ResId& );
    ~OptionsDialog();
    virtual BOOL    Close();


    DECL_LINK( ActivatePageHdl, TabControl * );
};

class SpecialOptions : public TabPage
{
    GroupBox aDirs;
    ConfEdit aLog;
    ConfEdit aBasis;
    ConfEdit aHID;
    GroupBox aOther;
    FixedText aTimeoutText;
    TimeField aServerTimeout;
    CheckBox aAutoReload;

public:
    SpecialOptions( Window* );
    void Save();
};


DECLARE_LIST( StringList, String * );
#define C_KEY_ALLE          CByteString("Alle")
#define C_KEY_AKTUELL       CByteString("Aktuell")
#define C_KEY_DELETE        CByteString("Deleted Groups")

class GenericOptions : public TabPage
{
    GroupBox aGbArea;
    ComboBox aCbArea;
    PushButton aPbNewArea;
    PushButton aPbDelArea;

    GroupBox aGbValue;
    ComboBox aCbValue;
    PushButton aPbNewValue;
    PushButton aPbDelValue;

    Config aConf;
    String aTempConfName;
    Config *pTempConf;
    ByteString aLastGroupName;

    String ReadKey( const ByteString &aGroup, const ByteString &aKey );
    BOOL HasKey( Config &aConfig, const ByteString &aKey );

    void LoadValues();

    StringList* GetAllGroups();
    void LoadData();

    DECL_LINK( LoadGroup, ComboBox* );
    DECL_LINK( DelGroup, Button* );
    DECL_LINK( NewGroup, Button* );
    DECL_LINK( DelValue, Button* );
    DECL_LINK( NewValue, Button* );
    DECL_LINK( CheckButtonsHdl, ComboBox* );

    void CheckButtons( ComboBox &aCB, Button &aNewB, Button &aDelB );

public:
    GenericOptions( Window* );
    ~GenericOptions();
    void Save();
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

    ULONG nDisplayMode;

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

//  BOOL bCompare = FALSE;
//  String aCompareString;

public:
    VarEditDialog( Window * pParent, SbxVariable *pPVar );
};


#endif
