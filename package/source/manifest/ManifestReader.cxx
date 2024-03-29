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

#include "ManifestReader.hxx"
#include "ManifestImport.hxx"
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <vector>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::packages;
using namespace ::com::sun::star::xml::sax;

ManifestReader::ManifestReader( const Reference < XComponentContext > & xContext )
: m_xContext ( xContext )
{
}
ManifestReader::~ManifestReader()
{
}
Sequence< Sequence< PropertyValue > > SAL_CALL ManifestReader::readManifestSequence( const Reference< XInputStream >& rStream )
{
    Sequence < Sequence < PropertyValue > > aManifestSequence;
    Reference < XParser > xParser  = Parser::create(m_xContext);
    try
    {
        std::vector < Sequence < PropertyValue > > aManVector;
        Reference < XDocumentHandler > xFilter = new ManifestImport( aManVector );
        InputSource aParserInput;
        aParserInput.aInputStream = rStream;
        aParserInput.sSystemId = "META-INF/manifest.xml";
        xParser->setDocumentHandler ( xFilter );
        xParser->parseStream( aParserInput );
        aManifestSequence = comphelper::containerToSequence(aManVector);
    }
    catch (const SAXParseException&)
    {
        TOOLS_WARN_EXCEPTION("package", "ignoring");
    }
    catch (const SAXException&)
    {
        TOOLS_WARN_EXCEPTION("package", "ignoring");
    }
    catch (const IOException&)
    {
        TOOLS_WARN_EXCEPTION("package", "ignoring");
    }
    xParser->setDocumentHandler ( Reference < XDocumentHandler > () );
    return aManifestSequence;
}
// Component functions


OUString ManifestReader::getImplementationName()
{
    return "com.sun.star.packages.manifest.comp.ManifestReader";
}

sal_Bool SAL_CALL ManifestReader::supportsService(OUString const & rServiceName)
{
    return cppu::supportsService(this, rServiceName );
}

Sequence < OUString > ManifestReader::getSupportedServiceNames()
{
    return { "com.sun.star.packages.manifest.ManifestReader" };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
package_ManifestReader_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ManifestReader(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
