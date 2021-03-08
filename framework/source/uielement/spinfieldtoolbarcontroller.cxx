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

#include <uielement/spinfieldtoolbarcontroller.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/event.hxx>
#include <vcl/formatter.hxx>
#include <vcl/svapp.hxx>
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

class SpinfieldControl final : public InterimItemWindow
{
public:
    SpinfieldControl(vcl::Window* pParent, SpinfieldToolbarController* pSpinfieldToolbarController);
    virtual ~SpinfieldControl() override;
    virtual void dispose() override;

    Formatter& GetFormatter()
    {
        return m_xWidget->GetFormatter();
    }

    OUString get_entry_text() const { return m_xWidget->get_text(); }

    DECL_LINK(ValueChangedHdl, weld::FormattedSpinButton&, void);
    DECL_LINK(FormatOutputHdl, LinkParamNone*, bool);
    DECL_LINK(ParseInputHdl, sal_Int64*, TriState);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);
    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_LINK(KeyInputHdl, const ::KeyEvent&, bool);

private:
    std::unique_ptr<weld::FormattedSpinButton> m_xWidget;
    SpinfieldToolbarController* m_pSpinfieldToolbarController;
};

SpinfieldControl::SpinfieldControl(vcl::Window* pParent, SpinfieldToolbarController* pSpinfieldToolbarController)
    : InterimItemWindow(pParent, "svt/ui/spinfieldcontrol.ui", "SpinFieldControl")
    , m_xWidget(m_xBuilder->weld_formatted_spin_button("spinbutton"))
    , m_pSpinfieldToolbarController(pSpinfieldToolbarController)
{
    InitControlBase(m_xWidget.get());

    m_xWidget->connect_focus_in(LINK(this, SpinfieldControl, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, SpinfieldControl, FocusOutHdl));
    Formatter& rFormatter = m_xWidget->GetFormatter();
    rFormatter.SetOutputHdl(LINK(this, SpinfieldControl, FormatOutputHdl));
    rFormatter.SetInputHdl(LINK(this, SpinfieldControl, ParseInputHdl));
    m_xWidget->connect_value_changed(LINK(this, SpinfieldControl, ValueChangedHdl));
    m_xWidget->connect_changed(LINK(this, SpinfieldControl, ModifyHdl));
    m_xWidget->connect_activate(LINK(this, SpinfieldControl, ActivateHdl));
    m_xWidget->connect_key_press(LINK(this, SpinfieldControl, KeyInputHdl));

    // so a later narrow size request can stick
    m_xWidget->set_width_chars(3);
    m_xWidget->set_size_request(42, -1);

    SetSizePixel(get_preferred_size());
}

