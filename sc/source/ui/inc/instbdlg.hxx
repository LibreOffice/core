/*************************************************************************
 *
 *  $RCSfile: instbdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:59 $
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

#ifndef SC_INSTBDLG_HXX
#define SC_INSTBDLG_HXX

#ifndef _SV_HXX
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
#ifndef _EMBOBJ_HXX //autogen
#include <so3/embobj.hxx>
#endif

#ifndef _SC_EXPFTEXT_HXX
#include "expftext.hxx"
#endif

#ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
#define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
SO2_DECL_REF(SvEmbeddedObject)
#endif


class ScViewData;
class ScDocument;
class ScDocShell;

//------------------------------------------------------------------------

class ScInsertTableDlg : public ModalDialog
{
public:
            ScInsertTableDlg( Window* pParent, ScViewData& rViewData, USHORT nTabCount);
            ~ScInsertTableDlg();

    virtual short   Execute();      // ueberladen, um Dialog-Parent zu setzen

    BOOL            GetTablesFromFile() { return aBtnFromFile.IsChecked(); }
    BOOL            GetTablesAsLink()   { return aBtnLink.IsChecked(); }

    const String*   GetFirstTable( USHORT* pN = NULL );
    const String*   GetNextTable( USHORT* pN = NULL );
    ScDocShell*     GetDocShellTables() { return pDocShTables; }
    BOOL            IsTableBefore() { return aBtnBefore.IsChecked(); }
    USHORT          GetTableCount() { return nTableCount;}

private:
    RadioButton             aBtnBefore;
    RadioButton             aBtnBehind;
    GroupBox                aGbPos;
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
    GroupBox                aGbTable;
    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;

    ScViewData&         rViewData;
    ScDocument&         rDoc;
    ScDocShell*         pDocShTables;
    SvEmbeddedObjectRef aDocShTablesRef;

    USHORT              nSelTabIndex;   // fuer GetFirstTable() / GetNextTable()
    String              aStrCurSelTable;
    USHORT              nTableCount;

#ifdef SC_INSTBDLG_CXX
    void    Init_Impl();
    void    SetNewTable_Impl();
    void    SetFromTo_Impl();
    void    FillTables_Impl( ScDocument* pSrcDoc );
    void    DoEnable_Impl();

    DECL_LINK( BrowseHdl_Impl, PushButton* );
    DECL_LINK( ChoiceHdl_Impl, RadioButton* );
    DECL_LINK( SelectHdl_Impl, MultiListBox* );
    DECL_LINK( CountHdl_Impl, NumericField* );
    DECL_LINK( DoEnterHdl, PushButton* );
#endif
};


#endif // SC_INSTBDLG_HXX


