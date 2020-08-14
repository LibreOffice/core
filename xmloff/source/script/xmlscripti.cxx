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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include "xmlbasicscript.hxx"

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

namespace {

class XMLScriptChildContext : public SvXMLImportContext
{
private:
    css::uno::Reference< css::frame::XModel >                 m_xModel;
    css::uno::Reference< css::document::XEmbeddedScripts >    m_xDocumentScripts;
    OUString m_aLanguage;

public:
    XMLScriptChildContext( SvXMLImport& rImport,
        const css::uno::Reference< css::frame::XModel>& rxModel,
        const OUString& rLanguage );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
};

}

XMLScriptChildContext::XMLScriptChildContext( SvXMLImport& rImport,
        const Reference< frame::XModel >& rxModel, const OUString& rLanguage )
    :SvXMLImportContext( rImport )
    ,m_xModel( rxModel )
    ,m_xDocumentScripts( rxModel, UNO_QUERY )
    ,m_aLanguage( rLanguage )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLScriptChildContext::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if ( m_xDocumentScripts.is() )
    {   // document supports embedding scripts/macros
        OUString aBasic( GetImport().GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_OOO ) + ":Basic" );

        if ( m_aLanguage == aBasic && nElement == XML_ELEMENT(OOO, XML_LIBRARIES) )
        {
            return new xmloff::BasicLibrariesElement( GetImport(), m_xModel );
        }
    }

    return nullptr;
}

// XMLScriptContext: context for <office:scripts> element

XMLScriptContext::XMLScriptContext( SvXMLImport& rImport,
        const Reference<XModel>& rDocModel )
    :SvXMLImportContext( rImport )
    ,m_xModel( rDocModel )
{
}

XMLScriptContext::~XMLScriptContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLScriptContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(OFFICE, XML_SCRIPT) )
    {
        if ( m_xModel.is() )
        {
            OUString aLanguage = xAttrList->getValue( XML_ELEMENT(SCRIPT, XML_LANGUAGE) );

            uno::Sequence< beans::PropertyValue > aMedDescr = m_xModel->getArgs();
            sal_Int32 nNewLen = aMedDescr.getLength() + 1;
            aMedDescr.realloc( nNewLen );
            aMedDescr[nNewLen-1].Name = "BreakMacroSignature";
            aMedDescr[nNewLen-1].Value <<= true;
            m_xModel->attachResource( m_xModel->getURL(), aMedDescr );

            return new XMLScriptChildContext( GetImport(), m_xModel, aLanguage );
        }
    }
    else if ( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        Reference< XEventsSupplier> xSupplier( GetImport().GetModel(), UNO_QUERY );
        return new XMLEventsImportContext( GetImport(), xSupplier );
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
