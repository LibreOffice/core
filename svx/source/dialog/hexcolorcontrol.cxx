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

void HexColorControl::SetColor(sal_Int32 nColor)
{
    OUStringBuffer aBuffer;
    sax::Converter::convertColor(aBuffer, nColor);
    SetText(aBuffer.makeStringAndClear().copy(1));
}

sal_Int32 HexColorControl::GetColor()
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
        SetControlBackground(Color(COL_RED));
    else
        SetControlBackground();

    return nColor;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
