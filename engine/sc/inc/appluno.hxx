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

#pragma once

#include <svl/itemprop.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XRecentFunctions.hpp>
#include <com/sun/star/sheet/XFunctionDescriptions.hpp>
#include <com/sun/star/sheet/XGlobalSheetSettings.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/sfxmodelfactory.hxx>

namespace com::sun::star::lang { class XMultiServiceFactory; }

cpo::uno::Reference<cpo::uno::XInterface>
    ScSpreadsheetSettings_CreateInstance(
        const cpo::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );
cpo::uno::Reference<cpo::uno::XInterface>
    ScRecentFunctionsObj_CreateInstance(
        const cpo::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );
cpo::uno::Reference<cpo::uno::XInterface>
    ScFunctionListObj_CreateInstance(
        const cpo::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );


class ScSpreadsheetSettings final : public cppu::WeakImplHelper<
                                        css::sheet::XGlobalSheetSettings,
                                        css::beans::XPropertySet,
                                        css::lang::XServiceInfo>
{
private:
    SfxItemPropertySet      aPropSet;

    /// @throws cpo::uno::RuntimeException
    bool getPropertyBool(const OUString& aPropertyName);
    /// @throws cpo::uno::RuntimeException
    sal_Int16 getPropertyInt16(const OUString& aPropertyName);
    /// @throws cpo::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, bool p1)
        { setPropertyValue( aPropertyName, cpo::uno::Any(p1) ); }
    /// @throws cpo::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, sal_Int16 p1)
        { setPropertyValue( aPropertyName, cpo::uno::Any(p1) ); }
public:
                            ScSpreadsheetSettings();
    virtual                 ~ScSpreadsheetSettings() override;

    // XGlobalSheetSettings
    virtual bool getMoveSelection() override
        { return getPropertyBool(u"MoveSelection"_ustr); }
    virtual void setMoveSelection(bool p1) override
        { setProperty(u"MoveSelection"_ustr, p1); }
    virtual sal_Int16 getMoveDirection() override
        { return getPropertyInt16(u"MoveDirection"_ustr); }
    virtual void setMoveDirection(sal_Int16 p1) override
        { setProperty(u"MoveDirection"_ustr, p1); }
    virtual bool getEnterEdit() override
        { return getPropertyBool(u"EnterEdit"_ustr); }
    virtual void setEnterEdit(bool p1) override
        { setProperty(u"EnterEdit"_ustr, p1); }
    virtual bool getExtendFormat() override
        { return getPropertyBool(u"ExtendFormat"_ustr); }
    virtual void setExtendFormat(bool p1) override
        { setProperty(u"ExtendFormat"_ustr, p1); }
    virtual bool getRangeFinder() override
        { return getPropertyBool(u"RangeFinder"_ustr); }
    virtual void setRangeFinder(bool p1) override
        { setProperty(u"RangeFinder"_ustr, p1); }
    virtual bool getExpandReferences() override
        { return getPropertyBool(u"ExpandReferences"_ustr); }
    virtual void setExpandReferences(bool p1) override
        { setProperty(u"ExpandReferences"_ustr, p1); }
    virtual bool getMarkHeader() override
        { return getPropertyBool(u"MarkHeader"_ustr); }
    virtual void setMarkHeader(bool p1) override
        { setProperty(u"MarkHeader"_ustr, p1); }
    virtual bool getUseTabCol() override
        { return getPropertyBool(u"UseTabCol"_ustr); }
    virtual void setUseTabCol(bool p1) override
        { setProperty(u"UseTabCol"_ustr, p1); }
    virtual sal_Int16 getMetric() override
        { return getPropertyInt16(u"Metric"_ustr); }
    virtual void setMetric(sal_Int16 p1) override
        { setProperty(u"Metric"_ustr, p1); }
    virtual sal_Int16 getScale() override
        { return getPropertyInt16(u"Scale"_ustr); }
    virtual void setScale(sal_Int16 p1) override
        { setProperty(u"Scale"_ustr, p1); }
    virtual bool getDoAutoComplete() override
        { return getPropertyBool(u"DoAutoComplete"_ustr); }
    virtual void setDoAutoComplete(bool p1) override
        { setProperty(u"DoAutoComplete"_ustr, p1); }
    virtual sal_Int16 getStatusBarFunction() override
        { return getPropertyInt16(u"StatusBarFunction"_ustr); }
    virtual void setStatusBarFunction(sal_Int16 p1) override
        { setProperty(u"StatusBarFunction"_ustr, p1); }
    virtual cpo::uno::Sequence<OUString> getUserLists() override
        {
           cpo::uno::Any any = getPropertyValue(u"UserLists"_ustr);
           cpo::uno::Sequence<OUString> b;
           any >>= b;
           return b;
        }
    virtual void setUserLists(const cpo::uno::Sequence<OUString>& p1) override
        { setPropertyValue( u"UserLists"_ustr, cpo::uno::Any(p1) ); }
    virtual sal_Int16 getLinkUpdateMode() override
        { return getPropertyInt16(u"LinkUpdateMode"_ustr); }
    virtual void setLinkUpdateMode(sal_Int16 p1) override
        { setProperty(u"LinkUpdateMode"_ustr, p1); }
    virtual bool getPrintAllSheets() override
        { return getPropertyBool(u"PrintAllSheets"_ustr); }
    virtual void setPrintAllSheets(bool p1) override
        { setProperty(u"PrintAllSheets"_ustr, p1); }
    virtual bool getPrintEmptyPages() override
        { return getPropertyBool(u"PrintEmptyPages"_ustr); }
    virtual void setPrintEmptyPages(bool p1) override
        { setProperty(u"PrintEmptyPages"_ustr, p1); }
    virtual bool getUsePrinterMetrics() override
        { return getPropertyBool(u"UsePrinterMetrics"_ustr); }
    virtual void setUsePrinterMetrics(bool p1) override
        { setProperty(u"UsePrinterMetrics"_ustr, p1); }
    virtual bool getReplaceCellsWarning() override
        { return getPropertyBool(u"ReplaceCellsWarning"_ustr); }
    virtual void setReplaceCellsWarning(bool p1) override
        { setProperty(u"ReplaceCellsWarning"_ustr,p1); }

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScRecentFunctionsObj final : public cppu::WeakImplHelper<
                                        css::sheet::XRecentFunctions,
                                        css::lang::XServiceInfo>
{
public:
                            ScRecentFunctionsObj();
    virtual                 ~ScRecentFunctionsObj() override;

                            // XRecentFunctions
    virtual cpo::uno::Sequence< sal_Int32 > getRecentFunctionIds() override;
    virtual void setRecentFunctionIds( const cpo::uno::Sequence< sal_Int32 >& aRecentFunctionIds ) override;
    virtual sal_Int32 getMaxRecentFunctions() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScFunctionListObj final : public cppu::WeakImplHelper<
                                        css::sheet::XFunctionDescriptions,
                                        css::container::XEnumerationAccess,
                                        css::container::XNameAccess,
                                        css::lang::XServiceInfo>
{
public:
                            ScFunctionListObj();
    virtual                 ~ScFunctionListObj() override;

                            // XFunctionDescriptions
    virtual cpo::uno::Sequence< css::beans::PropertyValue >
                            getById( sal_Int32 nId ) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
