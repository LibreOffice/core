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

#include <sal/config.h>

#include <string_view>

#include <numberformatsbuffer.hxx>
#include <biffhelper.hxx>

#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <officecfg/Setup.hxx>
#include <officecfg/System.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svl/numformat.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/tokens.hxx>
#include <scitems.hxx>
#include <document.hxx>
#include <ftools.hxx>
#include <docuno.hxx>

namespace oox::xls {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

namespace {

/** Stores the number format used in Calc for an Excel built-in number format. */
struct BuiltinFormat
{
    sal_Int32           mnNumFmtId;         /// Built-in number format index.
    OUString            macFmtCode;         /// Format string, may be empty (mnPredefId is used then).
    sal_Int16           mnPredefId;         /// Predefined format index, if mpcFmtCode is 0.
    sal_Int32           mnReuseId;          /// Use this format, if mpcFmtCode is 0 and mnPredefId is -1.
};

/** Defines a literal built-in number format. */
#define NUMFMT_STRING( INDEX, FORMATCODE ) \
    { INDEX, u"" FORMATCODE ""_ustr, -1, -1 }

/** Defines a built-in number format that maps to an own predefined format. */
#define NUMFMT_PREDEF( INDEX, PREDEFINED ) \
    { INDEX, {}, css::i18n::NumberFormatIndex::PREDEFINED, -1 }

/** Defines a built-in number format that is the same as the specified in nReuseId. */
#define NUMFMT_REUSE( INDEX, REUSED_INDEX ) \
    { INDEX, {}, -1, REUSED_INDEX }

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

// currency unit characters
#define UTF8_BAHT           "฿"
#define UTF8_COLON          "₡"
#define UTF8_CURR_AR_AE     "د.إ."
#define UTF8_CURR_AR_BH     "د.ب."
#define UTF8_CURR_AR_DZ     "د.ج."
#define UTF8_CURR_AR_EG     "ج.م."
#define UTF8_CURR_AR_IQ     "د.ع."
#define UTF8_CURR_AR_JO     "د.ا."
#define UTF8_CURR_AR_KW     "د.ك."
#define UTF8_CURR_AR_LB     "ل.ل."
#define UTF8_CURR_AR_LY     "د.ل."
#define UTF8_CURR_AR_MA     "د.م."
#define UTF8_CURR_AR_OM     "ر.ع."
#define UTF8_CURR_AR_QA     "ر.ق."
#define UTF8_CURR_AR_SA     "ر.س."
#define UTF8_CURR_AR_SY     "ل.س."
#define UTF8_CURR_AR_TN     "د.ت."
#define UTF8_CURR_AR_YE     "ر.ي."
#define UTF8_CURR_BN_IN     "টা"
#define UTF8_CURR_FA_IR     "ريال"
#define UTF8_CURR_GU_IN     "રૂ"
#define UTF8_CURR_HI_IN     "रु"
#define UTF8_CURR_KN_IN     "ರೂ"
#define UTF8_CURR_ML_IN     "ക"
#define UTF8_CURR_PA_IN     "ਰੁ"
#define UTF8_CURR_TA_IN     "ரூ"
#define UTF8_CURR_TE_IN     "రూ"
#define UTF8_DONG           "₫"
#define UTF8_EURO           "€"
#define UTF8_POUND_GB       "£"
#define UTF8_RUFIYAA        "ރ"
#define UTF8_SHEQEL         "₪"
#define UTF8_TUGRUG         "₮"
#define UTF8_WON            "₩"
#define UTF8_YEN_CN         "￥"
#define UTF8_YEN_JP         "¥"

// Unicode characters for currency units
#define UTF8_CCARON_LC      "č"
#define UTF8_LSTROKE_LC     "ł"
// Armenian
#define UTF8_HY_DA_LC       "դ"
#define UTF8_HY_REH_LC      "ր"
// Cyrillic
#define UTF8_CYR_G_LC       "г"
#define UTF8_CYR_L_LC       "л"
#define UTF8_CYR_M_LC       "м"
#define UTF8_CYR_N_LC       "н"
#define UTF8_CYR_O_LC       "о"
#define UTF8_CYR_R_LC       "р"
#define UTF8_CYR_S_LC       "с"
#define UTF8_CYR_W_LC       "в"

// Japanese/Chinese date/time characters
#define UTF8_CJ_YEAR        "年"
#define UTF8_CJ_MON         "月"
#define UTF8_CJ_DAY         "日"
#define UTF8_CJ_HOUR        "時"
#define UTF8_CJ_MIN         "分"
#define UTF8_CJ_SEC         "秒"

// Chinese Simplified date/time characters
#define UTF8_CS_YEAR        "年"
#define UTF8_CS_MON         "月"
#define UTF8_CS_DAY         "日"
#define UTF8_CS_HOUR        "时"
#define UTF8_CS_MIN         "分"
#define UTF8_CS_SEC         "秒"

// Korean date/time characters
#define UTF8_KO_YEAR        "년"
#define UTF8_KO_MON         "월"
#define UTF8_KO_DAY         "일"
#define UTF8_KO_HOUR        "시"
#define UTF8_KO_MIN         "분"
#define UTF8_KO_SEC         "초"

/** Default number format table. Last parent of all other tables, used for unknown locales. */
const BuiltinFormat spBuiltinFormats_BASE[] =
{
    // 0..13 numeric and currency formats
    NUMFMT_PREDEF(   0, NUMBER_STANDARD ),          // General
    NUMFMT_PREDEF(   1, NUMBER_INT ),               // 0
    NUMFMT_PREDEF(   2, NUMBER_DEC2 ),              // 0.00
    NUMFMT_PREDEF(   3, NUMBER_1000INT ),           // #,##0
    NUMFMT_PREDEF(   4, NUMBER_1000DEC2 ),          // #,##0.00
    NUMFMT_PREDEF(   5, CURRENCY_1000INT ),         // #,##0[symbol]
    NUMFMT_PREDEF(   6, CURRENCY_1000INT_RED ),     // #,##0[symbol];[RED]-#,##0[symbol]
    NUMFMT_PREDEF(   7, CURRENCY_1000DEC2 ),        // #,##0.00[symbol]
    NUMFMT_PREDEF(   8, CURRENCY_1000DEC2_RED ),    // #,##0.00[symbol];[RED]-#,##0.00[symbol]
    NUMFMT_PREDEF(   9, PERCENT_INT ),              // 0%
    NUMFMT_PREDEF(  10, PERCENT_DEC2 ),             // 0.00%
    NUMFMT_PREDEF(  11, SCIENTIFIC_000E00 ),        // 0.00E+00
    NUMFMT_PREDEF(  12, FRACTION_1 ),               // # ?/?
    NUMFMT_PREDEF(  13, FRACTION_2 ),               // # ??/??

    // 14...22 date and time formats
    NUMFMT_PREDEF(  14, DATE_SYS_DDMMYYYY ),
    NUMFMT_PREDEF(  15, DATE_SYS_DMMMYY ),
    NUMFMT_PREDEF(  16, DATE_SYS_DDMMM ),
    NUMFMT_PREDEF(  17, DATE_SYS_MMYY ),
    NUMFMT_PREDEF(  18, TIME_HHMMAMPM ),
    NUMFMT_PREDEF(  19, TIME_HHMMSSAMPM ),
    NUMFMT_PREDEF(  20, TIME_HHMM ),
    NUMFMT_PREDEF(  21, TIME_HHMMSS ),
    NUMFMT_PREDEF(  22, DATETIME_SYSTEM_SHORT_HHMM ),

    // 23...36 international formats
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

    // 37...44 accounting formats, defaults without currency symbol here
    NUMFMT_CURRENCY_MINUS_SYMBOL_NUMBER( 37, "", "", "" ),
    NUMFMT_ACCOUNTING_MINUS_SYMBOL_NUMBER( 41, "", "" ),

    // 45...49 more special formats
    NUMFMT_STRING(  45, "mm:ss" ),
    NUMFMT_STRING(  46, "[h]:mm:ss" ),
    NUMFMT_STRING(  47, "mm:ss.0" ),
    NUMFMT_STRING(  48, "##0.0E+0" ),
    NUMFMT_PREDEF(  49, TEXT ),

    // 50...81 international formats
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

    // 82...163 not used, must not occur in a file (Excel may crash)
};

/** Arabic, U.A.E. */
const BuiltinFormat spBuiltinFormats_ar_AE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_AE "\"", " " ),
};

