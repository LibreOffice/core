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
#include <vbahelper/helperdecl.hxx>
#include <ooo/vba/word/WdWindowState.hpp>
#include <sfx2/viewfrm.hxx>
#include <vcl/wrkwin.hxx>

#include "vbawindow.hxx"
#include "vbaglobals.hxx"
#include "vbadocument.hxx"
#include "vbaview.hxx"
#include "vbapanes.hxx"
#include "vbapane.hxx"
#include "wordvbahelper.hxx"
#include <view.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

SwVbaWindow::SwVbaWindow(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< frame::XController >& xController ) :
    WindowImpl_BASE( xParent, xContext, xModel, xController )
{
}

void
SwVbaWindow::Activate()
{
    rtl::Reference<SwVbaDocument> document( new SwVbaDocument(uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel) );

    document->Activate();
}

void
SwVbaWindow::Close( const uno::Any& SaveChanges, const uno::Any& RouteDocument )
{
    // FIXME: it is incorrect when there are more than 1 windows
    rtl::Reference<SwVbaDocument> document( new SwVbaDocument(uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel) );
    uno::Any FileName;
    document->Close(SaveChanges, FileName, RouteDocument );
}

uno::Any SAL_CALL
SwVbaWindow::getView()
{
    return uno::makeAny( uno::Reference< word::XView >( new SwVbaView( this,  mxContext, m_xModel ) ) );
}

void SAL_CALL SwVbaWindow::setView( const uno::Any& _view )
{
    sal_Int32 nType = 0;
    if( _view >>= nType )
    {
        rtl::Reference<SwVbaView> view( new SwVbaView(this,  mxContext, m_xModel) );
        view->setType( nType );
    }
}

uno::Any SAL_CALL
SwVbaWindow::getWindowState()
{
    sal_Int32 nwindowState = word::WdWindowState::wdWindowStateNormal;
    SwView* pView = word::getView( m_xModel );
    SfxViewFrame* pViewFrame = pView -> GetViewFrame();
    WorkWindow* pWork = static_cast<WorkWindow*>( pViewFrame->GetFrame().GetSystemWindow() );
    if ( pWork )
    {
        if ( pWork -> IsMaximized())
            nwindowState = word::WdWindowState::wdWindowStateMaximize;
        else if (pWork -> IsMinimized())
            nwindowState = word::WdWindowState::wdWindowStateMinimize;
    }
    return uno::makeAny( nwindowState );
}

void SAL_CALL
SwVbaWindow::setWindowState( const uno::Any& _windowstate )
{
    sal_Int32 nwindowState = word::WdWindowState::wdWindowStateMaximize;
    _windowstate >>= nwindowState;
    SwView* pView = word::getView( m_xModel );
    SfxViewFrame* pViewFrame = pView -> GetViewFrame();
    WorkWindow* pWork = static_cast<WorkWindow*>( pViewFrame->GetFrame().GetSystemWindow() );
    if ( pWork )
    {
        if ( nwindowState == word::WdWindowState::wdWindowStateMaximize )
            pWork -> Maximize();
        else if (nwindowState == word::WdWindowState::wdWindowStateMinimize)
            pWork -> Minimize();
        else if (nwindowState == word::WdWindowState::wdWindowStateNormal)
            pWork -> Restore();
        else
            SAL_WARN("sw.vba", "Unhandled window state " << nwindowState);
    }
}

OUString SAL_CALL
SwVbaWindow::getCaption()
{
    SwView* pView = word::getView( m_xModel );
    if( !pView )
        return OUString("");

    uno::Reference< css::beans::XPropertySet > xFrameProps( pView->GetViewFrame()->GetFrame().GetFrameInterface()->getController()->getFrame(), uno::UNO_QUERY );
    if( !xFrameProps.is() )
        return OUString("");

    OUString sTitle;
    xFrameProps->getPropertyValue( "Title" ) >>= sTitle;

    return sTitle;
}

void SAL_CALL
SwVbaWindow::setCaption( const OUString& _caption )
{
    SwView* pView = word::getView( m_xModel );
    if( !pView )
        return;

    uno::Reference< css::beans::XPropertySet > xFrameProps( pView->GetViewFrame()->GetFrame().GetFrameInterface()->getController()->getFrame(), uno::UNO_QUERY );
    if( !xFrameProps.is() )
        return;

    xFrameProps->setPropertyValue( "Title", uno::makeAny( _caption ) );
}

uno::Any SAL_CALL
SwVbaWindow::Panes( const uno::Any& aIndex )
{
    uno::Reference< XCollection > xPanes( new SwVbaPanes( this,  mxContext, m_xModel ) );
    if(  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::makeAny( xPanes );

    return xPanes->Item( aIndex, uno::Any() );
}

uno::Any SAL_CALL
SwVbaWindow::ActivePane()
{
    return uno::makeAny( uno::Reference< word::XPane >( new SwVbaPane( this,  mxContext, m_xModel ) ) );
}

OUString
SwVbaWindow::getServiceImplName()
{
    return OUString("SwVbaWindow");
}

uno::Sequence< OUString >
SwVbaWindow::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.word.Window"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
