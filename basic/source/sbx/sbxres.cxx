/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

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

const char* GetSbxRes( sal_uInt16 nId )
{
    return ( ( nId > SBXRES_MAX ) ? "???" : pSbxRes[ nId ] );
}

SbxRes::SbxRes( sal_uInt16 nId )
    : ::rtl::OUString( ::rtl::OUString::createFromAscii( GetSbxRes( nId ) ) )
{}

