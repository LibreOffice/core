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



#ifndef _XMLOFF_XMLGRAPHICSDEFAULTSTYLE_HXX
#define _XMLOFF_XMLGRAPHICSDEFAULTSTYLE_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/prstylei.hxx>

namespace com { namespace sun { namespace star { namespace container
{
    class XNameContainer;
} } } }

class XMLOFF_DLLPUBLIC XMLGraphicsDefaultStyle : public XMLPropStyleContext
{
public:
    XMLGraphicsDefaultStyle( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
            SvXMLStylesContext& rStyles);

    virtual ~XMLGraphicsDefaultStyle();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    // This method is called for every default style
    virtual void SetDefaults();
};

#endif  //  _XMLOFF_XMLGRAPHICSDEFAULTSTYLE_HXX

