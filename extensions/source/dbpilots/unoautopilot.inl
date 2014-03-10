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

// no include protection
// this file is included from unoautopilot.hxx directly


//= OUnoAutoPilot

template <class TYPE, class SERVICEINFO>
OUnoAutoPilot<TYPE, SERVICEINFO>::OUnoAutoPilot(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB)
    :OUnoAutoPilot_Base(_rxORB)
{
}


template <class TYPE, class SERVICEINFO>
::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}


template <class TYPE, class SERVICEINFO>
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
{
    return *(new OUnoAutoPilot<TYPE, SERVICEINFO>( comphelper::getComponentContext(_rxFactory) ));
}


template <class TYPE, class SERVICEINFO>
::rtl::OUString SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getImplementationName_Static();
}


template <class TYPE, class SERVICEINFO>
::rtl::OUString OUnoAutoPilot<TYPE, SERVICEINFO>::getImplementationName_Static() throw(::com::sun::star::uno::RuntimeException)
{
    return SERVICEINFO().getImplementationName();
}


template <class TYPE, class SERVICEINFO>
::comphelper::StringSequence SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}


template <class TYPE, class SERVICEINFO>
::comphelper::StringSequence OUnoAutoPilot<TYPE, SERVICEINFO>::getSupportedServiceNames_Static() throw(::com::sun::star::uno::RuntimeException)
{
    return SERVICEINFO().getServiceNames();
}


template <class TYPE, class SERVICEINFO>
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


template <class TYPE, class SERVICEINFO>
::cppu::IPropertyArrayHelper& OUnoAutoPilot<TYPE, SERVICEINFO>::getInfoHelper()
{
    return *const_cast<OUnoAutoPilot*>(this)->getArrayHelper();
}


template <class TYPE, class SERVICEINFO>
::cppu::IPropertyArrayHelper* OUnoAutoPilot<TYPE, SERVICEINFO>::createArrayHelper( ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}


template <class TYPE, class SERVICEINFO>
Dialog* OUnoAutoPilot<TYPE, SERVICEINFO>::createDialog(Window* _pParent)
{
    return new TYPE(_pParent, m_xObjectModel, m_aContext);
}


template <class TYPE, class SERVICEINFO>
void OUnoAutoPilot<TYPE, SERVICEINFO>::implInitialize(const com::sun::star::uno::Any& _rValue)
{
    ::com::sun::star::beans::PropertyValue aArgument;
    if (_rValue >>= aArgument)
        if (0 == aArgument.Name.compareToAscii("ObjectModel"))
        {
            aArgument.Value >>= m_xObjectModel;
            return;
        }

    OUnoAutoPilot_Base::implInitialize(_rValue);
}


