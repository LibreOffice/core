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

#include <SwXMLSectionList.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <vector>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

class SvXMLSectionListContext : public SvXMLImportContext
{
private:
    SwXMLSectionList & m_rImport;

public:
    SvXMLSectionListContext(SwXMLSectionList& rImport,
           sal_uInt16 nPrefix,
           const OUString& rLocalName,
           const uno::Reference<xml::sax::XAttributeList> & xAttrList);
    virtual SvXMLImportContextRef CreateChildContext(sal_uInt16 nPrefix,
           const OUString& rLocalName,
           const uno::Reference<xml::sax::XAttributeList> & xAttrList) override;
};

class SwXMLParentContext : public SvXMLImportContext
{
private:
    SwXMLSectionList & m_rImport;

public:
    SwXMLParentContext(SwXMLSectionList& rImport,
           sal_uInt16 nPrefix,
           const OUString& rLocalName)
        : SvXMLImportContext(rImport, nPrefix, rLocalName)
        , m_rImport(rImport)
    {
    }

    virtual SvXMLImportContextRef CreateChildContext(sal_uInt16 nPrefix,
           const OUString& rLocalName,
           const uno::Reference<xml::sax::XAttributeList> & xAttrList) override
    {
        if ((nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken(rLocalName, XML_BODY)) ||
            (nPrefix == XML_NAMESPACE_TEXT &&
                (   IsXMLToken(rLocalName, XML_P)
                 || IsXMLToken(rLocalName, XML_H)
                 || IsXMLToken(rLocalName, XML_A)
                 || IsXMLToken(rLocalName, XML_SPAN)
                 || IsXMLToken(rLocalName, XML_SECTION)
                 || IsXMLToken(rLocalName, XML_INDEX_BODY)
                 || IsXMLToken(rLocalName, XML_INDEX_TITLE)
                 || IsXMLToken(rLocalName, XML_INSERTION)
                 || IsXMLToken(rLocalName, XML_DELETION))))
        {
            return new SvXMLSectionListContext(m_rImport, nPrefix, rLocalName, xAttrList);
        }
        else
        {
            return new SwXMLParentContext(m_rImport, nPrefix, rLocalName);
        }
    }
};


SwXMLSectionList::SwXMLSectionList(
    const uno::Reference< uno::XComponentContext >& rContext,
    std::vector<OUString> &rNewSectionList)
: SvXMLImport(rContext, "")
, m_rSectionList(rNewSectionList)
{
    // TODO: verify if these should match the same-name constants
    //       in xmloff/source/core/xmlimp.cxx ("_office" and "_office")
    GetNamespaceMap().Add( "_ooffice",
                            GetXMLToken(XML_N_OFFICE_OOO),
                            XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( "_otext",
                            GetXMLToken(XML_N_TEXT_OOO),
                            XML_NAMESPACE_TEXT );
}

SwXMLSectionList::~SwXMLSectionList()
    throw()
{
}

SvXMLImportContext *SwXMLSectionList::CreateDocumentContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference<xml::sax::XAttributeList> & )
{
    return new SwXMLParentContext(*this, nPrefix, rLocalName);
}

SvXMLSectionListContext::SvXMLSectionListContext(
   SwXMLSectionList& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const uno::Reference<   xml::sax::XAttributeList > & ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   m_rImport(rImport)
{
}

SvXMLImportContextRef SvXMLSectionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if (nPrefix == XML_NAMESPACE_TEXT && ( IsXMLToken ( rLocalName, XML_SECTION ) ||
                                           IsXMLToken ( rLocalName, XML_BOOKMARK) ) )
    {
        OUString sName;
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

        for (sal_Int16 i=0; i < nAttrCount; i++)
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefx = m_rImport.GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
            if (XML_NAMESPACE_TEXT == nPrefx && IsXMLToken ( aLocalName, XML_NAME ) )
                sName = xAttrList->getValueByIndex( i );
        }
        if ( !sName.isEmpty() )
            m_rImport.m_rSectionList.push_back(sName);
    }

    pContext = new SvXMLSectionListContext(m_rImport, nPrefix, rLocalName, xAttrList);
    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
