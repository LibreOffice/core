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
#include <com/sun/star/i18n/NativeNumberMode.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include "dbu_reghelper.hxx"
#include "dbu_qry.hrc"
#include "moduledbu.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n::NativeNumberMode;


////////////////
///LimitBox
////////////////

namespace dbaui
{

namespace global{

/// Default values
OUString aDefLimitAry[] =
{
    "5",
    "10",
    "20",
    "50"
};

}

LimitBox::LimitBox( Window* pParent, LimitBoxController* pCtrl )
    : ComboBox( pParent, WinBits( WB_DROPDOWN | WB_VSCROLL) )
    , m_pControl( pCtrl )
    , m_aNatNumWrapper( Application::GetAppLocaleDataWrapper().getComponentContext() )
    , m_aLocale( Application::GetSettings().GetLanguageTag().getLocale() )
    , m_bUseNativeNumbers( m_aNatNumWrapper.isValidNatNum(m_aLocale, NATNUM1) )
{
    LoadDefaultLimits();
    EnableAutocomplete(sal_False);
    Size aSize(
        CalcMinimumSize().Width() + 30 ,
        CalcWindowSizePixel(GetEntryCount() + 1) );
    SetSizePixel(aSize);
}

LimitBox::~LimitBox()
{
}

long LimitBox::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    switch ( rNEvt.GetType() )
    {
        /**
         * Check the current data
         * If it can a limit value than dispatch it
         * Else set the previous data (checked value)
         */
        case EVENT_LOSEFOCUS:
        {
            OUString sText = GetText();
            if ( CheckAndConvertData(sText) )
            {
                uno::Sequence< beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name  = OUString( "DBLimit.Value" );
                aArgs[0].Value = uno::makeAny( sText );
                m_pControl->dispatchCommand( aArgs );
            }
            else
            {
                Undo();
            }
            nHandled = 1;
            break;
        }
    }
    return nHandled ? nHandled : ComboBox::Notify( rNEvt );
}

///Convert text to native number if it is neccessary
void LimitBox::SetText( const OUString& rStr )
{
    if ( m_bUseNativeNumbers )
    {
        ComboBox::SetText( AsciiToNatNum(rStr) );
    }
    else
        ComboBox::SetText( rStr );

}

///This method was hidden by the previous SetText() method so override this too
void LimitBox::SetText( const OUString& rStr, const Selection& rNewSelection )
{
    ComboBox::SetText( rStr, rNewSelection );
}

///Initialize entries
void LimitBox::LoadDefaultLimits()
{
    const OUString sAll = ModuleRes(STR_QUERY_LIMIT_ALL);
    ComboBox::SetText( sAll );
    InsertEntry( sAll );

    const unsigned nSize =
        sizeof(global::aDefLimitAry)/sizeof(global::aDefLimitAry[0]);
    for( unsigned nIndex = 0; nIndex< nSize; ++nIndex)
    {
        InsertEntry(
            AsciiToNatNum( global::aDefLimitAry[nIndex] ));
    }
}

/**
 * Convert ascii number string to native numeral string
 * due to the locale settings
 */
inline OUString LimitBox::AsciiToNatNum( const OUString& rAsciiNum ) const
{
    return m_aNatNumWrapper.getNativeNumberString(rAsciiNum, m_aLocale, NATNUM1);
}

/**
 * Convert native numeral string to ascii number string
 * due to the locale settings
 */
inline OUString LimitBox::NatNumToAscii( const OUString& rNatNum ) const
{
    return m_aNatNumWrapper.getNativeNumberString(rNatNum, m_aLocale, NATNUM0);
}


/**
 * Check input string whether it can be a limit value
 * and transilaterate it to ascii number string (if it is native)
 */
bool LimitBox::CheckAndConvertData( OUString& io_rData ) const
{
    if( io_rData.isEmpty() )
    {
        return false;
    }
    if( io_rData == String(ModuleRes(STR_QUERY_LIMIT_ALL)) )
    {
        return true;
    }

    ///Convert native number
    if( m_bUseNativeNumbers )
    {
        /**
         * If data contains ascii digits than this information will loose
         * by conertion so it must be checked before.
         */
        sal_Int32 nIndex = 0;
        sal_uInt32 nChar;
        while( nIndex < io_rData.getLength() )
        {
            nChar = io_rData.iterateCodePoints( &nIndex );
            if( nChar >= '0' && nChar <= '9' )
            {
                return false;
            }
        }
        io_rData = NatNumToAscii(io_rData);
    }
    ///Check ascii digits
    if( io_rData[0] == '0' && io_rData.getLength() > 1 )
        return false;

    sal_Int32 nIndex = 0;
    sal_uInt32 nChar;
    while( nIndex < io_rData.getLength() )
    {
        nChar = io_rData.iterateCodePoints( &nIndex );
        if( nChar < '0' || nChar > '9' )
        {
            return false;
        }
    }
    return true;

}


/////////////////////////
///LimitBoxController
/////////////////////////

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
                OUString sLimit;
                if ( (rEvent.State >>= sLimit) && !sLimit.isEmpty()  )
                {
                    m_pLimitBox->SetText( sLimit );
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
        m_pLimitBox = new LimitBox(pParent, this);
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
