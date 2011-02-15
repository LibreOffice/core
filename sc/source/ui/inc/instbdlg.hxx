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

#ifndef SC_INSTBDLG_HXX
#define SC_INSTBDLG_HXX

#include "address.hxx"

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#include <sfx2/objsh.hxx>
#include <vcl/field.hxx>
#include "expftext.hxx"

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

class ScViewData;
class ScDocument;
class ScDocShell;

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

//------------------------------------------------------------------------

class ScInsertTableDlg : public ModalDialog
{
public:
            ScInsertTableDlg( Window* pParent, ScViewData& rViewData, SCTAB nTabCount, bool bFromFile );
            ~ScInsertTableDlg();

    virtual short   Execute();      // ueberladen, um Dialog-Parent zu setzen

    sal_Bool            GetTablesFromFile() { return aBtnFromFile.IsChecked(); }
    sal_Bool            GetTablesAsLink()   { return aBtnLink.IsChecked(); }

    const String*   GetFirstTable( sal_uInt16* pN = NULL );
    const String*   GetNextTable( sal_uInt16* pN = NULL );
    ScDocShell*     GetDocShellTables() { return pDocShTables; }
    sal_Bool            IsTableBefore() { return aBtnBefore.IsChecked(); }
    SCTAB           GetTableCount() { return nTableCount;}

private:
    RadioButton             aBtnBefore;
    RadioButton             aBtnBehind;
    FixedLine               aFlPos;
    RadioButton             aBtnNew;
    RadioButton             aBtnFromFile;
    FixedText               aFtCount;
    NumericField            aNfCount;
    FixedText               aFtName;
    Edit                    aEdName;
    MultiListBox            aLbTables;
    ScExpandedFixedText     aFtPath;
    PushButton              aBtnBrowse;
    CheckBox                aBtnLink;
    FixedLine               aFlTable;
    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;

    Timer                   aBrowseTimer;
    ScViewData&             rViewData;
    ScDocument&             rDoc;
    ScDocShell*             pDocShTables;
    sfx2::DocumentInserter* pDocInserter;
    SfxObjectShellRef       aDocShTablesRef;

    bool                bMustClose;
    sal_uInt16              nSelTabIndex;   // fuer GetFirstTable() / GetNextTable()
    String              aStrCurSelTable;
    SCTAB               nTableCount;

#ifdef SC_INSTBDLG_CXX
    void    Init_Impl( bool bFromFile );
    void    SetNewTable_Impl();
    void    SetFromTo_Impl();
    void    FillTables_Impl( ScDocument* pSrcDoc );
    void    DoEnable_Impl();

    DECL_LINK( BrowseHdl_Impl, PushButton* );
    DECL_LINK( ChoiceHdl_Impl, RadioButton* );
    DECL_LINK( SelectHdl_Impl, MultiListBox* );
    DECL_LINK( CountHdl_Impl, NumericField* );
    DECL_LINK( DoEnterHdl, PushButton* );
    DECL_LINK( BrowseTimeoutHdl, Timer* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );
#endif
};

#include <layout/layout-post.hxx>

#endif // SC_INSTBDLG_HXX

