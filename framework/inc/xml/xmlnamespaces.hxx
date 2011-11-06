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



#ifndef __FRAMEWORK_XML_XMLNAMESPACES_HXX_
#define __FRAMEWORK_XML_XMLNAMESPACES_HXX_

#include <com/sun/star/xml/sax/SAXException.hpp>

#include <map>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC XMLNamespaces
{
    public:
        XMLNamespaces();
        XMLNamespaces( const XMLNamespaces& );
        virtual ~XMLNamespaces();

        void addNamespace( const ::rtl::OUString& aName, const ::rtl::OUString& aValue )
            throw(  ::com::sun::star::xml::sax::SAXException );

        ::rtl::OUString applyNSToAttributeName( const ::rtl::OUString& ) const
            throw(  ::com::sun::star::xml::sax::SAXException );
        ::rtl::OUString applyNSToElementName( const ::rtl::OUString& ) const
            throw(  ::com::sun::star::xml::sax::SAXException );

    private:
        typedef ::std::map< ::rtl::OUString, ::rtl::OUString > NamespaceMap;

        ::rtl::OUString getNamespaceValue( const ::rtl::OUString& aNamespace ) const
            throw( ::com::sun::star::xml::sax::SAXException );

        ::rtl::OUString m_aDefaultNamespace;
        NamespaceMap    m_aNamespaceMap;
};

}

#endif // __FRAMEWORK_XML_XMLNAMESPACES_HXX_
