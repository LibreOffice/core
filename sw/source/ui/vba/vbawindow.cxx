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
        const uno::Reference< frame::XController >& xController ) throw (uno::RuntimeException) :
    WindowImpl_BASE( xParent, xContext, xModel, xController )
{
}

void
SwVbaWindow::Activate() throw (css::uno::RuntimeException)
{
    SwVbaDocument document( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel );

    document.Activate();
}

void
SwVbaWindow::Close( const uno::Any& SaveChanges, const uno::Any& RouteDocument ) throw (uno::RuntimeException)
{
    // FIXME: it is incorrect when there are more than 1 windows
    SwVbaDocument document( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel );
    uno::Any FileName;
    document.Close(SaveChanges, FileName, RouteDocument );
}

uno::Any SAL_CALL
SwVbaWindow::getView() throw (uno::RuntimeException)
{
    return uno::makeAny( uno::Reference< word::XView >( new SwVbaView( this,  mxContext, m_xModel ) ) );
}

void SAL_CALL SwVbaWindow::setView( const uno::Any& _view ) throw (uno::RuntimeException)
{
    sal_Int32 nType = 0;
    if( _view >>= nType )
    {
        SwVbaView view( this,  mxContext, m_xModel );
        view.setType( nType );
    }
}

uno::Any SAL_CALL
SwVbaWindow::getWindowState() throw (uno::RuntimeException)
{
    sal_Int32 nwindowState = word::WdWindowState::wdWindowStateNormal;
    SwView* pView = word::getView( m_xModel );
    SfxViewFrame* pViewFrame = pView -> GetViewFrame();
    WorkWindow* pWork = (WorkWindow*) pViewFrame->GetFrame().GetSystemWindow();
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
SwVbaWindow::setWindowState( const uno::Any& _windowstate ) throw (uno::RuntimeException)
{
    sal_Int32 nwindowState = word::WdWindowState::wdWindowStateMaximize;
    _windowstate >>= nwindowState;
    SwView* pView = word::getView( m_xModel );
    SfxViewFrame* pViewFrame = pView -> GetViewFrame();
    WorkWindow* pWork = (WorkWindow*) pViewFrame->GetFrame().GetSystemWindow();
    if ( pWork )
    {
        if ( nwindowState == word::WdWindowState::wdWindowStateMaximize )
            pWork -> Maximize();
        else if (nwindowState == word::WdWindowState::wdWindowStateMinimize)
            pWork -> Minimize();
        else if (nwindowState == word::WdWindowState::wdWindowStateNormal)
            pWork -> Restore();
        else
            throw uno::RuntimeException( rtl::OUString( "Invalid Parameter" ), uno::Reference< uno::XInterface >() );
    }
}

uno::Any SAL_CALL
SwVbaWindow::Panes( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< XCollection > xPanes( new SwVbaPanes( this,  mxContext, m_xModel ) );
    if(  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::makeAny( xPanes );

    return uno::Any( xPanes->Item( aIndex, uno::Any() ) );
}

uno::Any SAL_CALL
SwVbaWindow::ActivePane() throw (uno::RuntimeException)
{
    return uno::makeAny( uno::Reference< word::XPane >( new SwVbaPane( this,  mxContext, m_xModel ) ) );
}

rtl::OUString
SwVbaWindow::getServiceImplName()
{
    return rtl::OUString("SwVbaWindow");
}

uno::Sequence< rtl::OUString >
SwVbaWindow::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.word.Window" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
