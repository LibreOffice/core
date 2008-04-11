/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spinfieldtoolbarcontroller.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <stdio.h>
#include <wchar.h>

#ifndef __FRAMEWORK_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX
#include "uielement/spinfieldtoolbarcontroller.hxx"
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_TOOLBAR_HXX_
#include "uielement/toolbar.hxx"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <svtools/toolboxcontroller.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <tools/urlobj.hxx>
#ifdef WINNT
#include <systools/win32/snprintf.h>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace framework
{

// ------------------------------------------------------------------

// Wrapper class to notify controller about events from combobox.
// Unfortunaltly the events are notifed through virtual methods instead
// of Listeners.

class SpinfieldControl : public SpinField
{
    public:
        SpinfieldControl( Window* pParent, WinBits nStyle, ISpinfieldListener* pSpinFieldListener );
        virtual ~SpinfieldControl();

        virtual void Up();
        virtual void Down();
        virtual void First();
        virtual void Last();
        virtual void KeyInput( const ::KeyEvent& rKEvt );
        virtual void Modify();
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void StateChanged( StateChangedType nType );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        virtual long PreNotify( NotifyEvent& rNEvt );

    private:
        ISpinfieldListener* m_pSpinFieldListener;
};

SpinfieldControl::SpinfieldControl( Window* pParent, WinBits nStyle, ISpinfieldListener* pSpinFieldListener ) :
    SpinField( pParent, nStyle )
    , m_pSpinFieldListener( pSpinFieldListener )
{
}

SpinfieldControl::~SpinfieldControl()
{
    m_pSpinFieldListener = 0;
}

void SpinfieldControl::Up()
{
    SpinField::Up();
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->Up();
}

void SpinfieldControl::Down()
{
    SpinField::Down();
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->Down();
}

void SpinfieldControl::First()
{
    SpinField::First();
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->First();
}

void SpinfieldControl::Last()
{
    SpinField::First();
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->Last();
}

void SpinfieldControl::KeyInput( const ::KeyEvent& rKEvt )
{
    SpinField::KeyInput( rKEvt );
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->KeyInput( rKEvt );
}

void SpinfieldControl::Modify()
{
    SpinField::Modify();
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->Modify();
}

void SpinfieldControl::GetFocus()
{
    SpinField::GetFocus();
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->GetFocus();
}

void SpinfieldControl::LoseFocus()
{
    SpinField::GetFocus();
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->GetFocus();
}

void SpinfieldControl::StateChanged( StateChangedType nType )
{
    SpinField::StateChanged( nType );
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->StateChanged( nType );
}

void SpinfieldControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );
    if ( m_pSpinFieldListener )
        m_pSpinFieldListener->DataChanged( rDCEvt );
}

long SpinfieldControl::PreNotify( NotifyEvent& rNEvt )
{
    long nRet( 0 );
    if ( m_pSpinFieldListener )
        nRet = m_pSpinFieldListener->PreNotify( rNEvt );
    if ( nRet == 0 )
        nRet = SpinField::PreNotify( rNEvt );

    return nRet;
}

// ------------------------------------------------------------------

SpinfieldToolbarController::SpinfieldToolbarController(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    USHORT                                   nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rServiceManager, rFrame, pToolbar, nID, aCommand )
    ,   m_bFloat( false )
    ,   m_bMaxSet( false )
    ,   m_bMinSet( false )
    ,   m_nMax( 0.0 )
    ,   m_nMin( 0.0 )
    ,   m_nValue( 0.0 )
    ,   m_nStep( 0.0 )
    ,   m_pSpinfieldControl( 0 )
{
    m_pSpinfieldControl = new SpinfieldControl( m_pToolbar, WB_SPIN|WB_BORDER, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // Calculate height of the spin field according to the application font height
    sal_Int32 nHeight = getFontSizePixel( m_pSpinfieldControl ) + 5 + 1;

    m_pSpinfieldControl->SetSizePixel( ::Size( nWidth, nHeight ));
    m_pToolbar->SetItemWindow( m_nID, m_pSpinfieldControl );
}

// ------------------------------------------------------------------

SpinfieldToolbarController::~SpinfieldToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL SpinfieldToolbarController::dispose()
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    m_pToolbar->SetItemWindow( m_nID, 0 );
    delete m_pSpinfieldControl;

    ComplexToolbarController::dispose();

    m_pSpinfieldControl = 0;
}

// ------------------------------------------------------------------

