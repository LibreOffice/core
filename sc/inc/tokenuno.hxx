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

#ifndef INCLUDED_SC_INC_TOKENUNO_HXX
#define INCLUDED_SC_INC_TOKENUNO_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>
#include <formula/FormulaOpCodeMapperObj.hxx>
#include "address.hxx"
#include "compiler.hxx"

class ScTokenArray;
class ScDocShell;

class ScTokenConversion
{
public:
    static SC_DLLPUBLIC bool ConvertToTokenArray(
                        ScDocument& rDoc,
                        ScTokenArray& rTokenArray,
                        const com::sun::star::uno::Sequence< com::sun::star::sheet::FormulaToken >& rSequence );
    static SC_DLLPUBLIC bool ConvertToTokenSequence(
                        const ScDocument& rDoc,
                        com::sun::star::uno::Sequence< com::sun::star::sheet::FormulaToken >& rSequence,
                        const ScTokenArray& rTokenArray );
};

class ScFormulaParserObj : public ::cppu::WeakImplHelper<
                            ::com::sun::star::sheet::XFormulaParser,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ::com::sun::star::uno::Sequence< const ::com::sun::star::sheet::FormulaOpCodeMapEntry > maOpCodeMapping;
    ::com::sun::star::uno::Sequence<com::sun::star::sheet::ExternalLinkInfo> maExternalLinks;
    ScCompiler::OpCodeMapPtr    mxOpCodeMap;
    ScDocShell*         mpDocShell;
    sal_Int16           mnConv;
    bool                mbEnglish;
    bool                mbIgnoreSpaces;
    bool                mbCompileFAP;

    void                    SetCompilerFlags( ScCompiler& rCompiler ) const;

public:
                            ScFormulaParserObj(ScDocShell* pDocSh);
    virtual                 ~ScFormulaParserObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XFormulaParser
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL parseFormula(
                                    const OUString& aFormula,
                                    const ::com::sun::star::table::CellAddress& rReferencePos )
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) override;
    virtual OUString SAL_CALL printFormula( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::FormulaToken >& aTokens,
                                    const ::com::sun::star::table::CellAddress& rReferencePos )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

class ScFormulaOpCodeMapperObj : public formula::FormulaOpCodeMapperObj
{
public:
    ScFormulaOpCodeMapperObj(::std::unique_ptr<formula::FormulaCompiler> && _pCompiler);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
