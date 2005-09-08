/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotype.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:07:17 $
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

#ifndef INCLUDED_codemaker_unotype_hxx
#define INCLUDED_codemaker_unotype_hxx

#include "sal/types.h"

#include <vector>

namespace rtl { class OString; }

namespace codemaker {

namespace UnoType {
    /**
       An enumeration of all the sorts of UNO types.

       All complex UNO types are subsumed under SORT_COMPLEX.
     */
    enum Sort {
        SORT_VOID,
        SORT_BOOLEAN,
        SORT_BYTE,
        SORT_SHORT,
        SORT_UNSIGNED_SHORT,
        SORT_LONG,
        SORT_UNSIGNED_LONG,
        SORT_HYPER,
        SORT_UNSIGNED_HYPER,
        SORT_FLOAT,
        SORT_DOUBLE,
        SORT_CHAR,
        SORT_STRING,
        SORT_TYPE,
        SORT_ANY,
        SORT_COMPLEX
    };

    /**
       Maps from a binary UNO type name or UNO type registry name to its type
       sort.

       @param type a binary UNO type name or UNO type registry name

       @return the sort of the UNO type denoted by the given type; the detection
       of the correct sort is purely syntactical (especially, if the given input
       is a UNO type registry name that denotes something other than a UNO type,
       SORT_COMPLEX is returned)
     */
    Sort getSort(rtl::OString const & type);

    /**
       Determines whether a UNO type name or UNO type registry name denotes a
       UNO sequence type.

       @param type a binary UNO type name or UNO type registry name

       @return true iff the given type denotes a UNO sequence type; the
       detection is purely syntactical
     */
    bool isSequenceType(rtl::OString const & type);

    /**
       Decomposes a UNO type name or UNO type registry name.

       @param type a binary UNO type name or UNO type registry name

       @param rank if non-null, returns the rank of the denoted UNO type (which
       is zero for any given type that does not denote a UNO sequence type)

       @param arguments if non-null, the type arguments are stripped from an
       instantiated polymorphic struct type and returned via this parameter (in
       the correct order); if null, type arguments are not stripped from
       instantiated polymorphic struct types

       @return the base part of the given type
     */
    rtl::OString decompose(
        rtl::OString const & type, sal_Int32 * rank = 0,
        std::vector< rtl::OString > * arguments = 0);
}

}

#endif
