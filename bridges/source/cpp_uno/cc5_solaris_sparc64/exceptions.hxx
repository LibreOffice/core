/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: exceptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-04-24 16:25:42 $
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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_EXCEPTIONS_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_EXCEPTIONS_HXX

#include "sal/config.h"

#include <cstddef>

#include "typelib/typedescription.h"
#include "uno/any2.h"
#include "uno/mapping.h"

// Private CC5 structures and functions:
namespace __Crun {
    struct class_base_descr {
        int type_hash[4];
        std::size_t offset;
    };
    struct static_type_info {
        std::ptrdiff_t ty_name;
        std::ptrdiff_t reserved;
        std::ptrdiff_t base_table;
        int type_hash[4];
        unsigned int flags;
        unsigned int cv_qualifiers;
    };
    void * ex_alloc(unsigned long);
    void ex_throw(void *, static_type_info const *, void (*)(void *));
    void * ex_get();
    void ex_rethrow_q() throw ();
}
namespace __Cimpl {
    char const * ex_name();
}

namespace bridges { namespace cpp_uno { namespace cc5_solaris_sparc64 {

void raiseException(uno_Any * exception, uno_Mapping * unoToCpp);

void fillUnoException(
    void * cppException, char const * cppName, uno_Any * unoException,
    uno_Mapping * cppToUno);

} } }

#endif
