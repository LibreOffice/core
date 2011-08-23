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

#ifndef OOX_XLS_OOXFORMULAPARSER_HXX
#define OOX_XLS_OOXFORMULAPARSER_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XFilterFormulaParser.hpp>
#include <cppuhelper/implbase3.hxx>

namespace oox {
namespace xls {

class OOXMLFormulaParserImpl;
class OOXMLFormulaPrinterImpl;

// ============================================================================

typedef ::cppu::WeakImplHelper3<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::sheet::XFilterFormulaParser > OOXMLFormulaParserBase;

/** OOXML formula parser/compiler service for usage in ODF filters. */
class OOXMLFormulaParser : public OOXMLFormulaParserBase
{
public:
    explicit            OOXMLFormulaParser();
    virtual             ~OOXMLFormulaParser();

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual ::rtl::OUString SAL_CALL
                        getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
                        supportsService( const ::rtl::OUString& rService )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                        getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // com.sun.star.lang.XInitialization interface ----------------------------

    virtual void SAL_CALL initialize(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArgs )
                            throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

    // com.sun.star.sheet.XFilterFormulaParser interface ----------------------

    virtual ::rtl::OUString SAL_CALL
                        getSupportedNamespace()
                            throw( ::com::sun::star::uno::RuntimeException );

    // com.sun.star.sheet.XFormulaParser interface ----------------------------

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL
                        parseFormula(
                            const ::rtl::OUString& rFormula,
                            const ::com::sun::star::table::CellAddress& rReferencePos )
                        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
                        printFormula(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& rTokens,
                            const ::com::sun::star::table::CellAddress& rReferencePos )
                        throw( ::com::sun::star::uno::RuntimeException );

private:
    typedef ::boost::shared_ptr< OOXMLFormulaParserImpl >   ParserImplRef;
    typedef ::boost::shared_ptr< OOXMLFormulaPrinterImpl >  PrinterImplRef;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                        mxComponent;
    ParserImplRef       mxParserImpl;       /// Implementation of import parser.
    PrinterImplRef      mxPrinterImpl;      /// Implementation of export printer.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

