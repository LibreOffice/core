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

css::uno::Reference<css::uno::XInterface>
    ScSpreadsheetSettings_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );
css::uno::Reference<css::uno::XInterface> SAL_CALL
    ScRecentFunctionsObj_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );
css::uno::Reference<css::uno::XInterface> SAL_CALL
    ScFunctionListObj_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );


class ScSpreadsheetSettings final : public cppu::WeakImplHelper<
                                        css::sheet::XGlobalSheetSettings,
                                        css::beans::XPropertySet,
                                        css::lang::XServiceInfo>
{
private:
    SfxItemPropertySet      aPropSet;

    /// @throws css::uno::RuntimeException
    bool getPropertyBool(const OUString& aPropertyName);
    /// @throws css::uno::RuntimeException
    sal_Int16 getPropertyInt16(const OUString& aPropertyName);
    /// @throws css::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, bool p1)
        { setPropertyValue( aPropertyName, css::uno::Any(p1) ); }
    /// @throws css::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, sal_Int16 p1)
        { setPropertyValue( aPropertyName, css::uno::Any(p1) ); }
public:
                            ScSpreadsheetSettings();
    virtual                 ~ScSpreadsheetSettings() override;

    // XGlobalSheetSettings
    virtual sal_Bool SAL_CALL getMoveSelection() override
        { return getPropertyBool(u"MoveSelection"_ustr); }
    virtual void SAL_CALL setMoveSelection(sal_Bool p1) override
        { setProperty(u"MoveSelection"_ustr, static_cast<bool>(p1)); }
    virtual sal_Int16 SAL_CALL getMoveDirection() override
        { return getPropertyInt16(u"MoveDirection"_ustr); }
    virtual void SAL_CALL setMoveDirection(sal_Int16 p1) override
        { setProperty(u"MoveDirection"_ustr, p1); }
    virtual sal_Bool SAL_CALL getEnterEdit() override
        { return getPropertyBool(u"EnterEdit"_ustr); }
    virtual void SAL_CALL setEnterEdit(sal_Bool p1) override
        { setProperty(u"EnterEdit"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getExtendFormat() override
        { return getPropertyBool(u"ExtendFormat"_ustr); }
    virtual void SAL_CALL setExtendFormat(sal_Bool p1) override
        { setProperty(u"ExtendFormat"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getRangeFinder() override
        { return getPropertyBool(u"RangeFinder"_ustr); }
    virtual void SAL_CALL setRangeFinder(sal_Bool p1) override
        { setProperty(u"RangeFinder"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getExpandReferences() override
        { return getPropertyBool(u"ExpandReferences"_ustr); }
    virtual void SAL_CALL setExpandReferences(sal_Bool p1) override
        { setProperty(u"ExpandReferences"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getMarkHeader() override
        { return getPropertyBool(u"MarkHeader"_ustr); }
    virtual void SAL_CALL setMarkHeader(sal_Bool p1) override
        { setProperty(u"MarkHeader"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getUseTabCol() override
        { return getPropertyBool(u"UseTabCol"_ustr); }
    virtual void SAL_CALL setUseTabCol(sal_Bool p1) override
        { setProperty(u"UseTabCol"_ustr, static_cast<bool>(p1)); }
    virtual sal_Int16 SAL_CALL getMetric() override
        { return getPropertyInt16(u"Metric"_ustr); }
    virtual void SAL_CALL setMetric(sal_Int16 p1) override
        { setProperty(u"Metric"_ustr, p1); }
    virtual sal_Int16 SAL_CALL getScale() override
        { return getPropertyInt16(u"Scale"_ustr); }
    virtual void SAL_CALL setScale(sal_Int16 p1) override
        { setProperty(u"Scale"_ustr, p1); }
    virtual sal_Bool SAL_CALL getDoAutoComplete() override
        { return getPropertyBool(u"DoAutoComplete"_ustr); }
    virtual void SAL_CALL setDoAutoComplete(sal_Bool p1) override
        { setProperty(u"DoAutoComplete"_ustr, static_cast<bool>(p1)); }
    virtual sal_Int16 SAL_CALL getStatusBarFunction() override
        { return getPropertyInt16(u"StatusBarFunction"_ustr); }
    virtual void SAL_CALL setStatusBarFunction(sal_Int16 p1) override
        { setProperty(u"StatusBarFunction"_ustr, p1); }
    virtual css::uno::Sequence<OUString> SAL_CALL getUserLists() override
        {
           css::uno::Any any = getPropertyValue(u"UserLists"_ustr);
           css::uno::Sequence<OUString> b;
           any >>= b;
           return b;
        }
    virtual void SAL_CALL setUserLists(const css::uno::Sequence<OUString>& p1) override
        { setPropertyValue( u"UserLists"_ustr, css::uno::Any(p1) ); }
    virtual sal_Int16 SAL_CALL getLinkUpdateMode() override
        { return getPropertyInt16(u"LinkUpdateMode"_ustr); }
    virtual void SAL_CALL setLinkUpdateMode(sal_Int16 p1) override
        { setProperty(u"LinkUpdateMode"_ustr, p1); }
    virtual sal_Bool SAL_CALL getPrintAllSheets() override
        { return getPropertyBool(u"PrintAllSheets"_ustr); }
    virtual void SAL_CALL setPrintAllSheets(sal_Bool p1) override
        { setProperty(u"PrintAllSheets"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getPrintEmptyPages() override
        { return getPropertyBool(u"PrintEmptyPages"_ustr); }
    virtual void SAL_CALL setPrintEmptyPages(sal_Bool p1) override
        { setProperty(u"PrintEmptyPages"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getUsePrinterMetrics() override
        { return getPropertyBool(u"UsePrinterMetrics"_ustr); }
    virtual void SAL_CALL setUsePrinterMetrics(sal_Bool p1) override
        { setProperty(u"UsePrinterMetrics"_ustr, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getReplaceCellsWarning() override
        { return getPropertyBool(u"ReplaceCellsWarning"_ustr); }
    virtual void SAL_CALL setReplaceCellsWarning(sal_Bool p1) override
        { setProperty(u"ReplaceCellsWarning"_ustr, static_cast<bool>(p1)); }

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScRecentFunctionsObj final : public cppu::WeakImplHelper<
                                        css::sheet::XRecentFunctions,
                                        css::lang::XServiceInfo>
{
public:
                            ScRecentFunctionsObj();
    virtual                 ~ScRecentFunctionsObj() override;

                            // XRecentFunctions
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getRecentFunctionIds() override;
    virtual void SAL_CALL setRecentFunctionIds( const css::uno::Sequence< sal_Int32 >& aRecentFunctionIds ) override;
    virtual sal_Int32 SAL_CALL getMaxRecentFunctions() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getById( sal_Int32 nId ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
