/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: querydeep.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:51:50 $
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

#ifndef _COMPHELPER_QUERYDEEPINTERFACE_HXX
#include <comphelper/querydeep.hxx>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif

//__________________________________________________________________________________________________

sal_Bool comphelper::isDerivedFrom(
    const ::com::sun::star::uno::Type & rBaseType,
    const ::com::sun::star::uno::Type & rType )
{
    using namespace ::com::sun::star::uno;

    TypeClass eClass = rBaseType.getTypeClass();

    if (eClass != TypeClass_INTERFACE)
        return sal_False;

    // supported TypeClass - do the types match ?
    if (eClass != rType.getTypeClass())
        return sal_False;

    sal_Bool bRet;

    // shortcut for simple case
    if (rBaseType == ::getCppuType(static_cast<const Reference< XInterface > *>(0)))
    {
        bRet = sal_True;
    }
    else
    {
        // now ask in cppu (aka typelib)
        ::typelib_TypeDescription *pBaseTD = 0, *pTD = 0;

        rBaseType.  getDescription(&pBaseTD);
        rType.      getDescription(&pTD);

        // interfaces are assignable to a base
        bRet = ::typelib_typedescription_isAssignableFrom(pBaseTD, pTD);

        ::typelib_typedescription_release(pBaseTD);
        ::typelib_typedescription_release(pTD);
    }

    return bRet;
}



