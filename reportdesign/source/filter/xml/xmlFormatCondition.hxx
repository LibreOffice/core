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


#ifndef RPT_XMLFORMATCONDITION_HXX
#define RPT_XMLFORMATCONDITION_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/report/XFormatCondition.hpp>

namespace rptxml
{
    class ORptFilter;
    class OXMLFormatCondition : public SvXMLImportContext
    {
        ORptFilter& m_rImport;
        ::rtl::OUString m_sStyleName;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormatCondition >  m_xComponent;
        OXMLFormatCondition(const OXMLFormatCondition&);
        void operator =(const OXMLFormatCondition&);
    public:

        OXMLFormatCondition( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormatCondition >& _xComponent
                    );
        virtual ~OXMLFormatCondition();
        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLFORMATTEDFIELD_HXX
