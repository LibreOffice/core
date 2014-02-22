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
#include "formula/FormulaOpCodeMapperObj.hxx"
#include "formula/opcode.hxx"
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace formula
{
    using namespace ::com::sun::star;

sal_Bool SAL_CALL FormulaOpCodeMapperObj::supportsService( const OUString& _rServiceName ) throw(uno::RuntimeException)
{
    return cppu::supportsService(this, _rServiceName);
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
FormulaOpCodeMapperObj::FormulaOpCodeMapperObj(::std::auto_ptr<FormulaCompiler> _pCompiler)
: m_pCompiler(_pCompiler)
{
}
SAL_WNODEPRECATED_DECLARATIONS_POP

FormulaOpCodeMapperObj::~FormulaOpCodeMapperObj()
{
}

::sal_Int32 SAL_CALL FormulaOpCodeMapperObj::getOpCodeExternal()
    throw (::com::sun::star::uno::RuntimeException)
{
    return ocExternal;
}


::sal_Int32 SAL_CALL FormulaOpCodeMapperObj::getOpCodeUnknown()
    throw (::com::sun::star::uno::RuntimeException)
{
    return FormulaCompiler::OpCodeMap::getOpCodeUnknown();
}


::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >
SAL_CALL FormulaOpCodeMapperObj::getMappings(
        const ::com::sun::star::uno::Sequence< OUString >& rNames,
        sal_Int32 nLanguage )
    throw ( ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException)
{
    FormulaCompiler::OpCodeMapPtr xMap = m_pCompiler->GetOpCodeMap( nLanguage);
    if (!xMap)
        throw lang::IllegalArgumentException();
    return xMap->createSequenceOfFormulaTokens( *m_pCompiler,rNames);
}


::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaOpCodeMapEntry >
SAL_CALL FormulaOpCodeMapperObj::getAvailableMappings(
        sal_Int32 nLanguage, sal_Int32 nGroups )
    throw ( ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException)
{
    FormulaCompiler::OpCodeMapPtr xMap = m_pCompiler->GetOpCodeMap( nLanguage);
    if (!xMap)
        throw lang::IllegalArgumentException();
    return xMap->createSequenceOfAvailableMappings( *m_pCompiler,nGroups);
}

OUString SAL_CALL FormulaOpCodeMapperObj::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

OUString SAL_CALL FormulaOpCodeMapperObj::getImplementationName_Static()
{
    return OUString( "simple.formula.FormulaOpCodeMapperObj" );
}

uno::Sequence< OUString > SAL_CALL FormulaOpCodeMapperObj::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
uno::Sequence< OUString > SAL_CALL FormulaOpCodeMapperObj::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq( 1 );
    aSeq[0] = "com.sun.star.sheet.FormulaOpCodeMapper";
    return aSeq;
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
uno::Reference< uno::XInterface > SAL_CALL FormulaOpCodeMapperObj::create(
                uno::Reference< uno::XComponentContext > const & /*_xContext*/)
{
    return static_cast<sheet::XFormulaOpCodeMapper*>(new FormulaOpCodeMapperObj(::std::auto_ptr<FormulaCompiler>(new FormulaCompiler())));
}
SAL_WNODEPRECATED_DECLARATIONS_POP



// =============================================================================
} // formula
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
