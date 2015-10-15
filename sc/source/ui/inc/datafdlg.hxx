/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_DATAFDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DATAFDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include "global.hxx"

#include "tabvwsh.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#define MAX_DATAFORM_COLS   256
#define MAX_DATAFORM_ROWS   32000

class ScDataFormDlg : public ModalDialog
{
private:

    VclPtr<PushButton>     m_pBtnNew;
    VclPtr<PushButton>     m_pBtnDelete;
    VclPtr<PushButton>     m_pBtnRestore;
    VclPtr<PushButton>     m_pBtnPrev;
    VclPtr<PushButton>     m_pBtnNext;
    VclPtr<PushButton>     m_pBtnClose;
    VclPtr<ScrollBar>      m_pSlider;
    VclPtr<VclGrid>        m_pGrid;
    VclPtr<FixedText>      m_pFixedText;
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
    bool            bNoSelection;

    std::vector<VclPtr<FixedText> > maFixedTexts;
    std::vector<VclPtr<Edit> >      maEdits;

public:
    ScDataFormDlg( vcl::Window* pParent, ScTabViewShell* pTabViewShell);
    virtual ~ScDataFormDlg();
    virtual void dispose() override;

    void FillCtrls(SCROW nCurrentRow);
private:

    void SetButtonState();

    // Handler:
    DECL_LINK_TYPED(Impl_NewHdl, Button*, void);
    DECL_LINK_TYPED(Impl_PrevHdl, Button*, void);
    DECL_LINK_TYPED(Impl_NextHdl, Button*, void);

    DECL_LINK_TYPED(Impl_RestoreHdl, Button*, void);
    DECL_LINK_TYPED(Impl_DeleteHdl, Button*, void);
    DECL_LINK_TYPED(Impl_CloseHdl, Button*, void);

    DECL_LINK_TYPED(Impl_ScrollHdl, ScrollBar*, void);
    DECL_LINK_TYPED(Impl_DataModifyHdl, Edit&, void);
};
#endif // INCLUDED_SC_SOURCE_UI_INC_DATAFDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
