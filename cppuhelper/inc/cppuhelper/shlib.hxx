/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shlib.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:21:21 $
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
#ifndef _CPPUHELPER_SHLIB_HXX_
#define _CPPUHELPER_SHLIB_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/registry/CannotRegisterImplementationException.hpp>


namespace cppu
{

/** Loads a shared library component and gets the factory out of it.  You can give either a
    fully qualified libname or single lib name.  The libname need not be pre/postfixed
    (e.g. xxx.dll).  You can give parameter rPath to force lookup of the library in a specific
    directory.  The resulting path of the library will be checked against environment variable
    CPLD_ACCESSPATH if set.

    @param rLibName name of the library
    @param rPath optional path
    @param rImplName implementation to be retrieved from the library
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
    @return
    factory instance (::com::sun::star::lang::XSingleComponentFactory or
    ::com::sun::star::lang::XSingleComponentFactory)
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL loadSharedLibComponentFactory(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::rtl::OUString const & rImplName,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::loader::CannotActivateFactoryException) );

/** Invokes component_writeInfo() function of specified component library.  You can give either
    a fully qualified libname or single lib name. The libname need not be pre/postfixed
    (e.g. xxx.dll).  You can give parameter rPath to force lookup of the library in a specific
    directory.  The resulting path of the library will be checked against environment variable
    CPLD_ACCESSPATH if set.

    @param rLibName name of the library
    @param rPath optional path
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
*/
void
SAL_CALL writeSharedLibComponentInfo(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::registry::CannotRegisterImplementationException) );

} // end namespace cppu

#endif
