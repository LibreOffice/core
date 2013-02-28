/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "limitboxcontroller.hxx"

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <osl/mutex.hxx>

#include "LimitBox.hxx"
#include "dbu_reghelper.hxx"
#include "moduledbu.hxx"


using namespace ::com::sun::star;

namespace dbaui
{

class LimitBoxImpl: public LimitBox
{
    public:
        LimitBoxImpl( Window* pParent, LimitBoxController* pCtrl );
        virtual ~LimitBoxImpl();

        virtual long    Notify( NotifyEvent& rNEvt );

    private:
        LimitBoxController* m_pControl;
};

LimitBoxImpl::LimitBoxImpl( Window* pParent, LimitBoxController* pCtrl )
    : LimitBox( pParent, WinBits( WB_DROPDOWN | WB_VSCROLL) )
    , m_pControl( pCtrl )
{
}

LimitBoxImpl::~LimitBoxImpl()
{
}

long LimitBoxImpl::Notify( NotifyEvent& rNEvt )
{
    switch ( rNEvt.GetType() )
    {
        case EVENT_LOSEFOCUS:
        {
            uno::Sequence< beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = OUString( "DBLimit.Value" );
            aArgs[0].Value = uno::makeAny( GetValue() );
            m_pControl->dispatchCommand( aArgs );
            break;
        }
    }
    return LimitBox::Notify( rNEvt );
}


LimitBoxController::LimitBoxController(
    const uno::Reference< lang::XMultiServiceFactory >& rServiceManager ) :
    svt::ToolboxController( rServiceManager,
                            uno::Reference< frame::XFrame >(),
                            OUString( ".uno:DBLimit" ) ),
    m_pLimitBox( NULL )
{
}

LimitBoxController::~LimitBoxController()
{
}

/// XInterface
uno::Any SAL_CALL LimitBoxController::queryInterface( const uno::Type& aType )
throw (uno::RuntimeException)
{
    uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ));
}

void SAL_CALL LimitBoxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL LimitBoxController::release() throw ()
{
    ToolboxController::release();
}


/// XServiceInfo
IMPLEMENT_SERVICE_INFO1_STATIC(LimitBoxController,"org.libreoffice.comp.dbu.LimitBoxController","com.sun.star.frame.ToolboxController")

/// XComponent
void SAL_CALL LimitBoxController::dispose()
throw (uno::RuntimeException)
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    delete m_pLimitBox;
    m_pLimitBox = 0;
}

/// XStatusListener
void SAL_CALL LimitBoxController::statusChanged(
    const frame::FeatureStateEvent& rEvent )
throw ( uno::RuntimeException )
{
    if ( m_pLimitBox )
    {
        SolarMutexGuard aSolarMutexGuard;
        if ( rEvent.FeatureURL.Path == "DBLimit" )
        {
            if ( rEvent.IsEnabled )
            {
                m_pLimitBox->Enable();
                sal_Int64 nLimit = 0;
                if ( (rEvent.State >>= nLimit) )
                {
                    m_pLimitBox->SetValue( nLimit );
                }
            }
            else
                m_pLimitBox->Disable();
        }
    }
}

/// XToolbarController
void SAL_CALL LimitBoxController::execute( sal_Int16 /*KeyModifier*/ )
throw (uno::RuntimeException)
{
}

void SAL_CALL LimitBoxController::click()
throw (uno::RuntimeException)
{
}

void SAL_CALL LimitBoxController::doubleClick()
throw (uno::RuntimeException)
{
}

uno::Reference< awt::XWindow > SAL_CALL LimitBoxController::createPopupWindow()
throw (uno::RuntimeException)
{
    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL LimitBoxController::createItemWindow(
    const uno::Reference< awt::XWindow >& Parent )
    throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xItemWindow;
    uno::Reference< awt::XWindow > xParent( Parent );

    Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        SolarMutexGuard aSolarMutexGuard;
        m_pLimitBox = new LimitBoxImpl(pParent, this);
        m_pLimitBox->SetSizePixel(m_pLimitBox->GetOptimalSize());
        xItemWindow = VCLUnoHelper::GetInterface( m_pLimitBox );
    }

    return xItemWindow;
}

void LimitBoxController::dispatchCommand(
    const uno::Sequence< beans::PropertyValue >& rArgs )
{
    uno::Reference< frame::XDispatchProvider > xDispatchProvider( m_xFrame, uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        util::URL                               aURL;
        uno::Reference< frame::XDispatch >      xDispatch;
        uno::Reference< util::XURLTransformer > xURLTransformer = getURLTransformer();

        aURL.Complete = OUString( ".uno:DBLimit" );
        xURLTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, rArgs );
    }
}

} ///dbaui namespace

extern "C" void SAL_CALL createRegistryInfo_LimitBoxController()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::LimitBoxController > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
