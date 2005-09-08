/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nfsymbol.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 10:00:25 $
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

#ifndef INCLUDED_SVTOOLS_NFSYMBOL_HXX
#define INCLUDED_SVTOOLS_NFSYMBOL_HXX

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

}   // namespace svt

#endif // INCLUDED_SVTOOLS_NFSYMBOL_HXX
