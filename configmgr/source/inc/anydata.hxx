/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anydata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:14:35 $
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

    namespace sharable
    {
    //-----------------------------------------------------------------------------

        union AnyData
        {
            typedef sal_uInt8 TypeCode;

            sal_Bool    boolValue;
            sal_Int16   shortValue;
            sal_Int32   intValue;
            sal_Int64  *longValue;
            double     *doubleValue;
            Vector      binaryValue;    // points to counted sal_(u)Int8 []
            String      stringValue;    // points to counted sal_Unicode []
            Vector      sequenceValue;  // points to counted AnyData [] (or SomeType [] ?)
            void       *data;           // used to initialize to 0
        };

    //-----------------------------------------------------------------------------

        AnyData::TypeCode getTypeCode(::com::sun::star::uno::Type const & _aType);
        ::com::sun::star::uno::Type getUnoType( AnyData::TypeCode _aType);

        AnyData allocData(AnyData::TypeCode _aType, ::com::sun::star::uno::Any const & _aAny);
        void    freeData(AnyData::TypeCode _aType, AnyData _aData);
        ::com::sun::star::uno::Any readData(AnyData::TypeCode _aType, AnyData _aData);

    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}


#endif // INCLUDED_SHARABLE_ANYDATA_HXX

