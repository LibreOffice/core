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

#include <uielement/progressbarwrapper.hxx>

#include <helper/statusindicator.hxx>
#include <uielement/statusindicatorinterfacewrapper.hxx>

#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace ::com::sun::star;

namespace framework{

ProgressBarWrapper::ProgressBarWrapper() :
UIElementWrapperBase( css::ui::UIElementType::PROGRESSBAR )
    ,   m_bOwnsInstance( false )
    ,   m_nRange( 100 )
    ,   m_nValue( 0 )
{
}

ProgressBarWrapper::~ProgressBarWrapper()
{
}

// public interfaces
void ProgressBarWrapper::setStatusBar( const uno::Reference< awt::XWindow >& rStatusBar, bool bOwnsInstance )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    if ( m_bOwnsInstance )
    {
        // dispose XWindow reference of our status bar
        uno::Reference< lang::XComponent > xComponent( m_xStatusBar, uno::UNO_QUERY );
        try
        {
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const uno::Exception& )
        {
        }
        m_xStatusBar.clear();
    }

    m_bOwnsInstance = bOwnsInstance;
    m_xStatusBar    = rStatusBar;
}

uno::Reference< awt::XWindow > ProgressBarWrapper::getStatusBar() const
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return uno::Reference< awt::XWindow >();

    return m_xStatusBar;
}

// wrapped methods of css::task::XStatusIndicator
void ProgressBarWrapper::start( const OUString& Text, ::sal_Int32 Range )
{
    uno::Reference< awt::XWindow > xWindow;
    sal_Int32                      nValue( 0 );

    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            return;

        xWindow  = m_xStatusBar;
        m_nValue = 0;
        m_nRange = Range;
        nValue   = m_nValue;
    }

    if ( xWindow.is() )
    {
        SolarMutexGuard aSolarMutexGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WindowType::STATUSBAR )
        {
            StatusBar* pStatusBar = static_cast<StatusBar *>(pWindow.get());
            if ( !pStatusBar->IsProgressMode() )
                pStatusBar->StartProgressMode( Text );
            else
            {
                pStatusBar->SetUpdateMode( false );
                pStatusBar->EndProgressMode();
                pStatusBar->StartProgressMode( Text );
                pStatusBar->SetProgressValue( sal_uInt16( nValue ));
                pStatusBar->SetUpdateMode( true );
            }
            pStatusBar->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );
        }
    }
}

void ProgressBarWrapper::end()
{
    uno::Reference< awt::XWindow > xWindow;

    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            return;

        xWindow  = m_xStatusBar;
        m_nRange = 100;
        m_nValue = 0;
    }

    if ( xWindow.is() )
    {
        SolarMutexGuard aSolarMutexGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WindowType::STATUSBAR )
        {
            StatusBar* pStatusBar = static_cast<StatusBar *>(pWindow.get());
            if ( pStatusBar->IsProgressMode() )
                pStatusBar->EndProgressMode();
        }
    }
}

void ProgressBarWrapper::setText( const OUString& Text )
{
    uno::Reference< awt::XWindow > xWindow;
    sal_Int32 nValue( 0 );

    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            return;

        xWindow  = m_xStatusBar;
        m_aText  = Text;
        nValue   = m_nValue;
    }

    if ( xWindow.is() )
    {
        SolarMutexGuard aSolarMutexGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WindowType::STATUSBAR )
        {
            StatusBar* pStatusBar = static_cast<StatusBar *>(pWindow.get());
            if( pStatusBar->IsProgressMode() )
            {
                pStatusBar->SetUpdateMode( false );
                pStatusBar->EndProgressMode();
                pStatusBar->StartProgressMode( Text );
                pStatusBar->SetProgressValue( sal_uInt16( nValue ));
                pStatusBar->SetUpdateMode( true );
            }
            else
                pStatusBar->SetText( Text );
        }
    }
}

void ProgressBarWrapper::setValue( ::sal_Int32 nValue )
{
    uno::Reference< awt::XWindow > xWindow;
    OUString aText;
    bool      bSetValue( false );

    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            return;

        xWindow  = m_xStatusBar;

        double fVal( 0 );
        if ( m_nRange > 0 )
        {
            fVal = ( double( nValue ) / double( m_nRange )) * 100;
            fVal = std::max( double( 0 ), std::min( fVal, double( 100 )));
        }

        if ( m_nValue != sal_Int32( fVal ))
        {
            m_nValue = sal_Int32( fVal );
            bSetValue = true;
        }

        nValue   = m_nValue;
        aText    = m_aText;
    }

    if ( xWindow.is() && bSetValue )
    {
        SolarMutexGuard aSolarMutexGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WindowType::STATUSBAR )
        {
            StatusBar* pStatusBar = static_cast<StatusBar *>(pWindow.get());
            if ( !pStatusBar->IsProgressMode() )
                pStatusBar->StartProgressMode( aText );
            pStatusBar->SetProgressValue( sal_uInt16( nValue ));
        }
    }
}

void ProgressBarWrapper::reset()
{
    setText( OUString() );
    setValue( 0 );
}

// XInitialization
void SAL_CALL ProgressBarWrapper::initialize( const uno::Sequence< uno::Any >& )
{
    // dummy - do nothing
}

// XUpdatable
void SAL_CALL ProgressBarWrapper::update()
{
    // dummy - do nothing
}

// XComponent
void SAL_CALL ProgressBarWrapper::dispose()
{
    uno::Reference< lang::XComponent > xThis(
        static_cast< cppu::OWeakObject* >(this),
        uno::UNO_QUERY );

    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            return;
    }

    {
        lang::EventObject aEvent( xThis );
        m_aListenerContainer.disposeAndClear( aEvent );

        SolarMutexGuard g;
        if ( m_bOwnsInstance )
        {
            try
            {
                uno::Reference< lang::XComponent > xComponent( m_xStatusBar, uno::UNO_QUERY );
                if ( xComponent.is() )
                    xComponent->dispose();
            }
            catch ( const lang::DisposedException& )
            {
            }
        }

        m_xStatusBar.clear();
        m_bDisposed = true;
    }
}

// XUIElement
uno::Reference< uno::XInterface > SAL_CALL ProgressBarWrapper::getRealInterface()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return uno::Reference< uno::XInterface >();
    else
    {
        uno::Reference< uno::XInterface > xComp( m_xProgressBarIfacWrapper );
        if ( !xComp.is() )
        {
            StatusIndicatorInterfaceWrapper* pWrapper =
                new StatusIndicatorInterfaceWrapper(
                    uno::Reference< lang::XComponent >(
                        static_cast< cppu::OWeakObject* >( this ),
                        uno::UNO_QUERY ));
            xComp.set(static_cast< cppu::OWeakObject* >( pWrapper ),
                        uno::UNO_QUERY );
            m_xProgressBarIfacWrapper = xComp;
        }

        return xComp;
    }
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
