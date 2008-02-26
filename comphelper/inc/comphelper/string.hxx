/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: string.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:13:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_COMPHELPER_STRING_HXX
#define INCLUDED_COMPHELPER_STRING_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <cstddef>

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


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
COMPHELPER_DLLPUBLIC rtl::OUString searchAndReplace(
    rtl::OUString const & source, char const * from, sal_Int32 fromLength,
    rtl::OUString const & to, sal_Int32 beginAt = 0,
    sal_Int32 * replacedAt = NULL);

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
