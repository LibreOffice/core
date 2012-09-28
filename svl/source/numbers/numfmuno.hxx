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

#ifndef _NUMFMUNO_HXX
#define _NUMFMUNO_HXX

#include <com/sun/star/util/XNumberFormatter2.hpp>
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/sharedmutex.hxx>
#include <rtl/ref.hxx>
#include <tools/solar.h>

class SvNumberFormatsSupplierObj;


//  SvNumberFormatterServiceObj wird global als Service angemeldet

class SvNumberFormatterServiceObj : public cppu::WeakImplHelper2<
                                        com::sun::star::util::XNumberFormatter2,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    ::rtl::Reference< SvNumberFormatsSupplierObj >  xSupplier;
    mutable ::comphelper::SharedMutex               m_aMutex;

public:
                        SvNumberFormatterServiceObj();
    virtual             ~SvNumberFormatterServiceObj();

                        // XNumberFormatter
    virtual void SAL_CALL attachNumberFormatsSupplier(
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XNumberFormatsSupplier >& xSupplier )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                            SAL_CALL getNumberFormatsSupplier()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL detectNumberFormat( sal_Int32 nKey, const ::rtl::OUString& aString )
                            throw(::com::sun::star::util::NotNumericException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL convertStringToNumber( sal_Int32 nKey, const ::rtl::OUString& aString )
                            throw(::com::sun::star::util::NotNumericException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL convertNumberToString( sal_Int32 nKey, double fValue )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::Color SAL_CALL queryColorForNumber( sal_Int32 nKey,
                            double fValue, ::com::sun::star::util::Color aDefaultColor )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL formatString( sal_Int32 nKey, const ::rtl::OUString& aString )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::Color SAL_CALL queryColorForString( sal_Int32 nKey,
                            const ::rtl::OUString& aString,
                                ::com::sun::star::util::Color aDefaultColor )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getInputString( sal_Int32 nKey, double fValue )
                            throw(::com::sun::star::uno::RuntimeException);

                        // XNumberFormatPreviewer
    virtual ::rtl::OUString SAL_CALL convertNumberToPreviewString(
                            const ::rtl::OUString& aFormat, double fValue,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bAllowEnglish )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::Color SAL_CALL queryPreviewColorForNumber(
                            const ::rtl::OUString& aFormat, double fValue,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bAllowEnglish,
                            ::com::sun::star::util::Color aDefaultColor )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);

                        // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                            throw(::com::sun::star::uno::RuntimeException);
};


class SvNumberFormatsObj : public cppu::WeakImplHelper3<
                                        com::sun::star::util::XNumberFormats,
                                        com::sun::star::util::XNumberFormatTypes,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj&         rSupplier;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
                    SvNumberFormatsObj(SvNumberFormatsSupplierObj& pParent, ::comphelper::SharedMutex& _rMutex);
    virtual         ~SvNumberFormatsObj();


                        // XNumberFormats
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
                            getByKey( sal_Int32 nKey ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL queryKeys( sal_Int16 nType,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bCreate )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL queryKey( const ::rtl::OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bScan )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL addNew( const ::rtl::OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL addNewConverted( const ::rtl::OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale,
                            const ::com::sun::star::lang::Locale& nNewLocale )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByKey( sal_Int32 nKey ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL generateFormat( sal_Int32 nBaseKey,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bThousands,
                            sal_Bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading )
                                throw(::com::sun::star::uno::RuntimeException);

                        // XNumberFormatTypes
    virtual sal_Int32 SAL_CALL getStandardIndex( const ::com::sun::star::lang::Locale& nLocale )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStandardFormat( sal_Int16 nType,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getFormatIndex( sal_Int16 nIndex,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getFormatForLocale( sal_Int32 nKey,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException);

                        // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                            throw(::com::sun::star::uno::RuntimeException);

private:
    SvNumberFormatsObj();   // never implemented
};


class SvNumberFormatObj : public cppu::WeakImplHelper3<
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::beans::XPropertyAccess,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj&         rSupplier;
    sal_uLong                               nKey;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
    SvNumberFormatObj( SvNumberFormatsSupplierObj& rParent, sal_uLong nK, const ::comphelper::SharedMutex& _rMutex );
    virtual         ~SvNumberFormatObj();

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getPropertyValues() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aProps )
                                    throw(::com::sun::star::beans::UnknownPropertyException,
                                            ::com::sun::star::beans::PropertyVetoException,
                                            ::com::sun::star::lang::IllegalArgumentException,
                                            ::com::sun::star::lang::WrappedTargetException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class SvNumberFormatSettingsObj : public cppu::WeakImplHelper2<
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj&         rSupplier;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
                    SvNumberFormatSettingsObj( SvNumberFormatsSupplierObj& rParent, const ::comphelper::SharedMutex& _rMutex);
    virtual         ~SvNumberFormatSettingsObj();


                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
