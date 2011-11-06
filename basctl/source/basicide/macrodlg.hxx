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


//
#ifndef _MACRODLG_HXX
#define _MACRODLG_HXX

#include <svheader.hxx>

#include <bastype2.hxx>
#include <bastype3.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/fixed.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#define MACRO_CLOSE         10
#define MACRO_OK_RUN        11
#define MACRO_NEW           12
#define MACRO_EDIT          14
#define MACRO_ORGANIZE      15
#define MACRO_ASSIGN        16

#define MACROCHOOSER_ALL            1
#define MACROCHOOSER_CHOOSEONLY     2
#define MACROCHOOSER_RECORDING      3

class BasicManager;

class MacroChooser : public SfxModalDialog
{
private:
    FixedText               aMacroNameTxt;
    Edit                    aMacroNameEdit;
    FixedText               aMacroFromTxT;
    FixedText               aMacrosSaveInTxt;
    BasicTreeListBox        aBasicBox;
    FixedText               aMacrosInTxt;
    String                  aMacrosInTxtBaseStr;
    SvTreeListBox           aMacroBox;

    PushButton              aRunButton;
    CancelButton            aCloseButton;
    PushButton              aAssignButton;
    PushButton              aEditButton;
    PushButton              aNewDelButton;
    PushButton              aOrganizeButton;
    HelpButton              aHelpButton;
    PushButton              aNewLibButton;
    PushButton              aNewModButton;

    sal_Bool                bNewDelIsDel;
    sal_Bool                bForceStoreBasic;

    sal_uInt16              nMode;

    DECL_LINK( MacroSelectHdl, SvTreeListBox * );
    DECL_LINK( MacroDoubleClickHdl, SvTreeListBox * );
    DECL_LINK( BasicSelectHdl, SvTreeListBox * );
    DECL_LINK( EditModifyHdl, Edit * );
    DECL_LINK( ButtonHdl, Button * );

    void                CheckButtons();
    void                SaveSetCurEntry( SvTreeListBox& rBox, SvLBoxEntry* pEntry );
    void                UpdateFields();

    void                EnableButton( Button& rButton, sal_Bool bEnable );

    String              GetInfo( SbxVariable* pVar );

    void                StoreMacroDescription();
    void                RestoreMacroDescription();

public:
                        MacroChooser( Window* pParent, sal_Bool bCreateEntries = sal_True );
                        ~MacroChooser();

    SbMethod*           GetMacro();
    void                DeleteMacro();
    SbMethod*           CreateMacro();

    virtual short       Execute();

    void                SetMode( sal_uInt16 nMode );
    sal_uInt16              GetMode() const { return nMode; }
};

#endif  // _MACRODLG_HXX