void SAL_CALL SpinfieldToolbarController::execute( sal_Int16 KeyModifier )
throw ( RuntimeException )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    OUString                     aCommandURL;
    OUString                     aSpinfieldText;
    ::com::sun::star::util::URL  aTargetURL;
    bool                         bFloat( false );

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             m_aCommandURL.getLength() )
        {
            xURLTransformer = m_xURLTransformer;
            xDispatch = getDispatchFromCommand( m_aCommandURL );
            aCommandURL = m_aCommandURL;
            aTargetURL = getInitializedURL();
            aSpinfieldText = m_pSpinfieldControl->GetText();
            bFloat = m_bFloat;
        }
    }

    if ( xDispatch.is() && aTargetURL.Complete.getLength() > 0 )
    {
        Sequence<PropertyValue>   aArgs( 2 );

        // Add key modifier to argument list
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeyModifier" ));
        aArgs[0].Value <<= KeyModifier;
        aArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Value" ));

        // Use correct type
        if ( bFloat )
            aArgs[1].Value <<= aSpinfieldText.toDouble();
        else
            aArgs[1].Value <<= aSpinfieldText.toInt32();

        // Execute dispatch asynchronously
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch     = xDispatch;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = aArgs;
        Application::PostUserEvent( STATIC_LINK(0, ComplexToolbarController , ExecuteHdl_Impl), pExecuteInfo );
    }
}

// ------------------------------------------------------------------

void SpinfieldToolbarController::Up()
{
    double nValue = m_nValue + m_nStep;
    if ( m_bMaxSet && nValue > m_nMax )
        return;

    m_nValue = nValue;

    rtl::OUString aText = impl_formatOutputString( m_nValue );
    m_pSpinfieldControl->SetText( aText );
    execute( 0 );
}

void SpinfieldToolbarController::Down()
{
    double nValue = m_nValue - m_nStep;
    if ( m_bMinSet && nValue < m_nMin )
        return;

    m_nValue = nValue;

    rtl::OUString aText = impl_formatOutputString( m_nValue );
    m_pSpinfieldControl->SetText( aText );
    execute( 0 );
}

void SpinfieldToolbarController::First()
{
    if ( m_bMinSet )
    {
        m_nValue = m_nMin;

        rtl::OUString aText = impl_formatOutputString( m_nValue );
        m_pSpinfieldControl->SetText( aText );
        execute( 0 );
    }
}

void SpinfieldToolbarController::Last()
{
    if ( m_bMaxSet )
    {
        m_nValue = m_nMax;

        rtl::OUString aText = impl_formatOutputString( m_nValue );
        m_pSpinfieldControl->SetText( aText );
        execute( 0 );
    }
}

void SpinfieldToolbarController::Modify()
{
    notifyTextChanged( m_pSpinfieldControl->GetText() );
}

void SpinfieldToolbarController::KeyInput( const ::KeyEvent& /*rKEvt*/ )
{
}

void SpinfieldToolbarController::GetFocus()
{
    notifyFocusGet();
}

void SpinfieldToolbarController::LoseFocus()
{
    notifyFocusLost();
}

void SpinfieldToolbarController::StateChanged( StateChangedType /*nType*/ )
{
}

void SpinfieldToolbarController::DataChanged( const DataChangedEvent& /*rDCEvt*/ )
{
}

long SpinfieldToolbarController::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const ::KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
        const KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
        if(( rKeyCode.GetModifier() | rKeyCode.GetCode()) == KEY_RETURN )
        {
            // Call execute only with non-empty text
            if ( m_pSpinfieldControl->GetText().Len() > 0 )
                execute( rKeyCode.GetModifier() );
            return 1;
        }
    }

    return 0;
}

// --------------------------------------------------------

void SpinfieldToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    rtl::OUString aValue;
    rtl::OUString aMax;
    rtl::OUString aMin;
    rtl::OUString aStep;
    bool          bFloatValue( false );

    if ( rControlCommand.Command.equalsAsciiL( "SetStep", 7 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            sal_Int32   nValue;
            double      fValue;
            bool        bFloat( false );

            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Step", 4 ))
            {
                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                    aStep = bFloat ? ::rtl::OUString::valueOf( fValue ) :
                                     ::rtl::OUString::valueOf( nValue );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetValue", 8 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Value", 5 ))
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );

                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                {
                    aValue = bFloat ? ::rtl::OUString::valueOf( fValue ) : ::rtl::OUString::valueOf( nValue );
                    bFloatValue = bFloat;
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetValues", 9 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            sal_Int32   nValue;
            double      fValue;
            bool        bFloat( false );

            rtl::OUString aName = rControlCommand.Arguments[i].Name;
            if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
            {
                if ( aName.equalsAsciiL( "Value", 5 ))
                {
                    aValue = bFloat ? ::rtl::OUString::valueOf( fValue ) : ::rtl::OUString::valueOf( nValue );
                    bFloatValue = bFloat;
                }
                else if ( aName.equalsAsciiL( "Step", 4 ))
                    aStep = bFloat ? ::rtl::OUString::valueOf( fValue ) :
                                     ::rtl::OUString::valueOf( nValue );
                else if ( aName.equalsAsciiL( "LowerLimit", 10 ))
                    aMin = bFloat ? ::rtl::OUString::valueOf( fValue ) :
                                    ::rtl::OUString::valueOf( nValue );
                else if ( aName.equalsAsciiL( "UpperLimit", 10 ))
                    aMax = bFloat ? ::rtl::OUString::valueOf( fValue ) :
                                    ::rtl::OUString::valueOf( nValue );
            }
            else if ( aName.equalsAsciiL( "OutputFormat", 12 ))
                rControlCommand.Arguments[i].Value >>= m_aOutFormat;
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetLowerLimit", 13 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            sal_Int32   nValue;
            double      fValue;
            bool        bFloat( false );

            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "LowerLimit", 10 ))
            {
                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                    aMin = bFloat ? ::rtl::OUString::valueOf( fValue ) :
                                    ::rtl::OUString::valueOf( nValue );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetUpperLimit", 13 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            sal_Int32   nValue;
            double      fValue;
            bool        bFloat( false );

            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "UpperLimit", 10 ))
            {
                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                    aMax = bFloat ? ::rtl::OUString::valueOf( fValue ) :
                                    ::rtl::OUString::valueOf( nValue );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetOutputFormat", 15 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "OutputFormat", 10 ))
            {
                rControlCommand.Arguments[i].Value >>= m_aOutFormat;
                break;
            }
        }
    }

    // Check values and set members
    if ( aValue.getLength() > 0 )
    {
        m_bFloat = bFloatValue;
        m_nValue = aValue.toDouble();

        rtl::OUString aOutString = impl_formatOutputString( m_nValue );
        m_pSpinfieldControl->SetText( aOutString );
        notifyTextChanged( aOutString );
    }
    if ( aMax.getLength() > 0 )
    {
        m_nMax = aMax.toDouble();
        m_bMaxSet = true;
    }
    if ( aMin.getLength() > 0 )
    {
        m_nMin = aMin.toDouble();
        m_bMinSet = true;
    }
    if ( aStep.getLength() > 0 )
        m_nStep = aStep.toDouble();
}

bool SpinfieldToolbarController::impl_getValue(
    const Any& rAny, sal_Int32& nValue, double& fValue, bool& bFloat )
{
    using ::com::sun::star::uno::TypeClass;

    bool bValueValid( false );

    bFloat = false;
    TypeClass aTypeClass = rAny.getValueTypeClass();
    if (( aTypeClass == TypeClass( typelib_TypeClass_LONG  )) ||
        ( aTypeClass == TypeClass( typelib_TypeClass_SHORT )) ||
        ( aTypeClass == TypeClass( typelib_TypeClass_BYTE  )))
        bValueValid = rAny >>= nValue;
    else if (( aTypeClass == TypeClass( typelib_TypeClass_FLOAT  )) ||
             ( aTypeClass == TypeClass( typelib_TypeClass_DOUBLE )))
    {
        bValueValid = rAny >>= fValue;
        bFloat = true;
    }

    return bValueValid;
}

rtl::OUString SpinfieldToolbarController::impl_formatOutputString( double fValue )
{
    if ( m_aOutFormat.getLength() == 0 )
    {
        if ( m_bFloat )
            return rtl::OUString::valueOf( fValue );
        else
            return rtl::OUString::valueOf( sal_Int32( fValue ));
    }
    else
    {
#ifdef WNT
        sal_Unicode aBuffer[128];

        aBuffer[0] = 0;
        if ( m_bFloat )
            snwprintf( reinterpret_cast<wchar_t *>(aBuffer), 128, reinterpret_cast<const wchar_t *>(m_aOutFormat.getStr()), fValue );
        else
            snwprintf( reinterpret_cast<wchar_t *>(aBuffer), 128, reinterpret_cast<const wchar_t *>(m_aOutFormat.getStr()), sal_Int32( fValue ));

        sal_Int32 nSize = rtl_ustr_getLength( aBuffer );
        return rtl::OUString( aBuffer, nSize );
#else
        // Currently we have no support for a format string using sal_Unicode. wchar_t
        // is 32 bit on Unix platform!
        char aBuffer[128];

        OString aFormat = OUStringToOString( m_aOutFormat, osl_getThreadTextEncoding() );
        if ( m_bFloat )
            snprintf( aBuffer, 128, aFormat.getStr(), fValue );
        else
            snprintf( aBuffer, 128, aFormat.getStr(), static_cast<long>( fValue ));

        sal_Int32 nSize = strlen( aBuffer );
        rtl::OString aTmp( aBuffer, nSize );
        return rtl::OStringToOUString( aTmp, osl_getThreadTextEncoding() );
#endif
    }
}

} // namespace
