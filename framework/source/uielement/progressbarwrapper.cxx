/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progressbarwrapper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:23:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_
#include <uielement/progressbarwrapper.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#include <helper/statusindicator.hxx>
#endif
#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_
#include <uielement/statusindicatorinterfacewrapper.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UI_UIELEMENTTYPE_HPP_
#include <com/sun/star/ui/UIElementType.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________
using namespace ::com::sun::star;

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

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

// public interfaces
void ProgressBarWrapper::setStatusBar( const uno::Reference< awt::XWindow >& rStatusBar, sal_Bool bOwnsInstance )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    if ( m_bOwnsInstance )
    {
        // dispose XWindow reference our our status bar
        uno::Reference< lang::XComponent > xComponent( m_xStatusBar, uno::UNO_QUERY );
        try
        {
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( uno::Exception& )
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

// wrapped methods of ::com::sun::star::task::XStatusIndicator
void ProgressBarWrapper::start( const ::rtl::OUString& Text, ::sal_Int32 Range )
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
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            if ( !pStatusBar->IsProgressMode() )
                pStatusBar->StartProgressMode( Text );
            else
            {
                pStatusBar->SetUpdateMode( FALSE );
                pStatusBar->EndProgressMode();
                pStatusBar->StartProgressMode( Text );
                pStatusBar->SetProgressValue( USHORT( nValue ));
                pStatusBar->SetUpdateMode( TRUE );
            }
            pStatusBar->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
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
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            if ( pStatusBar->IsProgressMode() )
                pStatusBar->EndProgressMode();
        }
    }
}

void ProgressBarWrapper::setText( const ::rtl::OUString& Text )
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
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            if( pStatusBar->IsProgressMode() )
            {
                pStatusBar->SetUpdateMode( FALSE );
                pStatusBar->EndProgressMode();
                pStatusBar->StartProgressMode( Text );
                pStatusBar->SetProgressValue( USHORT( nValue ));
                pStatusBar->SetUpdateMode( TRUE );
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
    rtl::OUString aText;
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
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR )
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            if ( !pStatusBar->IsProgressMode() )
                pStatusBar->StartProgressMode( aText );
            pStatusBar->SetProgressValue( USHORT( nValue ));
        }
    }
}

void ProgressBarWrapper::reset()
throw (uno::RuntimeException)
{
    setText( rtl::OUString() );
    setValue( 0 );
}

// XInitialization
void SAL_CALL ProgressBarWrapper::initialize( const uno::Sequence< uno::Any >& )
throw (uno::Exception, uno::RuntimeException)
{
    // dummy - do nothing
}

// XUpdatable
void SAL_CALL ProgressBarWrapper::update()
throw (uno::RuntimeException)
{
    // dummy - do nothing
}

// XComponent
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
            catch ( lang::DisposedException& )
            {
            }
        }

        m_xStatusBar.clear();
        m_bDisposed = sal_True;
    }
}

// XUIElement
uno::Reference< uno::XInterface > SAL_CALL ProgressBarWrapper::getRealInterface()
throw (uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Ready for multithreading
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

}       //  namespace framework
