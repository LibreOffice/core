/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:19:26 $
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
