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

#ifndef INCLUDED_SC_INC_FMTUNO_HXX
#define INCLUDED_SC_INC_FMTUNO_HXX

#include <vector>

#include <formula/grammar.hxx>
#include <svl/itemprop.hxx>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/XSheetCondition2.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/sheet/XMultiFormulaTokens.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/sheet/ConditionOperator2.hpp>

#include "address.hxx"
#include "conditio.hxx"

class ScDocument;
class ScTableConditionalEntry;
class ScConditionalFormat;
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

class ScTableConditionalFormat : public cppu::WeakImplHelper<
                            css::sheet::XSheetConditionalEntries,
                            css::container::XNameAccess,
                            css::container::XEnumerationAccess,
                            css::lang::XUnoTunnel,
                            css::lang::XServiceInfo >
{
private:
    std::vector<ScTableConditionalEntry*>   aEntries;

    ScTableConditionalEntry*    GetObjectByIndex_Impl(sal_uInt16 nIndex) const;
    void                        AddEntry_Impl(const ScCondFormatEntryItem& aEntry);

    ScTableConditionalFormat(); // disable
public:
                            ScTableConditionalFormat(ScDocument* pDoc, sal_uLong nKey,
                                    SCTAB nTab, formula::FormulaGrammar::Grammar eGrammar);
    virtual                 ~ScTableConditionalFormat();

    void                    FillFormat( ScConditionalFormat& rFormat, ScDocument* pDoc,
                                formula::FormulaGrammar::Grammar eGrammar) const;

                            // XSheetConditionalEntries
    virtual void SAL_CALL   addNew( const css::uno::Sequence< css::beans::PropertyValue >& aConditionalEntry )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   clear() throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableConditionalFormat* getImplementation(const css::uno::Reference<css::sheet::XSheetConditionalEntries>& rObj);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableConditionalEntry : public cppu::WeakImplHelper<
                            css::sheet::XSheetCondition2,
                            css::sheet::XSheetConditionalEntry,
                            css::lang::XServiceInfo >
{
private:
    ScCondFormatEntryItem       aData;

    ScTableConditionalEntry(); // disabled
public:
                            ScTableConditionalEntry(const ScCondFormatEntryItem& aItem);
    virtual                 ~ScTableConditionalEntry();

    void                    GetData(ScCondFormatEntryItem& rData) const;

                            // XSheetCondition
    virtual css::sheet::ConditionOperator SAL_CALL getOperator()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getConditionOperator()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setOperator( css::sheet::ConditionOperator nOperator )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setConditionOperator( sal_Int32 nOperator )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getFormula1() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormula1( const OUString& aFormula1 )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getFormula2() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormula2( const OUString& aFormula2 )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::table::CellAddress SAL_CALL getSourcePosition()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSourcePosition( const css::table::CellAddress& aSourcePosition )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetConditionalEntry
    virtual OUString SAL_CALL getStyleName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setStyleName( const OUString& aStyleName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableValidationObj : public cppu::WeakImplHelper<
                            css::sheet::XSheetCondition2,
                            css::sheet::XMultiFormulaTokens,
                            css::beans::XPropertySet,
                            css::lang::XUnoTunnel,
                            css::lang::XServiceInfo >
{
private:
    SfxItemPropertySet  aPropSet;
    sal_uInt16          nMode;          // enum ScConditionMode
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

    ScTableValidationObj(); // disabled
public:
                            ScTableValidationObj(ScDocument* pDoc, sal_uLong nKey,
                                                const formula::FormulaGrammar::Grammar eGrammar);
    virtual                 ~ScTableValidationObj();

    ScValidationData*       CreateValidationData( ScDocument* pDoc,
                                                formula::FormulaGrammar::Grammar eGrammar ) const;

                            // XSheetCondition
    virtual css::sheet::ConditionOperator SAL_CALL getOperator()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getConditionOperator()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setOperator( css::sheet::ConditionOperator nOperator )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setConditionOperator( sal_Int32 nOperator )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getFormula1() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormula1( const OUString& aFormula1 )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getFormula2() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormula2( const OUString& aFormula2 )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::table::CellAddress SAL_CALL getSourcePosition()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSourcePosition( const css::table::CellAddress& aSourcePosition )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XMultiFormulaTokens
    virtual css::uno::Sequence< css::sheet::FormulaToken >
                            SAL_CALL getTokens( sal_Int32 nIndex )
                                throw(css::uno::RuntimeException,css::lang::IndexOutOfBoundsException, std::exception) override;
    virtual void SAL_CALL setTokens( sal_Int32 nIndex,
                                     const css::uno::Sequence< css::sheet::FormulaToken >& aTokens )
                                throw(css::uno::RuntimeException,css::lang::IndexOutOfBoundsException, std::exception) override;
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
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
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableValidationObj* getImplementation(const css::uno::Reference<css::beans::XPropertySet>& rObj);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
