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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_OOXFORMULAPARSER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_OOXFORMULAPARSER_HXX

#include <memory>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XFilterFormulaParser.hpp>
#include <cppuhelper/implbase.hxx>

namespace oox {
namespace xls {

class OOXMLFormulaParserImpl;

typedef ::cppu::WeakImplHelper<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::sheet::XFilterFormulaParser > OOXMLFormulaParser_BASE;

/** OOXML formula parser/compiler service for usage in ODF filters. */
class OOXMLFormulaParser : public OOXMLFormulaParser_BASE
{
public:
    explicit            OOXMLFormulaParser();
    virtual             ~OOXMLFormulaParser();

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual ::rtl::OUString SAL_CALL
                        getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL
                        supportsService( const ::rtl::OUString& rService )
                            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                        getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // com.sun.star.lang.XInitialization interface ----------------------------

    virtual void SAL_CALL initialize(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArgs )
                            throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // com.sun.star.sheet.XFilterFormulaParser interface ----------------------

    virtual ::rtl::OUString SAL_CALL
                        getSupportedNamespace()
                            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // com.sun.star.sheet.XFormulaParser interface ----------------------------

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL
                        parseFormula(
                            const ::rtl::OUString& rFormula,
                            const ::com::sun::star::table::CellAddress& rReferencePos )
                        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual ::rtl::OUString SAL_CALL
                        printFormula(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& rTokens,
                            const ::com::sun::star::table::CellAddress& rReferencePos )
                        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    typedef std::shared_ptr< OOXMLFormulaParserImpl >   ParserImplRef;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                        mxComponent;
    ParserImplRef       mxParserImpl;       /// Implementation of import parser.
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
