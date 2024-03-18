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

#include <vector>

#include <formula/grammar.hxx>
#include <rtl/ref.hxx>
#include <svl/itemprop.hxx>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSheetCondition2.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/sheet/XMultiFormulaTokens.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <cppuhelper/implbase.hxx>

#include "address.hxx"
#include "conditio.hxx"

class ScDocument;
class ScTableConditionalEntry;
class ScValidationData;

struct ScCondFormatEntryItem
{
    css::uno::Sequence< css::sheet::FormulaToken > maTokens1;
    css::uno::Sequence< css::sheet::FormulaToken > maTokens2;
    OUString            maExpr1;
    OUString            maExpr2;
    OUString            maExprNmsp1;
    OUString            maExprNmsp2;
    OUString            maPosStr;  // formula position as text
    OUString            maStyle;   // display name as stored in ScStyleSheet
    ScAddress           maPos;
    formula::FormulaGrammar::Grammar meGrammar1; // grammar used with maExpr1
    formula::FormulaGrammar::Grammar meGrammar2; // grammar used with maExpr2
    ScConditionMode     meMode;

    // Make sure the grammar is initialized for API calls.
    ScCondFormatEntryItem();
};

class ScTableConditionalFormat final : public cppu::WeakImplHelper<
                            css::sheet::XSheetConditionalEntries,
                            css::container::XNameAccess,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >
{
private:
    std::vector<rtl::Reference<ScTableConditionalEntry>>   maEntries;

    ScTableConditionalEntry*    GetObjectByIndex_Impl(sal_uInt16 nIndex) const;
    void                        AddEntry_Impl(const ScCondFormatEntryItem& aEntry);
public:
                            ScTableConditionalFormat() = delete;
                            ScTableConditionalFormat(const ScDocument* pDoc, sal_uLong nKey,
                                    SCTAB nTab, formula::FormulaGrammar::Grammar eGrammar);
    virtual                 ~ScTableConditionalFormat() override;

    void                    FillFormat( ScConditionalFormat& rFormat, ScDocument& rDoc,
                                formula::FormulaGrammar::Grammar eGrammar) const;

                            // XSheetConditionalEntries
    virtual void SAL_CALL   addNew( const css::uno::Sequence< css::beans::PropertyValue >& aConditionalEntry ) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex ) override;
    virtual void SAL_CALL   clear() override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

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

class ScTableConditionalEntry final : public cppu::WeakImplHelper<
                            css::sheet::XSheetCondition2,
                            css::sheet::XSheetConditionalEntry,
                            css::lang::XServiceInfo >
{
private:
    ScCondFormatEntryItem       aData;

public:
                            ScTableConditionalEntry() = delete;
                            ScTableConditionalEntry(ScCondFormatEntryItem aItem);
    virtual                 ~ScTableConditionalEntry() override;

    void                    GetData(ScCondFormatEntryItem& rData) const;

                            // XSheetCondition
    virtual css::sheet::ConditionOperator SAL_CALL getOperator() override;
    virtual sal_Int32 SAL_CALL getConditionOperator() override;
    virtual void SAL_CALL   setOperator( css::sheet::ConditionOperator nOperator ) override;
    virtual void SAL_CALL   setConditionOperator( sal_Int32 nOperator ) override;
    virtual OUString SAL_CALL getFormula1() override;
    virtual void SAL_CALL   setFormula1( const OUString& aFormula1 ) override;
    virtual OUString SAL_CALL getFormula2() override;
    virtual void SAL_CALL   setFormula2( const OUString& aFormula2 ) override;
    virtual css::table::CellAddress SAL_CALL getSourcePosition() override;
    virtual void SAL_CALL setSourcePosition( const css::table::CellAddress& aSourcePosition ) override;

                            // XSheetConditionalEntry
    virtual OUString SAL_CALL getStyleName() override;
    virtual void SAL_CALL   setStyleName( const OUString& aStyleName ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScTableValidationObj final : public cppu::WeakImplHelper<
                            css::sheet::XSheetCondition2,
                            css::sheet::XMultiFormulaTokens,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
private:
    SfxItemPropertySet  aPropSet;
    ScConditionMode     nMode;
    OUString            aExpr1;
    OUString            aExpr2;
    OUString            maExprNmsp1;
    OUString            maExprNmsp2;
    formula::FormulaGrammar::Grammar  meGrammar1;      // grammar used with aExpr1 and aExpr2
    formula::FormulaGrammar::Grammar  meGrammar2;      // grammar used with aExpr1 and aExpr2
    css::uno::Sequence< css::sheet::FormulaToken > aTokens1;
    css::uno::Sequence< css::sheet::FormulaToken > aTokens2;
    ScAddress           aSrcPos;
    OUString            aPosString;     // formula position as text
    sal_uInt16          nValMode;       // enum ScValidationMode
    bool                bIgnoreBlank;
    sal_Int16           nShowList;
    bool                bShowInput;
    OUString            aInputTitle;
    OUString            aInputMessage;
    bool                bShowError;
    sal_uInt16          nErrorStyle;    // enum ScValidErrorStyle
    OUString            aErrorTitle;
    OUString            aErrorMessage;

    void                    ClearData_Impl();

public:

                            ScTableValidationObj() = delete;
                            ScTableValidationObj(const ScDocument& rDoc, sal_uInt32 nKey,
                                                const formula::FormulaGrammar::Grammar eGrammar);
    virtual                 ~ScTableValidationObj() override;

    ScValidationData*       CreateValidationData( ScDocument& rDoc,
                                                formula::FormulaGrammar::Grammar eGrammar ) const;

                            // XSheetCondition
    virtual css::sheet::ConditionOperator SAL_CALL getOperator() override;
    virtual sal_Int32 SAL_CALL getConditionOperator() override;
    virtual void SAL_CALL   setOperator( css::sheet::ConditionOperator nOperator ) override;
    virtual void SAL_CALL   setConditionOperator( sal_Int32 nOperator ) override;
    virtual OUString SAL_CALL getFormula1() override;
    virtual void SAL_CALL   setFormula1( const OUString& aFormula1 ) override;
    virtual OUString SAL_CALL getFormula2() override;
    virtual void SAL_CALL   setFormula2( const OUString& aFormula2 ) override;
    virtual css::table::CellAddress SAL_CALL getSourcePosition() override;
    virtual void SAL_CALL setSourcePosition( const css::table::CellAddress& aSourcePosition ) override;

                            // XMultiFormulaTokens
    virtual css::uno::Sequence< css::sheet::FormulaToken >
                            SAL_CALL getTokens( sal_Int32 nIndex ) override;
    virtual void SAL_CALL setTokens( sal_Int32 nIndex,
                                     const css::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;
    virtual sal_Int32 SAL_CALL getCount() override;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
