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

#include <sal/config.h>

#include <stdio.h>
#include <wchar.h>

#include <uielement/spinfieldtoolbarcontroller.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/toolbox.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace framework
{

// Wrapper class to notify controller about events from combobox.
// Unfortunaltly the events are notified through virtual methods instead
// of Listeners.

class SpinfieldControl : public SpinField
{
    public:
        SpinfieldControl( vcl::Window* pParent, WinBits nStyle, SpinfieldToolbarController* pSpinfieldToolbarController );
        virtual ~SpinfieldControl() override;
        virtual void dispose() override;

        virtual void Up() override;
        virtual void Down() override;
        virtual void First() override;
        virtual void Last() override;
        virtual void Modify() override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;

    private:
        SpinfieldToolbarController* m_pSpinfieldToolbarController;
};

SpinfieldControl::SpinfieldControl( vcl::Window* pParent, WinBits nStyle, SpinfieldToolbarController* pSpinfieldToolbarController ) :
    SpinField( pParent, nStyle )
    , m_pSpinfieldToolbarController( pSpinfieldToolbarController )
{
}

SpinfieldControl::~SpinfieldControl()
{
    disposeOnce();
}

void SpinfieldControl::dispose()
{
    m_pSpinfieldToolbarController = nullptr;
    SpinField::dispose();
}

void SpinfieldControl::Up()
{
    SpinField::Up();
    if ( m_pSpinfieldToolbarController )
        m_pSpinfieldToolbarController->Up();
}

void SpinfieldControl::Down()
{
    SpinField::Down();
    if ( m_pSpinfieldToolbarController )
        m_pSpinfieldToolbarController->Down();
}

void SpinfieldControl::First()
{
    SpinField::First();
    if ( m_pSpinfieldToolbarController )
        m_pSpinfieldToolbarController->First();
}

void SpinfieldControl::Last()
{
    SpinField::First();
    if ( m_pSpinfieldToolbarController )
        m_pSpinfieldToolbarController->Last();
}

void SpinfieldControl::Modify()
{
    SpinField::Modify();
    if ( m_pSpinfieldToolbarController )
        m_pSpinfieldToolbarController->Modify();
}

void SpinfieldControl::GetFocus()
{
    SpinField::GetFocus();
    if ( m_pSpinfieldToolbarController )
        m_pSpinfieldToolbarController->GetFocus();
}

void SpinfieldControl::LoseFocus()
{
    SpinField::GetFocus();
    if ( m_pSpinfieldToolbarController )
        m_pSpinfieldToolbarController->GetFocus();
}

bool SpinfieldControl::PreNotify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    if ( m_pSpinfieldToolbarController )
        bRet = m_pSpinfieldToolbarController->PreNotify( rNEvt );
    if ( !bRet )
        bRet = SpinField::PreNotify( rNEvt );

    return bRet;
}

SpinfieldToolbarController::SpinfieldToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_bFloat( false )
    ,   m_bMaxSet( false )
    ,   m_bMinSet( false )
    ,   m_nMax( 0.0 )
    ,   m_nMin( 0.0 )
    ,   m_nValue( 0.0 )
    ,   m_nStep( 0.0 )
    ,   m_pSpinfieldControl( nullptr )
{
    m_pSpinfieldControl = VclPtr<SpinfieldControl>::Create( m_pToolbar, WB_SPIN|WB_BORDER, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // Calculate height of the spin field according to the application font height
    sal_Int32 nHeight = getFontSizePixel( m_pSpinfieldControl ) + 5 + 1;

    m_pSpinfieldControl->SetSizePixel( ::Size( nWidth, nHeight ));
    m_pToolbar->SetItemWindow( m_nID, m_pSpinfieldControl );
}

SpinfieldToolbarController::~SpinfieldToolbarController()
{
}

void SAL_CALL SpinfieldToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_pToolbar->SetItemWindow( m_nID, nullptr );
    m_pSpinfieldControl.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> SpinfieldToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSpinfieldText = m_pSpinfieldControl->GetText();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Value";
    if ( m_bFloat )
        aArgs[1].Value <<= aSpinfieldText.toDouble();
    else
        aArgs[1].Value <<= aSpinfieldText.toInt32();
    return aArgs;
}

void SpinfieldToolbarController::Up()
{
    double nValue = m_nValue + m_nStep;
    if ( m_bMaxSet && nValue > m_nMax )
        return;

    m_nValue = nValue;

    OUString aText = impl_formatOutputString( m_nValue );
    m_pSpinfieldControl->SetText( aText );
    execute( 0 );
}

void SpinfieldToolbarController::Down()
{
    double nValue = m_nValue - m_nStep;
    if ( m_bMinSet && nValue < m_nMin )
        return;

    m_nValue = nValue;

    OUString aText = impl_formatOutputString( m_nValue );
    m_pSpinfieldControl->SetText( aText );
    execute( 0 );
}

