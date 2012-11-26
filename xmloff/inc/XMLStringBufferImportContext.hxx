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


#ifndef _XMLOFF_XMLSTRINGBUFFERIMPORTCONTEXT_HXX
#define _XMLOFF_XMLSTRINGBUFFERIMPORTCONTEXT_HXX


#include <xmloff/xmlictxt.hxx>

#ifndef _XMLOFF_XMLIMP_HXX_
#include <xmloff/xmlimp.hxx>
#endif
#include <rtl/ustrbuf.hxx>


/**
 * Import all text into a string buffer.  Paragraph elements (<text:p>)
 * are recognized and cause a return character (0x0a) to be added.
 */
class XMLStringBufferImportContext : public SvXMLImportContext
{
    ::rtl::OUStringBuffer& rTextBuffer;

public:

    XMLStringBufferImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& sLocalName,
        ::rtl::OUStringBuffer& rBuffer);

    virtual ~XMLStringBufferImportContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void Characters(
        const ::rtl::OUString& rChars );

    virtual void EndElement();
};

#endif
