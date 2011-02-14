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

#include <xmloff/xmlscripti.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include "xmlbasici.hxx"

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>

using ::rtl::OUString;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::document;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;


// =============================================================================
// XMLScriptChildContext: context for <office:script> element
// =============================================================================

class XMLScriptChildContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts >    m_xDocumentScripts;
    ::rtl::OUString m_aLanguage;

public:
    XMLScriptChildContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& rxModel,
        const ::rtl::OUString& rLanguage );
    virtual ~XMLScriptChildContext();

    virtual SvXMLImportContext* CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();
};

// -----------------------------------------------------------------------------

XMLScriptChildContext::XMLScriptChildContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const Reference< frame::XModel >& rxModel, const ::rtl::OUString& rLanguage )
    :SvXMLImportContext( rImport, nPrfx, rLName )
    ,m_xModel( rxModel )
    ,m_xDocumentScripts( rxModel, UNO_QUERY )
    ,m_aLanguage( rLanguage )
{
}

// -----------------------------------------------------------------------------

XMLScriptChildContext::~XMLScriptChildContext()
{
}

// -----------------------------------------------------------------------------

SvXMLImportContext* XMLScriptChildContext::CreateChildContext(
    sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( m_xDocumentScripts.is() )
    {   // document supports embedding scripts/macros
        ::rtl::OUString aBasic( GetImport().GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_OOO ) );
        aBasic += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":Basic" ) );

        if ( m_aLanguage == aBasic && nPrefix == XML_NAMESPACE_OOO && IsXMLToken( rLocalName, XML_LIBRARIES ) )
            pContext = new XMLBasicImportContext( GetImport(), nPrefix, rLocalName, m_xModel );
    }

    if ( !pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

// -----------------------------------------------------------------------------

void XMLScriptChildContext::EndElement()
{
}

// =============================================================================
// XMLScriptContext: context for <office:scripts> element
// =============================================================================

XMLScriptContext::XMLScriptContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference<XModel>& rDocModel )
    :SvXMLImportContext( rImport, nPrfx, rLName )
    ,m_xModel( rDocModel )
{
}

// -----------------------------------------------------------------------------

XMLScriptContext::~XMLScriptContext()
{
}

// -----------------------------------------------------------------------------

SvXMLImportContext* XMLScriptContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( nPrefix == XML_NAMESPACE_OFFICE )
    {
        if ( IsXMLToken( rLName, XML_EVENT_LISTENERS ) )
        {
            Reference< XEventsSupplier> xSupplier( GetImport().GetModel(), UNO_QUERY );
            pContext = new XMLEventsImportContext( GetImport(), nPrefix, rLName, xSupplier );
        }
        else if ( IsXMLToken( rLName, XML_SCRIPT ) )
        {
            ::rtl::OUString aAttrName( GetImport().GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_SCRIPT ) );
            aAttrName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":language" ) );
            if ( xAttrList.is() )
            {
                ::rtl::OUString aLanguage = xAttrList->getValueByName( aAttrName );

                if ( m_xModel.is() )
                {
                    uno::Sequence< beans::PropertyValue > aMedDescr = m_xModel->getArgs();
                    sal_Int32 nNewLen = aMedDescr.getLength() + 1;
                    aMedDescr.realloc( nNewLen );
                    aMedDescr[nNewLen-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BreakMacroSignature" ) );
                    aMedDescr[nNewLen-1].Value <<= (sal_Bool)sal_True;
                    m_xModel->attachResource( m_xModel->getURL(), aMedDescr );

                    pContext = new XMLScriptChildContext( GetImport(), nPrefix, rLName, m_xModel, aLanguage );
                }
            }
        }
    }

    if ( !pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLName, xAttrList);

    return pContext;
}

// -----------------------------------------------------------------------------

void XMLScriptContext::EndElement()
{
}

// -----------------------------------------------------------------------------