void SpinfieldToolbarController::First()
{
    if ( m_bMinSet )
    {
        m_nValue = m_nMin;

        OUString aText = impl_formatOutputString( m_nValue );
        m_pSpinfieldControl->SetText( aText );
        execute( 0 );
    }
}

void SpinfieldToolbarController::Last()
{
    if ( m_bMaxSet )
    {
        m_nValue = m_nMax;

        OUString aText = impl_formatOutputString( m_nValue );
        m_pSpinfieldControl->SetText( aText );
        execute( 0 );
    }
}

void SpinfieldToolbarController::Modify()
{
    notifyTextChanged( m_pSpinfieldControl->GetText() );
}

void SpinfieldToolbarController::GetFocus()
{
    notifyFocusGet();
}

bool SpinfieldToolbarController::PreNotify( NotifyEvent const & rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const ::KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
        if(( rKeyCode.GetModifier() | rKeyCode.GetCode()) == KEY_RETURN )
        {
            // Call execute only with non-empty text
            if ( !m_pSpinfieldControl->GetText().isEmpty() )
                execute( rKeyCode.GetModifier() );
            return true;
        }
    }

    return false;
}

void SpinfieldToolbarController::executeControlCommand( const css::frame::ControlCommand& rControlCommand )
{
    OUString aValue;
    OUString aMax;
    OUString aMin;
    OUString aStep;
    bool          bFloatValue( false );

    if ( rControlCommand.Command == "SetStep" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Step" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );
                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                    aStep = bFloat ? OUString::number( fValue ) :
                                     OUString::number( nValue );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetValue" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Value" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );

                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                {
                    aValue = bFloat ? OUString::number( fValue ) : OUString::number( nValue );
                    bFloatValue = bFloat;
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetValues" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            sal_Int32   nValue;
            double      fValue;
            bool        bFloat( false );

            OUString aName = rControlCommand.Arguments[i].Name;
            if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
            {
                if ( aName == "Value" )
                {
                    aValue = bFloat ? OUString::number( fValue ) : OUString::number( nValue );
                    bFloatValue = bFloat;
                }
                else if ( aName == "Step" )
                    aStep = bFloat ? OUString::number( fValue ) :
                                     OUString::number( nValue );
                else if ( aName == "LowerLimit" )
                    aMin = bFloat ? OUString::number( fValue ) :
                                    OUString::number( nValue );
                else if ( aName == "UpperLimit" )
                    aMax = bFloat ? OUString::number( fValue ) :
                                    OUString::number( nValue );
            }
            else if ( aName == "OutputFormat" )
                rControlCommand.Arguments[i].Value >>= m_aOutFormat;
        }
    }
    else if ( rControlCommand.Command == "SetLowerLimit" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "LowerLimit" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );
                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                    aMin = bFloat ? OUString::number( fValue ) :
                                    OUString::number( nValue );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetUpperLimit" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "UpperLimit" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );
                if ( impl_getValue( rControlCommand.Arguments[i].Value, nValue, fValue, bFloat ))
                    aMax = bFloat ? OUString::number( fValue ) :
                                    OUString::number( nValue );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetOutputFormat" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "OutputFormat" )
            {
                rControlCommand.Arguments[i].Value >>= m_aOutFormat;
                break;
            }
        }
    }

    // Check values and set members
    if ( !aValue.isEmpty() )
    {
        m_bFloat = bFloatValue;
        m_nValue = aValue.toDouble();

        OUString aOutString = impl_formatOutputString( m_nValue );
        m_pSpinfieldControl->SetText( aOutString );
        notifyTextChanged( aOutString );
    }
    if ( !aMax.isEmpty() )
    {
        m_nMax = aMax.toDouble();
        m_bMaxSet = true;
    }
    if ( !aMin.isEmpty() )
    {
        m_nMin = aMin.toDouble();
        m_bMinSet = true;
    }
    if ( !aStep.isEmpty() )
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

OUString SpinfieldToolbarController::impl_formatOutputString( double fValue )
{
    if ( m_aOutFormat.isEmpty() )
    {
        if ( m_bFloat )
            return OUString::number( fValue );
        else
            return OUString::number( sal_Int32( fValue ));
    }
    else
    {
#ifdef _WIN32
        sal_Unicode aBuffer[128];

        aBuffer[0] = 0;
        if ( m_bFloat )
            _snwprintf( o3tl::toW(aBuffer), SAL_N_ELEMENTS(aBuffer), o3tl::toW(m_aOutFormat.getStr()), fValue );
        else
            _snwprintf( o3tl::toW(aBuffer), SAL_N_ELEMENTS(aBuffer), o3tl::toW(m_aOutFormat.getStr()), sal_Int32( fValue ));

        return aBuffer;
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
        OString aTmp( aBuffer, nSize );
        return OStringToOUString( aTmp, osl_getThreadTextEncoding() );
#endif
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
