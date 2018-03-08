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

#include <svx/hexcolorcontrol.hxx>

HexColorControl::HexColorControl( vcl::Window* pParent, WinBits nStyle )
    : Edit(pParent, nStyle)
{
    SetMaxTextLen( 6 );
}

VCL_BUILDER_FACTORY_ARGS(HexColorControl, WB_BORDER)

void HexColorControl::SetColor(Color nColor)
{
    OUStringBuffer aBuffer;
    sax::Converter::convertColor(aBuffer, nColor);
    SetText(aBuffer.makeStringAndClear().copy(1));
}

Color HexColorControl::GetColor()
{
    sal_Int32 nColor = -1;

    OUString aStr("#");
    aStr += GetText();
    sal_Int32 nLen = aStr.getLength();

    if (nLen < 7)
    {
        static const sal_Char* const pNullStr = "000000";
        aStr += OUString::createFromAscii( &pNullStr[nLen-1] );
    }

    sax::Converter::convertColor(nColor, aStr);

    if (nColor == -1)
        SetControlBackground(COL_RED);
    else
        SetControlBackground();

    return Color(nColor);
}

bool HexColorControl::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplProcessKeyInput( *rNEvt.GetKeyEvent() ) )
            return true;
    }

    return Edit::PreNotify( rNEvt );
}

void HexColorControl::Paste()
{
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> aClipboard(GetClipboard());
    if (aClipboard.is())
    {
        css::uno::Reference<css::datatransfer::XTransferable> xDataObj;

        try
        {
            SolarMutexReleaser aReleaser;
            xDataObj = aClipboard->getContents();
        }
        catch (const css::uno::Exception&)
        {
        }

        if (xDataObj.is())
        {
            css::datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor(SotClipboardFormatId::STRING, aFlavor);
            try
            {
                css::uno::Any aData = xDataObj->getTransferData(aFlavor);
                OUString aText;
                aData >>= aText;

                if( !aText.isEmpty() && aText.startsWith( "#" ) )
                    aText = aText.copy(1);

                if( aText.getLength() > 6 )
                    aText = aText.copy( 0, 6 );

                SetText(aText);
            }
            catch(const css::uno::Exception&)
            {}
        }
    }
}

bool HexColorControl::ImplProcessKeyInput( const KeyEvent& rKEv )
{
    const vcl::KeyCode& rKeyCode = rKEv.GetKeyCode();

    if( rKeyCode.GetGroup() == KEYGROUP_ALPHA && !rKeyCode.IsMod1() && !rKeyCode.IsMod2() )
    {
        if( (rKeyCode.GetCode() < KEY_A) || (rKeyCode.GetCode() > KEY_F) )
            return true;
    }
    else if( rKeyCode.GetGroup() == KEYGROUP_NUM )
    {
        if( rKeyCode.IsShift() )
            return true;
    }
    return false;
}

namespace weld {

HexColorControl::HexColorControl(weld::Entry* pEntry)
    : m_xEntry(pEntry)
{
    m_xEntry->set_max_length(6);
    m_xEntry->set_width_chars(6);
    m_xEntry->connect_insert_text(LINK(this, HexColorControl, ImplProcessInputHdl));
}

void HexColorControl::SetColor(Color nColor)
{
    OUStringBuffer aBuffer;
    sax::Converter::convertColor(aBuffer, nColor);
    m_xEntry->set_text(aBuffer.makeStringAndClear().copy(1));
}

Color HexColorControl::GetColor()
{
    sal_Int32 nColor = -1;

    OUString aStr("#");
    aStr += m_xEntry->get_text();
    sal_Int32 nLen = aStr.getLength();

    if (nLen < 7)
    {
        static const sal_Char* const pNullStr = "000000";
        aStr += OUString::createFromAscii( &pNullStr[nLen-1] );
    }

    sax::Converter::convertColor(nColor, aStr);

#if 0
    if (nColor == -1)
        SetControlBackground(COL_RED);
    else
        SetControlBackground();
#endif

    return Color(nColor);
}

IMPL_LINK(HexColorControl, ImplProcessInputHdl, OUString&, rTest, bool)
{
    const sal_Unicode* pTest = rTest.getStr();
    sal_Int32 nLen = rTest.getLength();

    OUStringBuffer aFilter(nLen);
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        if (rtl::isAsciiHexDigit(*pTest))
            aFilter.append(*pTest);
        ++pTest;
    }

    rTest = aFilter.makeStringAndClear();
    return true;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
