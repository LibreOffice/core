/*************************************************************************
 *
 *  $RCSfile: basicio.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif

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
    _rxOutStream->writeShort( _rFont.Slant );
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
    _rFont.CharacterWidth = _rxInStream->readDouble();
    _rFont.Weight = _rxInStream->readDouble();
    _rFont.Slant = (starawt::FontSlant)_rxInStream->readShort();
    _rFont.Underline = _rxInStream->readShort();
    _rFont.Strikeout = _rxInStream->readShort();
    _rFont.Orientation = _rxInStream->readDouble();
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

