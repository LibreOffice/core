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

#ifndef INCLUDED_SVL_SOURCE_NUMBERS_NUMFMUNO_HXX
#define INCLUDED_SVL_SOURCE_NUMBERS_NUMFMUNO_HXX

#include <com/sun/star/util/XNumberFormatter2.hpp>
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/sharedmutex.hxx>
#include <rtl/ref.hxx>
#include <tools/solar.h>

class SvNumberFormatsSupplierObj;


/**
 * SvNumberFormatterServiceObj is registered globally as a Service
 */
class SvNumberFormatterServiceObj : public cppu::WeakImplHelper<
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
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                            SAL_CALL getNumberFormatsSupplier()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL detectNumberFormat( sal_Int32 nKey, const OUString& aString )
                            throw(::com::sun::star::util::NotNumericException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL convertStringToNumber( sal_Int32 nKey, const OUString& aString )
                            throw(::com::sun::star::util::NotNumericException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL convertNumberToString( sal_Int32 nKey, double fValue )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::util::Color SAL_CALL queryColorForNumber( sal_Int32 nKey,
                            double fValue, ::com::sun::star::util::Color aDefaultColor )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL formatString( sal_Int32 nKey, const OUString& aString )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::util::Color SAL_CALL queryColorForString( sal_Int32 nKey,
                            const OUString& aString,
                                ::com::sun::star::util::Color aDefaultColor )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getInputString( sal_Int32 nKey, double fValue )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNumberFormatPreviewer
    virtual OUString SAL_CALL convertNumberToPreviewString(
                            const OUString& aFormat, double fValue,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bAllowEnglish )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::util::Color SAL_CALL queryPreviewColorForNumber(
                            const OUString& aFormat, double fValue,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bAllowEnglish,
                            ::com::sun::star::util::Color aDefaultColor )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};


class SvNumberFormatsObj : public cppu::WeakImplHelper<
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
                            getByKey( sal_Int32 nKey ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL queryKeys( sal_Int16 nType,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bCreate )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL queryKey( const OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bScan )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL addNew( const OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL addNewConverted( const OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale,
                            const ::com::sun::star::lang::Locale& nNewLocale )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByKey( sal_Int32 nKey ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL generateFormat( sal_Int32 nBaseKey,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bThousands,
                            sal_Bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNumberFormatTypes
    virtual sal_Int32 SAL_CALL getStandardIndex( const ::com::sun::star::lang::Locale& nLocale )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getStandardFormat( sal_Int16 nType,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getFormatIndex( sal_Int16 nIndex,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getFormatForLocale( sal_Int32 nKey,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    SvNumberFormatsObj();   // never implemented
};


class SvNumberFormatObj : public cppu::WeakImplHelper<
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
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getPropertyValues() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aProps )
                                    throw(::com::sun::star::beans::UnknownPropertyException,
                                            ::com::sun::star::beans::PropertyVetoException,
                                            ::com::sun::star::lang::IllegalArgumentException,
                                            ::com::sun::star::lang::WrappedTargetException,
                                            ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};


class SvNumberFormatSettingsObj : public cppu::WeakImplHelper<
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
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
