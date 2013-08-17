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
#include "dbu_app.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include "dbaccess_helpid.hrc"
#include <vcl/toolbox.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/waitobj.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <unotools/syslocale.hxx>
#include "UITools.hxx"
#include "AppDetailView.hxx"
#include "tabletree.hxx"
#include "AppSwapWindow.hxx"
#include <vcl/svapp.hxx>
#include "AppTitleWindow.hxx"
#include "dsntypes.hxx"
#include "dbustrings.hrc"
#include <dbaccess/IController.hxx>
#include "browserids.hxx"
#include <unotools/pathoptions.hxx>
#include "IApplicationController.hxx"

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using ::com::sun::star::sdb::application::NamedDatabaseObject;

// class OAppBorderWindow
DBG_NAME(OAppBorderWindow)
OAppBorderWindow::OAppBorderWindow(OApplicationView* _pParent,PreviewMode _ePreviewMode) : Window(_pParent,WB_DIALOGCONTROL)
    ,m_pPanel(NULL)
    ,m_pDetailView(NULL)
    ,m_pView(_pParent)
{
    DBG_CTOR(OAppBorderWindow,NULL);

    SetBorderStyle(WINDOW_BORDER_MONO);

    m_pPanel = new OTitleWindow(this,STR_DATABASE,WB_BORDER | WB_DIALOGCONTROL ,sal_False);
    m_pPanel->SetBorderStyle(WINDOW_BORDER_MONO);
    OApplicationSwapWindow* pSwap = new OApplicationSwapWindow( m_pPanel, *this );
    pSwap->Show();
    pSwap->SetUniqueId(UID_APP_SWAP_VIEW);

    m_pPanel->setChildWindow(pSwap);
    m_pPanel->SetUniqueId(UID_APP_DATABASE_VIEW);
    m_pPanel->Show();

    m_pDetailView = new OApplicationDetailView(*this,_ePreviewMode);
    m_pDetailView->Show();

    ImplInitSettings();
}

OAppBorderWindow::~OAppBorderWindow()
{
    // destroy children
    if ( m_pPanel )
    {
        m_pPanel->Hide();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pPanel);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pPanel = NULL;
    }
    if ( m_pDetailView )
    {
        m_pDetailView->Hide();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pDetailView);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pDetailView = NULL;
    }

    DBG_DTOR(OAppBorderWindow,NULL);
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

    Size aFLSize = LogicToPixel( Size( 3, 8 ), MAP_APPFONT );
    if ( m_pPanel )
    {
        OApplicationSwapWindow* pSwap = getPanel();
        if ( pSwap )
        {
            if ( pSwap->GetEntryCount() != 0 )
                nX = pSwap->GetBoundingBox( pSwap->GetEntry(0) ).GetWidth() + aFLSize.Height();
        }
        nX = ::std::max(m_pPanel->GetWidthPixel() ,nX);
        m_pPanel->SetPosSizePixel(Point(0,0),Size(nX,nOutputHeight));
    }

    if ( m_pDetailView )
        m_pDetailView->SetPosSizePixel(Point(nX + aFLSize.Width(),0),Size(nOutputWidth - nX - aFLSize.Width(),nOutputHeight));
}

void OAppBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
        (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
        (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OAppBorderWindow::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( true )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( true )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( true )
        SetBackground( rStyleSettings.GetDialogColor() );
}

OApplicationView* OAppBorderWindow::getView() const
{
    return m_pView;
}

OApplicationSwapWindow* OAppBorderWindow::getPanel() const
{
    return static_cast< OApplicationSwapWindow* >( m_pPanel->getChildWindow() );
}

OApplicationDetailView* OAppBorderWindow::getDetailView() const
{
    return m_pDetailView;
}

// class OApplicationView
DBG_NAME(OApplicationView);
OApplicationView::OApplicationView( Window* pParent
                                    ,const Reference< XComponentContext >& _rxOrb
                                    ,IApplicationController& _rAppController
                                    ,PreviewMode _ePreviewMode
                                   ) :
    ODataView( pParent, _rAppController, _rxOrb, WB_DIALOGCONTROL )
    ,m_rAppController( _rAppController )
    ,m_eChildFocus(NONE)
{
    DBG_CTOR(OApplicationView,NULL);

    try
    {
        m_aLocale = SvtSysLocale().GetLanguageTag().getLocale();
    }
    catch(Exception&)
    {
    }

    m_pWin = new OAppBorderWindow(this,_ePreviewMode);
    m_pWin->SetUniqueId(UID_APP_VIEW_BORDER_WIN);
    m_pWin->Show();

    ImplInitSettings();
}

OApplicationView::~OApplicationView()
{
    DBG_DTOR(OApplicationView,NULL);

    {
        stopComponentListening(m_xObject);
        m_pWin->Hide();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pWin);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pWin = NULL;
    }
}

