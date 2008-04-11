/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: string.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_COMPHELPER_STRING_HXX
#define INCLUDED_COMPHELPER_STRING_HXX

#include "sal/config.h"

#include <cstddef>
#include "comphelper/comphelperdllapi.h"
#include "sal/types.h"
#include <com/sun/star/uno/Sequence.hxx>


namespace rtl { class OUString; }

// rtl::OUString helper functions that are not widespread or mature enough to
// go into the stable URE API:
namespace comphelper { namespace string {

/**
   Replace the first occurrence of a substring with another string.

   @param source
   The source string, in which the search will take place.

   @param from
   The ASCII substring to search for.  Must point to at least fromLength ASCII
   characters.

   @param fromLength
   The length of the from substring.  Must not be negative.

   @param to
   The string to use as replacement.

   @param beginAt
   The index at which to begin the search.  Must be between zero and the length
   of source, inclusive.

   @param replacedAt
   If non-null, receives the starting index at which the replacement took place
   or -1 if from was not found.

   @return
   The resulting string, in which the replacement has taken place.
*/
COMPHELPER_DLLPUBLIC rtl::OUString searchAndReplaceAsciiL(
    rtl::OUString const & source, char const * from, sal_Int32 fromLength,
    rtl::OUString const & to, sal_Int32 beginAt = 0,
    sal_Int32 * replacedAt = NULL);

/** does an in-place replacement of the first occurance of a sub string with
    another string

    @param source
        the string to search and replace in.
    @param asciiPattern
        the ASCII sub string to search for. Must point to a 0-terminated string.
    @param replace
        The string to use as replacement.
    @param beginAt
        The index at which to begin the search.  Must be between zero and the length
        of source, inclusive.

    @param replacedAt
        If non-null, receives the starting index at which the replacement took place
        or -1 if from was not found.

    @return
        a reference to <code>source</code>
*/
COMPHELPER_DLLPUBLIC ::rtl::OUString&
    searchAndReplaceAsciiI( ::rtl::OUString & source, sal_Char const * asciiPattern,
                            ::rtl::OUString const & replace, sal_Int32 beginAt = 0,
                            sal_Int32 * replacedAt = NULL );

/** Convert a sequence of strings to a single comma separated string.

    Note that no escaping of commas or anything fancy is done.

    @param i_rSeq   A list of strings to be concatenated.

    @return         A single string containing the concatenation of the given
                    list, interspersed with the string ", ".
 */
COMPHELPER_DLLPUBLIC ::rtl::OUString convertCommaSeparated(
    ::com::sun::star::uno::Sequence< ::rtl::OUString > const & i_rSeq);

/** Convert a single comma separated string to a sequence of strings.

    Note that no escaping of commas or anything fancy is done.

    @param i_rString    A string containing comma-separated words.

    @return         A sequence of strings resulting from splitting the given
                    string at ',' tokens and stripping whitespace.
 */
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Sequence< ::rtl::OUString >
    convertCommaSeparated( ::rtl::OUString const & i_rString );

} }

#endif