/** Arabic, Bahrain. */
const BuiltinFormat spBuiltinFormats_ar_BH[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_BH "\"", " " ),
};

/** Arabic, Algeria. */
const BuiltinFormat spBuiltinFormats_ar_DZ[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_DZ "\"", " " ),
};

/** Arabic, Egypt. */
const BuiltinFormat spBuiltinFormats_ar_EG[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_EG "\"", " " ),
};

/** Arabic, Iraq. */
const BuiltinFormat spBuiltinFormats_ar_IQ[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_IQ "\"", " " ),
};

/** Arabic, Jordan. */
const BuiltinFormat spBuiltinFormats_ar_JO[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_JO "\"", " " ),
};

/** Arabic, Kuwait. */
const BuiltinFormat spBuiltinFormats_ar_KW[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_KW "\"", " " ),
};

/** Arabic, Lebanon. */
const BuiltinFormat spBuiltinFormats_ar_LB[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_LB "\"", " " ),
};

/** Arabic, Libya. */
const BuiltinFormat spBuiltinFormats_ar_LY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_LY "\"", " " ),
};

/** Arabic, Morocco. */
const BuiltinFormat spBuiltinFormats_ar_MA[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_MA "\"", " " ),
};

/** Arabic, Oman. */
const BuiltinFormat spBuiltinFormats_ar_OM[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_OM "\"", " " ),
};

/** Arabic, Qatar. */
const BuiltinFormat spBuiltinFormats_ar_QA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_QA "\"", " " ),
};

/** Arabic, Saudi Arabia. */
const BuiltinFormat spBuiltinFormats_ar_SA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_SA "\"", " " ),
};

/** Arabic, Syria. */
const BuiltinFormat spBuiltinFormats_ar_SY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_SY "\"", " " ),
};

/** Arabic, Tunisia. */
const BuiltinFormat spBuiltinFormats_ar_TN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_TN "\"", " " ),
};

/** Arabic, Yemen. */
const BuiltinFormat spBuiltinFormats_ar_YE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_AR_YE "\"", " " ),
};

/** Belarusian, Belarus. */
const BuiltinFormat spBuiltinFormats_be_BY[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_R_LC ".\"", "_" UTF8_CYR_R_LC "_.", "\\ " ),
};

/** Bulgarian, Bulgaria. */
const BuiltinFormat spBuiltinFormats_bg_BG[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.M.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_L_LC UTF8_CYR_W_LC "\"", "_" UTF8_CYR_L_LC "_" UTF8_CYR_W_LC, "\\ " ),
};

/** Bengali, India. */
const BuiltinFormat spBuiltinFormats_bn_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_BN_IN "\"", " " ),
};

/** Czech, Czech Republic. */
const BuiltinFormat spBuiltinFormats_cs_CZ[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"K" UTF8_CCARON_LC "\"", "_K_" UTF8_CCARON_LC, "\\ " ),
};

/** Danish, Denmark. */
const BuiltinFormat spBuiltinFormats_da_DK[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"kr\"", " " ),
};

/** German, Austria. */
const BuiltinFormat spBuiltinFormats_de_AT[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, " " ),
};

/** German, Switzerland. */
const BuiltinFormat spBuiltinFormats_de_CH[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ". ", "MMM", " ", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"SFr.\"", " " ),
};

/** German, Germany. */
const BuiltinFormat spBuiltinFormats_de_DE[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ". ", "MMM", " ", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
};

/** German, Liechtenstein. */
const BuiltinFormat spBuiltinFormats_de_LI[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ". ", "MMM", " ", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"CHF\"", " " ),
};

/** German, Luxembourg. */
const BuiltinFormat spBuiltinFormats_de_LU[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
};

/** Divehi, Maldives. */
const BuiltinFormat spBuiltinFormats_div_MV[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_NUMBER_SYMBOL_MINUS( "\"" UTF8_RUFIYAA ".\"", "_" UTF8_RUFIYAA "_.", " " ),
};

/** Greek, Greece. */
const BuiltinFormat spBuiltinFormats_el_GR[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
};

/** English, Australia. */
const BuiltinFormat spBuiltinFormats_en_AU[] =
{
    NUMFMT_ALLDATETIMES( "D/MM/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
};

/** English, Belize. */
const BuiltinFormat spBuiltinFormats_en_BZ[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"BZ$\"", "" ),
};

/** English, Canada. */
const BuiltinFormat spBuiltinFormats_en_CA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
};

/** English, Caribbean. */
const BuiltinFormat spBuiltinFormats_en_CB[] =
{
    NUMFMT_ALLDATETIMES( "MM/DD/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
};

/** English, United Kingdom. */
const BuiltinFormat spBuiltinFormats_en_GB[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_POUND_GB, "" ),
};

/** English, Ireland. */
const BuiltinFormat spBuiltinFormats_en_IE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, "" ),
};

/** English, Jamaica. */
const BuiltinFormat spBuiltinFormats_en_JM[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\"J$\"", "" ),
};

/** English, New Zealand. */
const BuiltinFormat spBuiltinFormats_en_NZ[] =
{
    NUMFMT_ALLDATETIMES( "D/MM/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
};

/** English, Philippines. */
const BuiltinFormat spBuiltinFormats_en_PH[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Php\"", "" ),
};

/** English, Trinidad and Tobago. */
const BuiltinFormat spBuiltinFormats_en_TT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"TT$\"", "" ),
};

/** English, USA. */
const BuiltinFormat spBuiltinFormats_en_US[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
};

/** English, South Africa. */
const BuiltinFormat spBuiltinFormats_en_ZA[] =
{
    NUMFMT_ALLDATETIMES( "YYYY/MM/DD", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\\R", " " ),
};

/** English, Zimbabwe. */
const BuiltinFormat spBuiltinFormats_en_ZW[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Z$\"", "" ),
};

/** Spanish, Argentina. */
const BuiltinFormat spBuiltinFormats_es_AR[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "$", " " ),
};

/** Spanish, Bolivia. */
const BuiltinFormat spBuiltinFormats_es_BO[] =
{
    // slashes must be quoted to prevent conversion to minus
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"$b\"", " " ),
};

/** Spanish, Chile. */
const BuiltinFormat spBuiltinFormats_es_CL[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", " " ),
};

/** Spanish, Colombia. */
const BuiltinFormat spBuiltinFormats_es_CO[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", " " ),
};

/** Spanish, Costa Rica. */
const BuiltinFormat spBuiltinFormats_es_CR[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( UTF8_COLON, "" ),
};

/** Spanish, Dominican Republic. */
const BuiltinFormat spBuiltinFormats_es_DO[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"RD$\"", "" ),
};

/** Spanish, Ecuador. */
const BuiltinFormat spBuiltinFormats_es_EC[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", " " ),
};

/** Spanish, Spain. */
const BuiltinFormat spBuiltinFormats_es_ES[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
};

/** Spanish, Guatemala. */
const BuiltinFormat spBuiltinFormats_es_GT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\Q", "" ),
};

/** Spanish, Honduras. */
const BuiltinFormat spBuiltinFormats_es_HN[] =
{
    // slashes must be quoted to prevent conversion to minus
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"L.\"", " " ),
};

