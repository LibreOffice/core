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

#include <comphelper/basicio.hxx>


namespace comphelper
{



const css::uno::Reference<css::io::XObjectOutputStream>& operator << (
        const css::uno::Reference<css::io::XObjectOutputStream>& _rxOutStream,
        const css::awt::FontDescriptor& _rFont)
{
    _rxOutStream->writeUTF( _rFont.Name );
    _rxOutStream->writeShort( _rFont.Height );
    _rxOutStream->writeShort( _rFont.Width );
    _rxOutStream->writeUTF( _rFont.StyleName );
    _rxOutStream->writeShort( _rFont.Family );
    _rxOutStream->writeShort( _rFont.CharSet );
    _rxOutStream->writeShort( _rFont.Pitch );
    _rxOutStream->writeDouble( _rFont.CharacterWidth );
    _rxOutStream->writeDouble( _rFont.Weight );
    _rxOutStream->writeShort( static_cast< sal_Int16 >(_rFont.Slant) );
    _rxOutStream->writeShort( _rFont.Underline );
    _rxOutStream->writeShort( _rFont.Strikeout );
    _rxOutStream->writeDouble( _rFont.Orientation );
    _rxOutStream->writeBoolean( _rFont.Kerning );
    _rxOutStream->writeBoolean( _rFont.WordLineMode );
    _rxOutStream->writeShort( _rFont.Type );
    return _rxOutStream;
}

// FontDescriptor

const css::uno::Reference<css::io::XObjectInputStream>& operator >> (
        const css::uno::Reference<css::io::XObjectInputStream>& _rxInStream,
        css::awt::FontDescriptor& _rFont)
{
    // schreiben des Fontdescriptors
    _rFont.Name = _rxInStream->readUTF();
    _rFont.Height = _rxInStream->readShort();
    _rFont.Width = _rxInStream->readShort();
    _rFont.StyleName = _rxInStream->readUTF();
    _rFont.Family = _rxInStream->readShort();
    _rFont.CharSet = _rxInStream->readShort();
    _rFont.Pitch = _rxInStream->readShort();
    _rFont.CharacterWidth = static_cast< float >(_rxInStream->readDouble());
    _rFont.Weight = static_cast< float >(_rxInStream->readDouble());
    _rFont.Slant = (css::awt::FontSlant)_rxInStream->readShort();
    _rFont.Underline = _rxInStream->readShort();
    _rFont.Strikeout = _rxInStream->readShort();
    _rFont.Orientation = static_cast< float >(_rxInStream->readDouble());
    _rFont.Kerning = _rxInStream->readBoolean();
    _rFont.WordLineMode = _rxInStream->readBoolean();
    _rFont.Type = _rxInStream->readShort();
    return _rxInStream;
}


const css::uno::Reference<css::io::XObjectInputStream>& operator >> (const css::uno::Reference<css::io::XObjectInputStream>& _rxInStream, bool& _rVal)
{
    _rVal = _rxInStream->readBoolean();
    return _rxInStream;
}


const css::uno::Reference<css::io::XObjectOutputStream>& operator << (const css::uno::Reference<css::io::XObjectOutputStream>& _rxOutStream, bool _bVal)
{
    _rxOutStream->writeBoolean(_bVal);
    return _rxOutStream;
}


const css::uno::Reference<css::io::XObjectInputStream>& operator >> (const css::uno::Reference<css::io::XObjectInputStream>& _rxInStream, OUString& rStr)
{
    rStr = _rxInStream->readUTF();
    return _rxInStream;
}


const css::uno::Reference<css::io::XObjectOutputStream>& operator << (const css::uno::Reference<css::io::XObjectOutputStream>& _rxOutStream, const OUString& rStr)
{
    _rxOutStream->writeUTF(rStr);
    return _rxOutStream;
}


const css::uno::Reference<css::io::XObjectInputStream>& operator >> (const css::uno::Reference<css::io::XObjectInputStream>& _rxInStream, sal_Int16& _rValue)
{
    _rValue = _rxInStream->readShort();
    return _rxInStream;
}


const css::uno::Reference<css::io::XObjectOutputStream>& operator << (const css::uno::Reference<css::io::XObjectOutputStream>& _rxOutStream, sal_Int16 _nValue)
{
    _rxOutStream->writeShort(_nValue);
    return _rxOutStream;
}


const css::uno::Reference<css::io::XObjectInputStream>& operator >> (const css::uno::Reference<css::io::XObjectInputStream>& _rxInStream, sal_uInt16& _rValue)
{
    _rValue = _rxInStream->readShort();
    return _rxInStream;
}


const css::uno::Reference<css::io::XObjectOutputStream>& operator << (const css::uno::Reference<css::io::XObjectOutputStream>& _rxOutStream, sal_uInt16 _nValue)
{
    _rxOutStream->writeShort(_nValue);
    return _rxOutStream;
}


const css::uno::Reference<css::io::XObjectInputStream>& operator >> (const css::uno::Reference<css::io::XObjectInputStream>& _rxInStream, sal_uInt32& _rValue)
{
    _rValue = _rxInStream->readLong();
    return _rxInStream;
}


const css::uno::Reference<css::io::XObjectOutputStream>& operator << (const css::uno::Reference<css::io::XObjectOutputStream>& _rxOutStream, sal_uInt32 _nValue)
{
    _rxOutStream->writeLong(_nValue);
    return _rxOutStream;
}


const css::uno::Reference<css::io::XObjectInputStream>& operator >> (const css::uno::Reference<css::io::XObjectInputStream>& _rxInStream, sal_Int32& _rValue)
{
    _rValue = _rxInStream->readLong();
    return _rxInStream;
}


const css::uno::Reference<css::io::XObjectOutputStream>& operator << (const css::uno::Reference<css::io::XObjectOutputStream>& _rxOutStream, sal_Int32 _nValue)
{
    _rxOutStream->writeLong(_nValue);
    return _rxOutStream;
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
