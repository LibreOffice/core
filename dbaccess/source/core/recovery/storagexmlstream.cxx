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

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>
#include <xmloff/attrlist.hxx>

#include <stack>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::xml::sax::XDocumentHandler;
    using ::com::sun::star::xml::sax::XAttributeList;
    using ::com::sun::star::xml::sax::XWriter;
    using ::com::sun::star::xml::sax::Writer;
    using ::com::sun::star::io::XStream;
    using ::com::sun::star::io::XOutputStream;
    using ::com::sun::star::io::XActiveDataSource;
    using ::com::sun::star::xml::sax::Parser;
    using ::com::sun::star::xml::sax::XParser;
    using ::com::sun::star::xml::sax::InputSource;

    // StorageXMLOutputStream_Data
    struct StorageXMLOutputStream_Data
    {
        Reference< XDocumentHandler >           xHandler;
        ::std::stack< OUString >         aElements;
        ::rtl::Reference< SvXMLAttributeList >  xAttributes;
    };

    // StorageXMLOutputStream
    StorageXMLOutputStream::StorageXMLOutputStream( const Reference<XComponentContext>& i_rContext,
                                                    const Reference< XStorage >& i_rParentStorage,
                                                    const OUString& i_rStreamName )
        :StorageOutputStream( i_rContext, i_rParentStorage, i_rStreamName )
        ,m_pData( new StorageXMLOutputStream_Data )
    {
        const Reference< XWriter > xSaxWriter = Writer::create( i_rContext );
        xSaxWriter->setOutputStream( getOutputStream() );

        m_pData->xHandler.set( xSaxWriter, UNO_QUERY_THROW );
        m_pData->xHandler->startDocument();

        m_pData->xAttributes = new SvXMLAttributeList;
    }

    StorageXMLOutputStream::~StorageXMLOutputStream()
    {
    }

    void StorageXMLOutputStream::close()
    {
        ENSURE_OR_RETURN_VOID( m_pData->xHandler.is(), "illegal document handler" );
        m_pData->xHandler->endDocument();
        // do not call the base class, it would call closeOutput on the output stream, which is already done by
        // endDocument
    }

    void StorageXMLOutputStream::addAttribute( const OUString& i_rName, const OUString& i_rValue ) const
    {
        m_pData->xAttributes->AddAttribute( i_rName, i_rValue );
    }

    void StorageXMLOutputStream::startElement( const OUString& i_rElementName ) const
    {
        ENSURE_OR_RETURN_VOID( m_pData->xHandler.is(), "no document handler" );

        m_pData->xHandler->startElement( i_rElementName, m_pData->xAttributes.get() );
        m_pData->xAttributes = new SvXMLAttributeList;
        m_pData->aElements.push( i_rElementName );
    }

    void StorageXMLOutputStream::endElement() const
    {
        ENSURE_OR_RETURN_VOID( m_pData->xHandler.is(), "no document handler" );
        ENSURE_OR_RETURN_VOID( !m_pData->aElements.empty(), "no element on the stack" );

        const OUString sElementName( m_pData->aElements.top() );
        m_pData->xHandler->endElement( sElementName );
        m_pData->aElements.pop();
    }

    void StorageXMLOutputStream::ignorableWhitespace( const OUString& i_rWhitespace ) const
    {
        ENSURE_OR_RETURN_VOID( m_pData->xHandler.is(), "no document handler" );

        m_pData->xHandler->ignorableWhitespace( i_rWhitespace );
    }

    void StorageXMLOutputStream::characters( const OUString& i_rCharacters ) const
    {
        ENSURE_OR_RETURN_VOID( m_pData->xHandler.is(), "no document handler" );

        m_pData->xHandler->characters( i_rCharacters );
    }

    // StorageXMLInputStream_Data
    struct StorageXMLInputStream_Data
    {
        Reference< XParser >    xParser;
    };

    // StorageXMLInputStream
    StorageXMLInputStream::StorageXMLInputStream( const Reference<XComponentContext>& i_rContext,
                                                  const Reference< XStorage >& i_rParentStorage,
                                                  const OUString& i_rStreamName )
        :StorageInputStream( i_rContext, i_rParentStorage, i_rStreamName )
        ,m_pData( new StorageXMLInputStream_Data )
    {
        m_pData->xParser.set( Parser::create(i_rContext) );
    }

    void StorageXMLInputStream::import( const Reference< XDocumentHandler >& i_rHandler )
    {
        ENSURE_OR_THROW( i_rHandler.is(), "illegal document handler (NULL)" );

        InputSource aInputSource;
        aInputSource.aInputStream = getInputStream();

        m_pData->xParser->setDocumentHandler( i_rHandler );
        m_pData->xParser->parseStream( aInputSource );
    }

    StorageXMLInputStream::~StorageXMLInputStream()
    {
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