/** Spanish, Mexico. */
const BuiltinFormat spBuiltinFormats_es_MX[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
};

/** Spanish, Nicaragua. */
const BuiltinFormat spBuiltinFormats_es_NI[] =
{
    // slashes must be quoted to prevent conversion to minus
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"C$\"", " " ),
};

/** Spanish, Panama. */
const BuiltinFormat spBuiltinFormats_es_PA[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"B/.\"", " " ),
};

/** Spanish, Peru. */
const BuiltinFormat spBuiltinFormats_es_PE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"S/.\"", " " ),
};

/** Spanish, Puerto Rico. */
const BuiltinFormat spBuiltinFormats_es_PR[] =
{
    // slashes must be quoted to prevent conversion to minus
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", " " ),
};

/** Spanish, Paraguay. */
const BuiltinFormat spBuiltinFormats_es_PY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Gs\"", " " ),
};

/** Spanish, El Salvador. */
const BuiltinFormat spBuiltinFormats_es_SV[] =
{
    // slashes must be quoted to prevent conversion to minus
    NUMFMT_ALLDATETIMES( "DD\\/MM\\/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
};

/** Spanish, Uruguay. */
const BuiltinFormat spBuiltinFormats_es_UY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"$U\"", " " ),
};

/** Spanish, Venezuela. */
const BuiltinFormat spBuiltinFormats_es_VE[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "Bs", " " ),
};

/** Estonian, Estonia. */
const BuiltinFormat spBuiltinFormats_et_EE[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "D.MM.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kr\"", "_k_r", "\\ " ),
};

/** Farsi, Iran. */
const BuiltinFormat spBuiltinFormats_fa_IR[] =
{
    NUMFMT_ALLDATETIMES( "YYYY/MM/DD", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"" UTF8_CURR_FA_IR "\"", " " ),
};

/** Finnish, Finland. */
const BuiltinFormat spBuiltinFormats_fi_FI[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_STRING(  9, "0\\ %" ),
    NUMFMT_STRING( 10, "0.00\\ %" ),
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
};

/** Faroese, Faroe Islands. */
const BuiltinFormat spBuiltinFormats_fo_FO[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"kr\"", " " ),
};

/** French, Belgium. */
const BuiltinFormat spBuiltinFormats_fr_BE[] =
{
    NUMFMT_ALLDATETIMES( "D/MM/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
};

/** French, Canada. */
const BuiltinFormat spBuiltinFormats_fr_CA[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_NUMBER_SYMBOL_CLOSE( "$", "_$", "\\ " ),
};

/** French, Switzerland. */
const BuiltinFormat spBuiltinFormats_fr_CH[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"SFr.\"", " " ),
};

/** French, France. */
const BuiltinFormat spBuiltinFormats_fr_FR[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
};

/** French, Luxembourg. */
const BuiltinFormat spBuiltinFormats_fr_LU[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
};

/** French, Monaco. */
const BuiltinFormat spBuiltinFormats_fr_MC[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
};

/** Galizian, Spain. */
const BuiltinFormat spBuiltinFormats_gl_ES[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, " " ),
};

/** Gujarati, India. */
const BuiltinFormat spBuiltinFormats_gu_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_GU_IN "\"", " " ),
};

/** Hebrew, Israel. */
const BuiltinFormat spBuiltinFormats_he_IL[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( UTF8_SHEQEL, " " ),
};

/** Hindi, India. */
const BuiltinFormat spBuiltinFormats_hi_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_HI_IN "\"", " " ),
};

/** Croatian, Bosnia and Herzegowina. */
const BuiltinFormat spBuiltinFormats_hr_BA[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"KM\"", "_K_M", " " ),
};

/** Croatian, Croatia. */
const BuiltinFormat spBuiltinFormats_hr_HR[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kn\"", "_k_n", " " ),
};

/** Hungarian, Hungary. */
const BuiltinFormat spBuiltinFormats_hu_HU[] =
{
    // space character is group separator, literal spaces must be quoted
    // MMM is rendered differently in Calc and Excel (see #i41488#)
    NUMFMT_ALLDATETIMES( "YYYY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Ft\"", "_F_t", "\\ " ),
};

/** Armenian, Armenia. */
const BuiltinFormat spBuiltinFormats_hy_AM[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_HY_DA_LC UTF8_HY_REH_LC ".\"", "_" UTF8_HY_DA_LC "_" UTF8_HY_REH_LC "_.", " " ),
};

/** Indonesian, Indonesia. */
const BuiltinFormat spBuiltinFormats_id_ID[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"Rp\"", "" ),
};

/** Icelandic, Iceland. */
const BuiltinFormat spBuiltinFormats_is_IS[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kr.\"", "_k_r_.", " " ),
};

/** Italian, Switzerland. */
const BuiltinFormat spBuiltinFormats_it_CH[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"SFr.\"", " " ),
};

/** Italian, Italy. */
const BuiltinFormat spBuiltinFormats_it_IT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_EURO, " " ),
};

/** Georgian, Georgia. */
const BuiltinFormat spBuiltinFormats_ka_GE[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Lari\"", "_L_a_r_i", "\\ " ),
};

/** Kazakh, Kazakhstan. */
const BuiltinFormat spBuiltinFormats_kk_KZ[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\\T", "" ),
};

/** Kannada, India. */
const BuiltinFormat spBuiltinFormats_kn_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_KN_IN "\"", " " ),
};

/** Kyrgyz, Kyrgyzstan. */
const BuiltinFormat spBuiltinFormats_ky_KG[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_S_LC UTF8_CYR_O_LC UTF8_CYR_M_LC "\"", "_" UTF8_CYR_S_LC "_" UTF8_CYR_O_LC "_" UTF8_CYR_M_LC, "\\ " ),
};

/** Lithuanian, Lithuania. */
const BuiltinFormat spBuiltinFormats_lt_LT[] =
{
    NUMFMT_ALLDATETIMES( "YYYY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Lt\"", "_L_t", " " ),
};

/** Latvian, Latvia. */
const BuiltinFormat spBuiltinFormats_lv_LV[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "YYYY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\"Ls\"", "\\ " ),
};

/** Malayalam, India. */
const BuiltinFormat spBuiltinFormats_ml_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_ML_IN "\"", " " ),
};

/** Mongolian, Mongolia. */
const BuiltinFormat spBuiltinFormats_mn_MN[] =
{
    NUMFMT_ALLDATETIMES( "YY.MM.DD", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_TUGRUG, "_" UTF8_TUGRUG, "" ),
};

/** Malay, Brunei Darussalam. */
const BuiltinFormat spBuiltinFormats_ms_BN[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
};

/** Malay, Malaysia. */
const BuiltinFormat spBuiltinFormats_ms_MY[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\R", "" ),
};

/** Maltese, Malta. */
const BuiltinFormat spBuiltinFormats_mt_MT[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "\"Lm\"", "" ),
};

/** Dutch, Belgium. */
const BuiltinFormat spBuiltinFormats_nl_BE[] =
{
    // slashes must be quoted to prevent conversion to minus
    NUMFMT_ALLDATETIMES( "D\\/MM\\/YYYY", "D", "\\/", "MMM", "\\/", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
};

/** Dutch, Netherlands. */
const BuiltinFormat spBuiltinFormats_nl_NL[] =
{
    NUMFMT_ALLDATETIMES( "D-M-YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( UTF8_EURO, " " ),
};

/** Norwegian (Bokmal and Nynorsk), Norway. */
const BuiltinFormat spBuiltinFormats_no_NO[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"kr\"", "\\ " ),
};

/** Punjabi, India. */
const BuiltinFormat spBuiltinFormats_pa_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_PA_IN "\"", " " ),
};

