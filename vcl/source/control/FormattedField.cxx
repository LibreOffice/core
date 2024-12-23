/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <rtl/math.hxx>
#include <svl/numformat.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/debug.hxx>

#include <vcl/builder.hxx>
#include <vcl/event.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/toolkit/fmtfield.hxx>
#include <vcl/uitest/formattedfielduiobject.hxx>
#include <vcl/weldutils.hxx>

#include "FieldFormatter.hxx"

#include <svl/zformat.hxx>

#include <limits>

FormattedField::FormattedField(vcl::Window* pParent, WinBits nStyle)
    : SpinField(pParent, nStyle, WindowType::FORMATTEDFIELD)
    , m_pFormatter(nullptr)
{
}

void FormattedField::dispose()
{
    m_pFormatter = nullptr;
    m_xOwnFormatter.reset();
    SpinField::dispose();
}

void FormattedField::SetText(const OUString& rStr)
{
    GetFormatter().SetFieldText(rStr, Selection(0, 0));
}

void FormattedField::SetText(const OUString& rStr, const Selection& rNewSelection)
{
    GetFormatter().SetFieldText(rStr, rNewSelection);
    SetSelection(rNewSelection);
}

bool FormattedField::set_property(const OUString &rKey, const OUString &rValue)
{
    if (rKey == "digits")
        GetFormatter().SetDecimalDigits(rValue.toInt32());
    else if (rKey == "wrap")
        GetFormatter().SetWrapOnLimits(toBool(rValue));
    else
        return SpinField::set_property(rKey, rValue);
    return true;
}

void FormattedField::Up()
{
    Formatter& rFormatter = GetFormatter();
    auto nScale = weld::SpinButton::Power10(rFormatter.GetDecimalDigits());

    sal_Int64 nValue = std::round(rFormatter.GetValue() * nScale);
    sal_Int64 nSpinSize = std::round(rFormatter.GetSpinSize() * nScale);
    assert(nSpinSize != 0);
    sal_Int64 nRemainder = rFormatter.GetDisableRemainderFactor() || nSpinSize == 0 ? 0 : nValue % nSpinSize;
    if (nValue >= 0)
        nValue = (nRemainder == 0) ? nValue + nSpinSize : nValue + nSpinSize - nRemainder;
    else
        nValue = (nRemainder == 0) ? nValue + nSpinSize : nValue - nRemainder;

    // setValue handles under- and overflows (min/max) automatically
    rFormatter.SetValue(static_cast<double>(nValue) / nScale);
    SetModifyFlag();
    Modify();

    SpinField::Up();
}

void FormattedField::Down()
{
    Formatter& rFormatter = GetFormatter();
    auto nScale = weld::SpinButton::Power10(rFormatter.GetDecimalDigits());

    sal_Int64 nValue = std::round(rFormatter.GetValue() * nScale);
    sal_Int64 nSpinSize = std::round(rFormatter.GetSpinSize() * nScale);
    assert(nSpinSize != 0);
    sal_Int64 nRemainder = rFormatter.GetDisableRemainderFactor() || nSpinSize == 0 ? 0 : nValue % nSpinSize;
    if (nValue >= 0)
        nValue = (nRemainder == 0) ? nValue - nSpinSize : nValue - nRemainder;
    else
        nValue = (nRemainder == 0) ? nValue - nSpinSize : nValue - nSpinSize - nRemainder;

    // setValue handles under- and overflows (min/max) automatically
    rFormatter.SetValue(static_cast<double>(nValue) / nScale);
    SetModifyFlag();
    Modify();

    SpinField::Down();
}

void FormattedField::First()
{
    Formatter& rFormatter = GetFormatter();
    if (rFormatter.HasMinValue())
    {
        rFormatter.SetValue(rFormatter.GetMinValue());
        SetModifyFlag();
        Modify();
    }

    SpinField::First();
}

