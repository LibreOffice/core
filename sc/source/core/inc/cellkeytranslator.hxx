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

#ifndef SC_CELLKEY_TRANSLATOR_HXX
#define SC_CELLKEY_TRANSLATOR_HXX

#include "global.hxx"
#include "formula/opcode.hxx"
#include "unotools/transliterationwrapper.hxx"
#include <boost/unordered_map.hpp>
#include <list>
#include <memory>

#include <com/sun/star/lang/Locale.hpp>

struct TransItem;

struct ScCellKeyword
{
    const sal_Char* mpName;
    OpCode meOpCode;
    const ::com::sun::star::lang::Locale& mrLocale;

    ScCellKeyword(const sal_Char* pName, OpCode eOpCode, const ::com::sun::star::lang::Locale& rLocale);
};

typedef ::boost::unordered_map< String, ::std::list<ScCellKeyword>, ScStringHashCode, ::std::equal_to<String> > ScCellKeywordHashMap;

/** Translate cell function keywords.

    This class provides a convenient way to translate a string keyword used as
    a cell function argument.  Since Calc's built-in cell functions don't
    localize string keywords, this class is used mainly to deal with an Excel
    document where string names may be localized.

    To use, simply call the

       ScCellKeywordTranslator::transKeyword(...)

    function.

    Note that when the locale and/or the opcode is specified, the function
    tries to find a string with matching locale and/or opcode. But when it
    fails to find one that satisfies the specified locale and/or opcode, it
    returns a translated string with non-matching locale and/or opcode if
    available. */
class ScCellKeywordTranslator
{
public:
    static void transKeyword(String& rName, const ::com::sun::star::lang::Locale* pLocale = NULL, OpCode eOpCode = ocNone);
    ~ScCellKeywordTranslator();

private:
    ScCellKeywordTranslator();

    void init();
    void addToMap(const String& rKey, const sal_Char* pName,
                  const ::com::sun::star::lang::Locale& rLocale,
                  OpCode eOpCode = ocNone);
    void addToMap(const TransItem* pItems, const ::com::sun::star::lang::Locale& rLocale);

    static ::std::auto_ptr<ScCellKeywordTranslator> spInstance;
    ScCellKeywordHashMap maStringNameMap;
    ::utl::TransliterationWrapper maTransWrapper;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
