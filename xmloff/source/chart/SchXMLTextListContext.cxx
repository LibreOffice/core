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
#include "precompiled_xmloff.hxx"

#include "SchXMLImport.hxx"
#include "SchXMLTextListContext.hxx"
#include "SchXMLParagraphContext.hxx"

#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using namespace com::sun::star;
using namespace ::xmloff::token;

//-------------------------------------------------
class SchXMLListItemContext : public SvXMLImportContext
{
public:
    SchXMLListItemContext( SvXMLImport& rImport, const OUString& rLocalName, OUString& rText );
    virtual ~SchXMLListItemContext();
    virtual void StartElement( const Reference< xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    ::rtl::OUString& m_rText;
};

SchXMLListItemContext::SchXMLListItemContext(
        SvXMLImport& rImport
        , const OUString& rLocalName
        , OUString& rText )
        : SvXMLImportContext( rImport, XML_NAMESPACE_TEXT, rLocalName )
        , m_rText( rText )
{
}

SchXMLListItemContext::~SchXMLListItemContext()
{}

void SchXMLListItemContext::StartElement( const Reference< xml::sax::XAttributeList >& /*xAttrList*/ )
{
}

void SchXMLListItemContext::EndElement()
{
}

SvXMLImportContext* SchXMLListItemContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    SvXMLImportContext* pContext = 0;
    if( nPrefix == XML_NAMESPACE_TEXT && IsXMLToken( rLocalName, XML_P ) )
        pContext = new SchXMLParagraphContext( GetImport(), rLocalName, m_rText );
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}

//-------------------------------------------------

SchXMLTextListContext::SchXMLTextListContext(
        SvXMLImport& rImport
        , const OUString& rLocalName
        , Sequence< OUString>& rTextList )
        : SvXMLImportContext( rImport, XML_NAMESPACE_TEXT, rLocalName )
        , m_rTextList( rTextList )
        , m_aTextVector()
{
}

SchXMLTextListContext::~SchXMLTextListContext()
{
}

void SchXMLTextListContext::StartElement( const Reference< xml::sax::XAttributeList >& /*xAttrList*/ )
{
}

void SchXMLTextListContext::EndElement()
{
    sal_Int32 nCount = m_aTextVector.size();
    m_rTextList.realloc(nCount);
    for( sal_Int32 nN=0; nN<nCount; nN++ )
        m_rTextList[nN]=m_aTextVector[nN];
}

SvXMLImportContext* SchXMLTextListContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    SvXMLImportContext* pContext = 0;
    if( nPrefix == XML_NAMESPACE_TEXT && IsXMLToken( rLocalName, XML_LIST_ITEM ) )
    {
        m_aTextVector.push_back( OUString() );
        pContext = new SchXMLListItemContext( GetImport(), rLocalName, m_aTextVector.back() );
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}
