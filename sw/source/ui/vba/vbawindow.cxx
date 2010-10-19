/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

SwVbaWindow::SwVbaWindow( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : WindowImpl_BASE( xParent, xContext, xModel )
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
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Invalid Parameter" ) ), uno::Reference< uno::XInterface >() );
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

rtl::OUString&
SwVbaWindow::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaWindow") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaWindow::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Window" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
