/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:49:13 $
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

#ifndef INCLUDED_SHARABLE_BASETYPES_HXX
#define INCLUDED_SHARABLE_BASETYPES_HXX

#ifndef CONFIGMGR_MEMORYMODEL_HXX
#include "memorymodel.hxx"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

//-----------------------------------------------------------------------------
namespace rtl { class OUString; }
//-----------------------------------------------------------------------------

namespace configmgr
{
    namespace sharable
    {
    //-----------------------------------------------------------------------------
    // some base types
        typedef memory::Address  Address;  // points to absolute location in memory segment
        typedef memory::HeapSize HeapSize; // size of memory block within heap
        typedef sal_uInt16 Offset;  // Offset relative to 'this' in array of nodes
        typedef sal_uInt8  Byte;

    // some derived types
        typedef rtl_uString *  Name;
        typedef rtl_uString *  String;
        typedef Address List;    // singly linked intrusive, used for set elements
        typedef Address Vector;   // points to counted sequence of some type

    //-----------------------------------------------------------------------------

        Name allocName(::rtl::OUString const & _sString);
        void freeName(Name _aName);
        ::rtl::OUString readName(Name _aName);

    //-----------------------------------------------------------------------------

        String allocString(::rtl::OUString const & _sString);
        void   freeString(String _aString);
        ::rtl::OUString readString(String _aString);

    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}


#endif // INCLUDED_SHARABLE_BASETYPES_HXX
