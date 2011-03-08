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

#ifndef FORMULA_FORMULAOPCODEMAPPER_HXX_INCLUDED
#define FORMULA_FORMULAOPCODEMAPPER_HXX_INCLUDED

#include "formula/formuladllapi.h"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <memory>
#include "formula/FormulaCompiler.hxx"


namespace formula
{

class FORMULA_DLLPUBLIC FormulaOpCodeMapperObj : public ::cppu::WeakImplHelper2<
                            ::com::sun::star::sheet::XFormulaOpCodeMapper,
                            ::com::sun::star::lang::XServiceInfo >
{
    ::std::auto_ptr<FormulaCompiler> m_pCompiler;
public:
    static ::rtl::OUString  getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString> getSupportedServiceNames_Static();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext);

protected:
                            FormulaOpCodeMapperObj(::std::auto_ptr<FormulaCompiler> _pCompiler);
    virtual                 ~FormulaOpCodeMapperObj();

private:
                            // XFormulaOpCodeMapper
                            // Attributes
    virtual ::sal_Int32 SAL_CALL getOpCodeExternal() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getOpCodeUnknown() throw (::com::sun::star::uno::RuntimeException);
                            // Methods
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL getMappings(
                                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames,
                                    sal_Int32 nLanguage )
                                throw ( ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaOpCodeMapEntry > SAL_CALL getAvailableMappings(
                                    sal_Int32 nLanguage, sal_Int32 nGroups )
                                throw ( ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

};
// =============================================================================
} // formula
// =============================================================================

#endif // FORMULA_FORMULAOPCODEMAPPER_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
