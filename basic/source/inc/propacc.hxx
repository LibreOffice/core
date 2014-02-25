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
#ifndef INCLUDED_BASIC_SOURCE_INC_PROPACC_HXX
#define INCLUDED_BASIC_SOURCE_INC_PROPACC_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <boost/ptr_container/ptr_vector.hpp>

typedef ::boost::ptr_vector< ::com::sun::star::beans::PropertyValue >
    SbPropertyValueArr_Impl;

typedef ::cppu::WeakImplHelper2< ::com::sun::star::beans::XPropertySet,
                                 ::com::sun::star::beans::XPropertyAccess > SbPropertyValuesHelper;




class SbPropertyValues:     public SbPropertyValuesHelper
{
    SbPropertyValueArr_Impl m_aPropVals;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > m_xInfo;

private:
    size_t GetIndex_Impl( const OUString &rPropName ) const;

public:
                            SbPropertyValues();
    virtual                 ~SbPropertyValues();

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(void) throw( ::com::sun::star::uno::RuntimeException, std::exception );
    virtual void SAL_CALL   setPropertyValue(
                                const OUString& aPropertyName,
                                const ::com::sun::star::uno::Any& aValue)
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                ::com::sun::star::beans::PropertyVetoException,
                                ::com::sun::star::lang::IllegalArgumentException,
                                ::com::sun::star::lang::WrappedTargetException,
                                ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw(  ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   addPropertyChangeListener(
                                const OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
                                throw (std::exception);
    virtual void SAL_CALL   removePropertyChangeListener(
                                const OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
                                throw (std::exception);
    virtual void SAL_CALL   addVetoableChangeListener(
                                const OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
                                throw (std::exception);
    virtual void SAL_CALL   removeVetoableChangeListener(
                                const OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
                                throw (std::exception);

    // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues(void) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertyValues(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& PropertyValues_) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
};



typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo > SbPropertySetInfoHelper;

// AB 20.3.2000 Help Class for XPropertySetInfo implementation
class PropertySetInfoImpl
{
    friend class SbPropertySetInfo;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > _aProps;

    sal_Int32 GetIndex_Impl( const OUString &rPropName ) const;

public:
    PropertySetInfoImpl();
    PropertySetInfoImpl( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& rProps );

    // XPropertySetInfo
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(void) throw ();
    ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const OUString& Name)
        throw( ::com::sun::star::uno::RuntimeException );
    sal_Bool SAL_CALL hasPropertyByName(const OUString& Name)
        throw ( ::com::sun::star::uno::RuntimeException );
};

class SbPropertySetInfo:    public SbPropertySetInfoHelper
{
    PropertySetInfoImpl aImpl;

public:
                            SbPropertySetInfo( const SbPropertyValueArr_Impl &rPropVals );
    virtual                 ~SbPropertySetInfo();

    // XPropertySetInfo
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(void)
        throw( ::com::sun::star::uno::RuntimeException, std::exception );
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const OUString& Name)
        throw( ::com::sun::star::uno::RuntimeException, std::exception );
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& Name)
        throw( ::com::sun::star::uno::RuntimeException, std::exception );
};



class StarBASIC;
class SbxArray;

void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
