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

#ifndef INCLUDED_SC_SOURCE_UI_INC_INSTBDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_INSTBDLG_HXX

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
            ScInsertTableDlg( vcl::Window* pParent, ScViewData& rViewData, SCTAB nTabCount, bool bFromFile );
            virtual ~ScInsertTableDlg();
    virtual void dispose() override;

    virtual short   Execute() override; // override to set parent dialog

    bool        GetTablesFromFile() const { return m_pBtnFromFile->IsChecked(); }
    bool        GetTablesAsLink() const { return m_pBtnLink->IsChecked(); }

    const OUString* GetFirstTable( sal_uInt16* pN = NULL );
    const OUString* GetNextTable( sal_uInt16* pN = NULL );
    ScDocShell*     GetDocShellTables() { return pDocShTables; }
    bool        IsTableBefore() const { return m_pBtnBefore->IsChecked(); }
    SCTAB           GetTableCount() const { return nTableCount;}

private:
    VclPtr<RadioButton>            m_pBtnBefore;
    VclPtr<RadioButton>            m_pBtnBehind;
    VclPtr<RadioButton>            m_pBtnNew;
    VclPtr<RadioButton>            m_pBtnFromFile;
    VclPtr<FixedText>              m_pFtCount;
    VclPtr<NumericField>           m_pNfCount;
    VclPtr<FixedText>              m_pFtName;
    VclPtr<Edit>                   m_pEdName;
    VclPtr<ListBox>                m_pLbTables;
    VclPtr<FixedText>              m_pFtPath;
    VclPtr<PushButton>             m_pBtnBrowse;
    VclPtr<CheckBox>               m_pBtnLink;
    VclPtr<OKButton>               m_pBtnOk;

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

    void    Init_Impl( bool bFromFile );
    void    SetNewTable_Impl();
    void    SetFromTo_Impl();
    void    FillTables_Impl( ScDocument* pSrcDoc );
    void    DoEnable_Impl();

    DECL_LINK_TYPED(BrowseHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ChoiceHdl_Impl, Button*, void);
    DECL_LINK_TYPED(SelectHdl_Impl, ListBox&, void);
    DECL_LINK(CountHdl_Impl, void *);
    DECL_LINK_TYPED(DoEnterHdl, Button*, void);
    DECL_LINK_TYPED(BrowseTimeoutHdl, Timer *, void);
    DECL_LINK_TYPED( DialogClosedHdl, sfx2::FileDialogHelper*, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_INSTBDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
