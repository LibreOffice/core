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

#ifndef INCLUDED_SC_INC_APPLUNO_HXX
#define INCLUDED_SC_INC_APPLUNO_HXX

#include <svl/itemprop.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XRecentFunctions.hpp>
#include <com/sun/star/sheet/XFunctionDescriptions.hpp>
#include <com/sun/star/sheet/XGlobalSheetSettings.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/sfxmodelfactory.hxx>

css::uno::Reference<css::uno::XInterface> SAL_CALL
    ScSpreadsheetSettings_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );
css::uno::Reference<css::uno::XInterface> SAL_CALL
    ScRecentFunctionsObj_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );
css::uno::Reference<css::uno::XInterface> SAL_CALL
    ScFunctionListObj_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMgr );

// Calc document
css::uno::Sequence< OUString > SAL_CALL ScDocument_getSupportedServiceNames() throw();
OUString SAL_CALL ScDocument_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL ScDocument_createInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr, SfxModelFlags _nCreationFlags );

// Calc XML import
css::uno::Sequence< OUString > SAL_CALL ScXMLImport_getSupportedServiceNames() throw();
OUString SAL_CALL ScXMLImport_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL ScXMLImport_createInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );
css::uno::Sequence< OUString > SAL_CALL ScXMLImport_Meta_getSupportedServiceNames() throw();
OUString SAL_CALL ScXMLImport_Meta_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL ScXMLImport_Meta_createInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );
css::uno::Sequence< OUString > SAL_CALL ScXMLImport_Styles_getSupportedServiceNames() throw();
OUString SAL_CALL ScXMLImport_Styles_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL ScXMLImport_Styles_createInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );
css::uno::Sequence< OUString > SAL_CALL ScXMLImport_Content_getSupportedServiceNames() throw();
OUString SAL_CALL ScXMLImport_Content_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL ScXMLImport_Content_createInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );
css::uno::Sequence< OUString > SAL_CALL ScXMLImport_Settings_getSupportedServiceNames() throw();
OUString SAL_CALL ScXMLImport_Settings_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL ScXMLImport_Settings_createInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

class ScSpreadsheetSettings : public cppu::WeakImplHelper<
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

    static OUString  getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XGlobalSheetSettings
    virtual sal_Bool SAL_CALL getMoveSelection() override
        { return getPropertyBool("MoveSelection"); }
    virtual void SAL_CALL setMoveSelection(sal_Bool p1) override
        { setProperty("MoveSelection", (bool)p1); }
    virtual sal_Int16 SAL_CALL getMoveDirection() override
        { return getPropertyInt16("MoveDirection"); }
    virtual void SAL_CALL setMoveDirection(sal_Int16 p1) override
        { setProperty("MoveDirection", p1); }
    virtual sal_Bool SAL_CALL getEnterEdit() override
        { return getPropertyBool("EnterEdit"); }
    virtual void SAL_CALL setEnterEdit(sal_Bool p1) override
        { setProperty("EnterEdit", (bool)p1); }
    virtual sal_Bool SAL_CALL getExtendFormat() override
        { return getPropertyBool("ExtendFormat"); }
    virtual void SAL_CALL setExtendFormat(sal_Bool p1) override
        { setProperty("ExtendFormat", (bool)p1); }
    virtual sal_Bool SAL_CALL getRangeFinder() override
        { return getPropertyBool("RangeFinder"); }
    virtual void SAL_CALL setRangeFinder(sal_Bool p1) override
        { setProperty("RangeFinder", (bool)p1); }
    virtual sal_Bool SAL_CALL getExpandReferences() override
        { return getPropertyBool("ExpandReferences"); }
    virtual void SAL_CALL setExpandReferences(sal_Bool p1) override
        { setProperty("ExpandReferences", (bool)p1); }
    virtual sal_Bool SAL_CALL getMarkHeader() override
        { return getPropertyBool("MarkHeader"); }
    virtual void SAL_CALL setMarkHeader(sal_Bool p1) override
        { setProperty("MarkHeader", (bool)p1); }
    virtual sal_Bool SAL_CALL getUseTabCol() override
        { return getPropertyBool("UseTabCol"); }
    virtual void SAL_CALL setUseTabCol(sal_Bool p1) override
        { setProperty("UseTabCol", (bool)p1); }
    virtual sal_Int16 SAL_CALL getMetric() override
        { return getPropertyInt16("Metric"); }
    virtual void SAL_CALL setMetric(sal_Int16 p1) override
        { setProperty("Metric", p1); }
    virtual sal_Int16 SAL_CALL getScale() override
        { return getPropertyInt16("Scale"); }
    virtual void SAL_CALL setScale(sal_Int16 p1) override
        { setProperty("Scale", p1); }
    virtual sal_Bool SAL_CALL getDoAutoComplete() override
        { return getPropertyBool("DoAutoComplete"); }
    virtual void SAL_CALL setDoAutoComplete(sal_Bool p1) override
        { setProperty("DoAutoComplete", (bool)p1); }
    virtual sal_Int16 SAL_CALL getStatusBarFunction() override
        { return getPropertyInt16("StatusBarFunction"); }
    virtual void SAL_CALL setStatusBarFunction(sal_Int16 p1) override
        { setProperty("StatusBarFunction", p1); }
    virtual css::uno::Sequence<OUString> SAL_CALL getUserLists() override
        {
           css::uno::Any any = getPropertyValue("UserLists");
           css::uno::Sequence<OUString> b;
           any >>= b;
           return b;
        }
    virtual void SAL_CALL setUserLists(const css::uno::Sequence<OUString>& p1) override
        { setPropertyValue( "UserLists", css::uno::Any(p1) ); }
    virtual sal_Int16 SAL_CALL getLinkUpdateMode() override
        { return getPropertyInt16("LinkUpdateMode"); }
    virtual void SAL_CALL setLinkUpdateMode(sal_Int16 p1) override
        { setProperty("LinkUpdateMode", p1); }
    virtual sal_Bool SAL_CALL getPrintAllSheets() override
        { return getPropertyBool("PrintAllSheets"); }
    virtual void SAL_CALL setPrintAllSheets(sal_Bool p1) override
        { setProperty("PrintAllSheets", (bool)p1); }
    virtual sal_Bool SAL_CALL getPrintEmptyPages() override
        { return getPropertyBool("PrintEmptyPages"); }
    virtual void SAL_CALL setPrintEmptyPages(sal_Bool p1) override
        { setProperty("PrintEmptyPages", (bool)p1); }
    virtual sal_Bool SAL_CALL getUsePrinterMetrics() override
        { return getPropertyBool("UsePrinterMetrics"); }
    virtual void SAL_CALL setUsePrinterMetrics(sal_Bool p1) override
        { setProperty("UsePrinterMetrics", (bool)p1); }
    virtual sal_Bool SAL_CALL getReplaceCellsWarning() override
        { return getPropertyBool("ReplaceCellsWarning"); }
    virtual void SAL_CALL setReplaceCellsWarning(sal_Bool p1) override
        { setProperty("ReplaceCellsWarning", (bool)p1); }

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

class ScRecentFunctionsObj : public cppu::WeakImplHelper<
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

class ScFunctionListObj : public cppu::WeakImplHelper<
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
