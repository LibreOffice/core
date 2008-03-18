/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appendunixshellword.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:27:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2008 by Sun Microsystems, Inc.
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

#include "precompiled_tools.hxx"
#include "sal/config.h"

#if defined UNX

#include <cstddef>

#include "osl/diagnose.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "sal/types.h"
#include "tools/appendunixshellword.hxx"

namespace tools {

void appendUnixShellWord(
    rtl::OStringBuffer * accumulator, rtl::OString const & text)
{
    OSL_ASSERT(accumulator != NULL);
    if (text.getLength() == 0) {
        accumulator->append(RTL_CONSTASCII_STRINGPARAM("''"));
    } else {
        bool quoted = false;
        for (sal_Int32 i = 0; i < text.getLength(); ++i) {
            char c = text[i];
            if (c == '\'') {
                if (quoted) {
                    accumulator->append('\'');
                    quoted = false;
                }
                accumulator->append(RTL_CONSTASCII_STRINGPARAM("\\'"));
            } else {
                if (!quoted) {
                    accumulator->append('\'');
                    quoted = true;
                }
                accumulator->append(c);
            }
        }
        if (quoted) {
            accumulator->append('\'');
        }
    }
}

}

#endif
