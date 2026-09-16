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
    cpo::uno::Sequence< css::sheet::FormulaToken > maTokens1;
    cpo::uno::Sequence< css::sheet::FormulaToken > maTokens2;
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
                            ScTableConditionalFormat(const ScDocument& rDoc, sal_uLong nKey,
                                    SCTAB nTab, formula::FormulaGrammar::Grammar eGrammar);
    virtual                 ~ScTableConditionalFormat() override;

    void                    FillFormat( ScConditionalFormat& rFormat, ScDocument& rDoc,
                                formula::FormulaGrammar::Grammar eGrammar) const;

                            // XSheetConditionalEntries
    virtual void   addNew( const cpo::uno::Sequence< css::beans::PropertyValue >& aConditionalEntry ) override;
    virtual void   removeByIndex( sal_Int32 nIndex ) override;
    virtual void   clear() override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

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
    virtual css::sheet::ConditionOperator getOperator() override;
    virtual sal_Int32 getConditionOperator() override;
    virtual void   setOperator( css::sheet::ConditionOperator nOperator ) override;
    virtual void   setConditionOperator( sal_Int32 nOperator ) override;
    virtual OUString getFormula1() override;
    virtual void   setFormula1( const OUString& aFormula1 ) override;
    virtual OUString getFormula2() override;
    virtual void   setFormula2( const OUString& aFormula2 ) override;
    virtual css::table::CellAddress getSourcePosition() override;
    virtual void setSourcePosition( const css::table::CellAddress& aSourcePosition ) override;

                            // XSheetConditionalEntry
    virtual OUString getStyleName() override;
    virtual void   setStyleName( const OUString& aStyleName ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class SC_DLLPUBLIC ScTableValidationObj final : public cppu::WeakImplHelper<
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
    cpo::uno::Sequence< css::sheet::FormulaToken > aTokens1;
    cpo::uno::Sequence< css::sheet::FormulaToken > aTokens2;
    ScAddress           aSrcPos;
    OUString            aPosString;     // formula position as text
    sal_uInt16          nValMode;       // enum ScValidationMode
    bool                bIgnoreBlank;
    bool                bCaseSensitive;
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
    virtual css::sheet::ConditionOperator getOperator() override;
    virtual sal_Int32 getConditionOperator() override;
    virtual void   setOperator( css::sheet::ConditionOperator nOperator ) override;
    virtual void   setConditionOperator( sal_Int32 nOperator ) override;
    virtual OUString getFormula1() override;
    virtual void   setFormula1( const OUString& aFormula1 ) override;
    virtual OUString getFormula2() override;
    virtual void   setFormula2( const OUString& aFormula2 ) override;
    virtual css::table::CellAddress getSourcePosition() override;
    virtual void setSourcePosition( const css::table::CellAddress& aSourcePosition ) override;

                            // XMultiFormulaTokens
    virtual cpo::uno::Sequence< css::sheet::FormulaToken >
                            getTokens( sal_Int32 nIndex ) override;
    virtual void setTokens( sal_Int32 nIndex,
                                     const cpo::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;
    virtual sal_Int32 getCount() override;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
