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

#include "AppView.hxx"
#include <strings.hrc>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/toolbox.hxx>
#include <vcl/event.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/waitobj.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <unotools/syslocale.hxx>
#include <UITools.hxx>
#include "AppDetailView.hxx"
#include <tabletree.hxx>
#include "AppSwapWindow.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include "AppTitleWindow.hxx"
#include <dsntypes.hxx>
#include <stringconstants.hxx>
#include <dbaccess/IController.hxx>
#include <browserids.hxx>
#include <unotools/pathoptions.hxx>
#include "AppController.hxx"

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using ::com::sun::star::sdb::application::NamedDatabaseObject;

// class OAppBorderWindow
OAppBorderWindow::OAppBorderWindow(OApplicationView* _pParent,PreviewMode _ePreviewMode) : Window(_pParent,WB_DIALOGCONTROL)
    ,m_pPanel(nullptr)
    ,m_pDetailView(nullptr)
    ,m_pView(_pParent)
{

    SetBorderStyle(WindowBorderStyle::MONO);

    m_pPanel = VclPtr<OTitleWindow>::Create(this,STR_DATABASE,WB_BORDER | WB_DIALOGCONTROL, false);
    m_pPanel->SetBorderStyle(WindowBorderStyle::MONO);
    VclPtrInstance<OApplicationSwapWindow> pSwap( m_pPanel, *this );
    pSwap->Show();

    m_pPanel->setChildWindow(pSwap);
    m_pPanel->Show();

    m_pDetailView = VclPtr<OApplicationDetailView>::Create(*this,_ePreviewMode);
    m_pDetailView->Show();

    ImplInitSettings();
}

OAppBorderWindow::~OAppBorderWindow()
{
    disposeOnce();
}

void OAppBorderWindow::dispose()
{
    // destroy children
    if ( m_pPanel )
        m_pPanel->Hide();
    m_pPanel.disposeAndClear();
    if ( m_pDetailView )
        m_pDetailView->Hide();
    m_pDetailView.disposeAndClear();
    m_pView.clear();
    vcl::Window::dispose();
}

void OAppBorderWindow::GetFocus()
{
    if ( m_pPanel )
        m_pPanel->GrabFocus();
}

void OAppBorderWindow::Resize()
{
    // parent window dimension
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();
    long nX = 0;

    Size aFLSize = LogicToPixel(Size(3, 8), MapMode(MapUnit::MapAppFont));
    if ( m_pPanel )
    {
        OApplicationSwapWindow* pSwap = getPanel();
        if ( pSwap && pSwap->GetEntryCount() != 0 )
            nX = pSwap->GetBoundingBox( pSwap->GetEntry(0) ).GetWidth() + aFLSize.Height();
        nX = std::max(m_pPanel->GetWidthPixel() ,nX);
        m_pPanel->SetPosSizePixel(Point(0,0),Size(nX,nOutputHeight));
    }

    if ( m_pDetailView )
        m_pDetailView->SetPosSizePixel(Point(nX + aFLSize.Width(),0),Size(nOutputWidth - nX - aFLSize.Width(),nOutputHeight));
}

void OAppBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
        (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
        (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OAppBorderWindow::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    vcl::Font aFont;
    aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();

    SetBackground( rStyleSettings.GetDialogColor() );
}


OApplicationSwapWindow* OAppBorderWindow::getPanel() const
{
    return static_cast< OApplicationSwapWindow* >( m_pPanel->getChildWindow() );
}


// class OApplicationView
OApplicationView::OApplicationView( vcl::Window* pParent
                                    ,const Reference< XComponentContext >& _rxOrb
                                    ,OApplicationController& _rAppController
                                    ,PreviewMode _ePreviewMode
                                   ) :
    ODataView( pParent, _rAppController, _rxOrb, WB_DIALOGCONTROL )
    ,m_rAppController( _rAppController )
    ,m_eChildFocus(NONE)
{
    m_pWin = VclPtr<OAppBorderWindow>::Create(this,_ePreviewMode);
    m_pWin->Show();

    ImplInitSettings();
}

OApplicationView::~OApplicationView()
{
    disposeOnce();
}

void OApplicationView::dispose()
{
    stopComponentListening(m_xObject);
    m_xObject.clear();
    m_pWin->Hide();
    m_pWin.disposeAndClear();
    ODataView::dispose();
}

void OApplicationView::createIconAutoMnemonics( MnemonicGenerator& _rMnemonics )
{
    if ( m_pWin && m_pWin->getPanel() )
        m_pWin->getPanel()->createIconAutoMnemonics( _rMnemonics );
}

void OApplicationView::setTaskExternalMnemonics( MnemonicGenerator const & _rMnemonics )
{
    if ( m_pWin && m_pWin->getDetailView() )
        m_pWin->getDetailView()->setTaskExternalMnemonics( _rMnemonics );
}

void OApplicationView::DataChanged( const DataChangedEvent& rDCEvt )
{
    ODataView::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
        (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
        (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OApplicationView::resizeDocumentView(tools::Rectangle& _rPlayground)
{
    if ( m_pWin && !_rPlayground.IsEmpty() )
    {
        Size aFLSize = LogicToPixel(Size(3, 3), MapMode(MapUnit::MapAppFont));
        _rPlayground.Move( aFLSize.Width(),aFLSize.Height() );
        Size aOldSize = _rPlayground.GetSize();
        _rPlayground.SetSize( Size(aOldSize.Width() - 2*aFLSize.Width(), aOldSize.Height() - 2*aFLSize.Height()) );

        m_pWin->SetPosSizePixel(_rPlayground.TopLeft() , _rPlayground.GetSize() );
    }
    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

bool OApplicationView::PreNotify( NotifyEvent& rNEvt )
{
    switch(rNEvt.GetType())
    {
        case MouseNotifyEvent::GETFOCUS:
            if( m_pWin && getPanel() && getPanel()->HasChildPathFocus() )
                m_eChildFocus = PANELSWAP;
            else if ( m_pWin && getDetailView() && getDetailView()->HasChildPathFocus() )
                m_eChildFocus = DETAIL;
            else
                m_eChildFocus = NONE;
            break;
        case MouseNotifyEvent::KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            // give the pane the chance to intercept mnemonic accelerators
            // #i34790#
            if ( getPanel() && getPanel()->interceptKeyInput( *pKeyEvent ) )
                return true;
            // and ditto the detail view
            // #i72799#
            if ( getDetailView() && getDetailView()->interceptKeyInput( *pKeyEvent ) )
                return true;
        }
        break;
        default:
        break;
    }

    return ODataView::PreNotify(rNEvt);
}

IClipboardTest* OApplicationView::getActiveChild() const
{
    IClipboardTest* pTest = nullptr;
    if ( DETAIL == m_eChildFocus )
        pTest = getDetailView();
    return pTest;
}

bool OApplicationView::isCopyAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCopyAllowed();
}

bool OApplicationView::isCutAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCutAllowed();
}

bool OApplicationView::isPasteAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isPasteAllowed();
}

void OApplicationView::copy()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->copy();
}

void OApplicationView::cut()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->cut();
}

void OApplicationView::paste()
{
    IClipboardTest* pTest = getActiveChild();
    if ( pTest )
        pTest->paste();
}

OUString OApplicationView::getQualifiedName( SvTreeListEntry* _pEntry ) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getQualifiedName( _pEntry );
}

bool OApplicationView::isLeaf(SvTreeListEntry const * _pEntry) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return OApplicationDetailView::isLeaf(_pEntry);
}

bool OApplicationView::isALeafSelected() const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isALeafSelected();
}

void OApplicationView::selectAll()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->selectAll();
}

bool OApplicationView::isSortUp() const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isSortUp();
}

void OApplicationView::sortDown()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->sortDown();
}

void OApplicationView::sortUp()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->sortUp();
}

bool OApplicationView::isFilled() const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isFilled();
}

ElementType OApplicationView::getElementType() const
{
    OSL_ENSURE(m_pWin && getDetailView() && getPanel(),"Detail view is NULL! -> GPF");
    return getDetailView()->HasChildPathFocus() ? getDetailView()->getElementType() : getPanel()->getElementType();
}

