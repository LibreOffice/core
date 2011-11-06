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


#ifndef RPT_XMLFUNCTION_HXX
#define RPT_XMLFUNCTION_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/report/XFunctionsSupplier.hpp>
#include <com/sun/star/report/XFunctions.hpp>


namespace rptxml
{
    class ORptFilter;
    class OXMLFunction : public SvXMLImportContext
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions >    m_xFunctions;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction >     m_xFunction;
        bool                                                                        m_bAddToReport;

        ORptFilter& GetOwnImport();

        OXMLFunction(const OXMLFunction&);
        void operator =(const OXMLFunction&);
    public:

        OXMLFunction( ORptFilter& rImport
                    , sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctionsSupplier >&    _xFunctions
                    ,bool _bAddToReport = false
                    );
        virtual ~OXMLFunction();

        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLFunction_HXX
