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

#ifndef INCLUDED_XMLOFF_XMLEMENT_HXX
#define INCLUDED_XMLOFF_XMLEMENT_HXX

#include <sal/types.h>
#include <xmloff/xmltoken.hxx>

/** Map an XMLTokenEnum to an enum value.
 * To be used with SvXMLUnitConverter::convertEnum(...)
 * We store the enum internally as a fixed size field, since there are
 * places where we want to store a generic pointer to an array of SvXMLEnumMapEntry
 * and we don't want to templatize the class.
 */
template<typename EnumT>
struct SvXMLEnumMapEntry
{
private:
    ::xmloff::token::XMLTokenEnum   eToken;
    sal_uInt16                      nValue;
public:
    SvXMLEnumMapEntry(::xmloff::token::XMLTokenEnum eToken_, EnumT nValue_)
        : eToken(eToken_), nValue(static_cast<sal_uInt16>(nValue_)) {}
    ::xmloff::token::XMLTokenEnum   GetToken() const { return eToken; }
    sal_uInt16                      GetValue() const { return nValue; }
};

#if defined(_MSC_VER)
// specialisation to avoid lots of "C2398: conversion from 'const sal_Int16' to 'sal_uInt16' requires a narrowing conversion"
// errors when compiling on MSVC
template<>
struct SvXMLEnumMapEntry<sal_uInt16>
{
private:
    ::xmloff::token::XMLTokenEnum   eToken;
    sal_uInt16                      nValue;
public:
    SvXMLEnumMapEntry(::xmloff::token::XMLTokenEnum eToken_, sal_Int32 nValue_)
        : eToken(eToken_), nValue(nValue_) {}
    ::xmloff::token::XMLTokenEnum   GetToken() const { return eToken; }
    sal_uInt16                      GetValue() const { return nValue; }
};
#endif

#define ENUM_STRING_MAP_ENTRY(name,tok) { name, sizeof(name)-1, tok }

#define ENUM_STRING_MAP_END()           { nullptr, 0, 0 }

/** Map a const sal_Char* (with length) to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...)
 */
template<typename EnumT>
struct SvXMLEnumStringMapEntry
{
private:
    const char *    pName;
    sal_Int32       nNameLength;
    sal_uInt16      nValue;
public:
    SvXMLEnumStringMapEntry(const char * pName_, sal_Int32 nNameLength_, EnumT nValue_)
        : pName(pName_), nNameLength(nNameLength_), nValue(nValue_) {}
    const char * GetName() const { return pName; }
    sal_Int32    GetNameLength() const { return nNameLength; }
    EnumT        GetValue() const { return static_cast<EnumT>(nValue); }
};

#endif // INCLUDED_XMLOFF_XMLEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