sal_Int32 OApplicationView::getSelectionCount()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getSelectionCount();
}

sal_Int32 OApplicationView::getElementCount()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getElementCount();
}

void OApplicationView::getSelectionElementNames( std::vector< OUString>& _rNames ) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->getSelectionElementNames( _rNames );
}

void OApplicationView::describeCurrentSelectionForControl( const Control& _rControl, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->describeCurrentSelectionForControl( _rControl, _out_rSelectedObjects );
}

void OApplicationView::describeCurrentSelectionForType( const ElementType _eType, Sequence< NamedDatabaseObject >& _out_rSelectedObjects )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->describeCurrentSelectionForType( _eType, _out_rSelectedObjects );
}

void OApplicationView::selectElements(const Sequence< OUString>& _aNames)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->selectElements( _aNames );
}

SvTreeListEntry* OApplicationView::elementAdded(ElementType eType,const OUString& _rName, const Any& _rObject )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->elementAdded(eType,_rName,_rObject);
}

void OApplicationView::elementRemoved(ElementType eType,const OUString& _rName )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->elementRemoved(eType,_rName);
}

void OApplicationView::elementReplaced(ElementType _eType
                                                    ,const OUString& _rOldName
                                                    ,const OUString& _rNewName )
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->elementReplaced(_eType, _rOldName, _rNewName );
}

void OApplicationView::clearPages()
{
    OSL_ENSURE(m_pWin && getDetailView() && getPanel(),"Detail view is NULL! -> GPF");
    getPanel()->clearSelection();
    getDetailView()->clearPages();
}

void OApplicationView::selectContainer(ElementType _eType)
{
    OSL_ENSURE(m_pWin && getPanel(),"Detail view is NULL! -> GPF");
    WaitObject aWO(this);
    getPanel()->selectContainer(_eType);
}

SvTreeListEntry* OApplicationView::getEntry( const Point& _aPosPixel ) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getEntry(_aPosPixel);
}

PreviewMode OApplicationView::getPreviewMode()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->getPreviewMode();
}

bool OApplicationView::isPreviewEnabled()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isPreviewEnabled();
}

void OApplicationView::switchPreview(PreviewMode _eMode)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->switchPreview(_eMode);
}

void OApplicationView::showPreview(const Reference< XContent >& _xContent)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    stopComponentListening(m_xObject);
    m_xObject = nullptr;
    getDetailView()->showPreview(_xContent);
}

void OApplicationView::showPreview( const OUString& _sDataSourceName,
                                    const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                                    const OUString& _sName,
                                    bool _bTable)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    if ( isPreviewEnabled() )
    {
        stopComponentListening(m_xObject);
        m_xObject = nullptr;
        try
        {
            Reference<XNameAccess> xNameAccess;
            if ( _bTable )
            {
                Reference<XTablesSupplier> xSup(_xConnection,UNO_QUERY);
                if ( xSup.is() )
                    xNameAccess.set(xSup->getTables(),UNO_QUERY);
            }
            else
            {
                Reference<XQueriesSupplier> xSup(_xConnection,UNO_QUERY);
                if ( xSup.is() )
                    xNameAccess.set(xSup->getQueries(),UNO_QUERY);
            }
            if ( xNameAccess.is() && xNameAccess->hasByName(_sName) )
                m_xObject.set(xNameAccess->getByName(_sName),UNO_QUERY);
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        if ( m_xObject.is() )
            startComponentListening(m_xObject);
        getDetailView()->showPreview(_sDataSourceName,_sName,_bTable);
    }
}

void OApplicationView::GetFocus()
{
    if ( m_eChildFocus == NONE && m_pWin )
    {
        m_pWin->GrabFocus();
    }
}

void OApplicationView::_disposing( const css::lang::EventObject& /*_rSource*/ )
{
    if ( m_pWin && getDetailView() )
        showPreview(nullptr);
}

void OApplicationView::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    vcl::Font aFont;
    aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();

    SetBackground( rStyleSettings.GetFieldColor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
