/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: legacysingletonfactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 09:34:58 $
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

#ifndef COMPHELPER_LEGACYSINGLETONFACTORY_HXX
#define COMPHELPER_LEGACYSINGLETONFACTORY_HXX

#include "comphelper/comphelperdllapi.h"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/factory.hxx>

//........................................................................
namespace comphelper
{
//........................................................................

/** creates a factory which can be used when implementing old-style singletons

    Before UNO having the component context concept, singletons were implemented by providing
    a factory which return the very same component instance for every creation request.

    Nowadays, singletons are implemented by making them available at the component context,
    as dedicated property.

    To bridge between both worlds - sometimes necessary to support legacy code, which instantiates
    singletons at the global service manager, instead of obtaining the property at the
    component context -, you can use the function below. I creates a single-component
    factory, which behaves like the old factories did: Upon multiple creation requests,
    it will always return the same instance. Additionally, the signature of the function
    is the same as the signature of <code>::cppu::createSingleComponentFactory</code>
    (which creates a "normal" component factory), so you can use both factory functions
    in the same context.
*/
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >
    COMPHELPER_DLLPUBLIC createLegacySingletonFactory(
        ::cppu::ComponentFactoryFunc _componentFactoryFunc,
        const ::rtl::OUString& _rImplementationName,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
        rtl_ModuleCount* _pModCount = NULL
    );

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_LEGACYSINGLETONFACTORY_HXX
