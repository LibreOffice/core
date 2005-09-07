/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flushcode.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:18:51 $
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

#include "sal/config.h"

#include "flushcode.hxx"

extern "C" void doFlushCode(unsigned long address, unsigned long count);

namespace bridges { namespace cpp_uno { namespace cc50_solaris_sparc {

void flushCode(void const * begin, void const * end) {
    unsigned long n =
        static_cast< char const * >(end) - static_cast< char const * >(begin);
    if (n != 0) {
        unsigned long adr = reinterpret_cast< unsigned long >(begin);
        unsigned long off = adr & 7;
        doFlushCode(adr - off, (n + off + 7) >> 3);
    }
}

} } }
