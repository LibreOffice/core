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
#include <unotools/configmgr.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/settings.hxx>
#include <UITools.hxx>
#include <memory>

using namespace ::dbaui;
using namespace ::utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

// class OTableBorderWindow
OTableBorderWindow::OTableBorderWindow(vcl::Window* pParent) : Window(pParent,WB_BORDER)
    ,m_aHorzSplitter( VclPtr<Splitter>::Create(this) )
{

    ImplInitSettings();
    // create children
    m_pEditorCtrl   = VclPtr<OTableEditorCtrl>::Create( this);
    m_pFieldDescWin = VclPtr<OTableFieldDescWin>::Create( this );

    m_pFieldDescWin->SetHelpId(HID_TAB_DESIGN_DESCWIN);

    // set depending windows and controls
    m_pEditorCtrl->SetDescrWin(m_pFieldDescWin);

    // set up splitter
    m_aHorzSplitter->SetSplitHdl( LINK(this, OTableBorderWindow, SplitHdl) );
    m_aHorzSplitter->Show();
}

OTableBorderWindow::~OTableBorderWindow()
{
    disposeOnce();
}

void OTableBorderWindow::dispose()
{
    // destroy children
    //  ::dbaui::notifySystemWindow(this,m_pFieldDescWin,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    m_pEditorCtrl->Hide();
    m_pFieldDescWin->Hide();
    m_pEditorCtrl.disposeAndClear();
    m_pFieldDescWin.disposeAndClear();
    m_aHorzSplitter.disposeAndClear();
    vcl::Window::dispose();
}

void OTableBorderWindow::Resize()
{
    const long nSplitterHeight(3);

    // dimensions of parent window
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();
    long nSplitPos      = m_aHorzSplitter->GetSplitPosPixel();

    // shift range of the splitter is the middle third of the output
    long nDragPosY = nOutputHeight/3;
    long nDragSizeHeight = nOutputHeight/3;
    m_aHorzSplitter->SetDragRectPixel( tools::Rectangle(Point(0,nDragPosY), Size(nOutputWidth,nDragSizeHeight) ), this );
    if( (nSplitPos < nDragPosY) || (nSplitPos > (nDragPosY+nDragSizeHeight)) )
        nSplitPos = nDragPosY+nDragSizeHeight-5;

    // set splitter
    m_aHorzSplitter->SetPosSizePixel( Point( 0, nSplitPos ), Size(nOutputWidth, nSplitterHeight));
    m_aHorzSplitter->SetSplitPosPixel( nSplitPos );

    // set window
    m_pEditorCtrl->SetPosSizePixel( Point(0, 0), Size(nOutputWidth , nSplitPos) );

    m_pFieldDescWin->SetPosSizePixel(   Point(0, nSplitPos+nSplitterHeight),
                        Size(nOutputWidth, nOutputHeight-nSplitPos-nSplitterHeight) );
}

IMPL_LINK( OTableBorderWindow, SplitHdl, Splitter*, pSplit, void )
{
    if(pSplit == m_aHorzSplitter.get())
    {
        m_aHorzSplitter->SetPosPixel( Point( m_aHorzSplitter->GetPosPixel().X(),m_aHorzSplitter->GetSplitPosPixel() ) );
        Resize();
    }
}

void OTableBorderWindow::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // FIXME RenderContext

    vcl::Font aFont = rStyleSettings.GetAppFont();
    if ( IsControlFont() )
        aFont.Merge( GetControlFont() );
    SetPointFont(*this,  aFont);

    Color aTextColor = rStyleSettings.GetButtonTextColor();
    if ( IsControlForeground() )
        aTextColor = GetControlForeground();
    SetTextColor( aTextColor );

    if( IsControlBackground() )
        SetBackground( GetControlBackground() );
    else
        SetBackground( rStyleSettings.GetFaceColor() );
}

void OTableBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OTableBorderWindow::GetFocus()
{
    Window::GetFocus();

    // forward the focus to the current cell of the editor control
    if (m_pEditorCtrl)
        m_pEditorCtrl->GrabFocus();
}

// class OTableDesignView
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

bool OTableDesignView::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
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
