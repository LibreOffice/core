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

#include <xmloff/xmlscripti.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include "xmlbasici.hxx"

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::document;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;

// XMLScriptChildContext: context for <office:script> element

class XMLScriptChildContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts >    m_xDocumentScripts;
    OUString m_aLanguage;

public:
    XMLScriptChildContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& rxModel,
        const OUString& rLanguage );
    virtual ~XMLScriptChildContext();

    virtual SvXMLImportContext* CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();
};

XMLScriptChildContext::XMLScriptChildContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< frame::XModel >& rxModel, const OUString& rLanguage )
    :SvXMLImportContext( rImport, nPrfx, rLName )
    ,m_xModel( rxModel )
    ,m_xDocumentScripts( rxModel, UNO_QUERY )
    ,m_aLanguage( rLanguage )
{
}

XMLScriptChildContext::~XMLScriptChildContext()
{
}

SvXMLImportContext* XMLScriptChildContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( m_xDocumentScripts.is() )
    {   // document supports embedding scripts/macros
        OUString aBasic( GetImport().GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_OOO ) );
        aBasic += ":Basic";

        if ( m_aLanguage == aBasic && nPrefix == XML_NAMESPACE_OOO && IsXMLToken( rLocalName, XML_LIBRARIES ) )
            pContext = new XMLBasicImportContext( GetImport(), nPrefix, rLocalName, m_xModel );
    }

    if ( !pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

void XMLScriptChildContext::EndElement()
{
}

// XMLScriptContext: context for <office:scripts> element

XMLScriptContext::XMLScriptContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference<XModel>& rDocModel )
    :SvXMLImportContext( rImport, nPrfx, rLName )
    ,m_xModel( rDocModel )
{
}

XMLScriptContext::~XMLScriptContext()
{
}

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
            OUString aAttrName( GetImport().GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_SCRIPT ) );
            aAttrName += ":language";
            if ( xAttrList.is() )
            {
                OUString aLanguage = xAttrList->getValueByName( aAttrName );

                if ( m_xModel.is() )
                {
                    uno::Sequence< beans::PropertyValue > aMedDescr = m_xModel->getArgs();
                    sal_Int32 nNewLen = aMedDescr.getLength() + 1;
                    aMedDescr.realloc( nNewLen );
                    aMedDescr[nNewLen-1].Name = "BreakMacroSignature";
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

void XMLScriptContext::EndElement()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
