/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anydata.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_SHARABLE_ANYDATA_HXX
#define INCLUDED_SHARABLE_ANYDATA_HXX

#include "rtl/ustring.h"
#include "sal/types.h"

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
            sal_Bool    boolValue;
            sal_Int16   shortValue;
            sal_Int32   intValue;
            sal_Int64  *longValue;
            double     *doubleValue;
            sal_uInt8 *      binaryValue;    // points to counted sal_(u)Int8 []
            rtl_uString *      stringValue;    // points to counted sal_Unicode []
            sal_uInt8 *      sequenceValue;  // points to counted AnyData [] (or SomeType [] ?)
            void       *data;           // used to initialize to 0
        };

    //-----------------------------------------------------------------------------

        sal_uInt8 getTypeCode(::com::sun::star::uno::Type const & _aType);
        ::com::sun::star::uno::Type getUnoType( sal_uInt8 _aType);

        AnyData allocData(sal_uInt8 _aType, ::com::sun::star::uno::Any const & _aAny);
        void    freeData(sal_uInt8 _aType, AnyData _aData);
        ::com::sun::star::uno::Any readData(sal_uInt8 _aType, AnyData _aData);

    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}


#endif // INCLUDED_SHARABLE_ANYDATA_HXX

