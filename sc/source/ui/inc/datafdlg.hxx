/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_DATAFDLG_HXX
#define SC_DATAFDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include "global.hxx"

#include <tabvwsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <boost/ptr_container/ptr_vector.hpp>

#define MAX_DATAFORM_COLS   256
#define MAX_DATAFORM_ROWS   32000
#define FIXED_WIDTH         54
#define EDIT_WIDTH          86
#define FIXED_HEIGHT        10
#define EDIT_HEIGHT         12
#define FIXED_LEFT          6
#define EDIT_LEFT           62
#define LINE_HEIGHT         16

class ScDataFormDlg : public ModalDialog
{
private:

    PushButton*     m_pBtnNew;
    PushButton*     m_pBtnDelete;
    PushButton*     m_pBtnRestore;
    PushButton*     m_pBtnPrev;
    PushButton*     m_pBtnNext;
    PushButton*     m_pBtnClose;
    ScrollBar*      m_pSlider;
    VclGrid*        m_pGrid;
    FixedText*      m_pFixedText;
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

    boost::ptr_vector<boost::nullable<FixedText> > maFixedTexts;
    boost::ptr_vector<boost::nullable<Edit> > maEdits;

public:
    ScDataFormDlg( Window* pParent, ScTabViewShell* pTabViewShell);
    virtual ~ScDataFormDlg();

    void FillCtrls(SCROW nCurrentRow);
private:

    void SetButtonState();

    // Handler:
    DECL_LINK(Impl_NewHdl, void *);
    DECL_LINK(Impl_PrevHdl, void *);
    DECL_LINK(Impl_NextHdl, void *);

    DECL_LINK(Impl_RestoreHdl, void *);
    DECL_LINK(Impl_DeleteHdl, void *);
    DECL_LINK(Impl_CloseHdl, void *);

    DECL_LINK(Impl_ScrollHdl, void *);
    DECL_LINK( Impl_DataModifyHdl,  Edit*    );
};
#endif // SC_DATAFDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