/** Polish, Poland. */
const BuiltinFormat spBuiltinFormats_pl_PL[] =
{
    // space character is group separator, literal spaces must be quoted
    // MMM is rendered differently in Calc and Excel (see #i72300#)
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"z" UTF8_LSTROKE_LC "\"", "_z_" UTF8_LSTROKE_LC, "\\ " ),
};

/** Portuguese, Brazil. */
const BuiltinFormat spBuiltinFormats_pt_BR[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "/", "MMM", "/", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"R$\"", " " ),
};

/** Portuguese, Portugal. */
const BuiltinFormat spBuiltinFormats_pt_PT[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, " " ),
};

/** Romanian, Romania. */
const BuiltinFormat spBuiltinFormats_ro_RO[] =
{
    // space character is group separator, literal spaces must be quoted (but see #i75367#)
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"lei\"", "_l_e_i", "\\ " ),
};

/** Russian, Russian Federation. */
const BuiltinFormat spBuiltinFormats_ru_RU[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_R_LC ".\"", "_" UTF8_CYR_R_LC "_.", "" ),
};

/** Slovak, Slovakia. */
const BuiltinFormat spBuiltinFormats_sk_SK[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"Sk\"", "_S_k", "\\ " ),
};

/** Slovenian, Slovenia. */
const BuiltinFormat spBuiltinFormats_sl_SI[] =
{
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"SIT\"", "_S_I_T", " " ),
};

/** Swedish, Finland. */
const BuiltinFormat spBuiltinFormats_sv_FI[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_STRING(  9, "0\\ %" ),
    NUMFMT_STRING( 10, "0.00\\ %" ),
    NUMFMT_ALLDATETIMES( "D.M.YYYY", "D", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_EURO, "_" UTF8_EURO, "\\ " ),
};

/** Swedish, Sweden. */
const BuiltinFormat spBuiltinFormats_sv_SE[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"kr\"", "_k_r", "\\ " ),
};

/** Swahili, Tanzania. */
const BuiltinFormat spBuiltinFormats_sw_TZ[] =
{
    NUMFMT_ALLDATETIMES( "M/D/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\S", "" ),
};

/** Tamil, India. */
const BuiltinFormat spBuiltinFormats_ta_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YYYY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_TA_IN "\"", " " ),
};

/** Telugu, India. */
const BuiltinFormat spBuiltinFormats_te_IN[] =
{
    NUMFMT_ALLDATETIMES( "DD-MM-YY", "DD", "-", "MMM", "-", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_MINUS_NUMBER( "\"" UTF8_CURR_TE_IN "\"", " " ),
};

/** Thai, Thailand. */
const BuiltinFormat spBuiltinFormats_th_TH[] =
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
};

/** Turkish, Turkey. */
const BuiltinFormat spBuiltinFormats_tr_TR[] =
{
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"TL\"", "_T_L", " " ),
};

/** Tatar, Russian Federation. */
const BuiltinFormat spBuiltinFormats_tt_RU[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_R_LC ".\"", "_" UTF8_CYR_R_LC "_.", "\\ " ),
};

/** Ukrainian, Ukraine. */
const BuiltinFormat spBuiltinFormats_uk_UA[] =
{
    // space character is group separator, literal spaces must be quoted
    NUMFMT_ALLDATETIMES( "DD.MM.YYYY", "DD", ".", "MMM", ".", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( "\"" UTF8_CYR_G_LC UTF8_CYR_R_LC UTF8_CYR_N_LC ".\"", "_" UTF8_CYR_G_LC "_" UTF8_CYR_R_LC "_" UTF8_CYR_N_LC "_.", "\\ " ),
};

/** Urdu, Pakistan. */
const BuiltinFormat spBuiltinFormats_ur_PK[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_SYMBOL_NUMBER_MINUS( "\"Rs\"", "" ),
};

/** Vietnamese, Viet Nam. */
const BuiltinFormat spBuiltinFormats_vi_VN[] =
{
    NUMFMT_ALLDATETIMES( "DD/MM/YYYY", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_NUMBER_SYMBOL( UTF8_DONG, "_" UTF8_DONG, " " ),
};

// CJK ------------------------------------------------------------------------

/** Base table for CJK locales. */
const BuiltinFormat spBuiltinFormats_CJK[] =
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
};

/** Japanese, Japan. */
const BuiltinFormat spBuiltinFormats_ja_JP[] =
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
};

/** Korean, South Korea. */
const BuiltinFormat spBuiltinFormats_ko_KR[] =
{
    NUMFMT_ALLDATETIMES( "YYYY-MM-DD", "DD", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( UTF8_WON, "" ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "$", "", "" ),
    NUMFMT_STRING( 27, "YYYY" UTF8_CJ_YEAR " MM" UTF8_CJ_MON " DD" UTF8_CJ_DAY ),
    NUMFMT_STRING( 28, "MM-DD" ),
    NUMFMT_STRING( 30, "MM-DD-YY" ),
    NUMFMT_STRING( 31, "YYYY" UTF8_KO_YEAR " MM" UTF8_KO_MON " DD" UTF8_KO_DAY ),
    NUMFMT_TIME_CJK( 32, "h", UTF8_KO_HOUR, UTF8_KO_MIN, UTF8_KO_SEC ),
    // slashes must be quoted to prevent conversion to minus
    NUMFMT_STRING( 34, "YYYY\\/MM\\/DD" ),
    NUMFMT_REUSE( 35, 14 ),
};

/** Chinese, China. */
const BuiltinFormat spBuiltinFormats_zh_CN[] =
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
};

/** Chinese, Hong Kong. */
const BuiltinFormat spBuiltinFormats_zh_HK[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\"HK$\"", "" ),
    NUMFMT_ALLTIMES_CJK( "h", "h", UTF8_CJ_HOUR, UTF8_CJ_MIN, UTF8_CJ_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "\"US$\"", "", "" ),
    NUMFMT_STRING( 27, "[$-404]D/M/E" ),
    NUMFMT_STRING( 28, "[$-404]D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"E\"" UTF8_CJ_YEAR "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"YYYY\"" UTF8_CJ_YEAR "\"" ),
};

/** Chinese, Macau. */
const BuiltinFormat spBuiltinFormats_zh_MO[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "\\P", "" ),
    NUMFMT_ALLTIMES_CJK( "h", "h", UTF8_CJ_HOUR, UTF8_CJ_MIN, UTF8_CJ_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "\"US$\"", "", "" ),
    NUMFMT_STRING( 27, "[$-404]D/M/E" ),
    NUMFMT_STRING( 28, "[$-404]D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"E\"" UTF8_CJ_YEAR "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "D\"" UTF8_CJ_DAY "\"M\"" UTF8_CJ_MON "\"YYYY\"" UTF8_CJ_YEAR "\"" ),
};

/** Chinese, Singapore. */
const BuiltinFormat spBuiltinFormats_zh_SG[] =
{
    NUMFMT_ALLDATETIMES( "D/M/YYYY", "D", "-", "MMM", "-", "YY", "h", "h" ),
    NUMFMT_ALLCURRENCIES_OPEN_SYMBOL_NUMBER_CLOSE( "$", "" ),
    NUMFMT_ALLTIMES_CJK( "h", "h", UTF8_CS_HOUR, UTF8_CS_MIN, UTF8_CS_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "$", "", "" ),
    NUMFMT_STRING( 27, "YYYY\"" UTF8_CS_YEAR "\"M\"" UTF8_CS_MON "\"" ),
    NUMFMT_STRING( 28, "M\"" UTF8_CS_MON "\"D\"" UTF8_CS_DAY "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "D\"" UTF8_CS_DAY "\"M\"" UTF8_CS_MON "\"YYYY\"" UTF8_CS_YEAR "\"" ),
};

