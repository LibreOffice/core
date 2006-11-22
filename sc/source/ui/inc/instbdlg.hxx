/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: instbdlg.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:47:39 $
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

#ifndef SC_INSTBDLG_HXX
#define SC_INSTBDLG_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

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

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

#ifndef _SC_EXPFTEXT_HXX
#include "expftext.hxx"
#endif

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

    BOOL            GetTablesFromFile() { return aBtnFromFile.IsChecked(); }
    BOOL            GetTablesAsLink()   { return aBtnLink.IsChecked(); }

    const String*   GetFirstTable( USHORT* pN = NULL );
    const String*   GetNextTable( USHORT* pN = NULL );
    ScDocShell*     GetDocShellTables() { return pDocShTables; }
    BOOL            IsTableBefore() { return aBtnBefore.IsChecked(); }
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
    USHORT              nSelTabIndex;   // fuer GetFirstTable() / GetNextTable()
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

#endif // SC_INSTBDLG_HXX

