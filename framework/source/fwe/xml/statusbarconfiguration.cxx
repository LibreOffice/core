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

#include <framework/statusbarconfiguration.hxx>
#include <xml/statusbardocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>
#include <services.h>

#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;

namespace framework
{

bool StatusBarConfiguration::LoadStatusBar(
    const Reference< XComponentContext >& rxContext,
    const Reference< XInputStream >& xInputStream,
    const Reference< XIndexContainer >& rStatusbarConfiguration )
{
    Reference< XParser > xParser = Parser::create(rxContext);

    // connect stream to input stream to the parser
    InputSource aInputSource;
    aInputSource.aInputStream = xInputStream;

    // create namespace filter and set menudocument handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadStatusBarDocumentHandler( rStatusbarConfiguration ));
    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return true;
    }
    catch ( const RuntimeException& )
    {
        return false;
    }
    catch( const SAXException& )
    {
        return false;
    }
    catch( const css::io::IOException& )
    {
        return false;
    }
}

bool StatusBarConfiguration::StoreStatusBar(
    const Reference< XComponentContext >& rxContext,
    const Reference< XOutputStream >& xOutputStream,
    const Reference< XIndexAccess >& rStatusbarConfiguration )
{
    Reference< XWriter > xWriter = Writer::create( rxContext );
    xWriter->setOutputStream( xOutputStream );

    try
    {
        OWriteStatusBarDocumentHandler aWriteStatusBarDocumentHandler( rStatusbarConfiguration, xWriter );
        aWriteStatusBarDocumentHandler.WriteStatusBarDocument();
        return true;
    }
    catch ( const RuntimeException& )
    {
        return false;
    }
    catch ( const SAXException& )
    {
        return false;
    }
    catch ( const css::io::IOException& )
    {
        return false;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