IMPL_LINK(SpinfieldControl, KeyInputHdl, const ::KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

IMPL_LINK(SpinfieldControl, ParseInputHdl, sal_Int64*, result, TriState)
{
    *result = m_xWidget->get_text().toDouble() * weld::SpinButton::Power10(m_xWidget->GetFormatter().GetDecimalDigits());
    return TRISTATE_TRUE;
}

SpinfieldControl::~SpinfieldControl()
{
    disposeOnce();
}

void SpinfieldControl::dispose()
{
    m_pSpinfieldToolbarController = nullptr;
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

IMPL_LINK_NOARG(SpinfieldControl, ValueChangedHdl, weld::FormattedSpinButton&, void)
{
    if (m_pSpinfieldToolbarController)
        m_pSpinfieldToolbarController->execute(0);
}

IMPL_LINK_NOARG(SpinfieldControl, ModifyHdl, weld::Entry&, void)
{
    if (m_pSpinfieldToolbarController)
        m_pSpinfieldToolbarController->Modify();
}

IMPL_LINK_NOARG(SpinfieldControl, FocusInHdl, weld::Widget&, void)
{
    if (m_pSpinfieldToolbarController)
        m_pSpinfieldToolbarController->GetFocus();
}

IMPL_LINK_NOARG(SpinfieldControl, FocusOutHdl, weld::Widget&, void)
{
    if (m_pSpinfieldToolbarController)
        m_pSpinfieldToolbarController->LoseFocus();
}

IMPL_LINK_NOARG(SpinfieldControl, ActivateHdl, weld::Entry&, bool)
{
    bool bConsumed = false;
    if (m_pSpinfieldToolbarController)
    {
        m_pSpinfieldToolbarController->Activate();
        bConsumed = true;
    }
    return bConsumed;
}

IMPL_LINK_NOARG(SpinfieldControl, FormatOutputHdl, LinkParamNone*, bool)
{
    OUString aText = m_pSpinfieldToolbarController->FormatOutputString(m_xWidget->GetFormatter().GetValue());
    m_xWidget->set_text(aText);
    return true;
}

SpinfieldToolbarController::SpinfieldToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    ToolBoxItemId                            nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_bFloat( false )
    ,   m_nMax( 0.0 )
    ,   m_nMin( 0.0 )
    ,   m_nValue( 0.0 )
    ,   m_nStep( 0.0 )
    ,   m_pSpinfieldControl( nullptr )
{
    m_pSpinfieldControl = VclPtr<SpinfieldControl>::Create(m_xToolbar, this);
    if ( nWidth == 0 )
        nWidth = 100;

    // SpinFieldControl ctor has set a suitable height already
    auto nHeight = m_pSpinfieldControl->GetSizePixel().Height();

    m_pSpinfieldControl->SetSizePixel( ::Size( nWidth, nHeight ));
    m_xToolbar->SetItemWindow( m_nID, m_pSpinfieldControl );
}

SpinfieldToolbarController::~SpinfieldToolbarController()
{
}

void SAL_CALL SpinfieldToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_xToolbar->SetItemWindow( m_nID, nullptr );
    m_pSpinfieldControl.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> SpinfieldToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSpinfieldText = m_pSpinfieldControl->get_entry_text();

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

void SpinfieldToolbarController::Modify()
{
    notifyTextChanged(m_pSpinfieldControl->get_entry_text());
}

void SpinfieldToolbarController::GetFocus()
{
    notifyFocusGet();
}

void SpinfieldToolbarController::LoseFocus()
{
    notifyFocusLost();
}

void SpinfieldToolbarController::Activate()
{
    // Call execute only with non-empty text
    if (!m_pSpinfieldControl->get_entry_text().isEmpty())
        execute(0);
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
        for ( auto const & arg : rControlCommand.Arguments )
        {
            if ( arg.Name == "Step" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );
                if ( impl_getValue( arg.Value, nValue, fValue, bFloat ))
                    aStep = bFloat ? OUString( OUString::number( fValue )) :
                                     OUString( OUString::number( nValue ));
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetValue" )
    {
        for ( auto const & arg : rControlCommand.Arguments )
        {
            if ( arg.Name == "Value" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );

                if ( impl_getValue( arg.Value, nValue, fValue, bFloat ))
                {
                    aValue = bFloat ? OUString( OUString::number( fValue )) :
                                      OUString( OUString::number( nValue ));
                    bFloatValue = bFloat;
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetValues" )
    {
        for ( auto const & arg : rControlCommand.Arguments )
        {
            sal_Int32   nValue;
            double      fValue;
            bool        bFloat( false );

            OUString aName = arg.Name;
            if ( impl_getValue( arg.Value, nValue, fValue, bFloat ))
            {
                if ( aName == "Value" )
                {
                    aValue = bFloat ? OUString( OUString::number( fValue )) :
                                      OUString( OUString::number( nValue ));
                    bFloatValue = bFloat;
                }
                else if ( aName == "Step" )
                    aStep = bFloat ? OUString( OUString::number( fValue )) :
                                     OUString( OUString::number( nValue ));
                else if ( aName == "LowerLimit" )
                    aMin = bFloat ? OUString( OUString::number( fValue )) :
                                    OUString( OUString::number( nValue ));
                else if ( aName == "UpperLimit" )
                    aMax = bFloat ? OUString( OUString::number( fValue )) :
                                    OUString( OUString::number( nValue ));
            }
            else if ( aName == "OutputFormat" )
                arg.Value >>= m_aOutFormat;
        }
    }
    else if ( rControlCommand.Command == "SetLowerLimit" )
    {
        for ( auto const & arg : rControlCommand.Arguments )
        {
            if ( arg.Name == "LowerLimit" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );
                if ( impl_getValue( arg.Value, nValue, fValue, bFloat ))
                    aMin = bFloat ? OUString( OUString::number( fValue )) :
                                    OUString( OUString::number( nValue ));
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetUpperLimit" )
    {
        for ( auto const & arg : rControlCommand.Arguments )
        {
            if ( arg.Name == "UpperLimit" )
            {
                sal_Int32   nValue;
                double      fValue;
                bool        bFloat( false );
                if ( impl_getValue( arg.Value, nValue, fValue, bFloat ))
                    aMax = bFloat ? OUString( OUString::number( fValue )) :
                                    OUString( OUString::number( nValue ));
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetOutputFormat" )
    {
        for ( auto const & arg : rControlCommand.Arguments )
        {
            if ( arg.Name == "OutputFormat" )
            {
                arg.Value >>= m_aOutFormat;
                break;
            }
        }
    }

    Formatter& rFormatter = m_pSpinfieldControl->GetFormatter();

    // Check values and set members
    if (bFloatValue)
        rFormatter.SetDecimalDigits(2);
    if ( !aValue.isEmpty() )
    {
        m_bFloat = bFloatValue;
        m_nValue = aValue.toDouble();
        rFormatter.SetValue(m_nValue);
    }
    if ( !aMax.isEmpty() )
    {
        m_nMax = aMax.toDouble();
        rFormatter.SetMaxValue(m_nMax);
    }
    if ( !aMin.isEmpty() )
    {
        m_nMin = aMin.toDouble();
        rFormatter.SetMinValue(m_nMin);
    }
    if ( !aStep.isEmpty() )
    {
        m_nStep = aStep.toDouble();
        rFormatter.SetSpinSize(m_nStep);
    }
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

OUString SpinfieldToolbarController::FormatOutputString( double fValue )
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

        return OUString(aBuffer);
#else
        // Currently we have no support for a format string using sal_Unicode. wchar_t
        // is 32 bit on Unix platform!
        char aBuffer[128];

        OString aFormat = OUStringToOString( m_aOutFormat, osl_getThreadTextEncoding() );
        if ( m_bFloat )
            snprintf( aBuffer, 128, aFormat.getStr(), fValue );
        else
            snprintf( aBuffer, 128, aFormat.getStr(), static_cast<tools::Long>( fValue ));

        sal_Int32 nSize = strlen( aBuffer );
        OString aTmp( aBuffer, nSize );
        return OStringToOUString( aTmp, osl_getThreadTextEncoding() );
#endif
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
