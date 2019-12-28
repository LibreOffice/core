/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <types.hxx>
#include "viewfunc.hxx"

class ScTabViewShell;
class ScDocument;

#define MAX_DATAFORM_COLS   256
#define MAX_DATAFORM_ROWS   32000

class ScDataFormDlg : public weld::GenericDialogController
{
private:
    OUString        sNewRecord;

    ScTabViewShell* pTabViewShell;
    ScDocument*     pDoc;
    sal_uInt16      aColLength;
    SCROW           nCurrentRow;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCTAB           nTab;

    std::unique_ptr<weld::Button> m_xBtnNew;
    std::unique_ptr<weld::Button> m_xBtnDelete;
    std::unique_ptr<weld::Button> m_xBtnRestore;
    std::unique_ptr<weld::Button> m_xBtnPrev;
    std::unique_ptr<weld::Button> m_xBtnNext;
    std::unique_ptr<weld::Button> m_xBtnClose;
    std::unique_ptr<weld::ScrolledWindow> m_xSlider;
    std::unique_ptr<weld::Container> m_xGrid;
    std::unique_ptr<weld::Label> m_xFixedText;
    std::vector<std::unique_ptr<ScDataFormFragment>> m_aEntries;

public:
    ScDataFormDlg(weld::Window* pParent, ScTabViewShell* pTabViewShell);
    virtual ~ScDataFormDlg() override;

    void FillCtrls();
private:

    void SetButtonState();

    // Handler:
    DECL_LINK(Impl_NewHdl, weld::Button&, void);
    DECL_LINK(Impl_PrevHdl, weld::Button&, void);
    DECL_LINK(Impl_NextHdl, weld::Button&, void);

    DECL_LINK(Impl_RestoreHdl, weld::Button&, void);
    DECL_LINK(Impl_DeleteHdl, weld::Button&, void);
    DECL_LINK(Impl_CloseHdl, weld::Button&, void);

    DECL_LINK(Impl_ScrollHdl, weld::ScrolledWindow&, void);
    DECL_LINK(Impl_DataModifyHdl, weld::Entry&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
