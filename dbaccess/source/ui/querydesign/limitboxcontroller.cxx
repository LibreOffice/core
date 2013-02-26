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
#include <rtl/ustring.hxx>

#include "dbu_reghelper.hxx"
#include "dbu_qry.hrc"
#include "moduledbu.hxx"

#define ALL_STRING ModuleRes(STR_QUERY_LIMIT_ALL).toString()
#define ALL_INT -1

using namespace ::com::sun::star;

////////////////
///LimitBox
////////////////

namespace dbaui
{

namespace global{

/// Default values
sal_Int64 aDefLimitAry[] =
{
    5,
    10,
    20,
    50
};

}


LimitBox::LimitBox( Window* pParent, LimitBoxController* pCtrl )
    : NumericBox( pParent, WinBits( WB_DROPDOWN | WB_VSCROLL) )
    , m_pControl( pCtrl )
{
    SetShowTrailingZeros( sal_False );
    SetDecimalDigits( 0 );
    SetMin( -1 );
    SetMax( 9999 );
    LoadDefaultLimits();

    Size aSize(
        CalcMinimumSize().Width() + 20 ,
        CalcWindowSizePixel(GetEntryCount() + 1) );
    SetSizePixel(aSize);
}

LimitBox::~LimitBox()
{
}

void LimitBox::Reformat()
{

    if( GetText() == ALL_STRING )
    {
        SetValue( -1 );
    }
    ///Reformat only when text is not All
    else
    {
        ///Not allow user to type -1
        if( GetText() == "-1" )
        {
            Undo();
        }
        else
            NumericBox::Reformat();
    }
}

void LimitBox::ReformatAll()
{
    ///First entry is All, which do not need numeric reformat
    if ( GetEntryCount() > 0 )
    {
        RemoveEntry( 0 );
        NumericBox::ReformatAll();
        InsertEntry( ALL_STRING, 0);
    }
    else
    {
        NumericBox::ReformatAll();
    }
}

OUString LimitBox::CreateFieldText( sal_Int64 nValue ) const
{
    if( nValue == ALL_INT )
        return ALL_STRING;
    else
        return NumericBox::CreateFieldText( nValue );
}

long LimitBox::Notify( NotifyEvent& rNEvt )
{
    long nReturn = NumericBox::Notify( rNEvt );

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
        case EVENT_KEYINPUT:
        {
            const sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
            if( nCode == KEY_RETURN )
            {
                GrabFocusToDocument();
            }
            break;
        }
    }
    return nReturn;
}

///Initialize entries
void LimitBox::LoadDefaultLimits()
{
    SetValue( ALL_INT );
    InsertEntry( ALL_STRING );

    const unsigned nSize =
        sizeof(global::aDefLimitAry)/sizeof(global::aDefLimitAry[0]);
    for( unsigned nIndex = 0; nIndex< nSize; ++nIndex)
    {
        InsertValue( global::aDefLimitAry[nIndex] );
    }
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
