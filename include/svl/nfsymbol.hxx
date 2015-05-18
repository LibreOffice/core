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

#ifndef INCLUDED_SVL_NFSYMBOL_HXX
#define INCLUDED_SVL_NFSYMBOL_HXX

/* ATTENTION! If new types arrive that had its content previously handled as
 * SYMBOLTYPE_STRING, they have to be added at several places in zforscan.cxx
 * and/or zformat.cxx, and in xmloff/source/style/xmlnumfe.cxx. Mostly these
 * are places where already NF_SYMBOLTYPE_STRING together with
 * NF_SYMBOLTYPE_CURRENCY or NF_SYMBOLTYPE_DATESEP are used in the same case of
 * a switch respectively an if-condition.
 */

namespace svt {

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
    NF_SYMBOLTYPE_CURRENCY      = -12,  // currency symbol
    NF_SYMBOLTYPE_CURRDEL       = -13,  // currency symbol delimiter [$]
    NF_SYMBOLTYPE_CURREXT       = -14,  // currency symbol extension -xxx
    NF_SYMBOLTYPE_CALENDAR      = -15,  // calendar ID
    NF_SYMBOLTYPE_CALDEL        = -16,  // calendar delimiter [~]
    NF_SYMBOLTYPE_DATESEP       = -17,  // date separator
    NF_SYMBOLTYPE_TIMESEP       = -18,  // time separator
    NF_SYMBOLTYPE_TIME100SECSEP = -19,  // time 100th seconds separator
    NF_SYMBOLTYPE_PERCENT       = -20,  // percent %
    NF_SYMBOLTYPE_FRAC_FDIV     = -21   // forced divisors
};

}

#endif // INCLUDED_SVL_NFSYMBOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
