/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "limitboxcontroller.hxx"
#include "uiservices.hxx"

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/processfactory.hxx>

#include "LimitBox.hxx"
#include "dbu_reghelper.hxx"
#include "moduledbu.hxx"


using namespace ::com::sun::star;

namespace dbaui
{

class LimitBoxImpl: public LimitBox
{
    public:
        LimitBoxImpl( vcl::Window* pParent, LimitBoxController* pCtrl );

        virtual bool Notify( NotifyEvent& rNEvt ) override;

    private:
        LimitBoxController* m_pControl;
};

LimitBoxImpl::LimitBoxImpl( vcl::Window* pParent, LimitBoxController* pCtrl )
    : LimitBox( pParent, WinBits( WB_DROPDOWN | WB_VSCROLL) )
    , m_pControl( pCtrl )
{
}

bool LimitBoxImpl::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    switch ( rNEvt.GetType() )
    {
        case MouseNotifyEvent::LOSEFOCUS:
        {
            bHandled = LimitBox::Notify( rNEvt );
            uno::Sequence< beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = "DBLimit.Value";
            aArgs[0].Value = uno::makeAny( GetValue() );
            m_pControl->dispatchCommand( aArgs );
            break;
        }
        case MouseNotifyEvent::KEYINPUT:
        {
            const sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
            switch ( nCode )
            {
                case KEY_ESCAPE:
                    Undo();
                    SAL_FALLTHROUGH;
                case KEY_RETURN:
                    GrabFocusToDocument();
                    bHandled = true;
                    break;
                case KEY_TAB:
                    Select();
                    break;
            }
            break;
        }
        default:
            break;
    }
    return bHandled || LimitBox::Notify( rNEvt );
}


LimitBoxController::LimitBoxController(
    const uno::Reference< uno::XComponentContext >& rxContext ) :
    svt::ToolboxController( rxContext,
                            uno::Reference< frame::XFrame >(),
                            OUString( ".uno:DBLimit" ) ),
    m_pLimitBox( nullptr )
{
}

LimitBoxController::~LimitBoxController()
{
}

/// XInterface
uno::Any SAL_CALL LimitBoxController::queryInterface( const uno::Type& aType )
throw (uno::RuntimeException, std::exception)
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
IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(LimitBoxController, "org.libreoffice.comp.dbu.LimitBoxController")
IMPLEMENT_SERVICE_INFO_SUPPORTS(LimitBoxController)
IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(LimitBoxController, "com.sun.star.frame.ToolbarController")

uno::Reference< uno::XInterface >
    SAL_CALL LimitBoxController::Create(const uno::Reference< css::lang::XMultiServiceFactory >& _rxORB)
{
    return static_cast< XServiceInfo* >(new LimitBoxController( comphelper::getComponentContext(_rxORB) ));
}

/// XComponent
void SAL_CALL LimitBoxController::dispose()
throw (uno::RuntimeException, std::exception)
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    m_pLimitBox.disposeAndClear();
}

/// XStatusListener
void SAL_CALL LimitBoxController::statusChanged(
    const frame::FeatureStateEvent& rEvent )
throw ( uno::RuntimeException, std::exception )
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
throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL LimitBoxController::click()
throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL LimitBoxController::doubleClick()
throw (uno::RuntimeException, std::exception)
{
}

uno::Reference< awt::XWindow > SAL_CALL LimitBoxController::createPopupWindow()
throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL LimitBoxController::createItemWindow(
    const uno::Reference< awt::XWindow >& Parent )
    throw (uno::RuntimeException, std::exception)
{
    uno::Reference< awt::XWindow > xItemWindow;
    uno::Reference< awt::XWindow > xParent( Parent );

    vcl::Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        SolarMutexGuard aSolarMutexGuard;
        m_pLimitBox = VclPtr<LimitBoxImpl>::Create(pParent, this);
        m_pLimitBox->SetSizePixel(m_pLimitBox->CalcBlockSize(6,1));
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

        aURL.Complete = ".uno:DBLimit";
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
