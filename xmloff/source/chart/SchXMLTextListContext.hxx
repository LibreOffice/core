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


#ifndef _SCH_XMLTEXTLISTCONTEXT_HXX_
#define _SCH_XMLTEXTLISTCONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include "rtl/ustring.hxx"
#include <vector>

namespace com { namespace sun { namespace star { namespace xml { namespace sax {
        class XAttributeList;
}}}}}

class SchXMLTextListContext : public SvXMLImportContext
{
public:
    SchXMLTextListContext( SvXMLImport& rImport,
                            const ::rtl::OUString& rLocalName,
                            ::com::sun::star::uno::Sequence< ::rtl::OUString>& rTextList );
    virtual ~SchXMLTextListContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    ::com::sun::star::uno::Sequence< ::rtl::OUString>& m_rTextList;
    std::vector< ::rtl::OUString> m_aTextVector;
};

#endif  // _SCH_XMLTEXTLISTCONTEXT_HXX_
