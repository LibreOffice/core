/*************************************************************************
 *
 *  $RCSfile: implementationentry.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-06-22 15:19:54 $
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
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#define _CPPUHELPER_IMPLEMENATIONENTRY_HXX_

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

namespace cppu
{
/** One struct instance represents all data necessary for registering one service implementation.

 */
struct ImplementationEntry
{
    /** Function, that creates an instance of the implemenation
     */
       ComponentFactoryFunc create;

    /** Function, that returns the implemenation-name of the implemenation
       (same as XServiceInfo.getImplementationName() ).
     */
     rtl::OUString ( SAL_CALL * getImplementationName )();

    /** Function, that returns all supported servicenames of the implemenation
       ( same as XServiceInfo.getSupportedServiceNames() ).
    */
     com::sun::star::uno::Sequence< rtl::OUString > ( SAL_CALL * getSupportedServiceNames ) ();

    /** Function, that creates a SingleComponentFactory.
    */
     ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >
     ( SAL_CALL * createFactory )(
         ComponentFactoryFunc fptr,
         ::rtl::OUString const & rImplementationName,
         ::com::sun::star::uno::Sequence< ::rtl::OUString > const & rServiceNames,
         rtl_ModuleCount * pModCount );

    /** The shared-library module-counter of the implemenation. Maybe 0. The module-counter
        is used during by the createFactory()-function.
    */
     rtl_ModuleCount * moduleCounter;

    /** Must be set to 0 !
        For future extensions.
     */
    sal_Int32 nFlags;
};

/** Helper function for implementation of the component_writeInfo()-function.

    @param pServiceManager The first parameter passed to component_writeInfo()-function
                           (This is an instance of the service manager, that creates the factory).
    @param pRegistryKey    The second parameter passed to the component_writeInfo()-function.
                           This is a reference to the registry key, into which the implementation
                           data shall be written to.
    @param entries         Each element of the entries-array must contains a function pointer
                           table for registering an implemenation. The end of the array
                           must be marked with a 0 entry in the create-function.
    @return sal_True, if all implementations could be registered, otherwise sal_False.
 */
sal_Bool component_writeInfoHelper(
    void *pServiceManager, void *pRegistryKey , const struct ImplementationEntry entries[] );

/** Helper function for implementation of the component_getFactory()-function,
    that must be implemented by every shared library component.

    @param pImplName       The implementation-name to be instantiated ( This is the
                           first parameter passed to the component_getFactory
    @param pServiceManager The first parameter passed to component_writeInfo()-function
                           (This is a of the service manager, that creates the factory).
    @param pRegistryKey    The second parameter passed to the component_writeInfo()-function.
                           This is a reference to the registry key, where the implementation
                           data has been written to.
    @param entries         Each element of the entries-array must contains a function pointer
                           table for creating a factor of the implementation. The end of the array
                           must be marked with a 0 entry in the create-function.
    @return 0 if the helper failed to instantiate a factory, otherwise an acquired pointer
            to a factory.
 */
void *component_getFactoryHelper(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * pRegistryKey,
    const struct ImplementationEntry entries[] );

}
#endif
