/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macrodlg.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:06:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
//
#ifndef _MACRODLG_HXX
#define _MACRODLG_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#include <bastype2.hxx>
#include <bastype3.hxx>

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

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
    FixedText               aMacrosInTxt;
    String                  aMacrosInTxtBaseStr;
    SvTreeListBox           aMacroBox;
    FixedText               aMacroFromTxT;
    FixedText               aMacrosSaveInTxt;
    BasicTreeListBox        aBasicBox;

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
