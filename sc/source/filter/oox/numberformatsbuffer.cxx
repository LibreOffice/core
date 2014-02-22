/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "numberformatsbuffer.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <officecfg/Setup.hxx>
#include <officecfg/System.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>
#include <svl/intitem.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"
#include "biffinputstream.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "ftools.hxx"

namespace oox {
namespace xls {



using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;




namespace {

/** Stores the number format used in Calc for an Excel built-in number format. */
struct BuiltinFormat
{
    sal_Int32           mnNumFmtId;         
    const sal_Char*     mpcFmtCode;         
    sal_Int16           mnPredefId;         
    sal_Int32           mnReuseId;          
};

/** Defines a literal built-in number format. */
#define NUMFMT_STRING( INDEX, FORMATCODE ) \
    { INDEX, FORMATCODE, -1, -1 }

/** Defines a built-in number format that maps to an own predefined format. */
#define NUMFMT_PREDEF( INDEX, PREDEFINED ) \
    { INDEX, 0, ::com::sun::star::i18n::NumberFormatIndex::PREDEFINED, -1 }

/** Defines a built-in number format that is the same as the specified in nReuseId. */
#define NUMFMT_REUSE( INDEX, REUSED_INDEX ) \
    { INDEX, 0, -1, REUSED_INDEX }

/** Terminates a built-in number format table. */
#define NUMFMT_ENDTABLE() \
    { -1, 0, -1, -1 }

/** Defines builtin date and time formats 14...22.
    @param SYSTEMDATE  Complete short system date (for formats 14 and 22).
    @param DAY  Day format (for formats 15 and 16).
    @param DAYSEP  Separator between day and month (for formats 15 and 16).
    @param MONTH  Month format (for formats 15...17).
    @param MONTHSEP  Separator between month and year (for formats 15 and 17).
    @param YEAR  Year format (for formats 15 and 17).
    @param HOUR12  Hour format for 12-hour AM/PM formats (formats 18 and 19).
    @param HOUR24  Hour format for 24-hour formats (formats 20...22). */
#define NUMFMT_ALLDATETIMES( SYSTEMDATE, DAY, DAYSEP, MONTH, MONTHSEP, YEAR, HOUR12, HOUR24 ) \
    NUMFMT_STRING(  14, SYSTEMDATE ), \
    NUMFMT_STRING(  15, DAY DAYSEP MONTH MONTHSEP YEAR ), \
    NUMFMT_STRING(  16, DAY DAYSEP MONTH ), \
    NUMFMT_STRING(  17, MONTH MONTHSEP YEAR ), \
    NUMFMT_STRING(  18, HOUR12 ":mm AM/PM" ), \
    NUMFMT_STRING(  19, HOUR12 ":mm:ss AM/PM" ), \
    NUMFMT_STRING(  20, HOUR24 ":mm" ), \
    NUMFMT_STRING(  21, HOUR24 ":mm:ss" ), \
    NUMFMT_STRING(  22, SYSTEMDATE " " HOUR24 ":mm" )

/** Defines builtin time formats INDEX and INDEX+1 for CJK locales.
    @param INDEX  First number format index.
    @param HOURFORMAT  Hour format.
    @param HOUR  Hour symbol.
    @param MINUTE  Minute symbol.
    @param SECOND  Second symbol. */
#define NUMFMT_TIME_CJK( INDEX, HOURFORMAT, HOUR, MINUTE, SECOND ) \
    NUMFMT_STRING( INDEX + 0, HOURFORMAT "\"" HOUR "\"mm\"" MINUTE "\"" ), \
    NUMFMT_STRING( INDEX + 1, HOURFORMAT "\"" HOUR "\"mm\"" MINUTE "\"ss\"" SECOND "\"" )

/** Defines builtin time formats 32...35 for CJK locales.
    @param HOUR12  Hour format for 12-hour AM/PM formats (formats 34 and 35).
    @param HOUR24  Hour format for 24-hour formats (formats 32 and 33).
    @param HOUR  Hour symbol.
    @param MINUTE  Minute symbol.
    @param SECOND  Second symbol. */
#define NUMFMT_ALLTIMES_CJK( HOUR12, HOUR24, HOUR, MINUTE, SECOND ) \
    NUMFMT_TIME_CJK( 32, HOUR24, HOUR, MINUTE, SECOND ), \
    NUMFMT_TIME_CJK( 34, "AM/PM" HOUR12, HOUR, MINUTE, SECOND )

/** Defines builtin currency formats INDEX...INDEX+3 in the following format:
    "symbol, [minus], number".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number.
    @param MODIF  Leading modifier for each portion (e.g. "t" for Thai formats). */
#define NUMFMT_CURRENCY_SYMBOL_MINUS_NUMBER( INDEX, SYMBOL, SPACE, MODIF ) \
    NUMFMT_STRING( INDEX + 0, MODIF SYMBOL SPACE "#,##0;"            MODIF SYMBOL SPACE "-#,##0" ), \
    NUMFMT_STRING( INDEX + 1, MODIF SYMBOL SPACE "#,##0;"    "[RED]" MODIF SYMBOL SPACE "-#,##0" ), \
    NUMFMT_STRING( INDEX + 2, MODIF SYMBOL SPACE "#,##0.00;"         MODIF SYMBOL SPACE "-#,##0.00" ), \
    NUMFMT_STRING( INDEX + 3, MODIF SYMBOL SPACE "#,##0.00;" "[RED]" MODIF SYMBOL SPACE "-#,##0.00" )

/** Defines builtin accounting formats INDEX...INDEX+3 in the following format:
    "symbol, [minus], number".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ACCOUNTING_SYMBOL_MINUS_NUMBER( INDEX, SYMBOL, SPACE ) \
    NUMFMT_STRING( INDEX + 0, "_ "              "* #,##0_ ;"    "_ "              "* -#,##0_ ;"    "_ "              "* \"-\"_ ;"    "_ @_ " ), \
    NUMFMT_STRING( INDEX + 1, "_ " SYMBOL SPACE "* #,##0_ ;"    "_ " SYMBOL SPACE "* -#,##0_ ;"    "_ " SYMBOL SPACE "* \"-\"_ ;"    "_ @_ " ), \
    NUMFMT_STRING( INDEX + 2, "_ "              "* #,##0.00_ ;" "_ "              "* -#,##0.00_ ;" "_ "              "* \"-\"?\?_ ;" "_ @_ " ), \
    NUMFMT_STRING( INDEX + 3, "_ " SYMBOL SPACE "* #,##0.00_ ;" "_ " SYMBOL SPACE "* -#,##0.00_ ;" "_ " SYMBOL SPACE "* \"-\"?\?_ ;" "_ @_ " )

/** Defines builtin currency formats 5...8 (with currency symbol), 37...40
    (blind currency symbol), and 41...44 (accounting), in the following format:
    "symbol, [minus], number".
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( SYMBOL, SPACE ) \
    NUMFMT_CURRENCY_SYMBOL_MINUS_NUMBER( 5, SYMBOL, SPACE, "" ), \
    NUMFMT_CURRENCY_SYMBOL_MINUS_NUMBER( 37, "", "", "" ), \
    NUMFMT_ACCOUNTING_SYMBOL_MINUS_NUMBER( 41, SYMBOL, SPACE )

/** Defines builtin currency formats INDEX...INDEX+3 in the following format:
    "symbol, number, [minus]".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number.
    @param MODIF  Leading modifier for each portion (e.g. "t" for Thai formats). */
#define NUMFMT_CURRENCY_SYMBOL_NUMBER_MINUS( INDEX, SYMBOL, SPACE, MODIF ) \
    NUMFMT_STRING( INDEX + 0, MODIF SYMBOL SPACE "#,##0_-;"            MODIF SYMBOL SPACE "#,##0-" ), \
    NUMFMT_STRING( INDEX + 1, MODIF SYMBOL SPACE "#,##0_-;"    "[RED]" MODIF SYMBOL SPACE "#,##0-" ), \
    NUMFMT_STRING( INDEX + 2, MODIF SYMBOL SPACE "#,##0.00_-;"         MODIF SYMBOL SPACE "#,##0.00-" ), \
    NUMFMT_STRING( INDEX + 3, MODIF SYMBOL SPACE "#,##0.00_-;" "[RED]" MODIF SYMBOL SPACE "#,##0.00-" )

/** Defines builtin accounting formats INDEX...INDEX+3 in the following format:
    "symbol, number, [minus]".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ACCOUNTING_SYMBOL_NUMBER_MINUS( INDEX, SYMBOL, SPACE ) \
    NUMFMT_STRING( INDEX + 0, "_-"              "* #,##0_-;"    "_-"              "* #,##0-;"    "_-"              "* \"-\"_-;"    "_-@_-" ), \
    NUMFMT_STRING( INDEX + 1, "_-" SYMBOL SPACE "* #,##0_-;"    "_-" SYMBOL SPACE "* #,##0-;"    "_-" SYMBOL SPACE "* \"-\"_-;"    "_-@_-" ), \
    NUMFMT_STRING( INDEX + 2, "_-"              "* #,##0.00_-;" "_-"              "* #,##0.00-;" "_-"              "* \"-\"?\?_-;" "_-@_-" ), \
    NUMFMT_STRING( INDEX + 3, "_-" SYMBOL SPACE "* #,##0.00_-;" "_-" SYMBOL SPACE "* #,##0.00-;" "_-" SYMBOL SPACE "* \"-\"?\?_-;" "_-@_-" )

/** Defines builtin currency formats 5...8 (with currency symbol), 37...40
    (blind currency symbol), and 41...44 (accounting), in the following format:
    "symbol, number, [minus]".
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( SYMBOL, SPACE ) \
    NUMFMT_CURRENCY_SYMBOL_NUMBER_MINUS( 5, SYMBOL, SPACE, "" ), \
    NUMFMT_CURRENCY_SYMBOL_NUMBER_MINUS( 37, "", "", "" ), \
    NUMFMT_ACCOUNTING_SYMBOL_NUMBER_MINUS( 41, SYMBOL, SPACE )

/** Defines builtin currency formats INDEX...INDEX+3 in the following format:
    "number, symbol, [minus]".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between number and currency symbol.
    @param MODIF  Leading modifier for each portion (e.g. "t" for Thai formats). */
#define NUMFMT_CURRENCY_NUMBER_SYMBOL_MINUS( INDEX, SYMBOL, SPACE, MODIF ) \
    NUMFMT_STRING( INDEX + 0, MODIF "#,##0"    SPACE SYMBOL "_-;"         MODIF "#,##0"    SPACE SYMBOL "-" ), \
    NUMFMT_STRING( INDEX + 1, MODIF "#,##0"    SPACE SYMBOL "_-;" "[RED]" MODIF "#,##0"    SPACE SYMBOL "-" ), \
    NUMFMT_STRING( INDEX + 2, MODIF "#,##0.00" SPACE SYMBOL "_-;"         MODIF "#,##0.00" SPACE SYMBOL "-" ), \
    NUMFMT_STRING( INDEX + 3, MODIF "#,##0.00" SPACE SYMBOL "_-;" "[RED]" MODIF "#,##0.00" SPACE SYMBOL "-" )

/** Defines builtin accounting formats INDEX...INDEX+3 in the following format:
    "number, symbol, [minus]".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param BLINDS  Blind currency symbol.
    @param SPACE  Space character(s) between number and currency symbol. */
#define NUMFMT_ACCOUNTING_NUMBER_SYMBOL_MINUS( INDEX, SYMBOL, BLINDS, SPACE ) \
    NUMFMT_STRING( INDEX + 0, "_-* #,##0"    SPACE BLINDS "_-;_-* #,##0"    SPACE BLINDS "-;_-* \"-\""    SPACE BLINDS "_-;_-@_-" ), \
    NUMFMT_STRING( INDEX + 1, "_-* #,##0"    SPACE SYMBOL "_-;_-* #,##0"    SPACE SYMBOL "-;_-* \"-\""    SPACE SYMBOL "_-;_-@_-" ), \
    NUMFMT_STRING( INDEX + 2, "_-* #,##0.00" SPACE BLINDS "_-;_-* #,##0.00" SPACE BLINDS "-;_-* \"-\"?\?" SPACE BLINDS "_-;_-@_-" ), \
    NUMFMT_STRING( INDEX + 3, "_-* #,##0.00" SPACE SYMBOL "_-;_-* #,##0.00" SPACE SYMBOL "-;_-* \"-\"?\?" SPACE SYMBOL "_-;_-@_-" )

/** Defines builtin currency formats 5...8 (with currency symbol), 37...40
    (blind currency symbol), and 41...44 (accounting), in the following format:
    "number, symbol, [minus]".
    @param SYMBOL  Currency symbol.
    @param BLINDS  Blind currency symbol.
    @param SPACE  Space character(s) between number and currency symbol. */
#define NUMFMT_ALLCURRENCIES_NUMBER_SYMBOL_MINUS( SYMBOL, BLINDS, SPACE ) \
    NUMFMT_CURRENCY_NUMBER_SYMBOL_MINUS( 5, SYMBOL, SPACE, "" ), \
    NUMFMT_CURRENCY_NUMBER_SYMBOL_MINUS( 37, BLINDS, SPACE, "" ), \
    NUMFMT_ACCOUNTING_NUMBER_SYMBOL_MINUS( 41, SYMBOL, BLINDS, SPACE )

/** Defines builtin currency formats INDEX...INDEX+3 in the following format:
    "[minus], symbol, number".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number.
    @param MODIF  Leading modifier for each portion (e.g. "t" for Thai formats). */
#define NUMFMT_CURRENCY_MINUS_SYMBOL_NUMBER( INDEX, SYMBOL, SPACE, MODIF ) \
    NUMFMT_STRING( INDEX + 0, MODIF SYMBOL SPACE "#,##0;"            MODIF "-" SYMBOL SPACE "#,##0" ), \
    NUMFMT_STRING( INDEX + 1, MODIF SYMBOL SPACE "#,##0;"    "[RED]" MODIF "-" SYMBOL SPACE "#,##0" ), \
    NUMFMT_STRING( INDEX + 2, MODIF SYMBOL SPACE "#,##0.00;"         MODIF "-" SYMBOL SPACE "#,##0.00" ), \
    NUMFMT_STRING( INDEX + 3, MODIF SYMBOL SPACE "#,##0.00;" "[RED]" MODIF "-" SYMBOL SPACE "#,##0.00" )

/** Defines builtin accounting formats INDEX...INDEX+3 in the following order:
    "[minus], symbol, number".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ACCOUNTING_MINUS_SYMBOL_NUMBER( INDEX, SYMBOL, SPACE ) \
    NUMFMT_STRING( INDEX + 0, "_-"              "* #,##0_-;"    "-"              "* #,##0_-;"    "_-"              "* \"-\"_-;"    "_-@_-" ), \
    NUMFMT_STRING( INDEX + 1, "_-" SYMBOL SPACE "* #,##0_-;"    "-" SYMBOL SPACE "* #,##0_-;"    "_-" SYMBOL SPACE "* \"-\"_-;"    "_-@_-" ), \
    NUMFMT_STRING( INDEX + 2, "_-"              "* #,##0.00_-;" "-"              "* #,##0.00_-;" "_-"              "* \"-\"?\?_-;" "_-@_-" ), \
    NUMFMT_STRING( INDEX + 3, "_-" SYMBOL SPACE "* #,##0.00_-;" "-" SYMBOL SPACE "* #,##0.00_-;" "_-" SYMBOL SPACE "* \"-\"?\?_-;" "_-@_-" )

/** Defines builtin currency formats 5...8 (with currency symbol), 37...40
    (blind currency symbol), and 41...44 (accounting), in the following order:
    "[minus], symbol, number".
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( SYMBOL, SPACE ) \
    NUMFMT_CURRENCY_MINUS_SYMBOL_NUMBER( 5, SYMBOL, SPACE, "" ), \
    NUMFMT_CURRENCY_MINUS_SYMBOL_NUMBER( 37, "", "", "" ), \
    NUMFMT_ACCOUNTING_MINUS_SYMBOL_NUMBER( 41, SYMBOL, SPACE )

/** Defines builtin currency formats INDEX...INDEX+3 in the following format:
    "[minus], number, symbol".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between number and currency symbol.
    @param MODIF  Leading modifier for each portion (e.g. "t" for Thai formats). */
#define NUMFMT_CURRENCY_MINUS_NUMBER_SYMBOL( INDEX, SYMBOL, SPACE, MODIF ) \
    NUMFMT_STRING( INDEX + 0, MODIF "#,##0"    SPACE SYMBOL ";"         MODIF "-#,##0"    SPACE SYMBOL ), \
    NUMFMT_STRING( INDEX + 1, MODIF "#,##0"    SPACE SYMBOL ";" "[RED]" MODIF "-#,##0"    SPACE SYMBOL ), \
    NUMFMT_STRING( INDEX + 2, MODIF "#,##0.00" SPACE SYMBOL ";"         MODIF "-#,##0.00" SPACE SYMBOL ), \
    NUMFMT_STRING( INDEX + 3, MODIF "#,##0.00" SPACE SYMBOL ";" "[RED]" MODIF "-#,##0.00" SPACE SYMBOL )

/** Defines builtin accounting formats INDEX...INDEX+3 in the following format:
    "[minus], number, symbol".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param BLINDS  Blind currency symbol.
    @param SPACE  Space character(s) between number and currency symbol. */
#define NUMFMT_ACCOUNTING_MINUS_NUMBER_SYMBOL( INDEX, SYMBOL, BLINDS, SPACE ) \
    NUMFMT_STRING( INDEX + 0, "_-* #,##0"    SPACE BLINDS "_-;-* #,##0"    SPACE BLINDS "_-;_-* \"-\""    SPACE BLINDS "_-;_-@_-" ), \
    NUMFMT_STRING( INDEX + 1, "_-* #,##0"    SPACE SYMBOL "_-;-* #,##0"    SPACE SYMBOL "_-;_-* \"-\""    SPACE SYMBOL "_-;_-@_-" ), \
    NUMFMT_STRING( INDEX + 2, "_-* #,##0.00" SPACE BLINDS "_-;-* #,##0.00" SPACE BLINDS "_-;_-* \"-\"?\?" SPACE BLINDS "_-;_-@_-" ), \
    NUMFMT_STRING( INDEX + 3, "_-* #,##0.00" SPACE SYMBOL "_-;-* #,##0.00" SPACE SYMBOL "_-;_-* \"-\"?\?" SPACE SYMBOL "_-;_-@_-" )

/** Defines builtin currency formats 5...8 (with currency symbol), 37...40
    (blind currency symbol), and 41...44 (accounting), in the following format:
    "[minus], number, symbol".
    @param SYMBOL  Currency symbol.
    @param BLINDS  Blind currency symbol.
    @param SPACE  Space character(s) between number and currency symbol. */
#define NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( SYMBOL, BLINDS, SPACE ) \
    NUMFMT_CURRENCY_MINUS_NUMBER_SYMBOL( 5, SYMBOL, SPACE, "" ), \
    NUMFMT_CURRENCY_MINUS_NUMBER_SYMBOL( 37, BLINDS, SPACE, "" ), \
    NUMFMT_ACCOUNTING_MINUS_NUMBER_SYMBOL( 41, SYMBOL, BLINDS, SPACE )

/** Defines builtin currency formats INDEX...INDEX+3 in the following format:
    "[opening parenthesis], symbol, number, [closing parenthesis].".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number.
    @param MODIF  Leading modifier for each portion (e.g. "t" for Thai formats). */
#define NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( INDEX, SYMBOL, SPACE, MODIF ) \
    NUMFMT_STRING( INDEX + 0, MODIF SYMBOL SPACE "#,##0_);"            MODIF "(" SYMBOL SPACE "#,##0)" ), \
    NUMFMT_STRING( INDEX + 1, MODIF SYMBOL SPACE "#,##0_);"    "[RED]" MODIF "(" SYMBOL SPACE "#,##0)" ), \
    NUMFMT_STRING( INDEX + 2, MODIF SYMBOL SPACE "#,##0.00_);"         MODIF "(" SYMBOL SPACE "#,##0.00)" ), \
    NUMFMT_STRING( INDEX + 3, MODIF SYMBOL SPACE "#,##0.00_);" "[RED]" MODIF "(" SYMBOL SPACE "#,##0.00)" )

/** Defines builtin accounting formats INDEX...INDEX+3 in the following format:
    "[opening parenthesis], symbol, number, [closing parenthesis].".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ACCOUNTING_OPEN_SYMBOL_NUMBER_CLOSE( INDEX, SYMBOL, SPACE ) \
    NUMFMT_STRING( INDEX + 0, "_("              "* #,##0_);"    "_("              "* (#,##0);"    "_("              "* \"-\"_);"    "_(@_)" ), \
    NUMFMT_STRING( INDEX + 1, "_(" SYMBOL SPACE "* #,##0_);"    "_(" SYMBOL SPACE "* (#,##0);"    "_(" SYMBOL SPACE "* \"-\"_);"    "_(@_)" ), \
    NUMFMT_STRING( INDEX + 2, "_("              "* #,##0.00_);" "_("              "* (#,##0.00);" "_("              "* \"-\"?\?_);" "_(@_)" ), \
    NUMFMT_STRING( INDEX + 3, "_(" SYMBOL SPACE "* #,##0.00_);" "_(" SYMBOL SPACE "* (#,##0.00);" "_(" SYMBOL SPACE "* \"-\"?\?_);" "_(@_)" )

/** Defines builtin currency formats 5...8 (with currency symbol), 37...40
    (blind currency symbol), and 41...44 (accounting), in the following format:
    "[opening parenthesis], symbol, number, [closing parenthesis].".
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between currency symbol and number. */
#define NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( SYMBOL, SPACE ) \
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 5, SYMBOL, SPACE, "" ), \
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 37, "", "", "" ), \
    NUMFMT_ACCOUNTING_OPEN_SYMBOL_NUMBER_CLOSE( 41, SYMBOL, SPACE )

/** Defines builtin currency formats INDEX...INDEX+3 in the following format:
    "[opening parenthesis], number, symbol, [closing parenthesis].".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param SPACE  Space character(s) between number and currency symbol.
    @param MODIF  Leading modifier for each portion (e.g. "t" for Thai formats). */
#define NUMFMT_CURRENCY_OPEN_NUMBER_SYMBOL_CLOSE( INDEX, SYMBOL, SPACE, MODIF ) \
    NUMFMT_STRING( INDEX + 0, MODIF "#,##0"    SPACE SYMBOL "_);"         MODIF "(#,##0"    SPACE SYMBOL ")" ), \
    NUMFMT_STRING( INDEX + 1, MODIF "#,##0"    SPACE SYMBOL "_);" "[RED]" MODIF "(#,##0"    SPACE SYMBOL ")" ), \
    NUMFMT_STRING( INDEX + 2, MODIF "#,##0.00" SPACE SYMBOL "_);"         MODIF "(#,##0.00" SPACE SYMBOL ")" ), \
    NUMFMT_STRING( INDEX + 3, MODIF "#,##0.00" SPACE SYMBOL "_);" "[RED]" MODIF "(#,##0.00" SPACE SYMBOL ")" )

/** Defines builtin accounting formats INDEX...INDEX+3 in the following format:
    "[opening parenthesis], number, symbol, [closing parenthesis].".
    @param INDEX  First number format index.
    @param SYMBOL  Currency symbol.
    @param BLINDS  Blind currency symbol.
    @param SPACE  Space character(s) between number and currency symbol. */
#define NUMFMT_ACCOUNTING_OPEN_NUMBER_SYMBOL_CLOSE( INDEX, SYMBOL, BLINDS, SPACE ) \
    NUMFMT_STRING( INDEX + 0, "_ * #,##0_)"    SPACE BLINDS "_ ;_ * (#,##0)"    SPACE BLINDS "_ ;_ * \"-\"_)"    SPACE BLINDS "_ ;_ @_ " ), \
    NUMFMT_STRING( INDEX + 1, "_ * #,##0_)"    SPACE SYMBOL "_ ;_ * (#,##0)"    SPACE SYMBOL "_ ;_ * \"-\"_)"    SPACE SYMBOL "_ ;_ @_ " ), \
    NUMFMT_STRING( INDEX + 2, "_ * #,##0.00_)" SPACE BLINDS "_ ;_ * (#,##0.00)" SPACE BLINDS "_ ;_ * \"-\"?\?_)" SPACE BLINDS "_ ;_ @_ " ), \
    NUMFMT_STRING( INDEX + 3, "_ * #,##0.00_)" SPACE SYMBOL "_ ;_ * (#,##0.00)" SPACE SYMBOL "_ ;_ * \"-\"?\?_)" SPACE SYMBOL "_ ;_ @_ " )

/** Defines builtin currency formats 5...8 (with currency symbol), 37...40
    (blind currency symbol), and 41...44 (accounting), in the following format:
    "[opening parenthesis], number, symbol, [closing parenthesis].".
    @param SYMBOL  Currency symbol.
    @param BLINDS  Blind currency symbol.
    @param SPACE  Space character(s) between number and currency symbol. */
#define NUMFMT_ALLCURRENCIES_OPEN_NUMBER_SYMBOL_CLOSE( SYMBOL, BLINDS, SPACE ) \
    NUMFMT_CURRENCY_OPEN_NUMBER_SYMBOL_CLOSE( 5, SYMBOL, SPACE, "" ), \
    NUMFMT_CURRENCY_OPEN_NUMBER_SYMBOL_CLOSE( 37, BLINDS, SPACE, "" ), \
    NUMFMT_ACCOUNTING_OPEN_NUMBER_SYMBOL_CLOSE( 41, SYMBOL, BLINDS, SPACE )


#define UTF8_BAHT           "\340\270\277"
#define UTF8_COLON          "\342\202\241"
#define UTF8_CURR_AR_AE     "\330\257.\330\245."
#define UTF8_CURR_AR_BH     "\330\257.\330\250."
#define UTF8_CURR_AR_DZ     "\330\257.\330\254."
#define UTF8_CURR_AR_EG     "\330\254.\331\205."
#define UTF8_CURR_AR_IQ     "\330\257.\330\271."
#define UTF8_CURR_AR_JO     "\330\257.\330\247."
#define UTF8_CURR_AR_KW     "\330\257.\331\203."
#define UTF8_CURR_AR_LB     "\331\204.\331\204."
#define UTF8_CURR_AR_LY     "\330\257.\331\204."
#define UTF8_CURR_AR_MA     "\330\257.\331\205."
#define UTF8_CURR_AR_OM     "\330\261.\330\271."
#define UTF8_CURR_AR_QA     "\330\261.\331\202."
#define UTF8_CURR_AR_SA     "\330\261.\330\263."
#define UTF8_CURR_AR_SY     "\331\204.\330\263."
#define UTF8_CURR_AR_TN     "\330\257.\330\252."
#define UTF8_CURR_AR_YE     "\330\261.\331\212."
#define UTF8_CURR_BN_IN     "\340\246\237\340\246\276"
#define UTF8_CURR_FA_IR     "\330\261\331\212\330\247\331\204"
#define UTF8_CURR_GU_IN     "\340\252\260\340\253\202"
#define UTF8_CURR_HI_IN     "\340\244\260\340\245\201"
#define UTF8_CURR_KN_IN     "\340\262\260\340\263\202"
#define UTF8_CURR_ML_IN     "\340\264\225"
#define UTF8_CURR_PA_IN     "\340\250\260\340\251\201"
#define UTF8_CURR_TA_IN     "\340\256\260\340\257\202"
#define UTF8_CURR_TE_IN     "\340\260\260\340\261\202"
#define UTF8_DONG           "\342\202\253"
#define UTF8_EURO           "\342\202\254"
#define UTF8_POUND_GB       "\302\243"
#define UTF8_RUFIYAA        "\336\203"
#define UTF8_SHEQEL         "\342\202\252"
#define UTF8_TUGRUG         "\342\202\256"
#define UTF8_WON            "\342\202\251"
#define UTF8_YEN_CN         "\357\277\245"
#define UTF8_YEN_JP         "\302\245"


#define UTF8_CCARON_LC      "\304\215"
#define UTF8_LSTROKE_LC     "\305\202"

#define UTF8_HY_DA_LC       "\325\244"
#define UTF8_HY_REH_LC      "\326\200"

#define UTF8_CYR_G_LC       "\320\263"
#define UTF8_CYR_L_LC       "\320\273"
#define UTF8_CYR_M_LC       "\320\274"
#define UTF8_CYR_N_LC       "\320\275"
#define UTF8_CYR_O_LC       "\320\276"
#define UTF8_CYR_R_LC       "\321\200"
#define UTF8_CYR_S_LC       "\321\201"
#define UTF8_CYR_W_LC       "\320\262"


#define UTF8_CJ_YEAR        "\345\271\264"
#define UTF8_CJ_MON         "\346\234\210"
#define UTF8_CJ_DAY         "\346\227\245"
#define UTF8_CJ_HOUR        "\346\231\202"
#define UTF8_CJ_MIN         "\345\210\206"
#define UTF8_CJ_SEC         "\347\247\222"


#define UTF8_CS_YEAR        "\345\271\264"
#define UTF8_CS_MON         "\346\234\210"
#define UTF8_CS_DAY         "\346\227\245"
#define UTF8_CS_HOUR        "\346\227\266"
#define UTF8_CS_MIN         "\345\210\206"
#define UTF8_CS_SEC         "\347\247\222"


#define UTF8_KO_YEAR        "\353\205\204"
#define UTF8_KO_MON         "\354\233\224"
#define UTF8_KO_DAY         "\354\235\274"
#define UTF8_KO_HOUR        "\354\213\234"
#define UTF8_KO_MIN         "\353\266\204"
#define UTF8_KO_SEC         "\354\264\210"



/** Default number format table. Last parent of all other tables, used for unknown locales. */
static const BuiltinFormat spBuiltinFormats_BASE[] =
{
    
    NUMFMT_PREDEF(   0, NUMBER_STANDARD ),          
    NUMFMT_PREDEF(   1, NUMBER_INT ),               
    NUMFMT_PREDEF(   2, NUMBER_DEC2 ),              
    NUMFMT_PREDEF(   3, NUMBER_1000INT ),           
    NUMFMT_PREDEF(   4, NUMBER_1000DEC2 ),          
    NUMFMT_PREDEF(   5, CURRENCY_1000INT ),         
    NUMFMT_PREDEF(   6, CURRENCY_1000INT_RED ),     
    NUMFMT_PREDEF(   7, CURRENCY_1000DEC2 ),        
    NUMFMT_PREDEF(   8, CURRENCY_1000DEC2_RED ),    
    NUMFMT_PREDEF(   9, PERCENT_INT ),              
    NUMFMT_PREDEF(  10, PERCENT_DEC2 ),             
    NUMFMT_PREDEF(  11, SCIENTIFIC_000E00 ),        
    NUMFMT_PREDEF(  12, FRACTION_1 ),               
    NUMFMT_PREDEF(  13, FRACTION_2 ),               

    
    NUMFMT_PREDEF(  14, DATE_SYS_DDMMYYYY ),
    NUMFMT_PREDEF(  15, DATE_SYS_DMMMYY ),
    NUMFMT_PREDEF(  16, DATE_SYS_DDMMM ),
    NUMFMT_PREDEF(  17, DATE_SYS_MMYY ),
    NUMFMT_PREDEF(  18, TIME_HHMMAMPM ),
    NUMFMT_PREDEF(  19, TIME_HHMMSSAMPM ),
    NUMFMT_PREDEF(  20, TIME_HHMM ),
    NUMFMT_PREDEF(  21, TIME_HHMMSS ),
    NUMFMT_PREDEF(  22, DATETIME_SYSTEM_SHORT_HHMM ),

    
    NUMFMT_REUSE(   23, 0 ),
    NUMFMT_REUSE(   24, 0 ),
    NUMFMT_REUSE(   25, 0 ),
    NUMFMT_REUSE(   26, 0 ),
    NUMFMT_REUSE(   27, 14 ),
    NUMFMT_REUSE(   28, 14 ),
    NUMFMT_REUSE(   29, 14 ),
    NUMFMT_REUSE(   30, 14 ),
    NUMFMT_REUSE(   31, 14 ),
    NUMFMT_REUSE(   32, 21 ),
    NUMFMT_REUSE(   33, 21 ),
    NUMFMT_REUSE(   34, 21 ),
    NUMFMT_REUSE(   35, 21 ),
    NUMFMT_REUSE(   36, 14 ),

    
    NUMFMT_CURRENCY_MINUS_SYMBOL_NUMBER( 37, "", "", "" ),
    NUMFMT_ACCOUNTING_MINUS_SYMBOL_NUMBER( 41, "", "" ),

    
    NUMFMT_STRING(  45, "mm:ss" ),
    NUMFMT_STRING(  46, "[h]:mm:ss" ),
    NUMFMT_STRING(  47, "mm:ss.0" ),
    NUMFMT_STRING(  48, "##0.0E+0" ),
    NUMFMT_PREDEF(  49, TEXT ),

    
    NUMFMT_REUSE(   50, 14 ),
    NUMFMT_REUSE(   51, 14 ),
    NUMFMT_REUSE(   52, 14 ),
    NUMFMT_REUSE(   53, 14 ),
    NUMFMT_REUSE(   54, 14 ),
    NUMFMT_REUSE(   55, 14 ),
    NUMFMT_REUSE(   56, 14 ),
    NUMFMT_REUSE(   57, 14 ),
    NUMFMT_REUSE(   58, 14 ),
    NUMFMT_REUSE(   59, 1 ),
    NUMFMT_REUSE(   60, 2 ),
    NUMFMT_REUSE(   61, 3 ),
    NUMFMT_REUSE(   62, 4 ),
    NUMFMT_REUSE(   63, 5 ),
    NUMFMT_REUSE(   64, 6 ),
    NUMFMT_REUSE(   65, 7 ),
    NUMFMT_REUSE(   66, 8 ),
    NUMFMT_REUSE(   67, 9 ),
    NUMFMT_REUSE(   68, 10 ),
    NUMFMT_REUSE(   69, 12 ),
    NUMFMT_REUSE(   70, 13 ),
    NUMFMT_REUSE(   71, 14 ),
    NUMFMT_REUSE(   72, 14 ),
    NUMFMT_REUSE(   73, 15 ),
    NUMFMT_REUSE(   74, 16 ),
    NUMFMT_REUSE(   75, 17 ),
    NUMFMT_REUSE(   76, 20 ),
    NUMFMT_REUSE(   77, 21 ),
    NUMFMT_REUSE(   78, 22 ),
    NUMFMT_REUSE(   79, 45 ),
    NUMFMT_REUSE(   80, 46 ),
    NUMFMT_REUSE(   81, 47 ),

    

    NUMFMT_ENDTABLE()
};



/** Arabic, U.A.E. */
static const BuiltinFormat spBuiltinFormats_ar_AE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_AE "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Bahrain. */
static const BuiltinFormat spBuiltinFormats_ar_BH[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_BH "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Algeria. */
static const BuiltinFormat spBuiltinFormats_ar_DZ[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_DZ "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Egypt. */
static const BuiltinFormat spBuiltinFormats_ar_EG[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_EG "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Iraq. */
static const BuiltinFormat spBuiltinFormats_ar_IQ[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_IQ "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Jordan. */
static const BuiltinFormat spBuiltinFormats_ar_JO[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_JO "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Kuwait. */
static const BuiltinFormat spBuiltinFormats_ar_KW[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_KW "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Lebanon. */
static const BuiltinFormat spBuiltinFormats_ar_LB[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_LB "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Libya. */
static const BuiltinFormat spBuiltinFormats_ar_LY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_LY "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Morocco. */
static const BuiltinFormat spBuiltinFormats_ar_MA[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_MA "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Oman. */
static const BuiltinFormat spBuiltinFormats_ar_OM[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_OM "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Qatar. */
static const BuiltinFormat spBuiltinFormats_ar_QA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_QA "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Saudi Arabia. */
static const BuiltinFormat spBuiltinFormats_ar_SA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_SA "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Syria. */
static const BuiltinFormat spBuiltinFormats_ar_SY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_SY "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Tunisia. */
static const BuiltinFormat spBuiltinFormats_ar_TN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_TN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Arabic, Yemen. */
static const BuiltinFormat spBuiltinFormats_ar_YE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_YE "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Belarusian, Belarus. */
static const BuiltinFormat spBuiltinFormats_be_BY[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_R_LC ".\"", "_" UTF8_CYR_R_LC "_.", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Bulgarian, Bulgaria. */
static const BuiltinFormat spBuiltinFormats_bg_BG[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.M.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_L_LC UTF8_CYR_W_LC "\"", "_" UTF8_CYR_L_LC "_" UTF8_CYR_W_LC, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Bengali, India. */
static const BuiltinFormat spBuiltinFormats_bn_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_BN_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Czech, Czech Republic. */
static const BuiltinFormat spBuiltinFormats_cs_CZ[] =
{
    
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"K" UTF8_CCARON_LC "\"", "_K_" UTF8_CCARON_LC, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Danish, Denmark. */
static const BuiltinFormat spBuiltinFormats_da_DK[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"kr\"", " " ),
    NUMFMT_ENDTABLE()
};

/** German, Austria. */
static const BuiltinFormat spBuiltinFormats_de_AT[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** German, Switzerland. */
static const BuiltinFormat spBuiltinFormats_de_CH[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ". ", "MMM", " ", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"SFr.\"", " " ),
    NUMFMT_ENDTABLE()
};

/** German, Germany. */
static const BuiltinFormat spBuiltinFormats_de_DE[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ". ", "MMM", " ", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** German, Liechtenstein. */
static const BuiltinFormat spBuiltinFormats_de_LI[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ". ", "MMM", " ", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"CHF\"", " " ),
    NUMFMT_ENDTABLE()
};

/** German, Luxembourg. */
static const BuiltinFormat spBuiltinFormats_de_LU[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** Divehi, Maldives. */
static const BuiltinFormat spBuiltinFormats_div_MV[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_NUMBER_SYMBOL_MINUS( "\"" UTF8_RUFIYAA ".\"", "_" UTF8_RUFIYAA "_.", " " ),
    NUMFMT_ENDTABLE()
};

/** Greek, Greece. */
static const BuiltinFormat spBuiltinFormats_el_GR[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** English, Australia. */
static const BuiltinFormat spBuiltinFormats_en_AU[] =
{
    NUMFMT_ALLDATETIMES( "D/MM/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** English, Belize. */
static const BuiltinFormat spBuiltinFormats_en_BZ[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"BZ$\"", "" ),
    NUMFMT_ENDTABLE()
};

/** English, Canada. */
static const BuiltinFormat spBuiltinFormats_en_CA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** English, Caribbean. */
static const BuiltinFormat spBuiltinFormats_en_CB[] =
{
    NUMFMT_ALLDATETIMES( "MM/DD/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** English, United Kingdom. */
static const BuiltinFormat spBuiltinFormats_en_GB[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_POUND_GB, "" ),
    NUMFMT_ENDTABLE()
};

/** English, Ireland. */
static const BuiltinFormat spBuiltinFormats_en_IE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, "" ),
    NUMFMT_ENDTABLE()
};

/** English, Jamaica. */
static const BuiltinFormat spBuiltinFormats_en_JM[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\"J$\"", "" ),
    NUMFMT_ENDTABLE()
};

/** English, New Zealand. */
static const BuiltinFormat spBuiltinFormats_en_NZ[] =
{
    NUMFMT_ALLDATETIMES( "D/MM/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** English, Philippines. */
static const BuiltinFormat spBuiltinFormats_en_PH[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Php\"", "" ),
    NUMFMT_ENDTABLE()
};

/** English, Trinidad and Tobago. */
static const BuiltinFormat spBuiltinFormats_en_TT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"TT$\"", "" ),
    NUMFMT_ENDTABLE()
};

/** English, USA. */
static const BuiltinFormat spBuiltinFormats_en_US[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** English, South Africa. */
static const BuiltinFormat spBuiltinFormats_en_ZA[] =
{
    NUMFMT_ALLDATETIMES( "YYYY/MM/DD", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\\R", " " ),
    NUMFMT_ENDTABLE()
};

/** English, Zimbabwe. */
static const BuiltinFormat spBuiltinFormats_en_ZW[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Z$\"", "" ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Argentina. */
static const BuiltinFormat spBuiltinFormats_es_AR[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "$", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Bolivia. */
static const BuiltinFormat spBuiltinFormats_es_BO[] =
{
    
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"$b\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Chile. */
static const BuiltinFormat spBuiltinFormats_es_CL[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Colombia. */
static const BuiltinFormat spBuiltinFormats_es_CO[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Costa Rica. */
static const BuiltinFormat spBuiltinFormats_es_CR[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( UTF8_COLON, "" ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Dominican Republic. */
static const BuiltinFormat spBuiltinFormats_es_DO[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"RD$\"", "" ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Ecuador. */
static const BuiltinFormat spBuiltinFormats_es_EC[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Spain. */
static const BuiltinFormat spBuiltinFormats_es_ES[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Guatemala. */
static const BuiltinFormat spBuiltinFormats_es_GT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\Q", "" ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Honduras. */
static const BuiltinFormat spBuiltinFormats_es_HN[] =
{
    
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"L.\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Mexico. */
static const BuiltinFormat spBuiltinFormats_es_MX[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Nicaragua. */
static const BuiltinFormat spBuiltinFormats_es_NI[] =
{
    
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"C$\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Panama. */
static const BuiltinFormat spBuiltinFormats_es_PA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"B/.\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Peru. */
static const BuiltinFormat spBuiltinFormats_es_PE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"S/.\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Puerto Rico. */
static const BuiltinFormat spBuiltinFormats_es_PR[] =
{
    
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Paraguay. */
static const BuiltinFormat spBuiltinFormats_es_PY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Gs\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, El Salvador. */
static const BuiltinFormat spBuiltinFormats_es_SV[] =
{
    
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Uruguay. */
static const BuiltinFormat spBuiltinFormats_es_UY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"$U\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Spanish, Venezuela. */
static const BuiltinFormat spBuiltinFormats_es_VE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "Bs", " " ),
    NUMFMT_ENDTABLE()
};

/** Estonian, Estonia. */
static const BuiltinFormat spBuiltinFormats_et_EE[] =
{
    
    NUMFMT_ALLDATETIMES( "D.MM.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kr\"", "_k_r", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Farsi, Iran. */
static const BuiltinFormat spBuiltinFormats_fa_IR[] =
{
    NUMFMT_ALLDATETIMES( "YYYY/MM/DD", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_FA_IR "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Finnish, Finland. */
static const BuiltinFormat spBuiltinFormats_fi_FI[] =
{
    
    NUMFMT_STRING(  9, "0\\ %" ),
    NUMFMT_STRING( 10, "0.00\\ %" ),
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Faroese, Faroe Islands. */
static const BuiltinFormat spBuiltinFormats_fo_FO[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"kr\"", " " ),
    NUMFMT_ENDTABLE()
};

/** French, Belgium. */
static const BuiltinFormat spBuiltinFormats_fr_BE[] =
{
    NUMFMT_ALLDATETIMES( "D/MM/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** French, Canada. */
static const BuiltinFormat spBuiltinFormats_fr_CA[] =
{
    
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_NUMBER_SYMBOL_CLOSE( "$", "_$", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** French, Switzerland. */
static const BuiltinFormat spBuiltinFormats_fr_CH[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"SFr.\"", " " ),
    NUMFMT_ENDTABLE()
};

/** French, France. */
static const BuiltinFormat spBuiltinFormats_fr_FR[] =
{
    
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** French, Luxembourg. */
static const BuiltinFormat spBuiltinFormats_fr_LU[] =
{
    
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** French, Monaco. */
static const BuiltinFormat spBuiltinFormats_fr_MC[] =
{
    
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Galizian, Spain. */
static const BuiltinFormat spBuiltinFormats_gl_ES[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** Gujarati, India. */
static const BuiltinFormat spBuiltinFormats_gu_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_GU_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Hebrew, Israel. */
static const BuiltinFormat spBuiltinFormats_he_IL[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( UTF8_SHEQEL, " " ),
    NUMFMT_ENDTABLE()
};

/** Hindi, India. */
static const BuiltinFormat spBuiltinFormats_hi_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_HI_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Croatian, Bosnia and Herzegowina. */
static const BuiltinFormat spBuiltinFormats_hr_BA[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"KM\"", "_K_M", " " ),
    NUMFMT_ENDTABLE()
};

/** Croatian, Croatia. */
static const BuiltinFormat spBuiltinFormats_hr_HR[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kn\"", "_k_n", " " ),
    NUMFMT_ENDTABLE()
};

/** Hungarian, Hungary. */
static const BuiltinFormat spBuiltinFormats_hu_HU[] =
{
    
    
    NUMFMT_ALLDATETIMES( "YYYY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Ft\"", "_F_t", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Armenian, Armenia. */
static const BuiltinFormat spBuiltinFormats_hy_AM[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_HY_DA_LC UTF8_HY_REH_LC ".\"", "_" UTF8_HY_DA_LC "_" UTF8_HY_REH_LC "_.", " " ),
    NUMFMT_ENDTABLE()
};

/** Indonesian, Indonesia. */
static const BuiltinFormat spBuiltinFormats_id_ID[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Rp\"", "" ),
    NUMFMT_ENDTABLE()
};

/** Icelandic, Iceland. */
static const BuiltinFormat spBuiltinFormats_is_IS[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kr.\"", "_k_r_.", " " ),
    NUMFMT_ENDTABLE()
};

/** Italian, Switzerland. */
static const BuiltinFormat spBuiltinFormats_it_CH[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"SFr.\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Italian, Italy. */
static const BuiltinFormat spBuiltinFormats_it_IT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** Georgian, Georgia. */
static const BuiltinFormat spBuiltinFormats_ka_GE[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Lari\"", "_L_a_r_i", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Kazakh, Kazakhstan. */
static const BuiltinFormat spBuiltinFormats_kk_KZ[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\\T", "" ),
    NUMFMT_ENDTABLE()
};

/** Kannada, India. */
static const BuiltinFormat spBuiltinFormats_kn_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_KN_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Kyrgyz, Kyrgyzstan. */
static const BuiltinFormat spBuiltinFormats_ky_KG[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_S_LC UTF8_CYR_O_LC UTF8_CYR_M_LC "\"", "_" UTF8_CYR_S_LC "_" UTF8_CYR_O_LC "_" UTF8_CYR_M_LC, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Lithuanian, Lithuania. */
static const BuiltinFormat spBuiltinFormats_lt_LT[] =
{
    NUMFMT_ALLDATETIMES( "YYYY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Lt\"", "_L_t", " " ),
    NUMFMT_ENDTABLE()
};

/** Latvian, Latvia. */
static const BuiltinFormat spBuiltinFormats_lv_LV[] =
{
    
    NUMFMT_ALLDATETIMES( "YYYY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\"Ls\"", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Malayalam, India. */
static const BuiltinFormat spBuiltinFormats_ml_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_ML_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Mongolian, Mongolia. */
static const BuiltinFormat spBuiltinFormats_mn_MN[] =
{
    NUMFMT_ALLDATETIMES( "YY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_TUGRUG, "_" UTF8_TUGRUG, "" ),
    NUMFMT_ENDTABLE()
};

/** Malay, Brunei Darussalam. */
static const BuiltinFormat spBuiltinFormats_ms_BN[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
    NUMFMT_ENDTABLE()
};

/** Malay, Malaysia. */
static const BuiltinFormat spBuiltinFormats_ms_MY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\R", "" ),
    NUMFMT_ENDTABLE()
};

/** Maltese, Malta. */
static const BuiltinFormat spBuiltinFormats_mt_MT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\"Lm\"", "" ),
    NUMFMT_ENDTABLE()
};

/** Dutch, Belgium. */
static const BuiltinFormat spBuiltinFormats_nl_BE[] =
{
    
    NUMFMT_ALLDATETIMES( "D\\/MM\\/YYYY", "D", "\\/", "MMM", "\\/", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** Dutch, Netherlands. */
static const BuiltinFormat spBuiltinFormats_nl_NL[] =
{
    NUMFMT_ALLDATETIMES( "D-M-YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** Norwegian (Bokmal and Nynorsk), Norway. */
static const BuiltinFormat spBuiltinFormats_no_NO[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"kr\"", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Punjabi, India. */
static const BuiltinFormat spBuiltinFormats_pa_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_PA_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Polish, Poland. */
static const BuiltinFormat spBuiltinFormats_pl_PL[] =
{
    
    
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"z" UTF8_LSTROKE_LC "\"", "_z_" UTF8_LSTROKE_LC, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Portugese, Brazil. */
static const BuiltinFormat spBuiltinFormats_pt_BR[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "/", "MMM", "/", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"R$\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Portugese, Portugal. */
static const BuiltinFormat spBuiltinFormats_pt_PT[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
    NUMFMT_ENDTABLE()
};

/** Romanian, Romania. */
static const BuiltinFormat spBuiltinFormats_ro_RO[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"lei\"", "_l_e_i", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Russian, Russian Federation. */
static const BuiltinFormat spBuiltinFormats_ru_RU[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_R_LC ".\"", "_" UTF8_CYR_R_LC "_.", "" ),
    NUMFMT_ENDTABLE()
};

/** Slovak, Slovakia. */
static const BuiltinFormat spBuiltinFormats_sk_SK[] =
{
    
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Sk\"", "_S_k", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Slovenian, Slovenia. */
static const BuiltinFormat spBuiltinFormats_sl_SI[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"SIT\"", "_S_I_T", " " ),
    NUMFMT_ENDTABLE()
};

/** Swedish, Finland. */
static const BuiltinFormat spBuiltinFormats_sv_FI[] =
{
    
    NUMFMT_STRING(  9, "0\\ %" ),
    NUMFMT_STRING( 10, "0.00\\ %" ),
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Swedish, Sweden. */
static const BuiltinFormat spBuiltinFormats_sv_SE[] =
{
    
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kr\"", "_k_r", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Swahili, Tanzania. */
static const BuiltinFormat spBuiltinFormats_sw_TZ[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\S", "" ),
    NUMFMT_ENDTABLE()
};

/** Tamil, India. */
static const BuiltinFormat spBuiltinFormats_ta_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_TA_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Telugu, India. */
static const BuiltinFormat spBuiltinFormats_te_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_TE_IN "\"", " " ),
    NUMFMT_ENDTABLE()
};

/** Thai, Thailand. */
static const BuiltinFormat spBuiltinFormats_th_TH[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_BAHT, "" ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 63, UTF8_BAHT, "", "t" ),
    NUMFMT_STRING( 59, "t0" ),
    NUMFMT_STRING( 60, "t0.00" ),
    NUMFMT_STRING( 61, "t#,##0" ),
    NUMFMT_STRING( 62, "t#,##0.00" ),
    NUMFMT_STRING( 67, "t0%" ),
    NUMFMT_STRING( 68, "t0.00%" ),
    NUMFMT_STRING( 69, "t# ?/?" ),
    NUMFMT_STRING( 70, "t# ?\?/?\?" ),
    NUMFMT_STRING( 71, "tD/M/EE" ),
    NUMFMT_STRING( 72, "tD-MMM-E" ),
    NUMFMT_STRING( 73, "tD-MMM" ),
    NUMFMT_STRING( 74, "tMMM-E" ),
    NUMFMT_STRING( 75, "th:mm" ),
    NUMFMT_STRING( 76, "th:mm:ss" ),
    NUMFMT_STRING( 77, "tD/M/EE h:mm" ),
    NUMFMT_STRING( 78, "tmm:ss" ),
    NUMFMT_STRING( 79, "t[h]:mm:ss" ),
    NUMFMT_STRING( 80, "tmm:ss.0" ),
    NUMFMT_STRING( 81, "D/M/E" ),
    NUMFMT_ENDTABLE()
};

/** Turkish, Turkey. */
static const BuiltinFormat spBuiltinFormats_tr_TR[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"TL\"", "_T_L", " " ),
    NUMFMT_ENDTABLE()
};

/** Tatar, Russian Federation. */
static const BuiltinFormat spBuiltinFormats_tt_RU[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_R_LC ".\"", "_" UTF8_CYR_R_LC "_.", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Ukrainian, Ukraine. */
static const BuiltinFormat spBuiltinFormats_uk_UA[] =
{
    
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_G_LC UTF8_CYR_R_LC UTF8_CYR_N_LC ".\"", "_" UTF8_CYR_G_LC "_" UTF8_CYR_R_LC "_" UTF8_CYR_N_LC "_.", "\\ " ),
    NUMFMT_ENDTABLE()
};

/** Urdu, Pakistan. */
static const BuiltinFormat spBuiltinFormats_ur_PK[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"Rs\"", "" ),
    NUMFMT_ENDTABLE()
};

/** Vietnamese, Viet Nam. */
static const BuiltinFormat spBuiltinFormats_vi_VN[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_DONG, "_" UTF8_DONG, " " ),
    NUMFMT_ENDTABLE()
};



/** Base table for CJK locales. */
static const BuiltinFormat spBuiltinFormats_CJK[] =
{
    NUMFMT_REUSE( 29, 28 ),
    NUMFMT_REUSE( 36, 27 ),
    NUMFMT_REUSE( 50, 27 ),
    NUMFMT_REUSE( 51, 28 ),
    NUMFMT_REUSE( 52, 34 ),
    NUMFMT_REUSE( 53, 35 ),
    NUMFMT_REUSE( 54, 28 ),
    NUMFMT_REUSE( 55, 34 ),
    NUMFMT_REUSE( 56, 35 ),
    NUMFMT_REUSE( 57, 27 ),
    NUMFMT_REUSE( 58, 28 ),
    NUMFMT_ENDTABLE()
};

/** Japanese, Japan. */
static const BuiltinFormat spBuiltinFormats_ja_JP[] =
{
    NUMFMT_ALLDATETIMES( "YYYY/MM/DD", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_YEN_JP, "" ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "$", "", "" ),
    NUMFMT_STRING( 27, "[$-411]GE.MM.DD" ),
    NUMFMT_STRING( 28, "[$-411]GGGE\"" UTF8_CJ_YEAR "\"MM\"" UTF8_CJ_MON "\"DD\"" UTF8_CJ_DAY "\"" ),
    NUMFMT_STRING( 30, "MM/DD/YY" ),
    NUMFMT_STRING( 31, "YYYY\"" UTF8_CJ_YEAR "\"MM\"" UTF8_CJ_MON "\"DD\"" UTF8_CJ_DAY "\"" ),
    NUMFMT_TIME_CJK( 32, "h", UTF8_CJ_HOUR, UTF8_CJ_MIN, UTF8_CJ_SEC ),
    NUMFMT_STRING( 34, "YYYY\"" UTF8_CJ_YEAR "\"MM\"" UTF8_CJ_MON "\"" ),
    NUMFMT_STRING( 35, "MM\"" UTF8_CJ_MON "\"DD\"" UTF8_CJ_DAY "\"" ),
    NUMFMT_ENDTABLE()
};

/** Korean, South Korea. */
static const BuiltinFormat spBuiltinFormats_ko_KR[] =
{
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_WON, "" ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "$", "", "" ),
    NUMFMT_STRING( 27, "YYYY" UTF8_CJ_YEAR " MM" UTF8_CJ_MON " DD" UTF8_CJ_DAY ),
    NUMFMT_STRING( 28, "MM-DD" ),
    NUMFMT_STRING( 30, "MM-DD-YY" ),
    NUMFMT_STRING( 31, "YYYY" UTF8_KO_YEAR " MM" UTF8_KO_MON " DD" UTF8_KO_DAY ),
    NUMFMT_TIME_CJK( 32, "h", UTF8_KO_HOUR, UTF8_KO_MIN, UTF8_KO_SEC ),
    
    NUMFMT_STRING( 34, "YYYY\\/MM\\/DD" ),
    NUMFMT_REUSE( 35, 14 ),
    NUMFMT_ENDTABLE()
};

/** Chinese, China. */
static const BuiltinFormat spBuiltinFormats_zh_CN[] =
{
    NUMFMT_ALLDATETIMES( "YYYY-M-D", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( UTF8_YEN_CN, "" ),
    NUMFMT_ALLTIMES_CJK( "h", "h", UTF8_CS_HOUR, UTF8_CS_MIN, UTF8_CS_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "$", "", "" ),
    NUMFMT_STRING( 27, "YYYY\"" UTF8_CS_YEAR "\"M\"" UTF8_CS_MON "\"" ),
    NUMFMT_STRING( 28, "M\"" UTF8_CS_MON "\"D\"" UTF8_CS_DAY "\"" ),
    NUMFMT_STRING( 30, "M-D-YY" ),
    NUMFMT_STRING( 31, "YYYY\"" UTF8_CS_YEAR "\"M\"" UTF8_CS_MON "\"D\"" UTF8_CS_DAY "\"" ),
    NUMFMT_REUSE( 52, 27 ),
    NUMFMT_REUSE( 53, 28 ),
    NUMFMT_ENDTABLE()
};

/** Chinese, Hong Kong. */
static const BuiltinFormat spBuiltinFormats_zh_HK[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"HK$\"", "" ),
    NUMFMT_ALLTIMES_CJK( "h", "h", UTF8_CJ_HOUR, UTF8_CJ_MIN, UTF8_CJ_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "\"US$\"", "", "" ),
    NUMFMT_STRING( 27, "[$-404]D/M/E" ),
    NUMFMT_STRING( 28, "[$-404]D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"E\"" UTF8_CJ_YEAR "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"YYYY\"" UTF8_CJ_YEAR "\"" ),
    NUMFMT_ENDTABLE()
};

/** Chinese, Macau. */
static const BuiltinFormat spBuiltinFormats_zh_MO[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\P", "" ),
    NUMFMT_ALLTIMES_CJK( "h", "h", UTF8_CJ_HOUR, UTF8_CJ_MIN, UTF8_CJ_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "\"US$\"", "", "" ),
    NUMFMT_STRING( 27, "[$-404]D/M/E" ),
    NUMFMT_STRING( 28, "[$-404]D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"E\"" UTF8_CJ_YEAR "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"YYYY\"" UTF8_CJ_YEAR "\"" ),
    NUMFMT_ENDTABLE()
};

/** Chinese, Singapore. */
static const BuiltinFormat spBuiltinFormats_zh_SG[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
    NUMFMT_ALLTIMES_CJK( "h", "h", UTF8_CS_HOUR, UTF8_CS_MIN, UTF8_CS_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "$", "", "" ),
    NUMFMT_STRING( 27, "YYYY\"" UTF8_CS_YEAR "\"M\"" UTF8_CS_MON "\"" ),
    NUMFMT_STRING( 28, "M\"" UTF8_CS_MON "\"D\"" UTF8_CS_DAY "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "D\"" UTF8_CS_DAY "\"M\"" UTF8_CS_MON "\"YYYY\"" UTF8_CS_YEAR "\"" ),
    NUMFMT_ENDTABLE()
};

/** Chinese, Taiwan. */
static const BuiltinFormat spBuiltinFormats_zh_TW[] =
{
    NUMFMT_ALLDATETIMES( "YYYY/M/D", "D", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
    NUMFMT_ALLTIMES_CJK( "hh", "hh", UTF8_CJ_HOUR, UTF8_CJ_MIN, UTF8_CJ_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "\"US$\"", "", "" ),
    NUMFMT_STRING( 27, "[$-404]E/M/D" ),
    NUMFMT_STRING( 28, "[$-404]E\"" UTF8_CJ_YEAR "\"M\"" UTF8_CJ_MON "\"D\"" UTF8_CJ_DAY "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "YYYY\"" UTF8_CJ_YEAR "\"M\"" UTF8_CJ_MON "\"D\"" UTF8_CJ_DAY "\"" ),
    NUMFMT_ENDTABLE()
};



/** Specifies a built-in number format table for a specific locale. */
struct BuiltinFormatTable
{
    const sal_Char*     mpcLocale;          
    const sal_Char*     mpcParent;          
    const BuiltinFormat* mpFormats;         
};

static const BuiltinFormatTable spBuiltinFormatTables[] =
{ 
    { "*",      "",         spBuiltinFormats_BASE   },  
    { "af-ZA",  "*",        spBuiltinFormats_en_ZA  },  
    { "ar-AE",  "*",        spBuiltinFormats_ar_AE  },  
    { "ar-BH",  "*",        spBuiltinFormats_ar_BH  },  
    { "ar-DZ",  "*",        spBuiltinFormats_ar_DZ  },  
    { "ar-EG",  "*",        spBuiltinFormats_ar_EG  },  
    { "ar-IQ",  "*",        spBuiltinFormats_ar_IQ  },  
    { "ar-JO",  "*",        spBuiltinFormats_ar_JO  },  
    { "ar-KW",  "*",        spBuiltinFormats_ar_KW  },  
    { "ar-LB",  "*",        spBuiltinFormats_ar_LB  },  
    { "ar-LY",  "*",        spBuiltinFormats_ar_LY  },  
    { "ar-MA",  "*",        spBuiltinFormats_ar_MA  },  
    { "ar-OM",  "*",        spBuiltinFormats_ar_OM  },  
    { "ar-QA",  "*",        spBuiltinFormats_ar_QA  },  
    { "ar-SA",  "*",        spBuiltinFormats_ar_SA  },  
    { "ar-SY",  "*",        spBuiltinFormats_ar_SY  },  
    { "ar-TN",  "*",        spBuiltinFormats_ar_TN  },  
    { "ar-YE",  "*",        spBuiltinFormats_ar_YE  },  
    { "be-BY",  "*",        spBuiltinFormats_be_BY  },  
    { "bg-BG",  "*",        spBuiltinFormats_bg_BG  },  
    { "bn-IN",  "*",        spBuiltinFormats_bn_IN  },  
    { "ca-ES",  "*",        spBuiltinFormats_es_ES  },  
    { "cs-CZ",  "*",        spBuiltinFormats_cs_CZ  },  
    { "cy-GB",  "*",        spBuiltinFormats_en_GB  },  
    { "da-DK",  "*",        spBuiltinFormats_da_DK  },  
    { "de-AT",  "*",        spBuiltinFormats_de_AT  },  
    { "de-CH",  "*",        spBuiltinFormats_de_CH  },  
    { "de-DE",  "*",        spBuiltinFormats_de_DE  },  
    { "de-LI",  "*",        spBuiltinFormats_de_LI  },  
    { "de-LU",  "*",        spBuiltinFormats_de_LU  },  
    { "div-MV", "*",        spBuiltinFormats_div_MV },  
    { "el-GR",  "*",        spBuiltinFormats_el_GR  },  
    { "en-AU",  "*",        spBuiltinFormats_en_AU  },  
    { "en-BZ",  "*",        spBuiltinFormats_en_BZ  },  
    { "en-CA",  "*",        spBuiltinFormats_en_CA  },  
    { "en-CB",  "*",        spBuiltinFormats_en_CB  },  
    { "en-GB",  "*",        spBuiltinFormats_en_GB  },  
    { "en-IE",  "*",        spBuiltinFormats_en_IE  },  
    { "en-JM",  "*",        spBuiltinFormats_en_JM  },  
    { "en-NZ",  "*",        spBuiltinFormats_en_NZ  },  
    { "en-PH",  "*",        spBuiltinFormats_en_PH  },  
    { "en-TT",  "*",        spBuiltinFormats_en_TT  },  
    { "en-US",  "*",        spBuiltinFormats_en_US  },  
    { "en-ZA",  "*",        spBuiltinFormats_en_ZA  },  
    { "en-ZW",  "*",        spBuiltinFormats_en_ZW  },  
    { "es-AR",  "*",        spBuiltinFormats_es_AR  },  
    { "es-BO",  "*",        spBuiltinFormats_es_BO  },  
    { "es-CL",  "*",        spBuiltinFormats_es_CL  },  
    { "es-CO",  "*",        spBuiltinFormats_es_CO  },  
    { "es-CR",  "*",        spBuiltinFormats_es_CR  },  
    { "es-DO",  "*",        spBuiltinFormats_es_DO  },  
    { "es-EC",  "*",        spBuiltinFormats_es_EC  },  
    { "es-ES",  "*",        spBuiltinFormats_es_ES  },  
    { "es-GT",  "*",        spBuiltinFormats_es_GT  },  
    { "es-HN",  "*",        spBuiltinFormats_es_HN  },  
    { "es-MX",  "*",        spBuiltinFormats_es_MX  },  
    { "es-NI",  "*",        spBuiltinFormats_es_NI  },  
    { "es-PA",  "*",        spBuiltinFormats_es_PA  },  
    { "es-PE",  "*",        spBuiltinFormats_es_PE  },  
    { "es-PR",  "*",        spBuiltinFormats_es_PR  },  
    { "es-PY",  "*",        spBuiltinFormats_es_PY  },  
    { "es-SV",  "*",        spBuiltinFormats_es_SV  },  
    { "es-UY",  "*",        spBuiltinFormats_es_UY  },  
    { "es-VE",  "*",        spBuiltinFormats_es_VE  },  
    { "et-EE",  "*",        spBuiltinFormats_et_EE  },  
    { "fa-IR",  "*",        spBuiltinFormats_fa_IR  },  
    { "fi-FI",  "*",        spBuiltinFormats_fi_FI  },  
    { "fo-FO",  "*",        spBuiltinFormats_fo_FO  },  
    { "fr-BE",  "*",        spBuiltinFormats_fr_BE  },  
    { "fr-CA",  "*",        spBuiltinFormats_fr_CA  },  
    { "fr-CH",  "*",        spBuiltinFormats_fr_CH  },  
    { "fr-FR",  "*",        spBuiltinFormats_fr_FR  },  
    { "fr-LU",  "*",        spBuiltinFormats_fr_LU  },  
    { "fr-MC",  "*",        spBuiltinFormats_fr_MC  },  
    { "gl-ES",  "*",        spBuiltinFormats_gl_ES  },  
    { "gu-IN",  "*",        spBuiltinFormats_gu_IN  },  
    { "he-IL",  "*",        spBuiltinFormats_he_IL  },  
    { "hi-IN",  "*",        spBuiltinFormats_hi_IN  },  
    { "hr-BA",  "*",        spBuiltinFormats_hr_BA  },  
    { "hr-HR",  "*",        spBuiltinFormats_hr_HR  },  
    { "hu-HU",  "*",        spBuiltinFormats_hu_HU  },  
    { "hy-AM",  "*",        spBuiltinFormats_hy_AM  },  
    { "id-ID",  "*",        spBuiltinFormats_id_ID  },  
    { "is-IS",  "*",        spBuiltinFormats_is_IS  },  
    { "it-CH",  "*",        spBuiltinFormats_it_CH  },  
    { "it-IT",  "*",        spBuiltinFormats_it_IT  },  
    { "ka-GE",  "*",        spBuiltinFormats_ka_GE  },  
    { "kk-KZ",  "*",        spBuiltinFormats_kk_KZ  },  
    { "kn-IN",  "*",        spBuiltinFormats_kn_IN  },  
    { "kok-IN", "*",        spBuiltinFormats_hi_IN  },  
    { "ky-KG",  "*",        spBuiltinFormats_ky_KG  },  
    { "lt-LT",  "*",        spBuiltinFormats_lt_LT  },  
    { "lv-LV",  "*",        spBuiltinFormats_lv_LV  },  
    { "mi-NZ",  "*",        spBuiltinFormats_en_NZ  },  
    { "ml-IN",  "*",        spBuiltinFormats_ml_IN  },  
    { "mn-MN",  "*",        spBuiltinFormats_mn_MN  },  
    { "mr-IN",  "*",        spBuiltinFormats_hi_IN  },  
    { "ms-BN",  "*",        spBuiltinFormats_ms_BN  },  
    { "ms-MY",  "*",        spBuiltinFormats_ms_MY  },  
    { "mt-MT",  "*",        spBuiltinFormats_mt_MT  },  
    { "nb-NO",  "*",        spBuiltinFormats_no_NO  },  
    { "nl-BE",  "*",        spBuiltinFormats_nl_BE  },  
    { "nl-NL",  "*",        spBuiltinFormats_nl_NL  },  
    { "nn-NO",  "*",        spBuiltinFormats_no_NO  },  
    { "nso-ZA", "*",        spBuiltinFormats_en_ZA  },  
    { "pa-IN",  "*",        spBuiltinFormats_pa_IN  },  
    { "pl-PL",  "*",        spBuiltinFormats_pl_PL  },  
    { "pt-BR",  "*",        spBuiltinFormats_pt_BR  },  
    { "pt-PT",  "*",        spBuiltinFormats_pt_PT  },  
    { "qu-BO",  "*",        spBuiltinFormats_es_BO  },  
    { "qu-EC",  "*",        spBuiltinFormats_es_EC  },  
    { "qu-PE",  "*",        spBuiltinFormats_es_PE  },  
    { "ro-RO",  "*",        spBuiltinFormats_ro_RO  },  
    { "ru-RU",  "*",        spBuiltinFormats_ru_RU  },  
    { "sa-IN",  "*",        spBuiltinFormats_hi_IN  },  
    { "se-FI",  "*",        spBuiltinFormats_fi_FI  },  
    { "se-NO",  "*",        spBuiltinFormats_no_NO  },  
    { "se-SE",  "*",        spBuiltinFormats_sv_SE  },  
    { "sk-SK",  "*",        spBuiltinFormats_sk_SK  },  
    { "sl-SI",  "*",        spBuiltinFormats_sl_SI  },  
    { "sv-FI",  "*",        spBuiltinFormats_sv_FI  },  
    { "sv-SE",  "*",        spBuiltinFormats_sv_SE  },  
    { "sw-TZ",  "*",        spBuiltinFormats_sw_TZ  },  
    { "syr-SY", "*",        spBuiltinFormats_ar_SY  },  
    { "syr-TR", "*",        spBuiltinFormats_tr_TR  },  
    { "ta-IN",  "*",        spBuiltinFormats_ta_IN  },  
    { "te-IN",  "*",        spBuiltinFormats_te_IN  },  
    { "th-TH",  "*",        spBuiltinFormats_th_TH  },  
    { "tn-ZA",  "*",        spBuiltinFormats_en_ZA  },  
    { "tr-TR",  "*",        spBuiltinFormats_tr_TR  },  
    { "tt-RU",  "*",        spBuiltinFormats_tt_RU  },  
    { "uk-UA",  "*",        spBuiltinFormats_uk_UA  },  
    { "ur-PK",  "*",        spBuiltinFormats_ur_PK  },  
    { "vi-VN",  "*",        spBuiltinFormats_vi_VN  },  
    { "xh-ZA",  "*",        spBuiltinFormats_en_ZA  },  
    { "zu-ZA",  "*",        spBuiltinFormats_en_ZA  },  

    { "*CJK",   "*",        spBuiltinFormats_CJK    },  
    { "ja-JP",  "*CJK",     spBuiltinFormats_ja_JP  },  
    { "ko-KR",  "*CJK",     spBuiltinFormats_ko_KR  },  
    { "zh-CN",  "*CJK",     spBuiltinFormats_zh_CN  },  
    { "zh-HK",  "*CJK",     spBuiltinFormats_zh_HK  },  
    { "zh-MO",  "*CJK",     spBuiltinFormats_zh_MO  },  
    { "zh-SG",  "*CJK",     spBuiltinFormats_zh_SG  },  
    { "zh-TW",  "*CJK",     spBuiltinFormats_zh_TW  }   
};

} 



NumFmtModel::NumFmtModel() :
    mnPredefId( -1 )
{
}



ApiNumFmtData::ApiNumFmtData() :
    mnIndex( 0 )
{
}



namespace {

sal_Int32 lclCreatePredefinedFormat( const Reference< XNumberFormats >& rxNumFmts,
        sal_Int16 nPredefId, const Locale& rToLocale )
{
    sal_Int32 nIndex = 0;
    try
    {
        Reference< XNumberFormatTypes > xNumFmtTypes( rxNumFmts, UNO_QUERY_THROW );
        nIndex = (nPredefId >= 0) ?
            xNumFmtTypes->getFormatIndex( nPredefId, rToLocale ) :
            xNumFmtTypes->getStandardIndex( rToLocale );
    }
    catch( Exception& )
    {
        OSL_FAIL( OStringBuffer( "lclCreatePredefinedFormat - cannot create predefined number format " ).
            append( OString::number( nPredefId ) ).getStr() );
    }
    return nIndex;
}

sal_Int32 lclCreateFormat( const Reference< XNumberFormats >& rxNumFmts,
        const OUString& rFmtCode, const Locale& rToLocale, const Locale& rFromLocale )
{
    sal_Int32 nIndex = 0;
    try
    {
        nIndex = rxNumFmts->addNewConverted( rFmtCode, rFromLocale, rToLocale );
    }
    catch( Exception& )
    {
        
        if( rFmtCode.equalsIgnoreAsciiCase( "general" ) )
        {
            nIndex = lclCreatePredefinedFormat( rxNumFmts, 0, rToLocale );
        }
        else
        {
            
            OSL_ENSURE( rFmtCode.startsWith( "#\\ ?/" ) ||
                        rFmtCode.startsWith( "#\\ ?\?/" ) ||
                        rFmtCode.startsWith( "#\\ ?\?\?/" ),
                OStringBuffer( "lclCreateFormat - cannot create number format '" ).
                append( OUStringToOString( rFmtCode, osl_getThreadTextEncoding() ) ).
                append( '\'' ).getStr() );
        }
    }
    return nIndex;
}



/** Functor for converting an XML number format to an API number format index. */
class NumberFormatFinalizer
{
public:
    explicit            NumberFormatFinalizer( const WorkbookHelper& rHelper );

    inline void         operator()( NumberFormat& rNumFmt ) const
                            { rNumFmt.finalizeImport( mxNumFmts, maEnUsLocale ); }

private:
    Reference< XNumberFormats > mxNumFmts;
    Locale              maEnUsLocale;
};

NumberFormatFinalizer::NumberFormatFinalizer( const WorkbookHelper& rHelper ) :
    maEnUsLocale( "en", "US", OUString() )
{
    try
    {
        Reference< XNumberFormatsSupplier > xNumFmtsSupp( rHelper.getDocument(), UNO_QUERY_THROW );
        mxNumFmts = xNumFmtsSupp->getNumberFormats();
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxNumFmts.is(), "NumberFormatFinalizer::NumberFormatFinalizer - cannot get number formats" );
}

} 



NumberFormat::NumberFormat( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void NumberFormat::setFormatCode( const OUString& rFmtCode )
{
    
    
    

    maModel.maFmtCode = rFmtCode.replaceAll("\\", "");
}

void NumberFormat::setFormatCode( const Locale& rLocale, const sal_Char* pcFmtCode )
{
    maModel.maLocale = rLocale;
    maModel.maFmtCode = OStringToOUString( OString( pcFmtCode ), RTL_TEXTENCODING_UTF8 );
    maModel.mnPredefId = -1;
}

void NumberFormat::setPredefinedId( const Locale& rLocale, sal_Int16 nPredefId )
{
    maModel.maLocale = rLocale;
    maModel.maFmtCode = OUString();
    maModel.mnPredefId = nPredefId;
}

sal_Int32 NumberFormat::finalizeImport( const Reference< XNumberFormats >& rxNumFmts, const Locale& rFromLocale )
{
    if( rxNumFmts.is() && !maModel.maFmtCode.isEmpty() )
        maApiData.mnIndex = lclCreateFormat( rxNumFmts, maModel.maFmtCode, maModel.maLocale, rFromLocale );
    else
        maApiData.mnIndex = lclCreatePredefinedFormat( rxNumFmts, maModel.mnPredefId, maModel.maLocale );
    return maApiData.mnIndex;
}

void NumberFormat::fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs ) const
{
    const ScDocument& rDoc = getScDocument();
    static sal_uLong  nDflt = rDoc.GetFormatTable()->GetStandardFormat( ScGlobal::eLnge );
    sal_uLong nScNumFmt = nDflt;
    if ( maApiData.mnIndex )
        nScNumFmt = maApiData.mnIndex;
    ScfTools::PutItem( rItemSet, SfxUInt32Item( ATTR_VALUE_FORMAT, nScNumFmt ), bSkipPoolDefs );
    if( rItemSet.GetItemState( ATTR_VALUE_FORMAT, false ) == SFX_ITEM_SET )
        ScGlobal::AddLanguage( rItemSet, *(rDoc.GetFormatTable()) );
}

void NumberFormat::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    rPropMap[ PROP_NumberFormat ] <<= maApiData.mnIndex;
}



NumberFormatsBuffer::NumberFormatsBuffer( const WorkbookHelper& rHelper )
    : WorkbookHelper(rHelper)
    , mnHighestId(0)
{
    
    
    maLocaleStr = officecfg::Setup::L10N::ooSetupSystemLocale::get();
    
    if( maLocaleStr.isEmpty() )
        maLocaleStr = officecfg::System::L10N::Locale::get();

    
    insertBuiltinFormats();
}

NumberFormatRef NumberFormatsBuffer::createNumFmt( sal_Int32 nNumFmtId, const OUString& rFmtCode )
{
    NumberFormatRef xNumFmt;
    if( nNumFmtId >= 0 )
    {
        xNumFmt.reset( new NumberFormat( *this ) );
        maNumFmts[ nNumFmtId ] = xNumFmt;
        if ( nNumFmtId > mnHighestId )
            mnHighestId = nNumFmtId;
        xNumFmt->setFormatCode( rFmtCode );
    }
    return xNumFmt;
}

NumberFormatRef NumberFormatsBuffer::importNumFmt( const AttributeList& rAttribs )
{
    sal_Int32 nNumFmtId = rAttribs.getInteger( XML_numFmtId, -1 );
    OUString aFmtCode = rAttribs.getXString( XML_formatCode, OUString() );
    return createNumFmt( nNumFmtId, aFmtCode );
}

void NumberFormatsBuffer::importNumFmt( SequenceInputStream& rStrm )
{
    sal_Int32 nNumFmtId = rStrm.readuInt16();
    OUString aFmtCode = BiffHelper::readString( rStrm );
    createNumFmt( nNumFmtId, aFmtCode );
}

void NumberFormatsBuffer::finalizeImport()
{
    maNumFmts.forEach( NumberFormatFinalizer( *this ) );
}

void NumberFormatsBuffer::fillToItemSet( SfxItemSet& rItemSet, sal_Int32 nNumFmtId, bool bSkipPoolDefs ) const
{
    if( const NumberFormat* pNumFmt = maNumFmts.get( nNumFmtId ).get() )
        pNumFmt->fillToItemSet( rItemSet, bSkipPoolDefs);
}

void NumberFormatsBuffer::writeToPropertyMap( PropertyMap& rPropMap, sal_Int32 nNumFmtId ) const
{
    if( const NumberFormat* pNumFmt = maNumFmts.get( nNumFmtId ).get() )
        pNumFmt->writeToPropertyMap( rPropMap );
}

void NumberFormatsBuffer::insertBuiltinFormats()
{
    
    typedef ::std::map< OUString, const BuiltinFormatTable* > BuiltinMap;
    BuiltinMap aBuiltinMap;
    for( const BuiltinFormatTable* pTable = spBuiltinFormatTables;
            pTable != STATIC_ARRAY_END( spBuiltinFormatTables ); ++pTable )
        aBuiltinMap[ OUString::createFromAscii( pTable->mpcLocale ) ] = pTable;

    
    Locale aSysLocale( LanguageTag::convertToLocale( maLocaleStr));

    
    typedef ::std::vector< const BuiltinFormatTable* > BuiltinVec;
    BuiltinVec aBuiltinVec;
    BuiltinMap::const_iterator aMIt = aBuiltinMap.find( maLocaleStr ), aMEnd = aBuiltinMap.end();
    OSL_ENSURE( aMIt != aMEnd,
        OStringBuffer( "NumberFormatsBuffer::insertBuiltinFormats - locale '" ).
        append( OUStringToOString( maLocaleStr, RTL_TEXTENCODING_ASCII_US ) ).
        append( "' not supported (#i29949#)" ).getStr() );
    
    if( aMIt == aMEnd )
        aMIt = aBuiltinMap.find( "*" );
    OSL_ENSURE( aMIt != aMEnd, "NumberFormatsBuffer::insertBuiltinFormats - default map not found" );
    
    for( ; aMIt != aMEnd; aMIt = aBuiltinMap.find( OUString::createFromAscii( aMIt->second->mpcParent ) ) )
        aBuiltinVec.push_back( aMIt->second );

    
    typedef ::std::map< sal_Int32, sal_Int32 > ReuseMap;
    ReuseMap aReuseMap;
    for( BuiltinVec::reverse_iterator aVIt = aBuiltinVec.rbegin(), aVEnd = aBuiltinVec.rend(); aVIt != aVEnd; ++aVIt )
    {
        
        Locale aLocale;
        if( (*aVIt)->mpcParent[ 0 ] != '\0' )
            aLocale = aSysLocale;
        for( const BuiltinFormat* pBuiltin = (*aVIt)->mpFormats; pBuiltin && (pBuiltin->mnNumFmtId >= 0); ++pBuiltin )
        {
            NumberFormatRef& rxNumFmt = maNumFmts[ pBuiltin->mnNumFmtId ];
            rxNumFmt.reset( new NumberFormat( *this ) );

            bool bReuse = false;
            if( pBuiltin->mpcFmtCode )
                rxNumFmt->setFormatCode( aLocale, pBuiltin->mpcFmtCode );
            else if( pBuiltin->mnPredefId >= 0 )
                rxNumFmt->setPredefinedId( aLocale, pBuiltin->mnPredefId );
            else
                bReuse = pBuiltin->mnReuseId >= 0;

            if( bReuse )
                aReuseMap[ pBuiltin->mnNumFmtId ] = pBuiltin->mnReuseId;
            else
                aReuseMap.erase( pBuiltin->mnNumFmtId );
        }
    }

    
    for( ReuseMap::const_iterator aRIt = aReuseMap.begin(), aREnd = aReuseMap.end(); aRIt != aREnd; ++aRIt )
    {
        maNumFmts[ aRIt->first ] = maNumFmts[ aRIt->second ];
        if ( aRIt->first > mnHighestId )
            mnHighestId = aRIt->first;
    }
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
