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

#pragma once

#include <memory>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XFilterFormulaParser.hpp>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::lang { class XComponent; }

namespace oox::xls {

class OOXMLFormulaParserImpl;

typedef ::cppu::WeakImplHelper<
    css::lang::XServiceInfo,
    css::lang::XInitialization,
    css::sheet::XFilterFormulaParser > OOXMLFormulaParser_BASE;

/** OOXML formula parser/compiler service for usage in ODF filters. */
class OOXMLFormulaParser : public OOXMLFormulaParser_BASE
{
public:
    explicit            OOXMLFormulaParser();
    virtual             ~OOXMLFormulaParser() override;

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual OUString SAL_CALL
                        getImplementationName() override;

    virtual sal_Bool SAL_CALL
                        supportsService( const OUString& rService ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
                        getSupportedServiceNames() override;

    // com.sun.star.lang.XInitialization interface ----------------------------

    virtual void SAL_CALL initialize(
                            const css::uno::Sequence< css::uno::Any >& rArgs ) override;

    // com.sun.star.sheet.XFilterFormulaParser interface ----------------------

    virtual OUString SAL_CALL
                        getSupportedNamespace() override;

    // com.sun.star.sheet.XFormulaParser interface ----------------------------

    virtual css::uno::Sequence< css::sheet::FormulaToken > SAL_CALL
                        parseFormula(
                            const OUString& rFormula,
                            const css::table::CellAddress& rReferencePos ) override;

    virtual OUString SAL_CALL
                        printFormula(
                            const css::uno::Sequence< css::sheet::FormulaToken >& rTokens,
                            const css::table::CellAddress& rReferencePos ) override;

private:
    typedef std::shared_ptr< OOXMLFormulaParserImpl >   ParserImplRef;

    css::uno::Reference< css::lang::XComponent >
                        mxComponent;
    ParserImplRef       mxParserImpl;       /// Implementation of import parser.
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
