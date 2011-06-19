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

#define NS_BEANS    ::com::sun::star::beans
#define NS_LANG     ::com::sun::star::lang
#define NS_UNO      ::com::sun::star::uno

typedef NS_BEANS::PropertyValue* SbPropertyValuePtr;
SV_DECL_PTRARR( SbPropertyValueArr_Impl, SbPropertyValuePtr, 4, 4 )

typedef ::cppu::WeakImplHelper2< NS_BEANS::XPropertySet,
                                 NS_BEANS::XPropertyAccess > SbPropertyValuesHelper;


//==========================================================================

class SbPropertyValues:     public SbPropertyValuesHelper
{
    SbPropertyValueArr_Impl _aPropVals;
    NS_UNO::Reference< ::com::sun::star::beans::XPropertySetInfo > _xInfo;

private:
    sal_Int32                   GetIndex_Impl( const ::rtl::OUString &rPropName ) const;

public:
                            SbPropertyValues();
    virtual                 ~SbPropertyValues();

    // XPropertySet
    virtual NS_UNO::Reference< NS_BEANS::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(void) throw( NS_UNO::RuntimeException );
    virtual void SAL_CALL   setPropertyValue(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Any& aValue)
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                ::com::sun::star::beans::PropertyVetoException,
                                ::com::sun::star::lang::IllegalArgumentException,
                                ::com::sun::star::lang::WrappedTargetException,
                                ::com::sun::star::uno::RuntimeException);
    virtual NS_UNO::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(  NS_BEANS::UnknownPropertyException,
                NS_LANG::WrappedTargetException,
                NS_UNO::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XPropertyChangeListener >& )
                                throw ();
    virtual void SAL_CALL   removePropertyChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XPropertyChangeListener >& )
                                throw ();
    virtual void SAL_CALL   addVetoableChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XVetoableChangeListener >& )
                                throw ();
    virtual void SAL_CALL   removeVetoableChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XVetoableChangeListener >& )
                                throw ();

    // XPropertyAccess
    virtual NS_UNO::Sequence< NS_BEANS::PropertyValue > SAL_CALL getPropertyValues(void) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValues(const NS_UNO::Sequence< NS_BEANS::PropertyValue >& PropertyValues_) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

//==========================================================================

typedef ::cppu::WeakImplHelper1< NS_BEANS::XPropertySetInfo > SbPropertySetInfoHelper;

// AB 20.3.2000 Help Class for XPropertySetInfo implementation
class PropertySetInfoImpl
{
    friend class SbPropertySetInfo;
    friend class SbPropertyContainer;

    NS_UNO::Sequence< NS_BEANS::Property > _aProps;

    sal_Int32 GetIndex_Impl( const ::rtl::OUString &rPropName ) const;

public:
    PropertySetInfoImpl();
    PropertySetInfoImpl( NS_UNO::Sequence< NS_BEANS::Property >& rProps );

    // XPropertySetInfo
    NS_UNO::Sequence< NS_BEANS::Property > SAL_CALL getProperties(void) throw ();
    NS_BEANS::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
    sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
        throw ( NS_UNO::RuntimeException );
};

class SbPropertySetInfo:    public SbPropertySetInfoHelper
{
    PropertySetInfoImpl aImpl;

public:
                            SbPropertySetInfo();
                            SbPropertySetInfo( const SbPropertyValueArr_Impl &rPropVals );
    virtual                 ~SbPropertySetInfo();

    // XPropertySetInfo
    virtual NS_UNO::Sequence< NS_BEANS::Property > SAL_CALL getProperties(void)
        throw( NS_UNO::RuntimeException );
    virtual NS_BEANS::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
};

//==========================================================================

typedef ::cppu::WeakImplHelper2< NS_BEANS::XPropertySetInfo, NS_BEANS::XPropertyContainer > SbPropertyContainerHelper;

class SbPropertyContainer: public SbPropertyContainerHelper
{
    PropertySetInfoImpl aImpl;

public:
                            SbPropertyContainer();
    virtual                 ~SbPropertyContainer();

    // XPropertyContainer
    virtual void SAL_CALL addProperty(  const ::rtl::OUString& Name,
                                        sal_Int16 Attributes,
                                        const NS_UNO::Any& DefaultValue)
        throw(  NS_BEANS::PropertyExistException, NS_BEANS::IllegalTypeException,
                NS_LANG::IllegalArgumentException, NS_UNO::RuntimeException );
    virtual void SAL_CALL removeProperty(const ::rtl::OUString& Name)
        throw( NS_BEANS::UnknownPropertyException, NS_UNO::RuntimeException );

    // XPropertySetInfo
    virtual NS_UNO::Sequence< NS_BEANS::Property > SAL_CALL getProperties(void) throw();
    virtual NS_BEANS::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );

    // XPropertyAccess
    virtual NS_UNO::Sequence< NS_BEANS::PropertyValue > SAL_CALL getPropertyValues(void);
    virtual void SAL_CALL setPropertyValues(const NS_UNO::Sequence< NS_BEANS::PropertyValue >& PropertyValues_);
};

//=========================================================================

class StarBASIC;
class SbxArray;

void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );


#undef NS_BEANS
#undef NS_LANG
#undef NS_UNO



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
