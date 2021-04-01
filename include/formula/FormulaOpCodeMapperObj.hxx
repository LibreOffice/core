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

#include <memory>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>
#include <cppuhelper/implbase.hxx>
#include <formula/formuladllapi.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace sheet { struct FormulaOpCodeMapEntry; }
    namespace sheet { struct FormulaToken; }
    namespace uno { class XComponentContext; }
    namespace uno { class XInterface; }
}

namespace formula
{

class FormulaCompiler;

class FORMULA_DLLPUBLIC FormulaOpCodeMapperObj : public cppu::WeakImplHelper<
                            css::sheet::XFormulaOpCodeMapper,
                            css::lang::XServiceInfo >
{
    ::std::unique_ptr<FormulaCompiler> m_pCompiler;

public:
                            FormulaOpCodeMapperObj(::std::unique_ptr<FormulaCompiler> && _pCompiler);
    virtual                 ~FormulaOpCodeMapperObj() override;

private:
                            // XFormulaOpCodeMapper
                            // Attributes
    virtual ::sal_Int32 SAL_CALL getOpCodeExternal() override;
    virtual ::sal_Int32 SAL_CALL getOpCodeUnknown() override;
                            // Methods
    virtual css::uno::Sequence< css::sheet::FormulaToken > SAL_CALL getMappings(
                                    const css::uno::Sequence< OUString >& rNames,
                                    sal_Int32 nLanguage ) override;
    virtual css::uno::Sequence< css::sheet::FormulaOpCodeMapEntry > SAL_CALL getAvailableMappings(
                                    sal_Int32 nLanguage, sal_Int32 nGroups ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

} // formula


#endif // INCLUDED_FORMULA_FORMULAOPCODEMAPPEROBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
