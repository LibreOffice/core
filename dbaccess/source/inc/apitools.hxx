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

#ifndef INCLUDED_DBACCESS_SOURCE_INC_APITOOLS_HXX
#define INCLUDED_DBACCESS_SOURCE_INC_APITOOLS_HXX

#include <sal/config.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/component.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <strings.hxx>

// OSubComponent - a component which holds a hard ref to its parent
//                 and is been hold itself (by the parent) with a weak ref
class OSubComponent : public ::cppu::OComponentHelper
{
protected:
    // the parent must support the tunnel implementation
    css::uno::Reference< css::uno::XInterface > m_xParent;
    virtual ~OSubComponent() override;

public:
    OSubComponent(::osl::Mutex& _rMutex,
                  const css::uno::Reference< css::uno::XInterface >& _xParent);

// css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

// css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL release() noexcept override;

    operator css::uno::Reference< css::uno::XInterface > () const
        { return static_cast<css::uno::XWeak *>(const_cast<OSubComponent *>(this)); }

};

// helper for implementing the XServiceInfo interface

// (internal - not to be used outside - usually)
#define IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    OUString SAL_CALL classname::getImplementationName(  )   \
    {   \
        return implasciiname; \
    }   \


#define IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    sal_Bool SAL_CALL classname::supportsService( const OUString& _rServiceName ) \
    {   \
        const css::uno::Sequence< OUString > aSupported(getSupportedServiceNames());  \
        for (const OUString& s : aSupported)    \
            if (s == _rServiceName)  \
                return true;    \
    \
        return false;   \
    }   \



#define IMPLEMENT_SERVICE_INFO1(classname, implasciiname, serviceasciiname) \
    IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    css::uno::Sequence< OUString > SAL_CALL classname::getSupportedServiceNames(  )  \
    {   \
        return { serviceasciiname }; \
    }   \


#define IMPLEMENT_SERVICE_INFO2(classname, implasciiname, serviceasciiname1, serviceasciiname2) \
    IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    css::uno::Sequence< OUString > SAL_CALL classname::getSupportedServiceNames(  )  \
    {   \
        return { serviceasciiname1, serviceasciiname2 };    \
    }



#define IMPLEMENT_SERVICE_INFO3(classname, implasciiname, serviceasciiname1, serviceasciiname2, serviceasciiname3)  \
    IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    css::uno::Sequence< OUString > SAL_CALL classname::getSupportedServiceNames(  )  \
    {   \
        return { serviceasciiname1, serviceasciiname2, serviceasciiname3 };  \
    }   \


// XTypeProvider helpers

#define IMPLEMENT_GETTYPES2( classname, baseclass1, baseclass2 )    \
    css::uno::Sequence< css::uno::Type > classname::getTypes()    \
    {   \
        return  ::comphelper::concatSequences(  \
            baseclass1::getTypes( ),    \
            baseclass2::getTypes( ) \
        );  \
    }

#define IMPLEMENT_GETTYPES3( classname, baseclass1, baseclass2, baseclass3 )    \
    css::uno::Sequence< css::uno::Type > classname::getTypes()    \
    {   \
        return  ::comphelper::concatSequences(  \
            baseclass1::getTypes( ),    \
            baseclass2::getTypes( ),    \
            baseclass3::getTypes( ) \
        );  \
    }

// helper for declaring/implementing classes based on the OPropertyContainer and an OPropertyArrayUsageHelper
#define DECLARE_PROPERTYCONTAINER_DEFAULTS( )   \
    virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override; \
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override; \
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override

#define IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2( classname , baseclass1)  \
    css::uno::Reference< css::beans::XPropertySetInfo >  SAL_CALL classname::getPropertySetInfo()  \
    {   \
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );    \
        return xInfo;   \
    }   \
    ::cppu::IPropertyArrayHelper& classname::getInfoHelper()    \
    {   \
    return *baseclass1::getArrayHelper();   \
    }   \
    ::cppu::IPropertyArrayHelper* classname::createArrayHelper( ) const \
    {   \
        css::uno::Sequence< css::beans::Property > aProps;    \
        describeProperties(aProps); \
        return new ::cppu::OPropertyArrayHelper(aProps);    \
    }
#define IMPLEMENT_PROPERTYCONTAINER_DEFAULTS( classname )   \
    IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2( classname, classname )

// helper for implementing the createArrayHelper
#define BEGIN_PROPERTY_SEQUENCE(count)  \
    css::uno::Sequence< css::beans::Property> aDescriptor(count); \
    css::beans::Property* pDesc = aDescriptor.getArray();                  \
    sal_Int32 nPos = 0;                                             \

#define BEGIN_PROPERTY_HELPER(count)                            \
    BEGIN_PROPERTY_SEQUENCE(count)

#define DECL_PROP_IMPL(varname, cpputype) \
    pDesc[nPos++] = css::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, cpputype,

#define DECL_PROP0(varname, type)   \
    DECL_PROP_IMPL(varname, cppu::UnoType<type>::get()) 0)

#define DECL_PROP0_BOOL(varname)    \
    DECL_PROP_IMPL(varname, cppu::UnoType<bool>::get()) 0)

#define DECL_PROP1(varname, type, attrib1)  \
    DECL_PROP_IMPL(varname, cppu::UnoType<type>::get()) css::beans::PropertyAttribute::attrib1)

#define DECL_PROP1_BOOL(varname, attrib1)   \
    DECL_PROP_IMPL(varname, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::attrib1)

#define DECL_PROP2_IFACE(varname, iface, attrib1, attrib2)  \
    DECL_PROP_IMPL(varname, cppu::UnoType<iface>::get()) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2)

#define DECL_PROP2(varname, type, attrib1, attrib2) \
    DECL_PROP_IMPL(varname, cppu::UnoType<type>::get()) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2)

#define DECL_PROP3(varname, type, attrib1, attrib2, attrib3)    \
    DECL_PROP_IMPL(varname, cppu::UnoType<type>::get()) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3)

#define END_PROPERTY_SEQUENCE()                             \
    OSL_ENSURE(nPos == aDescriptor.getLength(), "forgot to adjust the count ?");    \

#define END_PROPERTY_HELPER()                               \
    END_PROPERTY_SEQUENCE() \
    return new ::cppu::OPropertyArrayHelper(aDescriptor);

#endif // INCLUDED_DBACCESS_SOURCE_INC_APITOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
