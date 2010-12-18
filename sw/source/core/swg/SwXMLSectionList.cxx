/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
