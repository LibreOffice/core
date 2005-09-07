/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxres.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:51:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "sbxres.hxx"

static const char* pSbxRes[] = {
    "Empty",
    "Null",
    "Integer",
    "Long",
    "Single",
    "Double",
    "Currency",
    "Date",
    "String",
    "Object",
    "Error",
    "Boolean",
    "Variant",
    "Any",
    "Type14",
    "Type15",
    "Char",
    "Byte",
    "UShort",
    "ULong",
    "Long64",
    "ULong64",
    "Int",
    "UInt",
    "Void",
    "HResult",
    "Pointer",
    "DimArray",
    "CArray",
    "Any",
    "LpStr",
    "LpWStr",
    " As ",
    "Optional ",
    "Byref ",

    "Name",
    "Parent",
    "Application",
    "Count",
    "Add",
    "Item",
    "Remove",

    "Error ",   // mit Blank!
    "False",
    "True"
};

const char* GetSbxRes( USHORT nId )
{
    return ( ( nId > SBXRES_MAX ) ? "???" : pSbxRes[ nId ] );
}

SbxRes::SbxRes( USHORT nId )
    : XubString( String::CreateFromAscii( GetSbxRes( nId ) ) )
{}

