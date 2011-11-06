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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>
#include <SwXMLSectionList.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;
using namespace ::xmloff::token;

sal_Char __READONLY_DATA sXML_np__office[] = "_ooffice";
sal_Char __READONLY_DATA sXML_np__text[] = "_otext";

// #110680#
SwXMLSectionList::SwXMLSectionList(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SvStrings & rNewSectionList)
:   SvXMLImport( xServiceFactory ),
    rSectionList ( rNewSectionList )
{
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office ) ),
                            GetXMLToken(XML_N_OFFICE_OOO),
                            XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__text ) ),
                            GetXMLToken(XML_N_TEXT_OOO),
                            XML_NAMESPACE_TEXT );
}

SwXMLSectionList::~SwXMLSectionList ( void )
    throw()
{
}

SvXMLImportContext *SwXMLSectionList::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if(( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken ( rLocalName, XML_BODY )) ||
        ( nPrefix == XML_NAMESPACE_TEXT &&
            (IsXMLToken ( rLocalName, XML_P ) ||
            IsXMLToken ( rLocalName, XML_H ) ||
            IsXMLToken ( rLocalName, XML_A ) ||
            IsXMLToken ( rLocalName, XML_SPAN ) ||
            IsXMLToken ( rLocalName, XML_SECTION ) ||
            IsXMLToken ( rLocalName, XML_INDEX_BODY ) ||
            IsXMLToken ( rLocalName, XML_INDEX_TITLE )||
            IsXMLToken ( rLocalName, XML_INSERTION ) ||
            IsXMLToken ( rLocalName, XML_DELETION ) )
        )
      )
    {
        pContext = new SvXMLSectionListContext (*this, nPrefix, rLocalName, xAttrList);
    }
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}

SvXMLSectionListContext::SvXMLSectionListContext(
   SwXMLSectionList& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<   xml::sax::XAttributeList > & ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
}

SvXMLImportContext *SvXMLSectionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    String sName;

    if (nPrefix == XML_NAMESPACE_TEXT && ( IsXMLToken ( rLocalName, XML_SECTION ) ||
                                           IsXMLToken ( rLocalName, XML_BOOKMARK) ) )
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

        for (sal_Int16 i=0; i < nAttrCount; i++)
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefx = rLocalRef.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            if (XML_NAMESPACE_TEXT == nPrefx && IsXMLToken ( aLocalName, XML_NAME ) )
                sName = rAttrValue;
        }
        if ( sName.Len() )
            rLocalRef.rSectionList.Insert ( new String(sName), rLocalRef.rSectionList.Count() );
    }

    pContext = new SvXMLSectionListContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    return pContext;
}
SvXMLSectionListContext::~SvXMLSectionListContext ( void )
{
}

SvXMLIgnoreSectionListContext::SvXMLIgnoreSectionListContext(
   SwXMLSectionList& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference< xml::sax::XAttributeList > & ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
}

SvXMLIgnoreSectionListContext::~SvXMLIgnoreSectionListContext ( void )
{
}
SvXMLImportContext *SvXMLIgnoreSectionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    return  new SvXMLIgnoreSectionListContext (rLocalRef, nPrefix, rLocalName, xAttrList);
}
