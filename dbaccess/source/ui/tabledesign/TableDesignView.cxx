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

#include "TableDesignView.hxx"
#include <tools/debug.hxx>
#include "TableController.hxx"
#include "dbaccess_helpid.hrc"
#include "FieldDescriptions.hxx"
#include "TEditControl.hxx"
#include "TableFieldDescWin.hxx"
#include "TableRow.hxx"
#include <unotools/configmgr.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <unotools/syslocale.hxx>
#include "UITools.hxx"

using namespace ::dbaui;
using namespace ::utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

// class OTableBorderWindow
DBG_NAME(OTableBorderWindow)
OTableBorderWindow::OTableBorderWindow(Window* pParent) : Window(pParent,WB_BORDER)
    ,m_aHorzSplitter( this )
{
    DBG_CTOR(OTableBorderWindow,NULL);

    ImplInitSettings( sal_True, sal_True, sal_True );
    // Children erzeugen
    m_pEditorCtrl   = new OTableEditorCtrl( this);
    m_pFieldDescWin = new OTableFieldDescWin( this );

    m_pFieldDescWin->SetHelpId(HID_TAB_DESIGN_DESCWIN);

    // set depending windows and controls
    m_pEditorCtrl->SetDescrWin(m_pFieldDescWin);

    // Splitter einrichten
    m_aHorzSplitter.SetSplitHdl( LINK(this, OTableBorderWindow, SplitHdl) );
    m_aHorzSplitter.Show();
}

OTableBorderWindow::~OTableBorderWindow()
{
    // Children zerstoeren
    //  ::dbaui::notifySystemWindow(this,m_pFieldDescWin,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    m_pEditorCtrl->Hide();
    m_pFieldDescWin->Hide();

    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pEditorCtrl);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pEditorCtrl = NULL;
    }
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pFieldDescWin);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pFieldDescWin = NULL;
    }

    DBG_DTOR(OTableBorderWindow,NULL);
}

void OTableBorderWindow::Resize()
{
    const long nSplitterHeight(3);

    // Abmessungen parent window
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();
    long nSplitPos      = m_aHorzSplitter.GetSplitPosPixel();

    // Verschiebebereich Splitter mittleres Drittel des Outputs
    long nDragPosY = nOutputHeight/3;
    long nDragSizeHeight = nOutputHeight/3;
    m_aHorzSplitter.SetDragRectPixel( Rectangle(Point(0,nDragPosY), Size(nOutputWidth,nDragSizeHeight) ), this );
    if( (nSplitPos < nDragPosY) || (nSplitPos > (nDragPosY+nDragSizeHeight)) )
        nSplitPos = nDragPosY+nDragSizeHeight-5;

    // Splitter setzen
    m_aHorzSplitter.SetPosSizePixel( Point( 0, nSplitPos ), Size(nOutputWidth, nSplitterHeight));
    m_aHorzSplitter.SetSplitPosPixel( nSplitPos );

    // Fenster setzen
    m_pEditorCtrl->SetPosSizePixel( Point(0, 0), Size(nOutputWidth , nSplitPos) );

    m_pFieldDescWin->SetPosSizePixel(   Point(0, nSplitPos+nSplitterHeight),
                        Size(nOutputWidth, nOutputHeight-nSplitPos-nSplitterHeight) );
}

IMPL_LINK( OTableBorderWindow, SplitHdl, Splitter*, pSplit )
{
    if(pSplit == &m_aHorzSplitter)
    {
        m_aHorzSplitter.SetPosPixel( Point( m_aHorzSplitter.GetPosPixel().X(),m_aHorzSplitter.GetSplitPosPixel() ) );
        Resize();
    }
    return 0;
}

void OTableBorderWindow::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetPointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetButtonTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetFaceColor() );
    }
}

void OTableBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
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
DBG_NAME(OTableDesignView);
OTableDesignView::OTableDesignView( Window* pParent,
                                    const Reference< XComponentContext >& _rxOrb,
                                    OTableController& _rController
                                   ) :
    ODataView( pParent, _rController,_rxOrb )
    ,m_rController( _rController )
    ,m_eChildFocus(NONE)
{
    DBG_CTOR(OTableDesignView,NULL);

    try
    {
        m_aLocale = SvtSysLocale().GetLanguageTag().getLocale();
    }
    catch(Exception&)
    {
    }

    m_pWin = new OTableBorderWindow(this);
    m_pWin->Show();
}

OTableDesignView::~OTableDesignView()
{
    DBG_DTOR(OTableDesignView,NULL);
    m_pWin->Hide();

    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pWin);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pWin = NULL;
    }
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

void OTableDesignView::resizeDocumentView(Rectangle& _rPlayground)
{
    m_pWin->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

long OTableDesignView::PreNotify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    switch(rNEvt.GetType())
    {
        case EVENT_GETFOCUS:
            if( GetDescWin() && GetDescWin()->HasChildPathFocus() )
                m_eChildFocus = DESCRIPTION;
            else if ( GetEditorCtrl() && GetEditorCtrl()->HasChildPathFocus() )
                m_eChildFocus = EDITOR;
            else
                m_eChildFocus = NONE;
            break;
    }

    return bHandled ? 1L : ODataView::PreNotify(rNEvt);
}

IClipboardTest* OTableDesignView::getActiveChild() const
{
    IClipboardTest* pTest = NULL;
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

sal_Bool OTableDesignView::isCopyAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCopyAllowed();
}

sal_Bool OTableDesignView::isCutAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCutAllowed();
}

sal_Bool OTableDesignView::isPasteAllowed()
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
void OTableDesignView::setReadOnly(sal_Bool _bReadOnly)
{
    GetDescWin()->SetReadOnly(_bReadOnly);
    GetEditorCtrl()->SetReadOnly(_bReadOnly);
}

void OTableDesignView::reSync()
{
    GetEditorCtrl()->DeactivateCell();
     ::boost::shared_ptr<OTableRow>  pRow = (*GetEditorCtrl()->GetRowList())[GetEditorCtrl()->GetCurRow()];
    OFieldDescription* pFieldDescr = pRow ? pRow->GetActFieldDescr() : NULL;
    if ( pFieldDescr )
        GetDescWin()->DisplayData(pFieldDescr);
}

void OTableDesignView::GetFocus()
{
    if ( GetEditorCtrl() )
        GetEditorCtrl()->GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
