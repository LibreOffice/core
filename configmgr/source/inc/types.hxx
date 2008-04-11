/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: types.hxx,v $
 * $Revision: 1.6 $
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

#ifndef INCLUDED_SHARABLE_BASETYPES_HXX
#define INCLUDED_SHARABLE_BASETYPES_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

//-----------------------------------------------------------------------------
namespace rtl { class OUString; }
//-----------------------------------------------------------------------------

namespace configmgr
{
    namespace sharable
    {
    //-----------------------------------------------------------------------------
    // some base types
        typedef sal_uInt16 Offset;  // Offset relative to 'this' in array of nodes

    // some derived types
        typedef rtl_uString *  Name;
        typedef rtl_uString *  String;
        typedef struct TreeFragment * List; // singly linked intrusive, used for set elements
        typedef sal_uInt8 *           Vector;   // points to counted sequence of some type

    //-----------------------------------------------------------------------------

        inline String allocString(::rtl::OUString const & _sString)
        {
            rtl_uString_acquire(_sString.pData);
            return _sString.pData;
        }
        inline void freeString(String _aString)
        {   rtl_uString_release(_aString);    }
        inline ::rtl::OUString readString(String _aString)
        {   return rtl::OUString(_aString);   }

    //-----------------------------------------------------------------------------

        inline Name allocName(::rtl::OUString const & _aName)
        {   return allocString(_aName); }
        inline void freeName(Name _aName)
        {   freeString(_aName); }
        inline ::rtl::OUString readName(Name _aName)
        {   return readString(_aName); }

    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // INCLUDED_SHARABLE_BASETYPES_HXX
