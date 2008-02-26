/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: string.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:14:25 $
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

#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include <cstddef>
#include <vector>

#include "comphelper/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"


namespace comphelper { namespace string {

rtl::OUString searchAndReplace(
    rtl::OUString const & source, char const * from, sal_Int32 fromLength,
    rtl::OUString const & to, sal_Int32 beginAt, sal_Int32 * replacedAt)
{
    sal_Int32 n = source.indexOfAsciiL(from, fromLength, beginAt);
    if (replacedAt != NULL) {
        *replacedAt = n;
    }
    return n == -1 ? source : source.replaceAt(n, fromLength, to);
}

// convert between sequence of string and comma separated string

::rtl::OUString convertCommaSeparated(
    ::com::sun::star::uno::Sequence< ::rtl::OUString > const& i_rSeq)
{
    ::rtl::OUString ret;
    for (sal_Int32 i = 0; i < i_rSeq.getLength(); ++i) {
        if (i != 0) ret += ::rtl::OUString::createFromAscii(", ");
        ret += i_rSeq[i];
    }
    return ret;
}

::com::sun::star::uno::Sequence< ::rtl::OUString >
    convertCommaSeparated( ::rtl::OUString const& i_rString )
{
    std::vector< ::rtl::OUString > vec;
    sal_Int32 idx = 0;
    do {
      ::rtl::OUString kw =
        i_rString.getToken(0, static_cast<sal_Unicode> (','), idx);
      kw = kw.trim();
      if (kw.getLength() > 0) {
          vec.push_back(kw);
      }
    } while (idx >= 0);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > kws(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        kws[i] = vec.at(i);
    }
    return kws;
}

} }
