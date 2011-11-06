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


#ifndef _SW_XMLBLOCKLISTCONTEXT_HXX
#define _SW_XMLBLOCKLISTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>

class SwXMLBlockListImport;
class SwXMLTextBlockImport;

class SwXMLBlockListContext : public SvXMLImportContext
{
private:
    SwXMLBlockListImport & rLocalRef;
public:
    SwXMLBlockListContext( SwXMLBlockListImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SwXMLBlockListContext ( void );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};
class SwXMLBlockContext : public SvXMLImportContext
{
private:
    SwXMLBlockListImport & rLocalRef;
public:
    SwXMLBlockContext(     SwXMLBlockListImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SwXMLBlockContext ( void );
};

class SwXMLTextBlockDocumentContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport & rLocalRef;
public:
    SwXMLTextBlockDocumentContext(     SwXMLTextBlockImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SwXMLTextBlockDocumentContext ( void );
};
class SwXMLTextBlockBodyContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport & rLocalRef;
public:
    SwXMLTextBlockBodyContext(     SwXMLTextBlockImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SwXMLTextBlockBodyContext ( void );
};
class SwXMLTextBlockTextContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport & rLocalRef;
public:
    SwXMLTextBlockTextContext(     SwXMLTextBlockImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SwXMLTextBlockTextContext ( void );
};
class SwXMLTextBlockParContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport & rLocalRef;
public:
    SwXMLTextBlockParContext(     SwXMLTextBlockImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual void Characters( const ::rtl::OUString& rChars );
    ~SwXMLTextBlockParContext ( void );
};
#endif
