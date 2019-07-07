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

#include <sal/config.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <oox/core/fastparser.hxx>

#include <oox/core/fasttokenhandler.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/helper.hxx>
#include <oox/helper/storagebase.hxx>
#include <oox/token/namespacemap.hxx>

#include <sax/fastparser.hxx>

namespace oox {
namespace core {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace {

class InputStreamCloseGuard
{
public:
    explicit            InputStreamCloseGuard( const Reference< XInputStream >& rxInStream, bool bCloseStream );
                        ~InputStreamCloseGuard();
private:
    Reference< XInputStream > mxInStream;
    bool const          mbCloseStream;
};

InputStreamCloseGuard::InputStreamCloseGuard( const Reference< XInputStream >& rxInStream, bool bCloseStream ) :
    mxInStream( rxInStream ),
    mbCloseStream( bCloseStream )
{
}

InputStreamCloseGuard::~InputStreamCloseGuard()
{
    if( mxInStream.is() && mbCloseStream ) try { mxInStream->closeInput(); } catch( Exception& ) {}
}

} // namespace

FastParser::FastParser() :
    mrNamespaceMap( StaticNamespaceMap::get() )
{
    // create a fast parser instance
    mxParser = new sax_fastparser::FastSaxParser;

    // create the fast tokenhandler
    mxTokenHandler.set( new FastTokenHandler );

    // create the fast token handler based on the OOXML token list
    mxParser->setTokenHandler( mxTokenHandler );
}

FastParser::~FastParser()
{
}

void FastParser::registerNamespace( sal_Int32 nNamespaceId )
{
    if( !mxParser.is() )
        throw RuntimeException();

    // add handling for OOXML strict here
    const OUString* pNamespaceUrl = ContainerHelper::getMapElement( mrNamespaceMap.maTransitionalNamespaceMap, nNamespaceId );
    if( !pNamespaceUrl )
        throw IllegalArgumentException();

    mxParser->registerNamespace( *pNamespaceUrl, nNamespaceId );

    //also register the OOXML strict namespaces for the same id
    const OUString* pNamespaceStrictUrl = ContainerHelper::getMapElement( mrNamespaceMap.maStrictNamespaceMap, nNamespaceId );
    if(pNamespaceStrictUrl && (*pNamespaceUrl != *pNamespaceStrictUrl))
    {
        mxParser->registerNamespace( *pNamespaceStrictUrl, nNamespaceId );
    }
}

void FastParser::setDocumentHandler( const Reference< XFastDocumentHandler >& rxDocHandler )
{
    if( !mxParser.is() )
        throw RuntimeException();
    mxParser->setFastDocumentHandler( rxDocHandler );
}

void FastParser::clearDocumentHandler()
{
    if (!mxParser.is())
        return;
    mxParser->setFastDocumentHandler(nullptr);
}

void FastParser::parseStream( const InputSource& rInputSource, bool bCloseStream )
{
    // guard closing the input stream also when exceptions are thrown
    InputStreamCloseGuard aGuard( rInputSource.aInputStream, bCloseStream );
    if( !mxParser.is() )
        throw RuntimeException();
    mxParser->parseStream( rInputSource );
}

void FastParser::parseStream( const Reference< XInputStream >& rxInStream, const OUString& rStreamName )
{
    InputSource aInputSource;
    aInputSource.sSystemId = rStreamName;
    aInputSource.aInputStream = rxInStream;
    parseStream( aInputSource );
}

void FastParser::parseStream( StorageBase& rStorage, const OUString& rStreamName )
{
    parseStream( rStorage.openInputStream( rStreamName ), rStreamName );
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
