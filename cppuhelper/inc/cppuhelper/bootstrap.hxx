/*************************************************************************
 *
 *  $RCSfile: bootstrap.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:54:15 $
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
#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#define _CPPUHELPER_BOOTSTRAP_HXX_

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace cppu
{

/** Creates a simple registry service instance.

    @rBootstrapPath optional bootstrap path for initial components
    @return simple registry service instance
*/
::com::sun::star::uno::Reference< ::com::sun::star::registry::XSimpleRegistry >
SAL_CALL createSimpleRegistry(
    const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( () );

/** Creates a nested registry service instance.

    @rBootstrapPath optional bootstrap path for initial components
    @return nested registry service instance
*/
::com::sun::star::uno::Reference< ::com::sun::star::registry::XSimpleRegistry >
SAL_CALL createNestedRegistry(
    const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( () );

/** Installs type description manager instance, i.e. registers a callback at cppu core.

    @param xTDMgr manager instance
    @return true, if successfully registered
*/
sal_Bool SAL_CALL installTypeDescriptionManager(
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess > const & xTDMgr )
    SAL_THROW( () );

/** Bootstraps an initial component context with service manager upon a given registry.
    This includes insertion of initial services:
      - (registry) service manager, shared lib loader,
      - simple registry, nested registry,
      - implementation registration
      - registry typedescription provider, typedescription manager (also installs it into cppu core)

    @param xRegistry registry for service manager and singleton objects of context (may be null)
    @param rBootstrapPath optional bootstrap path for initial components
    @return component context
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > SAL_CALL
bootstrap_InitialComponentContext(
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XSimpleRegistry > const & xRegistry,
    ::rtl::OUString const & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( (::com::sun::star::uno::Exception) );


/** Bootstraps an initial component context with service manager upon default types and
    services registry.
    This includes insertion of initial services:
      - (registry) service manager, shared lib loader,
      - simple registry, nested registry,
      - implementation registration
      - registry typedescription provider, typedescription manager (also installs it into cppu core)

    This function tries to find its parameters via these bootstrap variables:

      - UNO_TYPES         -- a space separated list of file urls of type rdbs
      - UNO_SERVICES      -- a space separated list of file urls of service rdbs
      - UNO_WRITERDB      -- a file url of a write rdb (e.g. user.rdb)

    Please look at http://udk.openoffice.org/common/man/concept/uno_default_bootstrapping.html
    for further info.

    @return component context
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > SAL_CALL
defaultBootstrap_InitialComponentContext() SAL_THROW( (::com::sun::star::uno::Exception) );


/** Bootstraps an initial component context with service manager upon default types and
    services registry.
    This includes insertion of initial services:
      - (registry) service manager, shared lib loader,
      - simple registry, nested registry,
      - implementation registration
      - registry typedescription provider, typedescription manager (also installs it into cppu core)

    This function tries to find its parameters via these bootstrap variables:

      - UNO_TYPES         -- a space separated list of file urls of type rdbs
      - UNO_SERVICES      -- a space separated list of file urls of service rdbs
      - UNO_WRITERDB      -- a file url of a write rdb (e.g. user.rdb)

    Please look at http://udk.openoffice.org/common/man/concept/uno_default_bootstrapping.html
    for further info.

    @param iniFile ini filename to get bootstrap variables
    @return component context
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > SAL_CALL
defaultBootstrap_InitialComponentContext(const ::rtl::OUString & iniFile) SAL_THROW( (::com::sun::star::uno::Exception) );


} // end namespace cppu

#endif

