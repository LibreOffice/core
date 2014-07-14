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

/** replaces, in the given source string, all occurrences of a given ASCII pattern
    with another ASCII pattern
*/
COMPHELPER_DLLPUBLIC ::rtl::OUString searchAndReplaceAllAsciiWithAscii(
    const ::rtl::OUString& source, const sal_Char* from, const sal_Char* to,
    const sal_Int32 beginAt = 0 );

/** does an in-place replacement of the first occurrence of a sub string with
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
