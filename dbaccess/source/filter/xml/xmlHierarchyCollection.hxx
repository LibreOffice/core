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


#ifndef DBA_XMLHIERARCHYCOLLECTION_HXX
#define DBA_XMLHIERARCHYCOLLECTION_HXX

#include <xmloff/xmlictxt.hxx>
#include "xmlEnums.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>


namespace dbaxml
{
    class ODBFilter;
    class OXMLHierarchyCollection : public SvXMLImportContext
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xParentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >    m_xTable;
        ::rtl::OUString     m_sName;
        ::rtl::OUString     m_sCollectionServiceName;
        ::rtl::OUString     m_sComponentServiceName;

        ODBFilter& GetOwnImport();
    public:

        OXMLHierarchyCollection( ODBFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                    ,const ::rtl::OUString& _sCollectionServiceName
                    ,const ::rtl::OUString& _sComponentServiceName
                    );
        OXMLHierarchyCollection( ODBFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >&    _xTable
                    );
        virtual ~OXMLHierarchyCollection();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLHIERARCHYCOLLECTION_HXX
