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
    virtual             ~SvNumberFormatterServiceObj() override;

    // XNumberFormatter
    virtual void SAL_CALL attachNumberFormatsSupplier(
                            const css::uno::Reference< css::util::XNumberFormatsSupplier >& xSupplier ) override;
    virtual css::uno::Reference< css::util::XNumberFormatsSupplier >
                            SAL_CALL getNumberFormatsSupplier() override;
    virtual sal_Int32 SAL_CALL detectNumberFormat( sal_Int32 nKey, const OUString& aString ) override;
    virtual double SAL_CALL convertStringToNumber( sal_Int32 nKey, const OUString& aString ) override;
    virtual OUString SAL_CALL convertNumberToString( sal_Int32 nKey, double fValue ) override;
    virtual sal_Int32 SAL_CALL queryColorForNumber( sal_Int32 nKey,
                            double fValue, sal_Int32 aDefaultColor ) override;
    virtual OUString SAL_CALL formatString( sal_Int32 nKey, const OUString& aString ) override;
    virtual sal_Int32 SAL_CALL queryColorForString( sal_Int32 nKey,
                                const OUString& aString,
                                sal_Int32 aDefaultColor ) override;
    virtual OUString SAL_CALL getInputString( sal_Int32 nKey, double fValue ) override;

    // XNumberFormatPreviewer
    virtual OUString SAL_CALL convertNumberToPreviewString(
                            const OUString& aFormat, double fValue,
                            const css::lang::Locale& nLocale, sal_Bool bAllowEnglish ) override;
    virtual sal_Int32 SAL_CALL queryPreviewColorForNumber(
                            const OUString& aFormat, double fValue,
                            const css::lang::Locale& nLocale, sal_Bool bAllowEnglish,
                            sal_Int32 aDefaultColor ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};


class SvNumberFormatsObj : public cppu::WeakImplHelper<
                                        css::util::XNumberFormats,
                                        css::util::XNumberFormatTypes,
                                        css::lang::XServiceInfo>
{
private:
    rtl::Reference<SvNumberFormatsSupplierObj> m_xSupplier;
    mutable ::comphelper::SharedMutex          m_aMutex;

public:
                    SvNumberFormatsObj(SvNumberFormatsSupplierObj& pParent, ::comphelper::SharedMutex const & _rMutex);
    virtual         ~SvNumberFormatsObj() override;


    // XNumberFormats
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
                            getByKey( sal_Int32 nKey ) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL queryKeys( sal_Int16 nType,
                            const css::lang::Locale& nLocale, sal_Bool bCreate ) override;
    virtual sal_Int32 SAL_CALL queryKey( const OUString& aFormat,
                            const css::lang::Locale& nLocale, sal_Bool bScan ) override;
    virtual sal_Int32 SAL_CALL addNew( const OUString& aFormat,
                            const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 SAL_CALL addNewConverted( const OUString& aFormat,
                            const css::lang::Locale& nLocale,
                            const css::lang::Locale& nNewLocale ) override;
    virtual void SAL_CALL removeByKey( sal_Int32 nKey ) override;
    virtual OUString SAL_CALL generateFormat( sal_Int32 nBaseKey,
                            const css::lang::Locale& nLocale, sal_Bool bThousands,
                            sal_Bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading ) override;

    // XNumberFormatTypes
    virtual sal_Int32 SAL_CALL getStandardIndex( const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 SAL_CALL getStandardFormat( sal_Int16 nType,
                            const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 SAL_CALL getFormatIndex( sal_Int16 nIndex,
                            const css::lang::Locale& nLocale ) override;
    virtual sal_Bool SAL_CALL isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType ) override;
    virtual sal_Int32 SAL_CALL getFormatForLocale( sal_Int32 nKey,
                            const css::lang::Locale& nLocale ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};


class SvNumberFormatObj : public cppu::WeakImplHelper<
                                        css::beans::XPropertySet,
                                        css::beans::XPropertyAccess,
                                        css::lang::XServiceInfo>
{
private:
    rtl::Reference<SvNumberFormatsSupplierObj>
                                        m_xSupplier;
    sal_uLong                           nKey;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
    SvNumberFormatObj( SvNumberFormatsSupplierObj& rParent, sal_uLong nK, const ::comphelper::SharedMutex& _rMutex );
    virtual         ~SvNumberFormatObj() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getPropertyValues() override;
    virtual void SAL_CALL   setPropertyValues( const css::uno::Sequence<
                                css::beans::PropertyValue >& aProps ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};


class SvNumberFormatSettingsObj : public cppu::WeakImplHelper<
                                        css::beans::XPropertySet,
                                        css::lang::XServiceInfo>
{
private:
    rtl::Reference<SvNumberFormatsSupplierObj>
                                        m_xSupplier;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
                    SvNumberFormatSettingsObj( SvNumberFormatsSupplierObj& rParent, const ::comphelper::SharedMutex& _rMutex);
    virtual         ~SvNumberFormatSettingsObj() override;


    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
