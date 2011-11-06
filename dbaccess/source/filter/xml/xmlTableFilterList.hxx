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


#ifndef DBA_XMLTABLEFILTERLIST_HXX
#define DBA_XMLTABLEFILTERLIST_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#include <vector>

namespace dbaxml
{
    class ODBFilter;
    class OXMLTableFilterList : public SvXMLImportContext
    {
        ::std::vector< ::rtl::OUString> m_aPatterns;
        ::std::vector< ::rtl::OUString> m_aTypes;

        ODBFilter& GetOwnImport();
    public:

        OXMLTableFilterList( SvXMLImport& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName);

        virtual ~OXMLTableFilterList();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();

        /** pushes a new TableFilterPattern to the list of patterns
            @param  _sTableFilterPattern
                The new filter pattern.
        */
        inline void pushTableFilterPattern(const ::rtl::OUString& _sTableFilterPattern)
        {
            m_aPatterns.push_back(_sTableFilterPattern);
        }

        /** pushes a new TableTypeFilter to the list of patterns
            @param  _sTypeFilter
                The new type filter.
        */
        inline void pushTableTypeFilter(const ::rtl::OUString& _sTypeFilter)
        {
            m_aTypes.push_back(_sTypeFilter);
        }
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLTABLEFILTERLIST_HXX
