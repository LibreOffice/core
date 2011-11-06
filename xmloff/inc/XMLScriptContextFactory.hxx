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



#ifndef _XMLOFF_XMLSCRIPTCONTEXTFACTORY_HXX
#define _XMLOFF_XMLSCRIPTCONTEXTFACTORY_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <xmloff/xmlevent.hxx>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl { class OUString; }
class SvXMLImport;
class XMLEventsImportContext;

class XMLScriptContextFactory : public XMLEventContextFactory
{
    const ::rtl::OUString sEventType;
    const ::rtl::OUString sScript;
    const ::rtl::OUString sURL;

public:
    XMLScriptContextFactory();
    virtual ~XMLScriptContextFactory();

    virtual SvXMLImportContext *
    CreateContext(SvXMLImport & rImport, /// import context
                  sal_uInt16 nPrefix,    /// element: namespace prefix
                  const ::rtl::OUString & rLocalName, /// element: local name
                  /// attribute list
                  const ::com::sun::star::uno::Reference<
                  ::com::sun::star::xml::sax::XAttributeList> & xAttrList,
                  /// the context for the enclosing <script:events> element
                  XMLEventsImportContext * rEvents,
                  /// the event name (as understood by the API)
                  const ::rtl::OUString & rApiEventName,
                  /// the event type name (as registered)
                  const ::rtl::OUString & rLanguage);
};

#endif // _XMLOFF_XMLSCRIPTCONTEXTFACTORY_HXX
