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

#ifndef SC_APPLUNO_HXX
#define SC_APPLUNO_HXX

#ifndef _SFX_ITEMPROP_HXX
#include <bf_svtools/itemprop.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XRECENTFUNCTIONS_HPP_
#include <com/sun/star/sheet/XRecentFunctions.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XFUNCTIONDESCRIPTIONS_HPP_
#include <com/sun/star/sheet/XFunctionDescriptions.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
namespace binfilter {

class ScFunctionDescriptionObj;



com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL
    ScSpreadsheetSettings_CreateInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory>& rSMgr );
com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL
    ScRecentFunctionsObj_CreateInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory>& rSMgr );
com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL
    ScFunctionListObj_CreateInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory>& rSMgr );


class ScSpreadsheetSettings : public cppu::WeakImplHelper2<
                                        ::com::sun::star::beans::XPropertySet,
                                        ::com::sun::star::lang::XServiceInfo>
{
private:
    SfxItemPropertySet		aPropSet;

public:
                            ScSpreadsheetSettings();
    virtual					~ScSpreadsheetSettings();

    static ::rtl::OUString	getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScRecentFunctionsObj : public cppu::WeakImplHelper2<
                                        ::com::sun::star::sheet::XRecentFunctions,
                                        ::com::sun::star::lang::XServiceInfo>
{
public:
                            ScRecentFunctionsObj();
    virtual					~ScRecentFunctionsObj();

    static ::rtl::OUString	getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

                            // XRecentFunctions
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getRecentFunctionIds()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRecentFunctionIds( const ::com::sun::star::uno::Sequence<
                                                sal_Int32 >& aRecentFunctionIds )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getMaxRecentFunctions()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScFunctionListObj : public cppu::WeakImplHelper4<
                                        ::com::sun::star::sheet::XFunctionDescriptions,
                                        ::com::sun::star::container::XEnumerationAccess,
                                        ::com::sun::star::container::XNameAccess,
                                        ::com::sun::star::lang::XServiceInfo>
{
public:
                            ScFunctionListObj();
    virtual					~ScFunctionListObj();

    static ::rtl::OUString	getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

                            // XFunctionDescriptions
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getById( sal_Int32 nId )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                ::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



} //namespace binfilter
#endif

