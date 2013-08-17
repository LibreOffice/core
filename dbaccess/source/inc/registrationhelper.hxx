/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _REGISTRATIONHELPER_INCLUDED_INDIRECTLY_
#error "don't include this file directly! use dbu_reghelper.hxx instead!"
#endif

typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > (SAL_CALL *FactoryInstantiation)
        (
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rServiceManager,
            const OUString & _rComponentName,
            ::cppu::ComponentInstantiation _pCreateFunction,
            const ::com::sun::star::uno::Sequence< OUString > & _rServiceNames,
            rtl_ModuleCount*
        );

class OModuleRegistration
{
    static  ::com::sun::star::uno::Sequence< OUString >*
        s_pImplementationNames;
    static  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > >*
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
        const OUString& _rImplementationName,
        const ::com::sun::star::uno::Sequence< OUString >& _rServiceNames,
        ::cppu::ComponentInstantiation _pCreateFunction,
        FactoryInstantiation _pFactoryFunction);

    /** revoke the registration for the specified component
        @param      _rImplementationName        the implementation name of the component
    */
    static void revokeComponent(
        const OUString& _rImplementationName);

    /** creates a Factory for the component with the given implementation name. Usually used from within component_getFactory.
        @param      _rxServiceManager       a pointer to an XMultiServiceFactory interface as got in component_getFactory
        @param      _pImplementationName    the implementation name of the component
        @return                             the XInterface access to a factory for the component
    */
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getComponentFactory(
        const OUString& _rImplementationName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager
        );
};

template <class TYPE>
class OMultiInstanceAutoRegistration
{
public:
    /** assumed that the template argument has the three methods<BR>
        <code>static OUString getImplementationName_Static()</code><BR>
        <code>static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static()</code><BR>
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

template <class TYPE>
class OOneInstanceAutoRegistration
{
public:
    /** provided that the template argument has three methods<BR>
        <code>static OUString getImplementationName_Static()</code><BR>
        <code>static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static()</code><BR>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
