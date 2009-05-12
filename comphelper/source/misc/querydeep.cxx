/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: querydeep.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/querydeep.hxx>
#include <typelib/typedescription.h>

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



