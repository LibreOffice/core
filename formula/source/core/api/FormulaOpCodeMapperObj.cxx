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

#include <memory>
#include <sal/config.h>

#include <utility>

#include <formula/FormulaCompiler.hxx>
#include <formula/FormulaOpCodeMapperObj.hxx>
#include <formula/opcode.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace formula
{
    using namespace ::com::sun::star;

sal_Bool SAL_CALL FormulaOpCodeMapperObj::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

FormulaOpCodeMapperObj::FormulaOpCodeMapperObj(::std::unique_ptr<FormulaCompiler> && _pCompiler)
: m_pCompiler(std::move(_pCompiler))
{
}

FormulaOpCodeMapperObj::~FormulaOpCodeMapperObj()
{
}

::sal_Int32 SAL_CALL FormulaOpCodeMapperObj::getOpCodeExternal()
{
    return ocExternal;
}


::sal_Int32 SAL_CALL FormulaOpCodeMapperObj::getOpCodeUnknown()
{
    return FormulaCompiler::OpCodeMap::getOpCodeUnknown();
}


css::uno::Sequence< css::sheet::FormulaToken >
SAL_CALL FormulaOpCodeMapperObj::getMappings(
        const css::uno::Sequence< OUString >& rNames,
        sal_Int32 nLanguage )
{
    FormulaCompiler::OpCodeMapPtr xMap = m_pCompiler->GetOpCodeMap( nLanguage);
    if (!xMap)
        throw lang::IllegalArgumentException();
    return xMap->createSequenceOfFormulaTokens( *m_pCompiler,rNames);
}


css::uno::Sequence< css::sheet::FormulaOpCodeMapEntry >
SAL_CALL FormulaOpCodeMapperObj::getAvailableMappings(
        sal_Int32 nLanguage, sal_Int32 nGroups )
{
    FormulaCompiler::OpCodeMapPtr xMap = m_pCompiler->GetOpCodeMap( nLanguage);
    if (!xMap)
        throw lang::IllegalArgumentException();
    return xMap->createSequenceOfAvailableMappings( *m_pCompiler,nGroups);
}

OUString SAL_CALL FormulaOpCodeMapperObj::getImplementationName(  )
{
    return getImplementationName_Static();
}

OUString FormulaOpCodeMapperObj::getImplementationName_Static()
{
    return OUString( "simple.formula.FormulaOpCodeMapperObj" );
}

uno::Sequence< OUString > SAL_CALL FormulaOpCodeMapperObj::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}
uno::Sequence< OUString > FormulaOpCodeMapperObj::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aSeq { "com.sun.star.sheet.FormulaOpCodeMapper" };
    return aSeq;
}

uno::Reference< uno::XInterface > FormulaOpCodeMapperObj::create(
                uno::Reference< uno::XComponentContext > const & /*_xContext*/)
{
    return static_cast<sheet::XFormulaOpCodeMapper*>(new FormulaOpCodeMapperObj(std::make_unique<FormulaCompiler>()));
}

} // formula


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
