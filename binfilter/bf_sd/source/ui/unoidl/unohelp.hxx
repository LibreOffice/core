/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
namespace binfilter {//STRIP009
namespace sd
{
inline sal_Bool any2bool( const ::com::sun::star::uno::Any& rAny, sal_Bool& rBool )
{
    if( rAny.getValueType() == ::getCppuBooleanType() )
    {
        rBool = *(sal_Bool*)rAny.getValue();
    }
    else
    {
        sal_Int32 nValue;
        if(!(rAny >>= nValue))
            return sal_False;
        rBool = nValue != 0;
    }

    return sal_True;
}

inline void bool2any( sal_Bool bBool, ::com::sun::star::uno::Any& rAny )
{
    rAny.setValue( &bBool, ::getCppuBooleanType() );
}

}

#define ITYPE( xint ) \
    ::getCppuType((const uno::Reference< xint >*)0)

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

} //namespace binfilter
