/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    virtual void attachNumberFormatsSupplier(
                            const css::uno::Reference< css::util::XNumberFormatsSupplier >& xSupplier ) override;
    virtual css::uno::Reference< css::util::XNumberFormatsSupplier >
                            getNumberFormatsSupplier() override;
    virtual sal_Int32 detectNumberFormat( sal_Int32 nKey, const OUString& aString ) override;
    virtual double convertStringToNumber( sal_Int32 nKey, const OUString& aString ) override;
    virtual OUString convertNumberToString( sal_Int32 nKey, double fValue ) override;
    virtual sal_Int32 queryColorForNumber( sal_Int32 nKey,
                            double fValue, sal_Int32 aDefaultColor ) override;
    virtual OUString formatString( sal_Int32 nKey, const OUString& aString ) override;
    virtual sal_Int32 queryColorForString( sal_Int32 nKey,
                                const OUString& aString,
                                sal_Int32 aDefaultColor ) override;
    virtual OUString getInputString( sal_Int32 nKey, double fValue ) override;

    // XNumberFormatPreviewer
    virtual OUString convertNumberToPreviewString(
                            const OUString& aFormat, double fValue,
                            const css::lang::Locale& nLocale, bool bAllowEnglish ) override;
    virtual sal_Int32 queryPreviewColorForNumber(
                            const OUString& aFormat, double fValue,
                            const css::lang::Locale& nLocale, bool bAllowEnglish,
                            sal_Int32 aDefaultColor ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
                    SvNumberFormatsObj(SvNumberFormatsSupplierObj& pParent, ::comphelper::SharedMutex _aMutex);
    virtual         ~SvNumberFormatsObj() override;


    // XNumberFormats
    virtual css::uno::Reference< css::beans::XPropertySet >
                            getByKey( sal_Int32 nKey ) override;
    virtual cpo::uno::Sequence< sal_Int32 > queryKeys( sal_Int16 nType,
                            const css::lang::Locale& nLocale, bool bCreate ) override;
    virtual sal_Int32 queryKey( const OUString& aFormat,
                            const css::lang::Locale& nLocale, bool bScan ) override;
    virtual sal_Int32 addNew( const OUString& aFormat,
                            const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 addNewConverted( const OUString& aFormat,
                            const css::lang::Locale& nLocale,
                            const css::lang::Locale& nNewLocale ) override;
    virtual void removeByKey( sal_Int32 nKey ) override;
    virtual OUString generateFormat( sal_Int32 nBaseKey,
                            const css::lang::Locale& nLocale, bool bThousands,
                            bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading ) override;

    // XNumberFormatTypes
    virtual sal_Int32 getStandardIndex( const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 getStandardFormat( sal_Int16 nType,
                            const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 getFormatIndex( sal_Int16 nIndex,
                            const css::lang::Locale& nLocale ) override;
    virtual bool isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType ) override;
    virtual sal_Int32 getFormatForLocale( sal_Int32 nKey,
                            const css::lang::Locale& nLocale ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};


class SvNumberFormatObj : public cppu::WeakImplHelper<
                                        css::beans::XPropertySet,
                                        css::beans::XPropertyAccess,
                                        css::lang::XServiceInfo>
{
private:
    rtl::Reference<SvNumberFormatsSupplierObj>
                                        m_xSupplier;
    sal_Int32                           nKey;
    mutable ::comphelper::SharedMutex   m_aMutex;

public:
    SvNumberFormatObj( SvNumberFormatsSupplierObj& rParent, sal_Int32 nK, ::comphelper::SharedMutex _aMutex );
    virtual         ~SvNumberFormatObj() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo(  ) override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

    // XPropertyAccess
    virtual cpo::uno::Sequence< css::beans::PropertyValue >
                            getPropertyValues() override;
    virtual void   setPropertyValues( const cpo::uno::Sequence<
                                css::beans::PropertyValue >& aProps ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
                    SvNumberFormatSettingsObj( SvNumberFormatsSupplierObj& rParent, ::comphelper::SharedMutex _aMutex);
    virtual         ~SvNumberFormatSettingsObj() override;


    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo(  ) override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
