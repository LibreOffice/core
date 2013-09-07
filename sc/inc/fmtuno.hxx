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

#ifndef SC_FMTUNO_HXX
#define SC_FMTUNO_HXX

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
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/sheet/ConditionOperator2.hpp>

#include "address.hxx"
#include "conditio.hxx"

class ScDocument;
class ScTableConditionalEntry;
class ScConditionalFormat;
class ScValidationData;


struct ScCondFormatEntryItem
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > maTokens1;
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > maTokens2;
    String              maExpr1;
    String              maExpr2;
    String              maExprNmsp1;
    String              maExprNmsp2;
    String              maPosStr;  // formula position as text
    String              maStyle;   // display name as stored in ScStyleSheet
    ScAddress           maPos;
    formula::FormulaGrammar::Grammar meGrammar1; // grammar used with maExpr1
    formula::FormulaGrammar::Grammar meGrammar2; // grammar used with maExpr2
    ScConditionMode     meMode;

    // Make sure the grammar is initialized for API calls.
    ScCondFormatEntryItem();
};

class ScTableConditionalFormat : public cppu::WeakImplHelper5<
                            com::sun::star::sheet::XSheetConditionalEntries,
                            com::sun::star::container::XNameAccess,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::lang::XUnoTunnel,
                            com::sun::star::lang::XServiceInfo >
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
    virtual void SAL_CALL   addNew( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aConditionalEntry )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   clear() throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableConditionalFormat* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XSheetConditionalEntries> xObj );

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



class ScTableConditionalEntry : public cppu::WeakImplHelper3<
                            com::sun::star::sheet::XSheetCondition2,
                            com::sun::star::sheet::XSheetConditionalEntry,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScCondFormatEntryItem       aData;

    ScTableConditionalEntry(); // disabled
public:
                            ScTableConditionalEntry(const ScCondFormatEntryItem& aItem);
    virtual                 ~ScTableConditionalEntry();

    void                    GetData(ScCondFormatEntryItem& rData) const;

                            // XSheetCondition
    virtual ::com::sun::star::sheet::ConditionOperator SAL_CALL getOperator()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getConditionOperator()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setOperator( ::com::sun::star::sheet::ConditionOperator nOperator )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setConditionOperator( sal_Int32 nOperator )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getFormula1() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula1( const OUString& aFormula1 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getFormula2() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula2( const OUString& aFormula2 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getSourcePosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSourcePosition( const ::com::sun::star::table::CellAddress& aSourcePosition )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetConditionalEntry
    virtual OUString SAL_CALL getStyleName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setStyleName( const OUString& aStyleName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



class ScTableValidationObj : public cppu::WeakImplHelper5<
                            com::sun::star::sheet::XSheetCondition2,
                            com::sun::star::sheet::XMultiFormulaTokens,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XUnoTunnel,
                            com::sun::star::lang::XServiceInfo >
{
private:
    SfxItemPropertySet  aPropSet;
    sal_uInt16              nMode;          // enum ScConditionMode
    String              aExpr1;
    String              aExpr2;
    String              maExprNmsp1;
    String              maExprNmsp2;
    formula::FormulaGrammar::Grammar  meGrammar1;      // grammar used with aExpr1 and aExpr2
    formula::FormulaGrammar::Grammar  meGrammar2;      // grammar used with aExpr1 and aExpr2
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > aTokens1;
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > aTokens2;
    ScAddress           aSrcPos;
    String              aPosString;     // formula position as text
    sal_uInt16              nValMode;       // enum ScValidationMode
    sal_Bool                bIgnoreBlank;
    sal_Int16           nShowList;
    sal_Bool                bShowInput;
    OUString              aInputTitle;
    OUString              aInputMessage;
    sal_Bool                bShowError;
    sal_uInt16              nErrorStyle;    // enum ScValidErrorStyle
    OUString              aErrorTitle;
    OUString              aErrorMessage;

    void                    ClearData_Impl();

    ScTableValidationObj(); // disabled
public:
                            ScTableValidationObj(ScDocument* pDoc, sal_uLong nKey,
                                                const formula::FormulaGrammar::Grammar eGrammar);
    virtual                 ~ScTableValidationObj();

    ScValidationData*       CreateValidationData( ScDocument* pDoc,
                                                formula::FormulaGrammar::Grammar eGrammar ) const;

                            // XSheetCondition
    virtual ::com::sun::star::sheet::ConditionOperator SAL_CALL getOperator()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getConditionOperator()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setOperator( ::com::sun::star::sheet::ConditionOperator nOperator )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setConditionOperator( sal_Int32 nOperator )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getFormula1() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula1( const OUString& aFormula1 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getFormula2() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula2( const OUString& aFormula2 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getSourcePosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSourcePosition( const ::com::sun::star::table::CellAddress& aSourcePosition )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XMultiFormulaTokens
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >
                            SAL_CALL getTokens( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException,::com::sun::star::lang::IndexOutOfBoundsException);
    virtual void SAL_CALL setTokens( sal_Int32 nIndex,
                                     const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& aTokens )
                                throw(::com::sun::star::uno::RuntimeException,::com::sun::star::lang::IndexOutOfBoundsException);
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableValidationObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::beans::XPropertySet> xObj );

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
