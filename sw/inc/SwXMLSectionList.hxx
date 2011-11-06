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


#ifndef _SW_XMLSECTIONLIST_HXX
#define _SW_XMLSECTIONLIST_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

class SvStrings;

class SwXMLSectionList : public SvXMLImport
{
protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SvStrings & rSectionList;

    // #110680#
    SwXMLSectionList(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SvStrings & rNewSectionList );

    virtual ~SwXMLSectionList ( )
        throw();
};

class SvXMLSectionListContext : public SvXMLImportContext
{
private:
    SwXMLSectionList & rLocalRef;
public:
    SvXMLSectionListContext ( SwXMLSectionList& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLSectionListContext ( void );
};

class SvXMLIgnoreSectionListContext : public SvXMLImportContext
{
private:
    SwXMLSectionList & rLocalRef;
public:
    SvXMLIgnoreSectionListContext ( SwXMLSectionList& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLIgnoreSectionListContext ( void );
};
#endif
