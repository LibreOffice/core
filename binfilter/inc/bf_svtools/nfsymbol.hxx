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

#ifndef INCLUDED_SVTOOLS_NFSYMBOL_HXX
#define INCLUDED_SVTOOLS_NFSYMBOL_HXX

/* ATTENTION! If new types arrive that had its content previously handled as
 * SYMBOLTYPE_STRING, they have to be added at several places in zforscan.cxx
 * and/or zformat.cxx, and in xmloff/source/style/xmlnumfe.cxx. Mostly these
 * are places where already NF_SYMBOLTYPE_STRING together with
 * NF_SYMBOLTYPE_CURRENCY or NF_SYMBOLTYPE_DATESEP are used in the same case of
 * a switch respectively an if-condition.
 */

namespace binfilter
{

/// Number formatter's symbol types of a token, if not key words, which are >0
enum NfSymbolType
{
    NF_SYMBOLTYPE_STRING        = -1,   // literal string in output
    NF_SYMBOLTYPE_DEL           = -2,   // special character
    NF_SYMBOLTYPE_BLANK         = -3,   // blank for '_'
    NF_SYMBOLTYPE_STAR          = -4,   // *-character
    NF_SYMBOLTYPE_DIGIT         = -5,   // digit place holder
    NF_SYMBOLTYPE_DECSEP        = -6,   // decimal separator
    NF_SYMBOLTYPE_THSEP         = -7,   // group AKA thousand separator
    NF_SYMBOLTYPE_EXP           = -8,   // exponent E
    NF_SYMBOLTYPE_FRAC          = -9,   // fraction /
    NF_SYMBOLTYPE_EMPTY         = -10,  // deleted symbols
    NF_SYMBOLTYPE_FRACBLANK     = -11,  // delimiter between integer and fraction
    NF_SYMBOLTYPE_COMMENT       = -12,  // comment is following
    NF_SYMBOLTYPE_CURRENCY      = -13,  // currency symbol
    NF_SYMBOLTYPE_CURRDEL       = -14,  // currency symbol delimiter [$]
    NF_SYMBOLTYPE_CURREXT       = -15,  // currency symbol extension -xxx
    NF_SYMBOLTYPE_CALENDAR      = -16,  // calendar ID
    NF_SYMBOLTYPE_CALDEL        = -17,  // calendar delimiter [~]
    NF_SYMBOLTYPE_DATESEP       = -18,  // date separator
    NF_SYMBOLTYPE_TIMESEP       = -19,  // time separator
    NF_SYMBOLTYPE_TIME100SECSEP = -20,  // time 100th seconds separator
    NF_SYMBOLTYPE_PERCENT       = -21   // percent %
};

}

#endif // INCLUDED_SVTOOLS_NFSYMBOL_HXX
