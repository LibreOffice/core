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
#ifndef _SFX_OBJUNO_HXX
#define _SFX_OBJUNO_HXX

#include <com/sun/star/document/XDocumentInfo.hpp>
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include <com/sun/star/io/IOException.hpp>

#include <svl/itemprop.hxx>
#include <cppuhelper/implbase10.hxx>

#include "sfxuno.hxx"


// this is now just a wrapper around a XDocumentProperties instance

class SAL_DLLPRIVATE SfxDocumentInfoObject: public ::cppu::WeakImplHelper10<
        ::com::sun::star::document::XDocumentInfo,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::beans::XFastPropertySet,
        ::com::sun::star::beans::XPropertyAccess,
        ::com::sun::star::beans::XPropertyContainer,
        ::com::sun::star::document::XDocumentPropertiesSupplier,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::util::XCloneable>
{
protected:
    struct SfxDocumentInfoObject_Impl* _pImp;

public:
    SfxDocumentInfoObject();
    ~SfxDocumentInfoObject();

    // XComponent
    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw( ::com::sun::star::uno::RuntimeException );

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::beans::PropertyVetoException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString& aPropertyName) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);

    // ::com::sun::star::beans::XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::beans::PropertyVetoException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);

    // ::com::sun::star::beans::XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps ) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::beans::XPropertyContainer
    virtual void SAL_CALL addProperty( const ::rtl::OUString& Name, ::sal_Int16 Attributes, const ::com::sun::star::uno::Any& DefaultValue ) throw (::com::sun::star::beans::PropertyExistException, ::com::sun::star::beans::IllegalTypeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeProperty( const ::rtl::OUString& Name ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::NotRemoveableException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::document::XDocumentInfo
    virtual sal_Int16 SAL_CALL getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldValue(sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::document::XDocumentPropertiesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties >
        SAL_CALL getDocumentProperties()
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifiable
    virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const com::sun::star::uno::Reference< com::sun::star::util::XModifyListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException ) ;
    virtual void SAL_CALL removeModifyListener( const com::sun::star::uno::Reference< com::sun::star::util::XModifyListener > & xListener) throw( ::com::sun::star::uno::RuntimeException ) ;

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments)
        throw (com::sun::star::uno::RuntimeException,
               com::sun::star::uno::Exception);

    // ::com::sun::star::util::XCloneable:
    virtual com::sun::star::uno::Reference<com::sun::star::util::XCloneable> SAL_CALL createClone()
        throw (com::sun::star::uno::RuntimeException);

    const SfxDocumentInfoObject& operator=( const SfxDocumentInfoObject & rOther);
};

class SfxStandaloneDocumentInfoObject: public SfxDocumentInfoObject,
                                       public ::com::sun::star::lang::XServiceInfo,
                                       public ::com::sun::star::document::XStandaloneDocumentInfo
{
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > _xFactory;

public:
    SfxStandaloneDocumentInfoObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual ~SfxStandaloneDocumentInfoObject();

    void Clear();

    // XInterface, XTypeProvider, XServiceInfo
    SFX_DECL_XINTERFACE_XTYPEPROVIDER_XSERVICEINFO

    // ::com::sun::star::document::XDocumentInfo
    virtual sal_Int16 SAL_CALL getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldValue(sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::document::XStandaloneDocumentInfo
    virtual void SAL_CALL loadFromURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL storeIntoURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
