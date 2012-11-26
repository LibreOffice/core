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



#ifndef _XMLTEXTLISTITEMCONTEXT_HXX
#define _XMLTEXTLISTITEMCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>

class XMLTextImportHelper;

class XMLTextListItemContext : public SvXMLImportContext
{
    XMLTextImportHelper& rTxtImport;

    sal_Int16                   nStartValue;

    // --> OD 2008-05-07 #refactorlists#
    // quantity of <text:list> child elements
    sal_Int16 mnSubListCount;
    // list style instance for text::style-override property
    ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > mxNumRulesOverride;
    // <--

public:

    XMLTextListItemContext(
            SvXMLImport& rImport,
            XMLTextImportHelper& rTxtImp,
            const sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bIsHeader = sal_False );
    virtual ~XMLTextListItemContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                 const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                     ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    sal_Bool HasStartValue() const { return -1 != nStartValue; }
    sal_Int16 GetStartValue() const { return nStartValue; }

    // --> OD 2008-05-08 #refactorlists#
    inline sal_Bool HasNumRulesOverride() const
    {
        return mxNumRulesOverride.is();
    }
    inline const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRulesOverride() const
    {
        return mxNumRulesOverride;
    }
};


#endif
