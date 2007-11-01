/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cellkeytranslator.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:21:13 $
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

#ifndef SC_CELLKEY_TRANSLATOR_HXX
#define SC_CELLKEY_TRANSLATOR_HXX

#include "global.hxx"
#include "opcode.hxx"
#include "unotools/transliterationwrapper.hxx"
#include <hash_map>
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

typedef ::std::hash_map< String, ::std::list<ScCellKeyword>, ScStringHashCode, ::std::equal_to<String> > ScCellKeywordHashMap;

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
