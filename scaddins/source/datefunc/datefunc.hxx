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

// date functions add in

#ifndef INCLUDED_SCADDINS_SOURCE_DATEFUNC_DATEFUNC_HXX
#define INCLUDED_SCADDINS_SOURCE_DATEFUNC_DATEFUNC_HXX

#include <string.h>
#include <vector>
#include <memory>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#include <com/sun/star/sheet/addin/XDateFunctions.hpp>
#include <com/sun/star/sheet/addin/XMiscFunctions.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/resid.hxx>

class ScaResId : public ResId
{
public:
    ScaResId(sal_uInt16 nResId, ResMgr& rResMgr);
};

enum class ScaCategory
{
    DateTime,
    Text,
    Finance,
    Inf,
    Math,
    Tech
};

struct ScaFuncDataBase
{
    const sal_Char*             pIntName;           // internal name (get***)
    const char*                 pUINameID;          // resource ID to UI name
    const char**                pDescrID;           // resource ID to description, parameter names and ~ description
    const char**                pCompListID;        // list of valid names
    sal_uInt16                  nParamCount;        // number of named / described parameters
    ScaCategory                 eCat;               // function category
    bool                        bDouble;            // name already exist in Calc
    bool                        bWithOpt;           // first parameter is internal
};

class ScaFuncData final
{
private:
    OUString                    aIntName;           // internal name (get***)
    const char*                 pUINameID;          // resource ID to UI name
    const char**                pDescrID;           // leads also to parameter descriptions!
    sal_uInt16                  nParamCount;        // num of parameters
    std::vector<OUString>       aCompList;          // list of all valid names
    ScaCategory                 eCat;               // function category
    bool                    bDouble;            // name already exist in Calc
    bool                    bWithOpt;           // first parameter is internal

public:
                                ScaFuncData(const ScaFuncDataBase& rBaseData);
                                ~ScaFuncData();

    const char*          GetUINameID() const     { return pUINameID; }
    const char**         GetDescrID() const      { return pDescrID; }
    ScaCategory          GetCategory() const     { return eCat; }
    bool                 IsDouble() const        { return bDouble; }

    sal_uInt16                  GetStrIndex( sal_uInt16 nParam ) const;
    bool                 Is( const OUString& rCompare ) const
                                                    { return aIntName == rCompare; }

    const std::vector<OUString>& GetCompNameList() const { return aCompList; }
};

typedef std::vector<ScaFuncData> ScaFuncDataList;

// Predicate for use with std::find_if
struct FindScaFuncData
{
    const OUString& m_rId;
    explicit FindScaFuncData( const OUString& rId ) : m_rId(rId) {}
    bool operator() ( ScaFuncData& rCandidate ) const { return rCandidate.Is(m_rId); }
};


css::uno::Reference< css::uno::XInterface > SAL_CALL DateFunctionAddIn_CreateInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& );


// THE AddIn class for date functions

class ScaDateAddIn : public ::cppu::WeakImplHelper<
                                css::sheet::XAddIn,
                                css::sheet::XCompatibilityNames,
                                css::sheet::addin::XDateFunctions,
                                css::sheet::addin::XMiscFunctions,
                                css::lang::XServiceName,
                                css::lang::XServiceInfo >
{
private:
    css::lang::Locale                          aFuncLoc;
    std::unique_ptr< css::lang::Locale[] >     pDefLocales;
    std::locale                                aResLocale;
    std::unique_ptr< ScaFuncDataList >         pFuncDataList;


    void                        InitDefLocales();
    const css::lang::Locale& GetLocale( sal_uInt32 nIndex );
    void                        InitData();

    /// @throws css::uno::RuntimeException
    OUString                    GetFuncDescrStr(const char** pResId, sal_uInt16 nStrIndex);

public:
                                ScaDateAddIn();

    OUString ScaResId(const char* pId);

    static OUString      getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

                                // XAddIn
    virtual OUString SAL_CALL getProgrammaticFuntionName( const OUString& aDisplayName ) override;
    virtual OUString SAL_CALL getDisplayFunctionName( const OUString& aProgrammaticName ) override;
    virtual OUString SAL_CALL getFunctionDescription( const OUString& aProgrammaticName ) override;
    virtual OUString SAL_CALL getDisplayArgumentName( const OUString& aProgrammaticName, sal_Int32 nArgument ) override;
    virtual OUString SAL_CALL getArgumentDescription( const OUString& aProgrammaticName, sal_Int32 nArgument ) override;
    virtual OUString SAL_CALL getProgrammaticCategoryName( const OUString& aProgrammaticName ) override;
    virtual OUString SAL_CALL getDisplayCategoryName( const OUString& aProgrammaticName ) override;

                                // XCompatibilityNames
    virtual css::uno::Sequence< css::sheet::LocalizedName > SAL_CALL getCompatibilityNames( const OUString& aProgrammaticName ) override;

                                // XLocalizable
    virtual void SAL_CALL       setLocale( const css::lang::Locale& eLocale ) override;
    virtual css::lang::Locale SAL_CALL getLocale() override;

                                // XServiceName
    virtual OUString SAL_CALL getServiceName() override;

                                // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL   supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //  methods from own interfaces start here

                                // XDateFunctions
    virtual sal_Int32 SAL_CALL  getDiffWeeks(
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    sal_Int32 nEndDate, sal_Int32 nStartDate,
                                    sal_Int32 nMode ) override;

    virtual sal_Int32 SAL_CALL  getDiffMonths(
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    sal_Int32 nEndDate, sal_Int32 nStartDate,
                                    sal_Int32 nMode ) override;

    virtual sal_Int32 SAL_CALL  getDiffYears(
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    sal_Int32 nEndDate, sal_Int32 nStartDate,
                                    sal_Int32 nMode ) override;

    virtual sal_Int32 SAL_CALL  getIsLeapYear(
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate ) override;

    virtual sal_Int32 SAL_CALL  getDaysInMonth(
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate ) override;

    virtual sal_Int32 SAL_CALL  getDaysInYear(
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate ) override;

    virtual sal_Int32 SAL_CALL  getWeeksInYear(
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate ) override;

                                // XMiscFunctions
    virtual OUString SAL_CALL getRot13(
                                    const OUString& aSrcText ) override;
};

#endif // INCLUDED_SCADDINS_SOURCE_DATEFUNC_DATEFUNC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