void FormattedField::Last()
{
    Formatter& rFormatter = GetFormatter();
    if (rFormatter.HasMaxValue())
    {
        rFormatter.SetValue(rFormatter.GetMaxValue());
        SetModifyFlag();
        Modify();
    }

    SpinField::Last();
}

void FormattedField::Modify()
{
    GetFormatter().Modify();
}

bool FormattedField::PreNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == NotifyEventType::KEYINPUT)
        GetFormatter().SetLastSelection(GetSelection());
    return SpinField::PreNotify(rNEvt);
}

bool FormattedField::EventNotify(NotifyEvent& rNEvt)
{
    if ((rNEvt.GetType() == NotifyEventType::KEYINPUT) && !IsReadOnly())
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        sal_uInt16 nMod = rKEvt.GetKeyCode().GetModifier();
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            {
                Formatter& rFormatter = GetFormatter();
                if (!nMod && rFormatter.GetOrCreateFormatter().IsTextFormat(rFormatter.GetFormatKey()))
                {
                    // the base class would translate this into calls to Up/Down/First/Last,
                    // but we don't want this if we are text-formatted
                    return true;
                }
            }
        }
    }

    if ((rNEvt.GetType() == NotifyEventType::COMMAND) && !IsReadOnly())
    {
        const CommandEvent* pCommand = rNEvt.GetCommandEvent();
        if (pCommand->GetCommand() == CommandEventId::Wheel)
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            Formatter& rFormatter = GetFormatter();
            if ((pData->GetMode() == CommandWheelMode::SCROLL) &&
                rFormatter.GetOrCreateFormatter().IsTextFormat(rFormatter.GetFormatKey()))
            {
                // same as above : prevent the base class from doing Up/Down-calls
                // (normally I should put this test into the Up/Down methods itself, shouldn't I ?)
                // FS - 71553 - 19.01.00
                return true;
            }
        }
    }

    if (rNEvt.GetType() == NotifyEventType::LOSEFOCUS && m_pFormatter)
        m_pFormatter->EntryLostFocus();

    return SpinField::EventNotify( rNEvt );
}

Formatter& FormattedField::GetFormatter()
{
    if (!m_pFormatter)
    {
        m_xOwnFormatter.reset(new FieldFormatter(*this));
        m_pFormatter = m_xOwnFormatter.get();
    }
    return *m_pFormatter;
}

void FormattedField::SetFormatter(Formatter* pFormatter)
{
    m_xOwnFormatter.reset();
    m_pFormatter = pFormatter;
}

// currently used by online
void FormattedField::SetValueFromString(const OUString& rStr)
{
    sal_Int32 nEnd;
    rtl_math_ConversionStatus eStatus;
    Formatter& rFormatter = GetFormatter();
    double fValue = ::rtl::math::stringToDouble(rStr, '.', rFormatter.GetDecimalDigits(), &eStatus, &nEnd );

    if (eStatus == rtl_math_ConversionStatus_Ok &&
        nEnd == rStr.getLength())
    {
        rFormatter.SetValue(fValue);
        SetModifyFlag();
        Modify();

        // Notify the value has changed
        SpinField::Up();
    }
    else
    {
        SAL_WARN("vcl", "fail to convert the value: " << rStr);
    }
}

void FormattedField::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    SpinField::DumpAsPropertyTree(rJsonWriter);
    Formatter& rFormatter = GetFormatter();

    if (dynamic_cast<weld::TimeFormatter*>(&rFormatter))
    {
        // weld::TimeFormatter uses h24 format
        rJsonWriter.put("type", "time");
    }
    rJsonWriter.put("min", rFormatter.GetMinValue());
    rJsonWriter.put("max", rFormatter.GetMaxValue());
    rJsonWriter.put("value", rFormatter.GetValue());
    rJsonWriter.put("step", rFormatter.GetSpinSize());
}

FactoryFunction FormattedField::GetUITestFactory() const
{
    return FormattedFieldUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
