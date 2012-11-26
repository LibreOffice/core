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



#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#define _XMLTEXTLISTBLOCKCONTEXT_HXX

#include <com/sun/star/container/XIndexReplace.hpp>
#include <xmloff/xmlictxt.hxx>

class XMLTextImportHelper;

class XMLTextListBlockContext : public SvXMLImportContext
{
    XMLTextImportHelper&    mrTxtImport;

    ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > mxNumRules;

    // text:style-name property of <list> element
    ::rtl::OUString         msListStyleName;
    ::rtl::OUString         sXmlId;

    SvXMLImportContextRef   mxParentListBlock;

    sal_Int16               mnLevel;
    sal_Bool                mbRestartNumbering;
    sal_Bool                mbSetDefaults;

    // --> OD 2008-04-22 #refactorlists#
    // text:id property of <list> element, only valid for root <list> element
    ::rtl::OUString msListId;
    // text:continue-list property of <list> element, only valid for root <list> element
    ::rtl::OUString msContinueListId;
    // <--

public:

    // --> OD 2008-05-07 #refactorlists#
    // add optional parameter <bRestartNumberingAtSubList>
    XMLTextListBlockContext(
                SvXMLImport& rImport,
                XMLTextImportHelper& rTxtImp,
                sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                const sal_Bool bRestartNumberingAtSubList = sal_False );
    // <--
    virtual ~XMLTextListBlockContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    const ::rtl::OUString& GetListStyleName() const { return msListStyleName; }
    sal_Int16 GetLevel() const { return mnLevel; }
    sal_Bool IsRestartNumbering() const { return mbRestartNumbering; }
    void ResetRestartNumbering() { mbRestartNumbering = sal_False; }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return mxNumRules; }

    // --> OD 2008-04-22 #refactorlists#
    const ::rtl::OUString& GetListId() const;
    const ::rtl::OUString& GetContinueListId() const;
    // <--

};


#endif
