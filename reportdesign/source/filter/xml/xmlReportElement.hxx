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


#ifndef RPT_XMLREPORTELEMENT_HXX
#define RPT_XMLREPORTELEMENT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/report/XReportControlModel.hpp>

namespace rptxml
{
    class ORptFilter;
    class OXMLReportElement : public SvXMLImportContext
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlModel >   m_xComponent;
        ORptFilter& GetOwnImport();
        OXMLReportElement(const OXMLReportElement&);
        void operator =(const OXMLReportElement&);
    public:

        OXMLReportElement( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlModel >& _xComponent);
        virtual ~OXMLReportElement();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLREPORTELEMENT_HXX
