/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <uielement/progressbarwrapper.hxx>

#include <helper/statusindicator.hxx>
#include <threadhelp/resetableguard.hxx>
#include <uielement/statusindicatorinterfacewrapper.hxx>

#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace ::com::sun::star;

namespace framework{

ProgressBarWrapper::ProgressBarWrapper() :
UIElementWrapperBase( ::com::sun::star::ui::UIElementType::PROGRESSBAR )
    ,   m_bOwnsInstance( sal_False )
    ,   m_nRange( 100 )
    ,   m_nValue( 0 )
{
}

ProgressBarWrapper::~ProgressBarWrapper()
{
}


void ProgressBarWrapper::setStatusBar( const uno::Reference< awt::XWindow >& rStatusBar, sal_Bool bOwnsInstance )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    if ( m_bOwnsInstance )
    {
        
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
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return uno::Reference< awt::XWindow >();

    return m_xStatusBar;
}


void ProgressBarWrapper::start( const OUString& Text, ::sal_Int32 Range )
throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWindow;
    sal_Int32                      nValue( 0 );

    {
        ResetableGuard aGuard( m_aLock );

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
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
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
            pStatusBar->Show( true, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
        }
    }
}

void ProgressBarWrapper::end()
throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWindow;

    {
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            return;

        xWindow  = m_xStatusBar;
        m_nRange = 100;
        m_nValue = 0;
    }

    if ( xWindow.is() )
    {
        SolarMutexGuard aSolarMutexGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            if ( pStatusBar->IsProgressMode() )
                pStatusBar->EndProgressMode();
        }
    }
}

void ProgressBarWrapper::setText( const OUString& Text )
throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWindow;
    sal_Int32 nValue( 0 );

    {
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            return;

        xWindow  = m_xStatusBar;
        m_aText  = Text;
        nValue   = m_nValue;
    }

    if ( xWindow.is() )
    {
        SolarMutexGuard aSolarMutexGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
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
throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWindow;
    OUString aText;
    sal_Bool      bSetValue( sal_False );

    {
        ResetableGuard aGuard( m_aLock );

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
            bSetValue = sal_True;
        }

        nValue   = m_nValue;
        aText    = m_aText;
    }

    if ( xWindow.is() && bSetValue )
    {
        SolarMutexGuard aSolarMutexGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            if ( !pStatusBar->IsProgressMode() )
                pStatusBar->StartProgressMode( aText );
            pStatusBar->SetProgressValue( sal_uInt16( nValue ));
        }
    }
}

void ProgressBarWrapper::reset()
throw (uno::RuntimeException)
{
    setText( OUString() );
    setValue( 0 );
}


void SAL_CALL ProgressBarWrapper::initialize( const uno::Sequence< uno::Any >& )
throw (uno::Exception, uno::RuntimeException)
{
    
}


void SAL_CALL ProgressBarWrapper::update()
throw (uno::RuntimeException)
{
    
}


void SAL_CALL ProgressBarWrapper::dispose()
throw (uno::RuntimeException)
{
    uno::Reference< lang::XComponent > xThis(
        static_cast< cppu::OWeakObject* >(this),
        uno::UNO_QUERY );

    {
        ResetableGuard aLock( m_aLock );

        if ( m_bDisposed )
            return;
    }

    {
        lang::EventObject aEvent( xThis );
        m_aListenerContainer.disposeAndClear( aEvent );

        ResetableGuard aLock( m_aLock );
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
        m_bDisposed = sal_True;
    }
}


uno::Reference< uno::XInterface > SAL_CALL ProgressBarWrapper::getRealInterface()
throw (uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    
    ResetableGuard aLock( m_aLock );

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
                xComp = uno::Reference< uno::XInterface >(
                    static_cast< cppu::OWeakObject* >( pWrapper ),
                    uno::UNO_QUERY );
             m_xProgressBarIfacWrapper = xComp;
        }

        return xComp;
    }
}

}       

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
