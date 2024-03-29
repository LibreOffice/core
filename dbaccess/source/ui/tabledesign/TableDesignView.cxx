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

#include <TableDesignView.hxx>
#include <TableController.hxx>
#include <helpids.h>
#include <FieldDescriptions.hxx>
#include "TEditControl.hxx"
#include "TableFieldDescWin.hxx"
#include <TableRow.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/syslocale.hxx>
#include <memory>

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::lang;

OTableBorderWindow::OTableBorderWindow(OTableDesignView* pParent)
    : InterimItemWindow(pParent, "dbaccess/ui/tableborderwindow.ui", "TableBorderWindow", false)
    , m_xHorzSplitter(m_xBuilder->weld_paned("splitter"))
    , m_xEditorParent(m_xBuilder->weld_container("editor"))
    , m_xEditorParentWin(m_xEditorParent->CreateChildFrame())
    , m_xEditorCtrl(VclPtr<OTableEditorCtrl>::Create(VCLUnoHelper::GetWindow(m_xEditorParentWin), pParent))
    , m_xFieldDescParent(m_xBuilder->weld_container("fielddesc"))
    , m_xFieldDescWin(new OTableFieldDescWin(m_xFieldDescParent.get(), pParent))
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    m_xFieldDescWin->SetHelpId(HID_TAB_DESIGN_DESCWIN);

    // set depending windows and controls
    m_xEditorCtrl->SetDescrWin(m_xFieldDescWin.get());
}

OTableBorderWindow::~OTableBorderWindow()
{
    disposeOnce();
}

void OTableBorderWindow::dispose()
{
    // destroy children
    m_xEditorCtrl.disposeAndClear();
    m_xEditorParentWin->dispose();
    m_xEditorParentWin.clear();
    m_xEditorParent.reset();
    m_xFieldDescWin.reset();
    m_xFieldDescParent.reset();
    m_xHorzSplitter.reset();
    InterimItemWindow::dispose();
}

void OTableBorderWindow::Layout()
{
    // dimensions of parent window
    auto nOutputHeight = GetSizePixel().Height();
    auto nOldSplitPos = m_xHorzSplitter->get_position();
    auto nSplitPos = nOldSplitPos;

    // shift range of the splitter is the middle third of the output
    auto nDragPosY = nOutputHeight/3;
    auto nDragSizeHeight = nOutputHeight/3;
    if (nSplitPos < nDragPosY || nSplitPos > nDragPosY + nDragSizeHeight)
        nSplitPos = nDragPosY + nDragSizeHeight;

    // set splitter
    m_xHorzSplitter->set_position(nSplitPos);

    InterimItemWindow::Layout();

    if (nOldSplitPos != nSplitPos)
        m_xHorzSplitter->set_position(nSplitPos);
}

void OTableBorderWindow::GetFocus()
{
    InterimItemWindow::GetFocus();

    // forward the focus to the current cell of the editor control
    if (m_xEditorCtrl)
        m_xEditorCtrl->GrabFocus();
}

OTableDesignView::OTableDesignView( vcl::Window* pParent,
                                    const Reference< XComponentContext >& _rxOrb,
                                    OTableController& _rController
                                   ) :
    ODataView( pParent, _rController,_rxOrb )
    ,m_rController( _rController )
    ,m_eChildFocus(NONE)
{

    try
    {
        m_aLocale = SvtSysLocale().GetLanguageTag().getLocale();
    }
    catch(Exception&)
    {
    }

    m_pWin = VclPtr<OTableBorderWindow>::Create(this);

    m_pWin->GetDescWin()->connect_focus_in(LINK(this, OTableDesignView, FieldDescFocusIn));

    m_pWin->Show();
}

OTableDesignView::~OTableDesignView()
{
    disposeOnce();
}

void OTableDesignView::dispose()
{
    m_pWin->Hide();
    m_pWin.disposeAndClear();
    ODataView::dispose();
}

void OTableDesignView::initialize()
{
    GetEditorCtrl()->Init();
    GetDescWin()->Init();
    // first call after the editctrl has been set

    GetEditorCtrl()->Show();
    GetDescWin()->Show();

    GetEditorCtrl()->DisplayData(0);
}

void OTableDesignView::resizeDocumentView(tools::Rectangle& _rPlayground)
{
    m_pWin->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

IMPL_LINK_NOARG(OTableDesignView, FieldDescFocusIn, weld::Widget&, void)
{
    m_eChildFocus = DESCRIPTION;
}

bool OTableDesignView::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == NotifyEventType::GETFOCUS)
    {
        if( GetDescWin() && GetDescWin()->HasChildPathFocus() )
            m_eChildFocus = DESCRIPTION;
        else if ( GetEditorCtrl() && GetEditorCtrl()->HasChildPathFocus() )
            m_eChildFocus = EDITOR;
        else
            m_eChildFocus = NONE;
    }

    return ODataView::PreNotify(rNEvt);
}

IClipboardTest* OTableDesignView::getActiveChild() const
{
    IClipboardTest* pTest = nullptr;
    switch(m_eChildFocus)
    {
        case DESCRIPTION:
            pTest = GetDescWin();
            break;
        case EDITOR:
            pTest = GetEditorCtrl();
            break;
        case NONE:
            break;
    }
    return pTest;
}

bool OTableDesignView::isCopyAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCopyAllowed();
}

bool OTableDesignView::isCutAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCutAllowed();
}

bool OTableDesignView::isPasteAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isPasteAllowed();
}

void OTableDesignView::copy()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->copy();
}

void OTableDesignView::cut()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->cut();
}

void OTableDesignView::paste()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->paste();
}

// set the view readonly or not
void OTableDesignView::setReadOnly(bool _bReadOnly)
{
    GetDescWin()->SetReadOnly(_bReadOnly);
    GetEditorCtrl()->SetReadOnly(_bReadOnly);
}

void OTableDesignView::reSync()
{
    GetEditorCtrl()->DeactivateCell();
    std::shared_ptr<OTableRow>  pRow = (*GetEditorCtrl()->GetRowList())[GetEditorCtrl()->GetCurRow()];
    OFieldDescription* pFieldDescr = pRow ? pRow->GetActFieldDescr() : nullptr;
    if ( pFieldDescr )
        GetDescWin()->DisplayData(pFieldDescr);
}

void OTableDesignView::GetFocus()
{
    if ( GetEditorCtrl() )
        GetEditorCtrl()->GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
