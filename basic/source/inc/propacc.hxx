/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX_PROPBAG_HXX
#define _SFX_PROPBAG_HXX

#include <svl/svarray.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

typedef ::com::sun::star::beans::PropertyValue* SbPropertyValuePtr;
SV_DECL_PTRARR( SbPropertyValueArr_Impl, SbPropertyValuePtr, 4 )

typedef ::cppu::WeakImplHelper2< ::com::sun::star::beans::XPropertySet,
                                 ::com::sun::star::beans::XPropertyAccess > SbPropertyValuesHelper;


//==========================================================================

class SbPropertyValues:     public SbPropertyValuesHelper
{
    SbPropertyValueArr_Impl _aPropVals;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > _xInfo;

private:
    size_t GetIndex_Impl( const ::rtl::OUString &rPropName ) const;

public:
                            SbPropertyValues();
    virtual                 ~SbPropertyValues();

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   setPropertyValue(
                                const ::rtl::OUString& aPropertyName,
                                const ::com::sun::star::uno::Any& aValue)
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                ::com::sun::star::beans::PropertyVetoException,
                                ::com::sun::star::lang::IllegalArgumentException,
                                ::com::sun::star::lang::WrappedTargetException,
                                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(  ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
                                throw ();
    virtual void SAL_CALL   removePropertyChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
                                throw ();
    virtual void SAL_CALL   addVetoableChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
                                throw ();
    virtual void SAL_CALL   removeVetoableChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
                                throw ();

    // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues(void) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValues(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& PropertyValues_) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

//==========================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo > SbPropertySetInfoHelper;

// AB 20.3.2000 Help Class for XPropertySetInfo implementation
class PropertySetInfoImpl
{
    friend class SbPropertySetInfo;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > _aProps;

    sal_Int32 GetIndex_Impl( const ::rtl::OUString &rPropName ) const;

public:
    PropertySetInfoImpl();
    PropertySetInfoImpl( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& rProps );

    // XPropertySetInfo
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(void) throw ();
    ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( ::com::sun::star::uno::RuntimeException );
    sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
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
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
        throw( ::com::sun::star::uno::RuntimeException );
};

//=========================================================================

class StarBASIC;
class SbxArray;

void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
