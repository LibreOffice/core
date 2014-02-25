/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_INSTBDLG_HXX
#define SC_INSTBDLG_HXX

#include "address.hxx"

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>

#include <sfx2/objsh.hxx>
#include <vcl/field.hxx>

class ScViewData;
class ScDocument;
class ScDocShell;

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }



class ScInsertTableDlg : public ModalDialog
{
public:
            ScInsertTableDlg( Window* pParent, ScViewData& rViewData, SCTAB nTabCount, bool bFromFile );
            ~ScInsertTableDlg();

    virtual short   Execute();      // overloaded to set parent dialog

    sal_Bool        GetTablesFromFile() const { return m_pBtnFromFile->IsChecked(); }
    sal_Bool        GetTablesAsLink() const { return m_pBtnLink->IsChecked(); }

    const OUString* GetFirstTable( sal_uInt16* pN = NULL );
    const OUString* GetNextTable( sal_uInt16* pN = NULL );
    ScDocShell*     GetDocShellTables() { return pDocShTables; }
    sal_Bool        IsTableBefore() const { return m_pBtnBefore->IsChecked(); }
    SCTAB           GetTableCount() const { return nTableCount;}

private:
    RadioButton*            m_pBtnBefore;
    RadioButton*            m_pBtnBehind;
    RadioButton*            m_pBtnNew;
    RadioButton*            m_pBtnFromFile;
    FixedText*              m_pFtCount;
    NumericField*           m_pNfCount;
    FixedText*              m_pFtName;
    Edit*                   m_pEdName;
    ListBox*                m_pLbTables;
    FixedText*              m_pFtPath;
    PushButton*             m_pBtnBrowse;
    CheckBox*               m_pBtnLink;
    OKButton*               m_pBtnOk;

    Timer                   aBrowseTimer;
    ScViewData&             rViewData;
    ScDocument&             rDoc;
    ScDocShell*             pDocShTables;
    sfx2::DocumentInserter* pDocInserter;
    SfxObjectShellRef       aDocShTablesRef;

    bool                bMustClose;
    sal_uInt16          nSelTabIndex;   // for GetFirstTable() / GetNextTable()
    OUString            aStrCurSelTable;
    SCTAB               nTableCount;
    OUString            m_sSheetDotDotDot;

#ifdef SC_INSTBDLG_CXX
    void    Init_Impl( bool bFromFile );
    void    SetNewTable_Impl();
    void    SetFromTo_Impl();
    void    FillTables_Impl( ScDocument* pSrcDoc );
    void    DoEnable_Impl();

    DECL_LINK(BrowseHdl_Impl, void *);
    DECL_LINK(ChoiceHdl_Impl, void *);
    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(CountHdl_Impl, void *);
    DECL_LINK(DoEnterHdl, void *);
    DECL_LINK(BrowseTimeoutHdl, void *);
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );
#endif
};

#endif // SC_INSTBDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
