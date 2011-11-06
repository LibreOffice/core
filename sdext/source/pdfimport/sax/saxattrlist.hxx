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



#ifndef INCLUDED_PDFI_SAXATTRLIST_HXX
#define INCLUDED_PDFI_SAXATTRLIST_HXX

#include <rtl/ustring.hxx>
#include <vector>
#include <hash_map>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace pdfi
{
    class SaxAttrList : public ::cppu::WeakImplHelper2<
            com::sun::star::xml::sax::XAttributeList,
            com::sun::star::util::XCloneable
            >
    {
        struct AttrEntry
        {
            rtl::OUString m_aName;
            rtl::OUString m_aValue;

            AttrEntry( const rtl::OUString& i_rName, const rtl::OUString& i_rValue )
            : m_aName( i_rName ), m_aValue( i_rValue ) {}
        };
        std::vector< AttrEntry >                                    m_aAttributes;
        std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >   m_aIndexMap;

    public:
        SaxAttrList() {}
        SaxAttrList( const std::hash_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >& );
        SaxAttrList( const SaxAttrList& );
        virtual ~SaxAttrList();

        // ::com::sun::star::xml::sax::XAttributeList
        virtual sal_Int16 SAL_CALL getLength() throw();
        virtual rtl::OUString SAL_CALL getNameByIndex(sal_Int16 i) throw();
        virtual rtl::OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw();
        virtual rtl::OUString SAL_CALL getTypeByName(const ::rtl::OUString& aName) throw();
        virtual rtl::OUString SAL_CALL getValueByIndex(sal_Int16 i) throw();
        virtual rtl::OUString SAL_CALL getValueByName(const ::rtl::OUString& aName) throw();

        // ::com::sun::star::util::XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone() throw();
    };
}

#endif
