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


#ifndef RPT_XMLCELL_HXX
#define RPT_XMLCELL_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/report/XReportComponent.hpp>

namespace rptxml
{
    class ORptFilter;
    class OXMLTable;
    class OXMLCell : public SvXMLImportContext
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >  m_xComponent;
        OXMLTable*      m_pContainer;
        OXMLCell*       m_pCell;
          ::rtl::OUString m_sStyleName;
        ::rtl::OUString m_sText;
        sal_Int32       m_nCurrentCount;
        bool            m_bContainsShape;

        ORptFilter& GetOwnImport();
        OXMLCell(const OXMLCell&);
        void operator =(const OXMLCell&);
    public:

        OXMLCell( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,OXMLTable* _pContainer
                    ,OXMLCell* _pCell = NULL);
        virtual ~OXMLCell();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void Characters( const ::rtl::OUString& rChars );
        virtual void EndElement();

        void setComponent(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xComponent);
        void setContainsShape(bool _bContainsShapes);
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLCELL_HXX
