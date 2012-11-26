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



#ifndef _XMLTEXTCOLUMNSCONTEXT_HXX
#define _XMLTEXTCOLUMNSCONTEXT_HXX

#include "XMLElementPropertyContext.hxx"


namespace rtl { class OUString; }
class XMLTextColumnsArray_Impl;
class XMLTextColumnSepContext_Impl;
class SvXMLTokenMap;

class XMLTextColumnsContext :public XMLElementPropertyContext
{
    const ::rtl::OUString sSeparatorLineIsOn;
    const ::rtl::OUString sSeparatorLineWidth;
    const ::rtl::OUString sSeparatorLineColor;
    const ::rtl::OUString sSeparatorLineRelativeHeight;
    const ::rtl::OUString sSeparatorLineVerticalAlignment;
    const ::rtl::OUString sIsAutomatic;
    const ::rtl::OUString sAutomaticDistance;


    XMLTextColumnsArray_Impl *pColumns;
    XMLTextColumnSepContext_Impl     *pColumnSep;
    SvXMLTokenMap            *pColumnAttrTokenMap;
    SvXMLTokenMap            *pColumnSepAttrTokenMap;
    sal_Int16                nCount;
    sal_Bool                 bAutomatic;
    sal_Int32                nAutomaticDistance;

public:
    XMLTextColumnsContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        ::std::vector< XMLPropertyState > &rProps );

    virtual ~XMLTextColumnsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();
};


#endif
