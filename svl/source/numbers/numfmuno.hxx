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
                                        css::util::XNumberFormatter2,
                                        css::lang::XServiceInfo>
{
private:
    ::rtl::Reference< SvNumberFormatsSupplierObj >  xSupplier;
    mutable ::comphelper::SharedMutex               m_aMutex;

public:
                        SvNumberFormatterServiceObj();
    virtual             ~SvNumberFormatterServiceObj();

    // XNumberFormatter
    virtual void SAL_CALL attachNumberFormatsSupplier(
                            const css::uno::Reference< css::util::XNumberFormatsSupplier >& xSupplier )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::util::XNumberFormatsSupplier >
                            SAL_CALL getNumberFormatsSupplier()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL detectNumberFormat( sal_Int32 nKey, const OUString& aString )
                            throw(css::util::NotNumericException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL convertStringToNumber( sal_Int32 nKey, const OUString& aString )
                            throw(css::util::NotNumericException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL convertNumberToString( sal_Int32 nKey, double fValue )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::util::Color SAL_CALL queryColorForNumber( sal_Int32 nKey,
                            double fValue, css::util::Color aDefaultColor )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL formatString( sal_Int32 nKey, const OUString& aString )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::util::Color SAL_CALL queryColorForString( sal_Int32 nKey,
                            const OUString& aString,
                                css::util::Color aDefaultColor )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getInputString( sal_Int32 nKey, double fValue )
                            throw(css::uno::RuntimeException, std::exception) override;

    // XNumberFormatPreviewer
    virtual OUString SAL_CALL convertNumberToPreviewString(
                            const OUString& aFormat, double fValue,
                            const css::lang::Locale& nLocale, sal_Bool bAllowEnglish )
                                throw(css::util::MalformedNumberFormatException,
                                        css::uno::RuntimeException, std::exception) override;
    virtual css::util::Color SAL_CALL queryPreviewColorForNumber(
                            const OUString& aFormat, double fValue,
                            const css::lang::Locale& nLocale, sal_Bool bAllowEnglish,
                            css::util::Color aDefaultColor )
                                throw(css::util::MalformedNumberFormatException,
                                        css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                            throw(css::uno::RuntimeException, std::exception) override;
};


class SvNumberFormatsObj : public cppu::WeakImplHelper<
                                        css::util::XNumberFormats,
                                        css::util::XNumberFormatTypes,
                                        css::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj&         rSupplier;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
                    SvNumberFormatsObj(SvNumberFormatsSupplierObj& pParent, ::comphelper::SharedMutex& _rMutex);
    virtual         ~SvNumberFormatsObj();


    // XNumberFormats
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
                            getByKey( sal_Int32 nKey ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL queryKeys( sal_Int16 nType,
                            const css::lang::Locale& nLocale, sal_Bool bCreate )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL queryKey( const OUString& aFormat,
                            const css::lang::Locale& nLocale, sal_Bool bScan )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL addNew( const OUString& aFormat,
                            const css::lang::Locale& nLocale )
                                throw(css::util::MalformedNumberFormatException,
                                        css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL addNewConverted( const OUString& aFormat,
                            const css::lang::Locale& nLocale,
                            const css::lang::Locale& nNewLocale )
                                throw(css::util::MalformedNumberFormatException,
                                        css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByKey( sal_Int32 nKey ) throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL generateFormat( sal_Int32 nBaseKey,
                            const css::lang::Locale& nLocale, sal_Bool bThousands,
                            sal_Bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading )
                                throw(css::uno::RuntimeException, std::exception) override;

    // XNumberFormatTypes
    virtual sal_Int32 SAL_CALL getStandardIndex( const css::lang::Locale& nLocale )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getStandardFormat( sal_Int16 nType,
                            const css::lang::Locale& nLocale )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getFormatIndex( sal_Int16 nIndex,
                            const css::lang::Locale& nLocale )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getFormatForLocale( sal_Int32 nKey,
                            const css::lang::Locale& nLocale )
                                throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                            throw(css::uno::RuntimeException, std::exception) override;

private:
    SvNumberFormatsObj();   // never implemented
};


class SvNumberFormatObj : public cppu::WeakImplHelper<
                                        css::beans::XPropertySet,
                                        css::beans::XPropertyAccess,
                                        css::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj&         rSupplier;
    sal_uLong                               nKey;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
    SvNumberFormatObj( SvNumberFormatsSupplierObj& rParent, sal_uLong nK, const ::comphelper::SharedMutex& _rMutex );
    virtual         ~SvNumberFormatObj();

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getPropertyValues() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValues( const css::uno::Sequence<
                                css::beans::PropertyValue >& aProps )
                                    throw(css::beans::UnknownPropertyException,
                                            css::beans::PropertyVetoException,
                                            css::lang::IllegalArgumentException,
                                            css::lang::WrappedTargetException,
                                            css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};


class SvNumberFormatSettingsObj : public cppu::WeakImplHelper<
                                        css::beans::XPropertySet,
                                        css::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj&         rSupplier;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
                    SvNumberFormatSettingsObj( SvNumberFormatsSupplierObj& rParent, const ::comphelper::SharedMutex& _rMutex);
    virtual         ~SvNumberFormatSettingsObj();


    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