/** Chinese, Taiwan. */
const BuiltinFormat spBuiltinFormats_zh_TW[] =
{
    NUMFMT_ALLDATETIMES( "YYYY/M/D", "D", "-", "MMM", "-", "YY", "hh", "hh" ),
    NUMFMT_ALLCURRENCIES_MINUS_SYMBOL_NUMBER( "$", "" ),
    NUMFMT_ALLTIMES_CJK( "hh", "hh", UTF8_CJ_HOUR, UTF8_CJ_MIN, UTF8_CJ_SEC ),
    NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE( 23, "\"US$\"", "", "" ),
    NUMFMT_STRING( 27, "[$-404]E/M/D" ),
    NUMFMT_STRING( 28, "[$-404]E\"" UTF8_CJ_YEAR "\"M\"" UTF8_CJ_MON "\"D\"" UTF8_CJ_DAY "\"" ),
    NUMFMT_STRING( 30, "M/D/YY" ),
    NUMFMT_STRING( 31, "YYYY\"" UTF8_CJ_YEAR "\"M\"" UTF8_CJ_MON "\"D\"" UTF8_CJ_DAY "\"" ),
};

/** Specifies a built-in number format table for a specific locale. */
struct BuiltinFormatTable
{
    OUString                       macLocale; /// The locale for this table.
    OUString                       macParent; /// The locale of the parent table.
    std::span<const BuiltinFormat> maFormats; /// The number format table (may be 0, if equal to parent).
};

