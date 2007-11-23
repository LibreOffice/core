/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: string.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 13:15:28 $
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

} }
