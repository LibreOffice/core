/*************************************************************************
 *
 *  $RCSfile: servicefactory.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:09 $
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

/*
 * This bootstraps an initial service factory up to the point
 * that a dll component loader, simple and default registry and an implementation registration
 * service is registered and available.
 * All other services have to be registered somewhere else.
 */

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#define _CPPUHELPER_SERVICEFACTORY_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>


namespace cppu
{

/**
 * This bootstraps an initial service factory working on a registry. If the first or both
 * parameters contain a value then the service factory is initialized with a simple registry
 * or a nested registry. Otherwise the service factory must be initialized later with a valid
 * registry.
 *<BR>
 * @param rWriteRegistryFile    file name of the simple registry or the first registry file of
 *                              the nested registry which will be opened with read/write rights. This
 *                              file will be created if necessary.
 * @param rReadRegistryFile     file name of the second registry file of the nested registry
 *                              which will be opened with readonly rights.
 * @param bReadOnly             flag which specify that the first registry file will be opened with
 *                              readonly rights. Default is FALSE. If this flag is used the registry
 *                              will not be created if not exist.
 * @param rBootstrapPath        specify a path where the bootstrap components could be find. If this
 *                              parameter is an empty string the compoents will be searched in the normal
 *                              path.
 *
 * @author Daniel Boelzle
 */
::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SAL_CALL
    createRegistryServiceFactory( const ::rtl::OUString & rWriteRegistryFile,
                                  const ::rtl::OUString & rReadRegistryFile,
                                  sal_Bool bReadOnly = sal_False,
                                  const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
        throw( ::com::sun::star::uno::Exception );


/**
 * This bootstraps an initial service factory working on a registry file.
 *<BR>
 * @param rRegistryFile         file name of the registry to use/ create; if this is an empty
 *                              string, the default registry is used instead
 * @param bReadOnly             flag which specify that the registry file will be opened with
 *                              readonly rights. Default is FALSE. If this flag is used the registry
 *                              will not be created if not exist.
 *
 * @author Daniel Boelzle
 */
inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SAL_CALL
    createRegistryServiceFactory( const ::rtl::OUString & rRegistryFile,
                                  sal_Bool bReadOnly = sal_False,
                                  const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
        throw( ::com::sun::star::uno::Exception )
{
    return ::cppu::createRegistryServiceFactory( rRegistryFile, ::rtl::OUString(), bReadOnly, rBootstrapPath );
}


/**
 * This bootstraps a service factory without initilaize a registry.
 *<BR>
 * @author Daniel Boelzle
 */
inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SAL_CALL
    createServiceFactory(const ::rtl::OUString & rBootstrapPath = ::rtl::OUString())
        throw( ::com::sun::star::uno::Exception )
{
    return ::cppu::createRegistryServiceFactory( ::rtl::OUString(), ::rtl::OUString(), sal_False, rBootstrapPath );
}

} // end namespace cppu

#endif