constexpr BuiltinFormatTable spBuiltinFormatTables[] =
{ //  locale    parent      format table
    { u"*"_ustr,      u""_ustr,         spBuiltinFormats_BASE   },  // Base table
    { u"af-ZA"_ustr,  u"*"_ustr,        spBuiltinFormats_en_ZA  },  // Afrikaans, South Africa
    { u"ar-AE"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_AE  },  // Arabic, U.A.E.
    { u"ar-BH"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_BH  },  // Arabic, Bahrain
    { u"ar-DZ"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_DZ  },  // Arabic, Algeria
    { u"ar-EG"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_EG  },  // Arabic, Egypt
    { u"ar-IQ"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_IQ  },  // Arabic, Iraq
    { u"ar-JO"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_JO  },  // Arabic, Jordan
    { u"ar-KW"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_KW  },  // Arabic, Kuwait
    { u"ar-LB"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_LB  },  // Arabic, Lebanon
    { u"ar-LY"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_LY  },  // Arabic, Libya
    { u"ar-MA"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_MA  },  // Arabic, Morocco
    { u"ar-OM"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_OM  },  // Arabic, Oman
    { u"ar-QA"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_QA  },  // Arabic, Qatar
    { u"ar-SA"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_SA  },  // Arabic, Saudi Arabia
    { u"ar-SY"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_SY  },  // Arabic, Syria
    { u"ar-TN"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_TN  },  // Arabic, Tunisia
    { u"ar-YE"_ustr,  u"*"_ustr,        spBuiltinFormats_ar_YE  },  // Arabic, Yemen
    { u"be-BY"_ustr,  u"*"_ustr,        spBuiltinFormats_be_BY  },  // Belarusian, Belarus
    { u"bg-BG"_ustr,  u"*"_ustr,        spBuiltinFormats_bg_BG  },  // Bulgarian, Bulgaria
    { u"bn-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_bn_IN  },  // Bengali, India
    { u"ca-ES"_ustr,  u"*"_ustr,        spBuiltinFormats_es_ES  },  // Catalan, Spain
    { u"cs-CZ"_ustr,  u"*"_ustr,        spBuiltinFormats_cs_CZ  },  // Czech, Czech Republic
    { u"cy-GB"_ustr,  u"*"_ustr,        spBuiltinFormats_en_GB  },  // Welsh, United Kingdom
    { u"da-DK"_ustr,  u"*"_ustr,        spBuiltinFormats_da_DK  },  // Danish, Denmark
    { u"de-AT"_ustr,  u"*"_ustr,        spBuiltinFormats_de_AT  },  // German, Austria
    { u"de-CH"_ustr,  u"*"_ustr,        spBuiltinFormats_de_CH  },  // German, Switzerland
    { u"de-DE"_ustr,  u"*"_ustr,        spBuiltinFormats_de_DE  },  // German, Germany
    { u"de-LI"_ustr,  u"*"_ustr,        spBuiltinFormats_de_LI  },  // German, Liechtenstein
    { u"de-LU"_ustr,  u"*"_ustr,        spBuiltinFormats_de_LU  },  // German, Luxembourg
    { u"div-MV"_ustr, u"*"_ustr,        spBuiltinFormats_div_MV },  // Divehi, Maldives
    { u"el-GR"_ustr,  u"*"_ustr,        spBuiltinFormats_el_GR  },  // Greek, Greece
    { u"en-AU"_ustr,  u"*"_ustr,        spBuiltinFormats_en_AU  },  // English, Australia
    { u"en-BZ"_ustr,  u"*"_ustr,        spBuiltinFormats_en_BZ  },  // English, Belize
    { u"en-CA"_ustr,  u"*"_ustr,        spBuiltinFormats_en_CA  },  // English, Canada
    { u"en-CB"_ustr,  u"*"_ustr,        spBuiltinFormats_en_CB  },  // English, Caribbean
    { u"en-GB"_ustr,  u"*"_ustr,        spBuiltinFormats_en_GB  },  // English, United Kingdom
    { u"en-IE"_ustr,  u"*"_ustr,        spBuiltinFormats_en_IE  },  // English, Ireland
    { u"en-JM"_ustr,  u"*"_ustr,        spBuiltinFormats_en_JM  },  // English, Jamaica
    { u"en-NZ"_ustr,  u"*"_ustr,        spBuiltinFormats_en_NZ  },  // English, New Zealand
    { u"en-PH"_ustr,  u"*"_ustr,        spBuiltinFormats_en_PH  },  // English, Philippines
    { u"en-TT"_ustr,  u"*"_ustr,        spBuiltinFormats_en_TT  },  // English, Trinidad and Tobago
    { u"en-US"_ustr,  u"*"_ustr,        spBuiltinFormats_en_US  },  // English, USA
    { u"en-ZA"_ustr,  u"*"_ustr,        spBuiltinFormats_en_ZA  },  // English, South Africa
    { u"en-ZW"_ustr,  u"*"_ustr,        spBuiltinFormats_en_ZW  },  // English, Zimbabwe
    { u"es-AR"_ustr,  u"*"_ustr,        spBuiltinFormats_es_AR  },  // Spanish, Argentina
    { u"es-BO"_ustr,  u"*"_ustr,        spBuiltinFormats_es_BO  },  // Spanish, Bolivia
    { u"es-CL"_ustr,  u"*"_ustr,        spBuiltinFormats_es_CL  },  // Spanish, Chile
    { u"es-CO"_ustr,  u"*"_ustr,        spBuiltinFormats_es_CO  },  // Spanish, Colombia
    { u"es-CR"_ustr,  u"*"_ustr,        spBuiltinFormats_es_CR  },  // Spanish, Costa Rica
    { u"es-DO"_ustr,  u"*"_ustr,        spBuiltinFormats_es_DO  },  // Spanish, Dominican Republic
    { u"es-EC"_ustr,  u"*"_ustr,        spBuiltinFormats_es_EC  },  // Spanish, Ecuador
    { u"es-ES"_ustr,  u"*"_ustr,        spBuiltinFormats_es_ES  },  // Spanish, Spain
    { u"es-GT"_ustr,  u"*"_ustr,        spBuiltinFormats_es_GT  },  // Spanish, Guatemala
    { u"es-HN"_ustr,  u"*"_ustr,        spBuiltinFormats_es_HN  },  // Spanish, Honduras
    { u"es-MX"_ustr,  u"*"_ustr,        spBuiltinFormats_es_MX  },  // Spanish, Mexico
    { u"es-NI"_ustr,  u"*"_ustr,        spBuiltinFormats_es_NI  },  // Spanish, Nicaragua
    { u"es-PA"_ustr,  u"*"_ustr,        spBuiltinFormats_es_PA  },  // Spanish, Panama
    { u"es-PE"_ustr,  u"*"_ustr,        spBuiltinFormats_es_PE  },  // Spanish, Peru
    { u"es-PR"_ustr,  u"*"_ustr,        spBuiltinFormats_es_PR  },  // Spanish, Puerto Rico
    { u"es-PY"_ustr,  u"*"_ustr,        spBuiltinFormats_es_PY  },  // Spanish, Paraguay
    { u"es-SV"_ustr,  u"*"_ustr,        spBuiltinFormats_es_SV  },  // Spanish, El Salvador
    { u"es-UY"_ustr,  u"*"_ustr,        spBuiltinFormats_es_UY  },  // Spanish, Uruguay
    { u"es-VE"_ustr,  u"*"_ustr,        spBuiltinFormats_es_VE  },  // Spanish, Venezuela
    { u"et-EE"_ustr,  u"*"_ustr,        spBuiltinFormats_et_EE  },  // Estonian, Estonia
    { u"fa-IR"_ustr,  u"*"_ustr,        spBuiltinFormats_fa_IR  },  // Farsi, Iran
    { u"fi-FI"_ustr,  u"*"_ustr,        spBuiltinFormats_fi_FI  },  // Finnish, Finland
    { u"fo-FO"_ustr,  u"*"_ustr,        spBuiltinFormats_fo_FO  },  // Faroese, Faroe Islands
    { u"fr-BE"_ustr,  u"*"_ustr,        spBuiltinFormats_fr_BE  },  // French, Belgium
    { u"fr-CA"_ustr,  u"*"_ustr,        spBuiltinFormats_fr_CA  },  // French, Canada
    { u"fr-CH"_ustr,  u"*"_ustr,        spBuiltinFormats_fr_CH  },  // French, Switzerland
    { u"fr-FR"_ustr,  u"*"_ustr,        spBuiltinFormats_fr_FR  },  // French, France
    { u"fr-LU"_ustr,  u"*"_ustr,        spBuiltinFormats_fr_LU  },  // French, Luxembourg
    { u"fr-MC"_ustr,  u"*"_ustr,        spBuiltinFormats_fr_MC  },  // French, Monaco
    { u"gl-ES"_ustr,  u"*"_ustr,        spBuiltinFormats_gl_ES  },  // Galizian, Spain
    { u"gu-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_gu_IN  },  // Gujarati, India
    { u"he-IL"_ustr,  u"*"_ustr,        spBuiltinFormats_he_IL  },  // Hebrew, Israel
    { u"hi-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_hi_IN  },  // Hindi, India
    { u"hr-BA"_ustr,  u"*"_ustr,        spBuiltinFormats_hr_BA  },  // Croatian, Bosnia and Herzegowina
    { u"hr-HR"_ustr,  u"*"_ustr,        spBuiltinFormats_hr_HR  },  // Croatian, Croatia
    { u"hu-HU"_ustr,  u"*"_ustr,        spBuiltinFormats_hu_HU  },  // Hungarian, Hungary
    { u"hy-AM"_ustr,  u"*"_ustr,        spBuiltinFormats_hy_AM  },  // Armenian, Armenia
    { u"id-ID"_ustr,  u"*"_ustr,        spBuiltinFormats_id_ID  },  // Indonesian, Indonesia
    { u"is-IS"_ustr,  u"*"_ustr,        spBuiltinFormats_is_IS  },  // Icelandic, Iceland
    { u"it-CH"_ustr,  u"*"_ustr,        spBuiltinFormats_it_CH  },  // Italian, Switzerland
    { u"it-IT"_ustr,  u"*"_ustr,        spBuiltinFormats_it_IT  },  // Italian, Italy
    { u"ka-GE"_ustr,  u"*"_ustr,        spBuiltinFormats_ka_GE  },  // Georgian, Georgia
    { u"kk-KZ"_ustr,  u"*"_ustr,        spBuiltinFormats_kk_KZ  },  // Kazakh, Kazakhstan
    { u"kn-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_kn_IN  },  // Kannada, India
    { u"kok-IN"_ustr, u"*"_ustr,        spBuiltinFormats_hi_IN  },  // Konkani, India
    { u"ky-KG"_ustr,  u"*"_ustr,        spBuiltinFormats_ky_KG  },  // Kyrgyz, Kyrgyzstan
    { u"lt-LT"_ustr,  u"*"_ustr,        spBuiltinFormats_lt_LT  },  // Lithuanian, Lithuania
    { u"lv-LV"_ustr,  u"*"_ustr,        spBuiltinFormats_lv_LV  },  // Latvian, Latvia
    { u"mi-NZ"_ustr,  u"*"_ustr,        spBuiltinFormats_en_NZ  },  // Maori, New Zealand
    { u"ml-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_ml_IN  },  // Malayalam, India
    { u"mn-MN"_ustr,  u"*"_ustr,        spBuiltinFormats_mn_MN  },  // Mongolian, Mongolia
    { u"mr-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_hi_IN  },  // Marathi, India
    { u"ms-BN"_ustr,  u"*"_ustr,        spBuiltinFormats_ms_BN  },  // Malay, Brunei Darussalam
    { u"ms-MY"_ustr,  u"*"_ustr,        spBuiltinFormats_ms_MY  },  // Malay, Malaysia
    { u"mt-MT"_ustr,  u"*"_ustr,        spBuiltinFormats_mt_MT  },  // Maltese, Malta
    { u"nb-NO"_ustr,  u"*"_ustr,        spBuiltinFormats_no_NO  },  // Norwegian Bokmal, Norway
    { u"nl-BE"_ustr,  u"*"_ustr,        spBuiltinFormats_nl_BE  },  // Dutch, Belgium
    { u"nl-NL"_ustr,  u"*"_ustr,        spBuiltinFormats_nl_NL  },  // Dutch, Netherlands
    { u"nn-NO"_ustr,  u"*"_ustr,        spBuiltinFormats_no_NO  },  // Norwegian Nynorsk, Norway
    { u"nso-ZA"_ustr, u"*"_ustr,        spBuiltinFormats_en_ZA  },  // Northern Sotho, South Africa
    { u"pa-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_pa_IN  },  // Punjabi, India
    { u"pl-PL"_ustr,  u"*"_ustr,        spBuiltinFormats_pl_PL  },  // Polish, Poland
    { u"pt-BR"_ustr,  u"*"_ustr,        spBuiltinFormats_pt_BR  },  // Portuguese, Brazil
    { u"pt-PT"_ustr,  u"*"_ustr,        spBuiltinFormats_pt_PT  },  // Portuguese, Portugal
    { u"qu-BO"_ustr,  u"*"_ustr,        spBuiltinFormats_es_BO  },  // Quechua, Bolivia
    { u"qu-EC"_ustr,  u"*"_ustr,        spBuiltinFormats_es_EC  },  // Quechua, Ecuador
    { u"qu-PE"_ustr,  u"*"_ustr,        spBuiltinFormats_es_PE  },  // Quechua, Peru
    { u"ro-RO"_ustr,  u"*"_ustr,        spBuiltinFormats_ro_RO  },  // Romanian, Romania
    { u"ru-RU"_ustr,  u"*"_ustr,        spBuiltinFormats_ru_RU  },  // Russian, Russian Federation
    { u"sa-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_hi_IN  },  // Sanskrit, India
    { u"se-FI"_ustr,  u"*"_ustr,        spBuiltinFormats_fi_FI  },  // Sami, Finland
    { u"se-NO"_ustr,  u"*"_ustr,        spBuiltinFormats_no_NO  },  // Sami, Norway
    { u"se-SE"_ustr,  u"*"_ustr,        spBuiltinFormats_sv_SE  },  // Sami, Sweden
    { u"sk-SK"_ustr,  u"*"_ustr,        spBuiltinFormats_sk_SK  },  // Slovak, Slovakia
    { u"sl-SI"_ustr,  u"*"_ustr,        spBuiltinFormats_sl_SI  },  // Slovenian, Slovenia
    { u"sv-FI"_ustr,  u"*"_ustr,        spBuiltinFormats_sv_FI  },  // Swedish, Finland
    { u"sv-SE"_ustr,  u"*"_ustr,        spBuiltinFormats_sv_SE  },  // Swedish, Sweden
    { u"sw-TZ"_ustr,  u"*"_ustr,        spBuiltinFormats_sw_TZ  },  // Swahili, Tanzania
    { u"syr-SY"_ustr, u"*"_ustr,        spBuiltinFormats_ar_SY  },  // Syriac, Syria
    { u"syr-TR"_ustr, u"*"_ustr,        spBuiltinFormats_tr_TR  },  // Syriac, Turkey
    { u"ta-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_ta_IN  },  // Tamil, India
    { u"te-IN"_ustr,  u"*"_ustr,        spBuiltinFormats_te_IN  },  // Telugu, India
    { u"th-TH"_ustr,  u"*"_ustr,        spBuiltinFormats_th_TH  },  // Thai, Thailand
    { u"tn-ZA"_ustr,  u"*"_ustr,        spBuiltinFormats_en_ZA  },  // Tswana, South Africa
    { u"tr-TR"_ustr,  u"*"_ustr,        spBuiltinFormats_tr_TR  },  // Turkish, Turkey
    { u"tt-RU"_ustr,  u"*"_ustr,        spBuiltinFormats_tt_RU  },  // Tatar, Russian Federation
    { u"uk-UA"_ustr,  u"*"_ustr,        spBuiltinFormats_uk_UA  },  // Ukrainian, Ukraine
    { u"ur-PK"_ustr,  u"*"_ustr,        spBuiltinFormats_ur_PK  },  // Urdu, Pakistan
    { u"vi-VN"_ustr,  u"*"_ustr,        spBuiltinFormats_vi_VN  },  // Vietnamese, Viet Nam
    { u"xh-ZA"_ustr,  u"*"_ustr,        spBuiltinFormats_en_ZA  },  // Xhosa, South Africa
    { u"zu-ZA"_ustr,  u"*"_ustr,        spBuiltinFormats_en_ZA  },  // Zulu, South Africa

    { u"*CJK"_ustr,   u"*"_ustr,        spBuiltinFormats_CJK    },  // CJK base table
    { u"ja-JP"_ustr,  u"*CJK"_ustr,     spBuiltinFormats_ja_JP  },  // Japanese, Japan
    { u"ko-KR"_ustr,  u"*CJK"_ustr,     spBuiltinFormats_ko_KR  },  // Korean, South Korea
    { u"zh-CN"_ustr,  u"*CJK"_ustr,     spBuiltinFormats_zh_CN  },  // Chinese, China
    { u"zh-HK"_ustr,  u"*CJK"_ustr,     spBuiltinFormats_zh_HK  },  // Chinese, Hong Kong
    { u"zh-MO"_ustr,  u"*CJK"_ustr,     spBuiltinFormats_zh_MO  },  // Chinese, Macau
    { u"zh-SG"_ustr,  u"*CJK"_ustr,     spBuiltinFormats_zh_SG  },  // Chinese, Singapore
    { u"zh-TW"_ustr,  u"*CJK"_ustr,     spBuiltinFormats_zh_TW  }   // Chinese, Taiwan
};

} // namespace

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
        // BIFF2-BIFF4 stores standard format explicitly in stream
        if( rFmtCode.equalsIgnoreAsciiCase( "general" ) )
        {
            nIndex = lclCreatePredefinedFormat( rxNumFmts, 0, rToLocale );
        }
        else
        {
            // do not assert fractional number formats with fixed denominator
            OSL_ENSURE( rFmtCode.startsWith( "#\\ ?/" ) ||
                        rFmtCode.startsWith( "#\\ ?\?/" ) ||
                        rFmtCode.startsWith( "#\\ ?\?\?/" ),
                OStringBuffer( "lclCreateFormat - cannot create number format '"
                    + OUStringToOString( rFmtCode, osl_getThreadTextEncoding() )
                    + "\'" ).getStr() );
        }
    }
    return nIndex;
}

/** Functor for converting an XML number format to an API number format index. */
class NumberFormatFinalizer
{
public:
    explicit            NumberFormatFinalizer( const WorkbookHelper& rHelper );

    void         operator()( NumberFormat& rNumFmt ) const
                            { rNumFmt.finalizeImport( mxNumFmts, maEnUsLocale ); }

private:
    Reference< XNumberFormats > mxNumFmts;
    Locale              maEnUsLocale;
};

NumberFormatFinalizer::NumberFormatFinalizer( const WorkbookHelper& rHelper ) :
    maEnUsLocale( u"en"_ustr, u"US"_ustr, OUString() )
{
    try
    {
        Reference< XNumberFormatsSupplier > xNumFmtsSupp( static_cast<cppu::OWeakObject*>(rHelper.getDocument().get()), UNO_QUERY_THROW );
        mxNumFmts = xNumFmtsSupp->getNumberFormats();
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxNumFmts.is(), "NumberFormatFinalizer::NumberFormatFinalizer - cannot get number formats" );
}

sal_Int32 lclPosToken ( std::u16string_view sFormat, std::u16string_view sSearch, sal_Int32 nStartPos )
{
    sal_Int32 nLength = sFormat.size();
    for ( sal_Int32 i = nStartPos; i < nLength && i >= 0 ; i++ )
    {
        size_t nFind = i;
        switch(sFormat[i])
        {
            case '\"' : // skip text
                nFind = sFormat.find('\"',i+1);
                break;
            case '['  : // skip condition
                nFind = sFormat.find(']',i+1);
                break;
            default :
                if ( o3tl::starts_with(sFormat.substr(i), sSearch) )
                    return i;
                break;
        }
        if ( nFind == std::u16string_view::npos )
            return -2;
        i = nFind;
    }
    return -2;
}

} // namespace

NumberFormat::NumberFormat( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void NumberFormat::setFormatCode( std::u16string_view aFmtCode )
{
    // Special case for fraction code '\ ?/?', it is passed to us in xml, the '\' is not
    // an escape character but merely should be telling the formatter to display the next
    // char in the format ( afaics it does that anyhow )
    sal_Int32 nPosEscape = 0;
    sal_Int32 nErase = 0;
    sal_Int32 nLastIndex = sal_Int32(aFmtCode.size()) - 1;
    OUStringBuffer sFormat(aFmtCode);

    while ( ( nPosEscape = lclPosToken( aFmtCode, u"\\ ", nPosEscape ) ) > 0 )
    {
        sal_Int32 nPos = nPosEscape + 2;
        while ( nPos < nLastIndex && ( aFmtCode[nPos] == '?' || aFmtCode[nPos] == '#' || aFmtCode[nPos] == '0' ) )
            nPos++;
        if ( nPos < nLastIndex && aFmtCode[nPos] == '/' )
        {
            sFormat.remove(nPosEscape - nErase, 1);
            nErase ++;
        }  // tdf#81939 preserve other escape characters
        nPosEscape = lclPosToken( aFmtCode, u";", nPosEscape ); // skip to next format
    }

    // tdf#161301 There may be a lone single stray leading "[$]" garbage, strip it.
    if (sFormat.getLength() >= 3 && sFormat[0] == '[' && sFormat[1] == '$' && sFormat[2] == ']')
    {
        SAL_WARN("sc.filter",
                "NumberFormat::setFormatCode: stripping leading [$] maybe due to x16r2:formatCode16 also being present: "
                << sFormat.toString());
        sFormat.remove(0, 3);
    }

    maModel.maFmtCode = sFormat.makeStringAndClear();
}

void NumberFormat::setFormatCode(const Locale& rLocale, const OUString& rcFmtCode)
{
    maModel.maLocale = rLocale;
    maModel.maFmtCode = rcFmtCode;
    maModel.mnPredefId = -1;
}

void NumberFormat::setPredefinedId( const Locale& rLocale, sal_Int16 nPredefId )
{
    maModel.maLocale = rLocale;
    maModel.maFmtCode.clear();
    maModel.mnPredefId = nPredefId;
}

void NumberFormat::finalizeImport( const Reference< XNumberFormats >& rxNumFmts, const Locale& rFromLocale )
{
    if( rxNumFmts.is() && !maModel.maFmtCode.isEmpty() )
        maApiData.mnIndex = lclCreateFormat( rxNumFmts, maModel.maFmtCode, maModel.maLocale, rFromLocale );
    else
        maApiData.mnIndex = lclCreatePredefinedFormat( rxNumFmts, maModel.mnPredefId, maModel.maLocale );
}

sal_uInt32 NumberFormat::fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs ) const
{
    const ScDocument& rDoc = getScDocument();
    static sal_uInt32  nDflt = rDoc.GetFormatTable()->GetStandardIndex( ScGlobal::eLnge );
    sal_uInt32 nScNumFmt = nDflt;
    if ( maApiData.mnIndex )
        nScNumFmt = maApiData.mnIndex;

    ScfTools::PutItem( rItemSet, SfxUInt32Item( ATTR_VALUE_FORMAT, nScNumFmt ), bSkipPoolDefs );
    if( rItemSet.GetItemState( ATTR_VALUE_FORMAT, false ) == SfxItemState::SET )
        ScGlobal::AddLanguage( rItemSet, *(rDoc.GetFormatTable()) );
    else
        nScNumFmt = 0;

    return nScNumFmt;
}

NumberFormatsBuffer::NumberFormatsBuffer( const WorkbookHelper& rHelper )
    : WorkbookHelper(rHelper)
    , mnHighestId(0)
{
    // get the current locale
    // try user-defined locale setting
    maLocaleStr = officecfg::Setup::L10N::ooSetupSystemLocale::get();
    // if set to "use system", get locale from system
    if( maLocaleStr.isEmpty() )
        maLocaleStr = officecfg::System::L10N::Locale::get();

    // create built-in formats for current locale
    insertBuiltinFormats();
}

NumberFormatRef NumberFormatsBuffer::createNumFmt( sal_uInt32 nNumFmtId, std::u16string_view aFmtCode )
{
    NumberFormatRef xNumFmt;
    xNumFmt = std::make_shared<NumberFormat>( *this );
    maNumFmts[ nNumFmtId ] = xNumFmt;
    if ( nNumFmtId > mnHighestId )
        mnHighestId = nNumFmtId;
    xNumFmt->setFormatCode( aFmtCode );
    return xNumFmt;
}

NumberFormatRef NumberFormatsBuffer::importNumFmt( const AttributeList& rAttribs )
{
    sal_Int32 nNumFmtId = rAttribs.getInteger( XML_numFmtId, -1 );
    OUString aFmtCode = rAttribs.getXString( XML_formatCode, OUString() );
    /* TODO: there may be a x16r2:formatCode16 attribute that would take
     * precedence over the formatCode attribute, see
     * https://learn.microsoft.com/en-us/openspecs/office_standards/ms-xlsx/8c82391e-f128-499a-80a1-734b8504f60e
     * The number format scanner would have to handle the
     * [$<currency string>-<culture info>[,<calendar type and numeral system>]]
     * part.*/
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

sal_uInt32 NumberFormatsBuffer::fillToItemSet( SfxItemSet& rItemSet, sal_uInt32 nNumFmtId, bool bSkipPoolDefs ) const
{
    const NumberFormat* pNumFmt = maNumFmts.get(nNumFmtId).get();
    if (!pNumFmt)
        return 0;

    return pNumFmt->fillToItemSet( rItemSet, bSkipPoolDefs);
}

void NumberFormatsBuffer::insertBuiltinFormats()
{
    // build a map containing pointers to all tables
    typedef ::std::map< OUString, const BuiltinFormatTable* > BuiltinMap;
    BuiltinMap aBuiltinMap;
    for(auto const &rTable : spBuiltinFormatTables)
        aBuiltinMap[rTable.macLocale] = &rTable;

    // convert locale string to locale struct
    Locale aSysLocale( LanguageTag::convertToLocale( maLocaleStr));

    // build a list of table pointers for the current locale, with all parent tables
    typedef ::std::vector< const BuiltinFormatTable* > BuiltinVec;
    BuiltinVec aBuiltinVec;
    BuiltinMap::const_iterator aMIt = aBuiltinMap.find( maLocaleStr ), aMEnd = aBuiltinMap.end();
    OSL_ENSURE( aMIt != aMEnd,
        OStringBuffer( "NumberFormatsBuffer::insertBuiltinFormats - locale '"  +
            OUStringToOString( maLocaleStr, RTL_TEXTENCODING_ASCII_US ) +
            "' not supported (#i29949#)" ).getStr() );
    // start with default table, if no table has been found
    if( aMIt == aMEnd )
        aMIt = aBuiltinMap.find( u"*"_ustr );
    OSL_ENSURE( aMIt != aMEnd, "NumberFormatsBuffer::insertBuiltinFormats - default map not found" );
    // insert all tables into the vector
    for (; aMIt != aMEnd; aMIt = aBuiltinMap.find(aMIt->second->macParent))
        aBuiltinVec.push_back( aMIt->second );

    // insert the default formats in the format map (in reverse order from default table to system locale)
    std::map< sal_uInt32, sal_uInt32 > aReuseMap;
    for( BuiltinVec::reverse_iterator aVIt = aBuiltinVec.rbegin(), aVEnd = aBuiltinVec.rend(); aVIt != aVEnd; ++aVIt )
    {
        // do not put the current system locale for default table
        Locale aLocale;
        if (!(*aVIt)->macParent.isEmpty() && (*aVIt)->macLocale != maLocaleStr)
            aLocale = aSysLocale;
        for (const BuiltinFormat& rBuiltin : (*aVIt)->maFormats)
        {
            NumberFormatRef& rxNumFmt = maNumFmts[rBuiltin.mnNumFmtId];
            rxNumFmt = std::make_shared<NumberFormat>( *this );

            bool bReuse = false;
            if (!rBuiltin.macFmtCode.isEmpty())
                rxNumFmt->setFormatCode(aLocale, rBuiltin.macFmtCode);
            else if (rBuiltin.mnPredefId >= 0)
                rxNumFmt->setPredefinedId(aLocale, rBuiltin.mnPredefId);
            else
                bReuse = rBuiltin.mnReuseId >= 0;

            if( bReuse )
                aReuseMap[rBuiltin.mnNumFmtId] = rBuiltin.mnReuseId;
            else
                aReuseMap.erase(rBuiltin.mnNumFmtId);
        }
    }

    // copy reused number formats
    for( const auto& [rNumFmtId, rReuseId] : aReuseMap )
    {
        maNumFmts[ rNumFmtId ] = maNumFmts[ rReuseId ];
        if ( rNumFmtId > mnHighestId )
            mnHighestId = rNumFmtId;
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
