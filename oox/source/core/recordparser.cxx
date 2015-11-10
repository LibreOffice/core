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

#include "oox/core/recordparser.hxx"

#include <vector>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>
#include "oox/core/fragmenthandler.hxx"

namespace oox {
namespace core {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace prv {

class Locator : public ::cppu::WeakImplHelper< XLocator >
{
public:
    inline explicit         Locator( RecordParser* pParser ) : mpParser( pParser ) {}

    void                    dispose();
    void                    checkDispose() throw( RuntimeException );

    // com.sun.star.sax.XLocator interface

    virtual sal_Int32 SAL_CALL getColumnNumber() throw( RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL getLineNumber() throw( RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getPublicId() throw( RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getSystemId() throw( RuntimeException, std::exception ) override;

private:
    RecordParser*           mpParser;
};

void Locator::dispose()
{
    mpParser = nullptr;
}

void Locator::checkDispose() throw( RuntimeException )
{
    if( !mpParser )
        throw DisposedException();
}

sal_Int32 SAL_CALL Locator::getColumnNumber() throw( RuntimeException, std::exception )
{
    return -1;
}

sal_Int32 SAL_CALL Locator::getLineNumber() throw( RuntimeException, std::exception )
{
    return -1;
}

OUString SAL_CALL Locator::getPublicId() throw( RuntimeException, std::exception )
{
    checkDispose();
    return mpParser->getInputSource().maPublicId;
}

OUString SAL_CALL Locator::getSystemId() throw( RuntimeException, std::exception )
{
    checkDispose();
    return mpParser->getInputSource().maSystemId;
}

class ContextStack
{
public:
    explicit            ContextStack( FragmentHandlerRef xHandler );

    inline bool         empty() const { return maStack.empty(); }

    sal_Int32           getCurrentRecId() const;
    bool                hasCurrentEndRecId() const;
    ContextHandlerRef   getCurrentContext() const;

    void                pushContext( const RecordInfo& rRec, const ContextHandlerRef& rxContext );
    void                popContext();

private:
    typedef ::std::pair< RecordInfo, ContextHandlerRef >    ContextInfo;
    typedef ::std::vector< ContextInfo >                    ContextInfoVec;

    FragmentHandlerRef  mxHandler;
    ContextInfoVec      maStack;
};

ContextStack::ContextStack( FragmentHandlerRef xHandler ) :
    mxHandler( xHandler )
{
}

sal_Int32 ContextStack::getCurrentRecId() const
{
    return maStack.empty() ? -1 : maStack.back().first.mnStartRecId;
}

bool ContextStack::hasCurrentEndRecId() const
{
    return !maStack.empty() && (maStack.back().first.mnEndRecId >= 0);
}

ContextHandlerRef ContextStack::getCurrentContext() const
{
    if( !maStack.empty() )
        return maStack.back().second;
    return mxHandler.get();
}

void ContextStack::pushContext( const RecordInfo& rRecInfo, const ContextHandlerRef& rxContext )
{
    OSL_ENSURE( (rRecInfo.mnEndRecId >= 0) || maStack.empty() || hasCurrentEndRecId(),
        "ContextStack::pushContext - nested incomplete context record identifiers" );
    maStack.push_back( ContextInfo( rRecInfo, rxContext ) );
}

void ContextStack::popContext()
{
    OSL_ENSURE( !maStack.empty(), "ContextStack::popContext - no context on stack" );
    if( !maStack.empty() )
    {
        ContextInfo& rContextInfo = maStack.back();
        if( rContextInfo.second.is() )
            rContextInfo.second->endRecord( rContextInfo.first.mnStartRecId );
        maStack.pop_back();
    }
}

} // namespace prv

namespace {

/** Reads a byte from the passed stream, returns true on success. */
inline bool lclReadByte( sal_uInt8& ornByte, BinaryInputStream& rStrm )
{
    return rStrm.readMemory( &ornByte, 1 ) == 1;
}

/** Reads a compressed signed 32-bit integer from the passed stream. */
bool lclReadCompressedInt( sal_Int32& ornValue, BinaryInputStream& rStrm )
{
    ornValue = 0;
    sal_uInt8 nByte;
    if( !lclReadByte( nByte, rStrm ) ) return false;
    ornValue = nByte & 0x7F;
    if( (nByte & 0x80) == 0 ) return true;
    if( !lclReadByte( nByte, rStrm ) ) return false;
    ornValue |= sal_Int32( nByte & 0x7F ) << 7;
    if( (nByte & 0x80) == 0 ) return true;
    if( !lclReadByte( nByte, rStrm ) ) return false;
    ornValue |= sal_Int32( nByte & 0x7F ) << 14;
    if( (nByte & 0x80) == 0 ) return true;
    if( !lclReadByte( nByte, rStrm ) ) return false;
    ornValue |= sal_Int32( nByte & 0x7F ) << 21;
    return true;
}

bool lclReadRecordHeader( sal_Int32& ornRecId, sal_Int32& ornRecSize, BinaryInputStream& rStrm )
{
    return
        lclReadCompressedInt( ornRecId, rStrm ) && (ornRecId >= 0) &&
        lclReadCompressedInt( ornRecSize, rStrm ) && (ornRecSize >= 0);
}

bool lclReadNextRecord( sal_Int32& ornRecId, StreamDataSequence& orData, BinaryInputStream& rStrm )
{
    sal_Int32 nRecSize = 0;
    bool bValid = lclReadRecordHeader( ornRecId, nRecSize, rStrm );
    if( bValid )
    {
        orData.realloc( nRecSize );
        bValid = (nRecSize == 0) || (rStrm.readData( orData, nRecSize ) == nRecSize);
    }
    return bValid;
}

} // namespace

RecordParser::RecordParser()
{
    mxLocator.set( new prv::Locator( this ) );
}

RecordParser::~RecordParser()
{
    if( mxLocator.is() )
        mxLocator->dispose();
}

void RecordParser::setFragmentHandler( const ::rtl::Reference< FragmentHandler >& rxHandler )
{
    mxHandler = rxHandler;

    // build record infos
    maStartMap.clear();
    maEndMap.clear();
    const RecordInfo* pRecs = mxHandler.is() ? mxHandler->getRecordInfos() : nullptr;
    OSL_ENSURE( pRecs, "RecordInfoProvider::RecordInfoProvider - missing record list" );
    for( ; pRecs && pRecs->mnStartRecId >= 0; ++pRecs )
    {
        maStartMap[ pRecs->mnStartRecId ] = *pRecs;
        if( pRecs->mnEndRecId >= 0 )
            maEndMap[ pRecs->mnEndRecId ] = *pRecs;
    }
}

void RecordParser::parseStream( const RecordInputSource& rInputSource ) throw( SAXException, IOException, RuntimeException )
{
    maSource = rInputSource;

    if( !maSource.mxInStream || maSource.mxInStream->isEof() )
        throw IOException();
    if( !mxHandler.is() )
        throw SAXException();

    // start the document
    Reference< XLocator > xLocator( mxLocator.get() );
    mxHandler->setDocumentLocator( xLocator );
    mxHandler->startDocument();

    // parse the stream
    mxStack.reset( new prv::ContextStack( mxHandler ) );
    sal_Int32 nRecId = 0;
    StreamDataSequence aRecData;
    while( lclReadNextRecord( nRecId, aRecData, *maSource.mxInStream ) )
    {
        // create record stream object from imported record data
        SequenceInputStream aRecStrm( aRecData );
        // try to leave a context, there may be other incomplete contexts on the stack
        if( const RecordInfo* pEndRecInfo = getEndRecordInfo( nRecId ) )
        {
            // finalize contexts without record identifier for context end
            while( !mxStack->empty() && !mxStack->hasCurrentEndRecId() )
                mxStack->popContext();
            // finalize the current context and pop context info from stack
            OSL_ENSURE( mxStack->getCurrentRecId() == pEndRecInfo->mnStartRecId, "RecordParser::parseStream - context records mismatch" );
            (void)pEndRecInfo;  // suppress compiler warning for unused variable
            ContextHandlerRef xCurrContext = mxStack->getCurrentContext();
            if( xCurrContext.is() )
            {
                // context end record may contain some data, handle it as simple record
                aRecStrm.seekToStart();
                xCurrContext->startRecord( nRecId, aRecStrm );
                xCurrContext->endRecord( nRecId );
            }
            mxStack->popContext();
        }
        else
        {
            // end context with incomplete record id, if the same id comes again
            if( (mxStack->getCurrentRecId() == nRecId) && !mxStack->hasCurrentEndRecId() )
                mxStack->popContext();
            // try to start a new context
            ContextHandlerRef xCurrContext = mxStack->getCurrentContext();
            if( xCurrContext.is() )
            {
                aRecStrm.seekToStart();
                xCurrContext = xCurrContext->createRecordContext( nRecId, aRecStrm );
            }
            // track all context identifiers on the stack (do not push simple records)
            const RecordInfo* pStartRecInfo = getStartRecordInfo( nRecId );
            if( pStartRecInfo )
                mxStack->pushContext( *pStartRecInfo, xCurrContext );
            // import the record
            if( xCurrContext.is() )
            {
                // import the record
                aRecStrm.seekToStart();
                xCurrContext->startRecord( nRecId, aRecStrm );
                // end simple records (context records are finished in ContextStack::popContext)
                if( !pStartRecInfo )
                    xCurrContext->endRecord( nRecId );
            }
        }
    }
    // close remaining contexts (missing context end records or stream error)
    while( !mxStack->empty() )
        mxStack->popContext();
    mxStack.reset();

    // finish document
    mxHandler->endDocument();

    maSource = RecordInputSource();
}

const RecordInfo* RecordParser::getStartRecordInfo( sal_Int32 nRecId ) const
{
    RecordInfoMap::const_iterator aIt = maStartMap.find( nRecId );
    return (aIt == maStartMap.end()) ? nullptr : &aIt->second;
}

const RecordInfo* RecordParser::getEndRecordInfo( sal_Int32 nRecId ) const
{
    RecordInfoMap::const_iterator aIt = maEndMap.find( nRecId );
    return (aIt == maEndMap.end()) ? nullptr : &aIt->second;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
