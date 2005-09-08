/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:01:08 $
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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//-----------------------------------------------------------------------------
namespace rtl { class OUString; }
//-----------------------------------------------------------------------------

namespace configmgr
{
//-----------------------------------------------------------------------------

    namespace memory { class Allocator; class Accessor; }
    //-----------------------------------------------------------------------------

    namespace sharable
    {
    //-----------------------------------------------------------------------------
    // some base types
        typedef sal_uInt32 Address; // points to absolute location in memory segment
        typedef sal_uInt16 Offset;  // Offset relative to 'this' in array of nodes
        typedef sal_uInt8  Byte;

    // some derived types
        typedef Address Name;    // points to counted sequence of char (7-bit ASCII) (!?)
        typedef Address String;  // points to counted sequence of sal_Unicode
        typedef Address List;    // singly linked intrusive, used for set elements
        typedef Address Vector;   // points to counted sequence of some type

    //-----------------------------------------------------------------------------

        // for now Name == String
        typedef sal_Unicode NameChar;
        Name allocName(memory::Allocator const& _anAllocator, ::rtl::OUString const & _sString);
        // Name copyName(memory::Allocator const& _anAllocator, Name _aName);
        void freeName(memory::Allocator const& _anAllocator, Name _aName);
        ::rtl::OUString readName(memory::Accessor const& _anAccessor, Name _aName);
        NameChar const * accessName(memory::Accessor const& _anAccessor, Name _aName);

    //-----------------------------------------------------------------------------

        typedef sal_Unicode StringChar;
        String allocString(memory::Allocator const& _anAllocator, ::rtl::OUString const & _sString);
        // String copyString(memory::Allocator const& _anAllocator, String _aString);
        void   freeString(memory::Allocator const& _anAllocator, String _aString);
        ::rtl::OUString readString(memory::Accessor const& _anAccessor, String _aString);
        StringChar const * accessString(memory::Accessor const& _anAccessor, String _aString);

    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}


#endif // INCLUDED_SHARABLE_BASETYPES_HXX
