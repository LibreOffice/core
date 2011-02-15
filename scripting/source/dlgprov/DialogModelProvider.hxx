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
#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase4.hxx"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"

/// anonymous implementation namespace
namespace dlgprov{

namespace css = ::com::sun::star;

class DialogModelProvider:
    public ::cppu::WeakImplHelper4<
        css::lang::XInitialization,
        css::container::XNameContainer,
        css::beans::XPropertySet,
        css::lang::XServiceInfo>
{
public:
    explicit DialogModelProvider(css::uno::Reference< css::uno::XComponentContext > const & context);
private:
    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception);

    // ::com::sun::star::container::XElementAccess:
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);

    // ::com::sun::star::container::XNameAccess:
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const ::rtl::OUString & aName) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasByName(const ::rtl::OUString & aName) throw (css::uno::RuntimeException);

    // ::com::sun::star::container::XNameReplace:
    virtual void SAL_CALL replaceByName(const ::rtl::OUString & aName, const ::com::sun::star::uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException);

    // ::com::sun::star::container::XNameContainer:
    virtual void SAL_CALL insertByName(const ::rtl::OUString & aName, const ::com::sun::star::uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException);
    virtual void SAL_CALL removeByName(const ::rtl::OUString & Name) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private:
    DialogModelProvider(const DialogModelProvider &); // not defined
    DialogModelProvider& operator=(const DialogModelProvider &); // not defined

    // destructor is private and will be called indirectly by the release call    virtual ~DialogModelProvider() {}

    css::uno::Reference< css::uno::XComponentContext >      m_xContext;
    css::uno::Reference< css::container::XNameContainer>    m_xDialogModel;
    css::uno::Reference< css::beans::XPropertySet>          m_xDialogModelProp;
};
} // closing anonymous implementation namespace
