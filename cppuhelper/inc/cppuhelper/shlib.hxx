/*************************************************************************
 *
 *  $RCSfile: shlib.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:15:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CPPUHELPER_SHLIB_HXX_
#define _CPPUHELPER_SHLIB_HXX_

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/registry/CannotRegisterImplementationException.hpp>


namespace cppu
{

/** Loads a shared library component and gets the factory out of it.
    You can give either a fully qualified libname or single lib name.
    The libname need not be pre/postfixed (e.g. xxx.dll).
    You can give an extra path to force lookup of the library.
    The resulting path of the library will be checked against
    environment variable CPLD_ACCESSPATH.

    @param rLibName
           name of the library
    @param rPath
           optional path
    @param rImplName
           implementation to be retrieved from the library
    @param xMgr
           service manager to be provided to the component
    @param xKey
           registry key to be provided to the component
    @throws ::com::sun::star::loader::CannotActivateFactoryException
            if activation failed
*/
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
SAL_CALL loadSharedLibComponentFactory(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::rtl::OUString const & rImplName,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::loader::CannotActivateFactoryException) );

/** Invokes component_writeInfo() function of specified component library.
    You can give either a fully qualified libname or single lib name.
    The libname need not be pre/postfixed (e.g. xxx.dll).
    You can give an extra path to force lookup of the library.
    The resulting path of the library will be checked against
    environment variable CPLD_ACCESSPATH.

    @param rLibName
           name of the library
    @param rPath
           optional path
    @param xMgr
           service manager to be provided to the component
    @param xKey
           registry key to be provided to the component
    @throws ::com::sun::star::registry::CannotRegisterImplementationException
            if writing the info failed
*/
void
SAL_CALL writeSharedLibComponentInfo(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::registry::CannotRegisterImplementationException) );

} // end namespace cppu

#endif
