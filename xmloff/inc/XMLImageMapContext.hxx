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



#ifndef _XMLOFF_XMLIMAGEMAPCONTEXT_HXX_
#define _XMLOFF_XMLIMAGEMAPCONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexContainer; }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }


class XMLImageMapContext : public SvXMLImportContext
{
    const ::rtl::OUString sImageMap;

    /// the image map to be imported
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XIndexContainer> xImageMap;

    /// the property set from which to get and where eventually to set the
    /// image map
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> xPropertySet;

public:
    XMLImageMapContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);

    virtual ~XMLImageMapContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();
};

#endif
