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


#ifndef RPT_XMLFIXEDCONTENT_HXX
#define RPT_XMLFIXEDCONTENT_HXX

#include "xmlReportElementBase.hxx"
#include <com/sun/star/drawing/XShapes.hpp>


namespace rptxml
{
    class ORptFilter;
    class OXMLCell;
    class OXMLFixedContent : public OXMLReportElementBase
    {
        ::rtl::OUString     m_sPageText; // page count and page number
        ::rtl::OUString     m_sLabel;
        OXMLCell&           m_rCell;
        OXMLFixedContent*   m_pInP; // if set than we are in text-p element
        bool                m_bFormattedField;

    protected:
        virtual SvXMLImportContext* _CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
        OXMLFixedContent(const OXMLFixedContent&);
        void operator =(const OXMLFixedContent&);
    public:

        OXMLFixedContent( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName
                    ,OXMLCell& _rCell
                    ,OXMLTable* _pContainer
                    ,OXMLFixedContent* _pInP = NULL);
        virtual ~OXMLFixedContent();

        // This method is called for all characters that are contained in the
        // current element. The default is to ignore them.
        virtual void Characters( const ::rtl::OUString& rChars );

        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLFIXEDCONTENT_HXX
