/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macrodlg.hxx,v $
 * $Revision: 1.10 $
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

    BOOL                bNewDelIsDel;
    BOOL                bForceStoreBasic;

    USHORT              nMode;

    DECL_LINK( MacroSelectHdl, SvTreeListBox * );
    DECL_LINK( MacroDoubleClickHdl, SvTreeListBox * );
    DECL_LINK( BasicSelectHdl, SvTreeListBox * );
    DECL_LINK( EditModifyHdl, Edit * );
    DECL_LINK( ButtonHdl, Button * );

    void                CheckButtons();
    void                SaveSetCurEntry( SvTreeListBox& rBox, SvLBoxEntry* pEntry );
    void                UpdateFields();

    void                EnableButton( Button& rButton, BOOL bEnable );

    String              GetInfo( SbxVariable* pVar );

    void                StoreMacroDescription();
    void                RestoreMacroDescription();

public:
                        MacroChooser( Window* pParent, BOOL bCreateEntries = TRUE );
                        ~MacroChooser();

    SbMethod*           GetMacro();
    void                DeleteMacro();
    SbMethod*           CreateMacro();

    virtual short       Execute();

    void                SetMode( USHORT nMode );
    USHORT              GetMode() const { return nMode; }
};

#endif  // _MACRODLG_HXX
