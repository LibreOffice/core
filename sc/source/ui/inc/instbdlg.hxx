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

#pragma once

#include <sfx2/objsh.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>
#include <types.hxx>

class ScViewData;
class ScDocument;
class ScDocShell;

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class ScInsertTableDlg : public weld::GenericDialogController
{
public:
    ScInsertTableDlg(weld::Window* pParent, ScViewData& rViewData, SCTAB nTabCount, bool bFromFile);
    virtual ~ScInsertTableDlg() override;

    virtual short run() override; // override to set parent dialog

    bool        GetTablesFromFile() const { return m_xBtnFromFile->get_active(); }
    bool        GetTablesAsLink() const { return m_xBtnLink->get_active(); }

    const OUString* GetFirstTable( sal_uInt16* pN );
    const OUString* GetNextTable( sal_uInt16* pN );
    ScDocShell*     GetDocShellTables() { return pDocShTables.get(); }
    bool        IsTableBefore() const { return m_xBtnBefore->get_active(); }
    SCTAB           GetTableCount() const { return nTableCount;}

private:
    Timer                   aBrowseTimer;
    ScViewData&             rViewData;
    ScDocument&             rDoc;
    rtl::Reference<ScDocShell> pDocShTables;
    std::unique_ptr<sfx2::DocumentInserter> pDocInserter;

    bool                bMustClose;
    sal_uInt16          nSelTabIndex;   // for GetFirstTable() / GetNextTable()
    OUString            aStrCurSelTable;
    SCTAB               nTableCount;
    OUString            m_sSheetDotDotDot;

    std::unique_ptr<weld::RadioButton> m_xBtnBefore;
    std::unique_ptr<weld::RadioButton> m_xBtnNew;
    std::unique_ptr<weld::RadioButton> m_xBtnFromFile;
    std::unique_ptr<weld::Label> m_xFtCount;
    std::unique_ptr<weld::SpinButton> m_xNfCount;
    std::unique_ptr<weld::Label> m_xFtName;
    std::unique_ptr<weld::Entry> m_xEdName;
    std::unique_ptr<weld::TreeView> m_xLbTables;
    std::unique_ptr<weld::Label> m_xFtPath;
    std::unique_ptr<weld::Button> m_xBtnBrowse;
    std::unique_ptr<weld::CheckButton> m_xBtnLink;
    std::unique_ptr<weld::Button> m_xBtnOk;

    void    Init_Impl( bool bFromFile );
    void    SetNewTable_Impl();
    void    SetFromTo_Impl();
    void    FillTables_Impl( const ScDocument* pSrcDoc );
    void    DoEnable_Impl();

    DECL_LINK( BrowseHdl_Impl, weld::Button&, void );
    DECL_LINK( ChoiceHdl_Impl, weld::Toggleable&, void );
    DECL_LINK( SelectHdl_Impl, weld::TreeView&, void );
    DECL_LINK( CountHdl_Impl, weld::SpinButton&, void );
    DECL_LINK( DoEnterHdl, weld::Button&, void );
    DECL_LINK( BrowseTimeoutHdl, Timer *, void );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper*, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
