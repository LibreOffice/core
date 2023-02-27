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

#include "storagexmlstream.hxx"

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <rtl/ref.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::xml::sax::XDocumentHandler;
    using ::com::sun::star::xml::sax::XWriter;
    using ::com::sun::star::xml::sax::Writer;
    using ::com::sun::star::xml::sax::Parser;
    using ::com::sun::star::xml::sax::InputSource;

    // StorageXMLOutputStream
    StorageXMLOutputStream::StorageXMLOutputStream( const Reference<XComponentContext>& i_rContext,
                                                    const Reference< XStorage >& i_rParentStorage,
                                                    const OUString& i_rStreamName )
        :StorageOutputStream( i_rParentStorage, i_rStreamName )
    {
        const Reference< XWriter > xSaxWriter = Writer::create( i_rContext );
        xSaxWriter->setOutputStream( getOutputStream() );

        mxHandler.set( xSaxWriter, UNO_QUERY_THROW );
        mxHandler->startDocument();

        mxAttributes = new comphelper::AttributeList;
    }

    StorageXMLOutputStream::~StorageXMLOutputStream()
    {
    }

    void StorageXMLOutputStream::close()
    {
        ENSURE_OR_RETURN_VOID( mxHandler.is(), "illegal document handler" );
        mxHandler->endDocument();
        // do not call the base class, it would call closeOutput on the output stream, which is already done by
        // endDocument
    }

    void StorageXMLOutputStream::addAttribute( const OUString& i_rName, const OUString& i_rValue ) const
    {
        mxAttributes->AddAttribute( i_rName, i_rValue );
    }

    void StorageXMLOutputStream::startElement( const OUString& i_rElementName )
    {
        ENSURE_OR_RETURN_VOID( mxHandler.is(), "no document handler" );

        mxHandler->startElement( i_rElementName, mxAttributes );
        mxAttributes = new comphelper::AttributeList;
        maElements.push( i_rElementName );
    }

    void StorageXMLOutputStream::endElement()
    {
        ENSURE_OR_RETURN_VOID( mxHandler.is(), "no document handler" );
        ENSURE_OR_RETURN_VOID( !maElements.empty(), "no element on the stack" );

        const OUString sElementName( maElements.top() );
        mxHandler->endElement( sElementName );
        maElements.pop();
    }

    void StorageXMLOutputStream::ignorableWhitespace( const OUString& i_rWhitespace ) const
    {
        ENSURE_OR_RETURN_VOID( mxHandler.is(), "no document handler" );

        mxHandler->ignorableWhitespace( i_rWhitespace );
    }

    void StorageXMLOutputStream::characters( const OUString& i_rCharacters ) const
    {
        ENSURE_OR_RETURN_VOID( mxHandler.is(), "no document handler" );

        mxHandler->characters( i_rCharacters );
    }

    // StorageXMLInputStream
    StorageXMLInputStream::StorageXMLInputStream( const Reference<XComponentContext>& i_rContext,
                                                  const Reference< XStorage >& i_rParentStorage,
                                                  const OUString& i_rStreamName )
    {
        ENSURE_OR_THROW( i_rParentStorage.is(), "illegal stream" );

        const Reference< css::io::XStream > xStream(
            i_rParentStorage->openStreamElement( i_rStreamName, css::embed::ElementModes::READ ), css::uno::UNO_SET_THROW );
        m_xInputStream.set( xStream->getInputStream(), css::uno::UNO_SET_THROW );

        m_xParser.set( Parser::create(i_rContext) );
    }

    void StorageXMLInputStream::import( const Reference< XDocumentHandler >& i_rHandler )
    {
        ENSURE_OR_THROW( i_rHandler.is(), "illegal document handler (NULL)" );

        InputSource aInputSource;
        aInputSource.aInputStream = m_xInputStream;

        m_xParser->setDocumentHandler( i_rHandler );
        m_xParser->parseStream( aInputSource );
    }

    StorageXMLInputStream::~StorageXMLInputStream()
    {
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
