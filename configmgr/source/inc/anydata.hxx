/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anydata.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:41:13 $
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

#ifndef INCLUDED_SHARABLE_ANYDATA_HXX
#define INCLUDED_SHARABLE_ANYDATA_HXX

#ifndef INCLUDED_SHARABLE_BASETYPES_HXX
#include "types.hxx"
#endif

//-----------------------------------------------------------------------------
namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class Type;
} } } }
//-----------------------------------------------------------------------------

namespace configmgr
{
//-----------------------------------------------------------------------------

    namespace memory { class Allocator; class Accessor; }
    //-----------------------------------------------------------------------------

    namespace sharable
    {
    //-----------------------------------------------------------------------------

        //typedef Address AnyData; // data that fits is stored inline
        union AnyData
        {
            typedef Byte TypeCode;

            Address     data;
            sal_Bool    boolValue;
            sal_Int16   shortValue;
            sal_Int32   intValue;
            Address     longValue;      // points to sal_Int64
            Address     doubleValue;    // points to double (IEEE 8-bit) ...
       //     float       floatValue;     // ... or should we use float (IEEE 4-bit) ?
            Vector      binaryValue;    // points to counted sal_(u)Int8 []
            String      stringValue;    // points to counted sal_Unicode []
            Vector      sequenceValue;  // points to counted AnyData [] (or SomeType [] ?)
        };

    //-----------------------------------------------------------------------------

        AnyData::TypeCode getTypeCode(::com::sun::star::uno::Type const & _aType);
        ::com::sun::star::uno::Type getUnoType( AnyData::TypeCode _aType);

        AnyData allocData(memory::Allocator const& _anAllocator, AnyData::TypeCode _aType, ::com::sun::star::uno::Any const & _aAny);
    //    AnyData copyData(memory::Allocator const& _anAllocator, AnyData::TypeCode _aType, AnyData _aData);
        void    freeData(memory::Allocator const& _anAllocator, AnyData::TypeCode _aType, AnyData _aData);
        ::com::sun::star::uno::Any readData(memory::Accessor const& _anAccessor, AnyData::TypeCode _aType, AnyData _aData);

    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}


#endif // INCLUDED_SHARABLE_ANYDATA_HXX

