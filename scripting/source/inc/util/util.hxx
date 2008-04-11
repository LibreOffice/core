/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: util.hxx,v $
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


#ifndef _COM_SUN_STAR_SCRIPTING_UTIL_UTIL_HXX_
#define _COM_SUN_STAR_SCRIPTING_UTIL_UTIL_HXX_

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#define OUSTR(x) ::rtl::OUString( ::rtl::OUString::createFromAscii(x) )

namespace scripting_util
{
    inline void validateXRef(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xRef, const sal_Char* Msg) throw (::com::sun::star::uno::RuntimeException)
    {
        OSL_ENSURE( xRef.is(), Msg );

        if(!xRef.is())
        {
            throw ::com::sun::star::uno::RuntimeException(OUSTR(Msg), ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >());
        }
    }
}
#endif //_COM_SUN_STAR_SCRIPTING_UTIL_UTIL_HXX_
