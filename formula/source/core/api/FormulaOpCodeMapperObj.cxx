/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_formula.hxx"
#include "formula/FormulaOpCodeMapperObj.hxx"
#include "formula/opcode.hxx"
#include <comphelper/sequence.hxx>

// =============================================================================
namespace formula
{
// =============================================================================

    using namespace ::com::sun::star;

    // -----------------------------------------------------------------------------
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL FormulaOpCodeMapperObj::supportsService( const ::rtl::OUString& _rServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::findValue( getSupportedServiceNames_Static(), _rServiceName, sal_True ).getLength() != 0;
}
//------------------------------------------------------------------------
FormulaOpCodeMapperObj::FormulaOpCodeMapperObj(::std::auto_ptr<FormulaCompiler> _pCompiler)
: m_pCompiler(_pCompiler)
{
}

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
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames,
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
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL FormulaOpCodeMapperObj::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL FormulaOpCodeMapperObj::getImplementationName_Static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "simple.formula.FormulaOpCodeMapperObj" ) );
}
// --------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL FormulaOpCodeMapperObj::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
uno::Sequence< rtl::OUString > SAL_CALL FormulaOpCodeMapperObj::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSeq( 1 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.FormulaOpCodeMapper" ));
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL FormulaOpCodeMapperObj::create(
                uno::Reference< uno::XComponentContext > const & /*_xContext*/)
{
    return static_cast<sheet::XFormulaOpCodeMapper*>(new FormulaOpCodeMapperObj(::std::auto_ptr<FormulaCompiler>(new FormulaCompiler())));
}
// -----------------------------------------------------------------------------

// =============================================================================
} // formula
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
