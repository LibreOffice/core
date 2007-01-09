/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unohelp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-09 11:34:29 $
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
        sal_Int32 nValue = 0;
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
