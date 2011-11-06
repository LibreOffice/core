/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    ::com::sun::star::sheet::XFilterFormulaParser > OOXMLFormulaParser_BASE;

/** OOXML formula parser/compiler service for usage in ODF filters. */
class OOXMLFormulaParser : public OOXMLFormulaParser_BASE
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
