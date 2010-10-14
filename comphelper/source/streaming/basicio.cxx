/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_comphelper.hxx"
#include <comphelper/basicio.hxx>

//.........................................................................
namespace comphelper
{
//.........................................................................

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectOutputStream>& operator << (
        const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream,
        const starawt::FontDescriptor& _rFont)
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
//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectInputStream>& operator >> (
        const staruno::Reference<stario::XObjectInputStream>& _rxInStream,
        starawt::FontDescriptor& _rFont)
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
    _rFont.Slant = (starawt::FontSlant)_rxInStream->readShort();
    _rFont.Underline = _rxInStream->readShort();
    _rFont.Strikeout = _rxInStream->readShort();
    _rFont.Orientation = static_cast< float >(_rxInStream->readDouble());
    _rFont.Kerning = _rxInStream->readBoolean();
    _rFont.WordLineMode = _rxInStream->readBoolean();
    _rFont.Type = _rxInStream->readShort();
    return _rxInStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Bool& _rVal)
{
    _rVal = _rxInStream->readBoolean();
    return _rxInStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_Bool _bVal)
{
    _rxOutStream->writeBoolean(_bVal);
    return _rxOutStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, ::rtl::OUString& rStr)
{
    rStr = _rxInStream->readUTF();
    return _rxInStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, const ::rtl::OUString& rStr)
{
    _rxOutStream->writeUTF(rStr);
    return _rxOutStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Int16& _rValue)
{
    _rValue = _rxInStream->readShort();
    return _rxInStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_Int16 _nValue)
{
    _rxOutStream->writeShort(_nValue);
    return _rxOutStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_uInt16& _rValue)
{
    _rValue = _rxInStream->readShort();
    return _rxInStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_uInt16 _nValue)
{
    _rxOutStream->writeShort(_nValue);
    return _rxOutStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_uInt32& _rValue)
{
    _rValue = _rxInStream->readLong();
    return _rxInStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_uInt32 _nValue)
{
    _rxOutStream->writeLong(_nValue);
    return _rxOutStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Int32& _rValue)
{
    _rValue = _rxInStream->readLong();
    return _rxInStream;
}

//------------------------------------------------------------------------------
const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_Int32 _nValue)
{
    _rxOutStream->writeLong(_nValue);
    return _rxOutStream;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
