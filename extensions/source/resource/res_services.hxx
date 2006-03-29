/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_services.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-29 12:42:32 $
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

#ifndef EXTENSIONS_RESOURCE_SERVICES_HXX
#define EXTENSIONS_RESOURCE_SERVICES_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

//........................................................................
namespace res
{
//........................................................................

    struct ComponentInfo
    {
        /// services supported by the component
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  aSupportedServices;
        /// implementation name of the component
        ::rtl::OUString                                     sImplementationName;
        /** name of the singleton instance of the component, if it is a singleton, empty otherwise
            If the component is a singleton, aSupportedServices must contain exactly one element.
        */
        ::rtl::OUString                                     sSingletonName;
        /// factory for creating the component
        ::cppu::ComponentFactoryFunc                        pFactory;
    };

    ComponentInfo   getComponentInfo_VclStringResourceLoader();
    ComponentInfo   getComponentInfo_OpenOfficeResourceLoader();

//........................................................................
}   // namespace res
//........................................................................

#endif // EXTENSIONS_RESOURCE_SERVICES_HXX
