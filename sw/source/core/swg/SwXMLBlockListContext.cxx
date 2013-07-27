/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <SwXMLBlockListContext.hxx>
#include <SwXMLBlockImport.hxx>
#include <SwXMLTextBlocks.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <unotools/charclass.hxx>
#include <swtypes.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SwXMLBlockListContext::SwXMLBlockListContext(
   SwXMLBlockListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<
   xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext ( rImport, nPrefix, rLocalName ),
    rLocalRef (rImport)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefx = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if ( XML_NAMESPACE_BLOCKLIST == nPrefx )
        {
            if ( IsXMLToken ( aLocalName, XML_LIST_NAME ) )
            {
                rImport.getBlockList().SetName(rAttrValue);
                break;
            }
        }
    }
}

SwXMLBlockListContext::~SwXMLBlockListContext ( void )
{
}

SvXMLImportContext *SwXMLBlockListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
        IsXMLToken ( rLocalName, XML_BLOCK ) )
        pContext = new SwXMLBlockContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}

SwXMLBlockContext::SwXMLBlockContext(
   SwXMLBlockListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<
   xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    static const CharClass & rCC = GetAppCharClass();
    OUString aShort;
    OUString aLong;
    OUString aPackageName;
    sal_Bool bTextOnly = sal_False;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefx = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nPrefx)
        {
            if ( IsXMLToken ( aLocalName, XML_ABBREVIATED_NAME ) )
            {
                aShort = rCC.uppercase(rAttrValue);
            }
            else if ( IsXMLToken ( aLocalName, XML_NAME ) )
            {
                aLong = rAttrValue;
            }
            else if ( IsXMLToken ( aLocalName, XML_PACKAGE_NAME ) )
            {
                aPackageName = rAttrValue;
            }
            else if ( IsXMLToken ( aLocalName, XML_UNFORMATTED_TEXT ) )
            {
                if ( IsXMLToken ( rAttrValue, XML_TRUE ) )
                    bTextOnly = sal_True;
            }
        }
    }
    if (aShort.isEmpty() || aLong.isEmpty() || aPackageName.isEmpty())
        return;
    rImport.getBlockList().AddName( aShort, aLong, aPackageName, bTextOnly);
}

SwXMLBlockContext::~SwXMLBlockContext ( void )
{
}

SwXMLTextBlockDocumentContext::SwXMLTextBlockDocumentContext(
   SwXMLTextBlockImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<
   xml::sax::XAttributeList > & ) :
    SvXMLImportContext ( rImport, nPrefix, rLocalName ),
    rLocalRef(rImport)
{
}

SvXMLImportContext *SwXMLTextBlockDocumentContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_OFFICE &&
        IsXMLToken ( rLocalName, XML_BODY ) )
        pContext = new SwXMLTextBlockBodyContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SwXMLTextBlockDocumentContext::~SwXMLTextBlockDocumentContext ( void )
{
}


SwXMLTextBlockTextContext::SwXMLTextBlockTextContext(
   SwXMLTextBlockImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<
   xml::sax::XAttributeList > & ) :
    SvXMLImportContext ( rImport, nPrefix, rLocalName ),
    rLocalRef(rImport)
{
}

SvXMLImportContext *SwXMLTextBlockTextContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_TEXT &&
        IsXMLToken ( rLocalName, XML_P ) )
        pContext = new SwXMLTextBlockParContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SwXMLTextBlockTextContext::~SwXMLTextBlockTextContext ( void )
{
}


SwXMLTextBlockBodyContext::SwXMLTextBlockBodyContext(
   SwXMLTextBlockImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<
   xml::sax::XAttributeList > & ) :
    SvXMLImportContext ( rImport, nPrefix, rLocalName ),
    rLocalRef(rImport)
{
}

SvXMLImportContext *SwXMLTextBlockBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_OFFICE &&
        IsXMLToken ( rLocalName, XML_TEXT ) )
        pContext = new SwXMLTextBlockTextContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else if (nPrefix == XML_NAMESPACE_TEXT &&
        IsXMLToken ( rLocalName, XML_P ) )
        pContext = new SwXMLTextBlockParContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SwXMLTextBlockBodyContext::~SwXMLTextBlockBodyContext ( void )
{
}
SwXMLTextBlockParContext::SwXMLTextBlockParContext(
   SwXMLTextBlockImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<
   xml::sax::XAttributeList > & ) :
    SvXMLImportContext ( rImport, nPrefix, rLocalName ),
    rLocalRef(rImport)
{
}

void SwXMLTextBlockParContext::Characters( const OUString& rChars )
{
    rLocalRef.m_rText += rChars;
}
SwXMLTextBlockParContext::~SwXMLTextBlockParContext ( void )
{
    if (rLocalRef.bTextOnly)
        rLocalRef.m_rText += "\015";
    else
    {
        if (!rLocalRef.m_rText.endsWith( " " ))
            rLocalRef.m_rText += " ";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