void OApplicationView::createIconAutoMnemonics( MnemonicGenerator& _rMnemonics )
{
    if ( m_pWin && m_pWin->getPanel() )
        m_pWin->getPanel()->createIconAutoMnemonics( _rMnemonics );
}

void OApplicationView::setTaskExternalMnemonics( MnemonicGenerator& _rMnemonics )
{
    if ( m_pWin && m_pWin->getDetailView() )
        m_pWin->getDetailView()->setTaskExternalMnemonics( _rMnemonics );
}

void OApplicationView::DataChanged( const DataChangedEvent& rDCEvt )
{
    ODataView::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
        (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
        (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OApplicationView::resizeDocumentView(Rectangle& _rPlayground)
{
    if ( m_pWin && !_rPlayground.IsEmpty() )
    {
        Size aFLSize = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
        _rPlayground.Move( aFLSize.A(),aFLSize.B() );
        Size aOldSize = _rPlayground.GetSize();
        _rPlayground.SetSize( Size(aOldSize.A() - 2*aFLSize.A(), aOldSize.B() - 2*aFLSize.B()) );

        m_pWin->SetPosSizePixel(_rPlayground.TopLeft() , _rPlayground.GetSize() );
    }
    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

long OApplicationView::PreNotify( NotifyEvent& rNEvt )
{
    switch(rNEvt.GetType())
    {
        case EVENT_GETFOCUS:
            if( m_pWin && getPanel() && getPanel()->HasChildPathFocus() )
                m_eChildFocus = PANELSWAP;
            else if ( m_pWin && getDetailView() && getDetailView()->HasChildPathFocus() )
                m_eChildFocus = DETAIL;
            else
                m_eChildFocus = NONE;
            break;
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            // give the pane the chance to intercept mnemonic accelerators
            // #i34790#
            if ( getPanel() && getPanel()->interceptKeyInput( *pKeyEvent ) )
                return 1L;
            // and ditto the detail view
            // #i72799#
            if ( getDetailView() && getDetailView()->interceptKeyInput( *pKeyEvent ) )
                return 1L;
        }
        break;
    }

    return ODataView::PreNotify(rNEvt);
}

IClipboardTest* OApplicationView::getActiveChild() const
{
    IClipboardTest* pTest = NULL;
    if ( DETAIL == m_eChildFocus )
        pTest = getDetailView();
    return pTest;
}

sal_Bool OApplicationView::isCopyAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCopyAllowed();
}

sal_Bool OApplicationView::isCutAllowed()
{
    IClipboardTest* pTest = getActiveChild();
    return pTest && pTest->isCutAllowed();
}

sal_Bool OApplicationView::isPasteAllowed()
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

sal_Bool OApplicationView::isLeaf(SvTreeListEntry* _pEntry) const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isLeaf(_pEntry);
}

sal_Bool OApplicationView::isALeafSelected() const
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    return getDetailView()->isALeafSelected();
}

void OApplicationView::selectAll()
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    getDetailView()->selectAll();
}

sal_Bool OApplicationView::isSortUp() const
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

sal_Bool OApplicationView::isFilled() const
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

void OApplicationView::getSelectionElementNames( ::std::vector< OUString>& _rNames ) const
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

void OApplicationView::clearPages(sal_Bool _bTaskAlso)
{
    OSL_ENSURE(m_pWin && getDetailView() && getPanel(),"Detail view is NULL! -> GPF");
    getPanel()->clearSelection();
    getDetailView()->clearPages(_bTaskAlso);
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

sal_Bool OApplicationView::isPreviewEnabled()
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
    m_xObject = NULL;
    getDetailView()->showPreview(_xContent);
}

void OApplicationView::showPreview( const OUString& _sDataSourceName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                    const OUString& _sName,
                                    sal_Bool _bTable)
{
    OSL_ENSURE(m_pWin && getDetailView(),"Detail view is NULL! -> GPF");
    if ( isPreviewEnabled() )
    {
        stopComponentListening(m_xObject);
        m_xObject = NULL;
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
            DBG_UNHANDLED_EXCEPTION();
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

void OApplicationView::_disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ )
{
    if ( m_pWin && getDetailView() )
        showPreview(NULL);
}

void OApplicationView::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( true )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
    }

    if( true )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( true )
        SetBackground( rStyleSettings.GetFieldColor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
