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

#include "oox/core/fastparser.hxx"

#include "oox/core/fasttokenhandler.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/helper.hxx"
#include "oox/helper/storagebase.hxx"
#include "oox/token/namespacemap.hxx"

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::rtl::OUString;

// ============================================================================

namespace {

class InputStreamCloseGuard
{
public:
    explicit            InputStreamCloseGuard( const Reference< XInputStream >& rxInStream, bool bCloseStream );
                        ~InputStreamCloseGuard();
private:
    Reference< XInputStream > mxInStream;
    bool                mbCloseStream;
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

// ============================================================================

FastParser::FastParser( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    mrNamespaceMap( StaticNamespaceMap::get() )
{
    // create a fast parser instance
    Reference< XMultiComponentFactory > xFactory( rxContext->getServiceManager(), UNO_SET_THROW );
    mxParser.set( xFactory->createInstanceWithContext( CREATE_OUSTRING( "com.sun.star.xml.sax.FastParser" ), rxContext ), UNO_QUERY_THROW );

    // create the fast token handler based on the OOXML token list
    mxParser->setTokenHandler( new FastTokenHandler );
}

FastParser::~FastParser()
{
}

void FastParser::registerNamespace( sal_Int32 nNamespaceId ) throw( IllegalArgumentException, RuntimeException )
{
    if( !mxParser.is() )
        throw RuntimeException();

    const OUString* pNamespaceUrl = ContainerHelper::getMapElement( mrNamespaceMap, nNamespaceId );
    if( !pNamespaceUrl )
        throw IllegalArgumentException();

    mxParser->registerNamespace( *pNamespaceUrl, nNamespaceId );
}

void FastParser::setDocumentHandler( const Reference< XFastDocumentHandler >& rxDocHandler ) throw( RuntimeException )
{
    if( !mxParser.is() )
        throw RuntimeException();
    mxParser->setFastDocumentHandler( rxDocHandler );
}

void FastParser::parseStream( const InputSource& rInputSource, bool bCloseStream ) throw( SAXException, IOException, RuntimeException )
{
    // guard closing the input stream also when exceptions are thrown
    InputStreamCloseGuard aGuard( rInputSource.aInputStream, bCloseStream );
    if( !mxParser.is() )
        throw RuntimeException();
    mxParser->parseStream( rInputSource );
}

void FastParser::parseStream( const Reference< XInputStream >& rxInStream, const OUString& rStreamName, bool bCloseStream ) throw( SAXException, IOException, RuntimeException )
{
    InputSource aInputSource;
    aInputSource.sSystemId = rStreamName;
    aInputSource.aInputStream = rxInStream;
    parseStream( aInputSource, bCloseStream );
}

void FastParser::parseStream( StorageBase& rStorage, const OUString& rStreamName, bool bCloseStream ) throw( SAXException, IOException, RuntimeException )
{
    parseStream( rStorage.openInputStream( rStreamName ), rStreamName, bCloseStream );
}

// ============================================================================

} // namespace core
} // namespace oox
