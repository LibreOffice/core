/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registrationhelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:38:49 $
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

#ifndef _REGISTRATIONHELPER_INCLUDED_INDIRECTLY_
#error "don't include this file directly! use dbu_reghelper.hxx instead!"
#endif

typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > (SAL_CALL *FactoryInstantiation)
        (
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rServiceManager,
            const ::rtl::OUString & _rComponentName,
            ::cppu::ComponentInstantiation _pCreateFunction,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString > & _rServiceNames,
            rtl_ModuleCount* _p
        );

//==========================================================================
class OModuleRegistration
{
    static  ::com::sun::star::uno::Sequence< ::rtl::OUString >*
        s_pImplementationNames;
    static  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > >*
        s_pSupportedServices;
    static  ::com::sun::star::uno::Sequence< sal_Int64 >*
        s_pCreationFunctionPointers;
    static  ::com::sun::star::uno::Sequence< sal_Int64 >*
        s_pFactoryFunctionPointers;

    // no direct instantiation, only static members/methods
    OModuleRegistration() { }

public:
    /** register a component implementing a service with the given data.
        @param      _rImplementationName        the implementation name of the component
        @param      _rServiceNames              the services the component supports
        @param      _pCreateFunction            a function for creating an instance of the component
        @param      _pFactoryFunction           a function for creating a factory for that component
        @see revokeComponent
    */
    static void registerComponent(
        const ::rtl::OUString& _rImplementationName,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
        ::cppu::ComponentInstantiation _pCreateFunction,
        FactoryInstantiation _pFactoryFunction);

    /** revoke the registration for the specified component
        @param      _rImplementationName        the implementation name of the component
    */
    static void revokeComponent(
        const ::rtl::OUString& _rImplementationName);

    /** writes the registration information of all components which are currently registered into the specified registry.
        Usually used from within component_writeInfo.
        @param      _rxServiceManager   the service manager
        @param      _rRootKey           the registry key under which the information will be stored
        @return                         sal_True if the registration of all implementations was successfull, sal_False otherwise
    */
    static sal_Bool writeComponentInfos(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager,
        const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rRootKey);

    /** creates a Factory for the component with the given implementation name. Usually used from within component_getFactory.
        @param      _rxServiceManager       a pointer to an XMultiServiceFactory interface as got in component_getFactory
        @param      _pImplementationName    the implementation name of the component
        @return                             the XInterface access to a factory for the component
    */
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getComponentFactory(
        const ::rtl::OUString& _rImplementationName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager
        );
};

//==========================================================================
template <class TYPE>
class OMultiInstanceAutoRegistration
{
public:
    /** assumed that the template argument has the three methods<BR>
        <code>static ::rtl::OUString getImplementationName_Static()</code><BR>
        <code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><BR>
        and<BR>
        <code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code><BR>
        the instantiation of this object will automatically register the class via <code>OModuleRegistration::registerComponent</code>.
        The factory creation function used is <code>::cppu::createSingleFactory</code>.<BR>
        @see OOneInstanceAutoRegistration
    */
    OMultiInstanceAutoRegistration();
    ~OMultiInstanceAutoRegistration();
};

template <class TYPE>
OMultiInstanceAutoRegistration<TYPE>::OMultiInstanceAutoRegistration()
{
    OModuleRegistration::registerComponent(
        TYPE::getImplementationName_Static(),
        TYPE::getSupportedServiceNames_Static(),
        TYPE::Create,
        ::cppu::createSingleFactory
        );
}

template <class TYPE>
OMultiInstanceAutoRegistration<TYPE>::~OMultiInstanceAutoRegistration()
{
    OModuleRegistration::revokeComponent(TYPE::getImplementationName_Static());
}

//==========================================================================
template <class TYPE>
class OOneInstanceAutoRegistration
{
public:
    /** provided that the template argument has three methods<BR>
        <code>static ::rtl::OUString getImplementationName_Static()</code><BR>
        <code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><BR>
        and<BR>
        <code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code><BR>
        the instantiation of this object will automatically register the class via <code>OModuleRegistration::registerComponent</code>.
        The factory creation function used is <code>::cppu::createSingleFactory</code>.<BR>
        @see OMultiInstanceAutoRegistration
    */
    OOneInstanceAutoRegistration();
    ~OOneInstanceAutoRegistration();
};

template <class TYPE>
OOneInstanceAutoRegistration<TYPE>::OOneInstanceAutoRegistration()
{
    OModuleRegistration::registerComponent(
        TYPE::getImplementationName_Static(),
        TYPE::getSupportedServiceNames_Static(),
        TYPE::Create,
        ::cppu::createOneInstanceFactory
        );
}

template <class TYPE>
OOneInstanceAutoRegistration<TYPE>::~OOneInstanceAutoRegistration()
{
    OModuleRegistration::revokeComponent(TYPE::getImplementationName_Static());
}

