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

#ifndef INCLUDED_FORMULA_FORMULAOPCODEMAPPEROBJ_HXX
#define INCLUDED_FORMULA_FORMULAOPCODEMAPPEROBJ_HXX

#include <formula/formuladllapi.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <memory>
#include <formula/FormulaCompiler.hxx>


namespace formula
{

class FORMULA_DLLPUBLIC FormulaOpCodeMapperObj : public ::cppu::WeakImplHelper2<
                            ::com::sun::star::sheet::XFormulaOpCodeMapper,
                            ::com::sun::star::lang::XServiceInfo >
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<FormulaCompiler> m_pCompiler;
    SAL_WNODEPRECATED_DECLARATIONS_POP
public:
    static OUString  getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_Static();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext);

protected:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                            FormulaOpCodeMapperObj(::std::auto_ptr<FormulaCompiler> _pCompiler);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual                 ~FormulaOpCodeMapperObj();

private:
                            // XFormulaOpCodeMapper
                            // Attributes
    virtual ::sal_Int32 SAL_CALL getOpCodeExternal() throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL getOpCodeUnknown() throw (::com::sun::star::uno::RuntimeException, std::exception);
                            // Methods
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL getMappings(
                                    const ::com::sun::star::uno::Sequence< OUString >& rNames,
                                    sal_Int32 nLanguage )
                                throw ( ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaOpCodeMapEntry > SAL_CALL getAvailableMappings(
                                    sal_Int32 nLanguage, sal_Int32 nGroups )
                                throw ( ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException, std::exception);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);

};

} // formula


#endif // INCLUDED_FORMULA_FORMULAOPCODEMAPPEROBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
