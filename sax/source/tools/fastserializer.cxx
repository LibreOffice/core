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

#include "fastserializer.hxx"
#include <rtl/ustrbuf.hxx>

#include <comphelper/sequenceasvector.hxx>

#include <com/sun/star/xml/Attribute.hpp>
#include <com/sun/star/xml/FastAttribute.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include <string.h>

#if DEBUG
#include <iostream>
#endif

using ::comphelper::SequenceAsVector;
using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringToOString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::toUnoSequence;
using ::com::sun::star::xml::FastAttribute;
using ::com::sun::star::xml::Attribute;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::NotConnectedException;
using ::com::sun::star::io::IOException;
using ::com::sun::star::io::BufferSizeExceededException;

#define HAS_NAMESPACE(x) ((x & 0xffff0000) != 0)
#define NAMESPACE(x) (x >> 16)
#define TOKEN(x) (x & 0xffff)

namespace sax_fastparser {
    FastSaxSerializer::FastSaxSerializer( )
        : mxOutputStream()
        , mxFastTokenHandler()
        , maMarkStack()
        , maClosingBracket((const sal_Int8 *)">", 1)
        , maSlashAndClosingBracket((const sal_Int8 *)"/>", 2)
        , maColon((const sal_Int8 *)":", 1)
        , maOpeningBracket((const sal_Int8 *)"<", 1)
        , maOpeningBracketAndSlash((const sal_Int8 *)"</", 2)
        , maQuote((const sal_Int8 *)"\"", 1)
        , maEqualSignAndQuote((const sal_Int8 *)"=\"", 2)
        , maSpace((const sal_Int8 *)" ", 1)
    {
    }
    FastSaxSerializer::~FastSaxSerializer() {}

    void SAL_CALL FastSaxSerializer::startDocument(  ) throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;
        rtl::ByteSequence aXmlHeader((const sal_Int8*) "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n", 56);
        writeBytes(toUnoSequence(aXmlHeader));
    }

    OUString FastSaxSerializer::escapeXml( const OUString& s )
    {
        ::rtl::OUStringBuffer sBuf( s.getLength() );
        const sal_Unicode* pStr = s.getStr();
        sal_Int32 nLen = s.getLength();
        for( sal_Int32 i = 0; i < nLen; ++i)
        {
            sal_Unicode c = pStr[ i ];
            switch( c )
            {
                case '<':   sBuf.appendAscii( "&lt;" );     break;
                case '>':   sBuf.appendAscii( "&gt;" );     break;
                case '&':   sBuf.appendAscii( "&amp;" );    break;
                case '\'':  sBuf.appendAscii( "&apos;" );   break;
                case '"':   sBuf.appendAscii( "&quot;" );   break;
                case '\n':  sBuf.appendAscii( "&#10;" );    break;
                case '\r':  sBuf.appendAscii( "&#13;" );    break;
                default:    sBuf.append( c );               break;
            }
        }
        return sBuf.makeStringAndClear();
    }

    void FastSaxSerializer::write( const OUString& s )
    {
        OString sOutput( OUStringToOString( s, RTL_TEXTENCODING_UTF8 ) );
        writeBytes( Sequence< sal_Int8 >(
                    reinterpret_cast< const sal_Int8*>( sOutput.getStr() ),
                    sOutput.getLength() ) );
    }

    void SAL_CALL FastSaxSerializer::endDocument(  ) throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;
    }

    void SAL_CALL FastSaxSerializer::writeId( ::sal_Int32 nElement )
    {
        if( HAS_NAMESPACE( nElement ) ) {
            writeBytes(mxFastTokenHandler->getUTF8Identifier(NAMESPACE(nElement)));
            writeBytes(toUnoSequence(maColon));
            writeBytes(mxFastTokenHandler->getUTF8Identifier(TOKEN(nElement)));
        } else
            writeBytes(mxFastTokenHandler->getUTF8Identifier(nElement));
    }

    void SAL_CALL FastSaxSerializer::startFastElement( ::sal_Int32 Element, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        if ( !maMarkStack.empty() )
            maMarkStack.top()->setCurrentElement( Element );

        writeBytes(toUnoSequence(maOpeningBracket));

        writeId(Element);
        writeFastAttributeList(Attribs);

        writeBytes(toUnoSequence(maClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::endFastElement( ::sal_Int32 Element )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(toUnoSequence(maOpeningBracketAndSlash));

        writeId(Element);

        writeBytes(toUnoSequence(maClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::singleFastElement( ::sal_Int32 Element, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        if ( !maMarkStack.empty() )
            maMarkStack.top()->setCurrentElement( Element );

        writeBytes(toUnoSequence(maOpeningBracket));

        writeId(Element);
        writeFastAttributeList(Attribs);

        writeBytes(toUnoSequence(maSlashAndClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::characters( const OUString& aChars )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        write( aChars );
    }

    void SAL_CALL FastSaxSerializer::setOutputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream )
        throw (::com::sun::star::uno::RuntimeException)
    {
        mxOutputStream = xOutputStream;
    }

    void SAL_CALL FastSaxSerializer::setFastTokenHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& xFastTokenHandler )
        throw (::com::sun::star::uno::RuntimeException)
    {
        mxFastTokenHandler = xFastTokenHandler;
    }
    void FastSaxSerializer::writeFastAttributeList( const Reference< XFastAttributeList >& Attribs )
    {
        Sequence< Attribute > aAttrSeq = Attribs->getUnknownAttributes();
        const Attribute *pAttr = aAttrSeq.getConstArray();
        sal_Int32 nAttrLength = aAttrSeq.getLength();
        for (sal_Int32 i = 0; i < nAttrLength; i++)
        {
            writeBytes(toUnoSequence(maSpace));

            write(pAttr[i].Name);
            writeBytes(toUnoSequence(maEqualSignAndQuote));
            write(escapeXml(pAttr[i].Value));
            writeBytes(toUnoSequence(maQuote));
        }

        Sequence< FastAttribute > aFastAttrSeq = Attribs->getFastAttributes();
        const FastAttribute *pFastAttr = aFastAttrSeq.getConstArray();
        sal_Int32 nFastAttrLength = aFastAttrSeq.getLength();
        for (sal_Int32 j = 0; j < nFastAttrLength; j++)
        {
            writeBytes(toUnoSequence(maSpace));

            sal_Int32 nToken = pFastAttr[j].Token;
            writeId(nToken);

            writeBytes(toUnoSequence(maEqualSignAndQuote));

            write(escapeXml(Attribs->getValue(pFastAttr[j].Token)));

            writeBytes(toUnoSequence(maQuote));
        }
    }

    void FastSaxSerializer::mark( Int32Sequence aOrder )
    {
        if ( aOrder.hasElements() )
        {
            boost::shared_ptr< ForMerge > pSort( new ForSort( aOrder ) );
            maMarkStack.push( pSort );
        }
        else
        {
            boost::shared_ptr< ForMerge > pMerge( new ForMerge( ) );
            maMarkStack.push( pMerge );
        }
    }

    void FastSaxSerializer::mergeTopMarks( sax_fastparser::MergeMarksEnum eMergeType )
    {
        if ( maMarkStack.empty() )
            return;

        if ( maMarkStack.size() == 1 )
        {
            mxOutputStream->writeBytes( maMarkStack.top()->getData() );
            maMarkStack.pop();
            return;
        }

        const Int8Sequence aMerge( maMarkStack.top()->getData() );
        maMarkStack.pop();

        switch ( eMergeType )
        {
            case MERGE_MARKS_APPEND:   maMarkStack.top()->append( aMerge );   break;
            case MERGE_MARKS_PREPEND:  maMarkStack.top()->prepend( aMerge );  break;
            case MERGE_MARKS_POSTPONE: maMarkStack.top()->postpone( aMerge ); break;
        }
    }

    void FastSaxSerializer::writeBytes( const Sequence< ::sal_Int8 >& aData ) throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
    {
        if ( maMarkStack.empty() )
            mxOutputStream->writeBytes( aData );
        else
            maMarkStack.top()->append( aData );
    }

    FastSaxSerializer::Int8Sequence& FastSaxSerializer::ForMerge::getData()
    {
        merge( maData, maPostponed, true );
        maPostponed.realloc( 0 );

        return maData;
    }

#if DEBUG
    void FastSaxSerializer::ForMerge::print( )
    {
        std::cerr << "Data: ";
        for ( sal_Int32 i=0, len=maData.getLength(); i < len; i++ )
        {
            std::cerr << maData[i];
        }

        std::cerr << "\nPostponed: ";
        for ( sal_Int32 i=0, len=maPostponed.getLength(); i < len; i++ )
        {
            std::cerr << maPostponed[i];
        }

        std::cerr << "\n";
    }
#endif

    void FastSaxSerializer::ForMerge::prepend( const Int8Sequence &rWhat )
    {
        merge( maData, rWhat, false );
    }

    void FastSaxSerializer::ForMerge::append( const Int8Sequence &rWhat )
    {
        merge( maData, rWhat, true );
    }

    void FastSaxSerializer::ForMerge::postpone( const Int8Sequence &rWhat )
    {
        merge( maPostponed, rWhat, true );
    }

    void FastSaxSerializer::ForMerge::merge( Int8Sequence &rTop, const Int8Sequence &rMerge, bool bAppend )
    {
        sal_Int32 nMergeLen = rMerge.getLength();
        if ( nMergeLen > 0 )
        {
            sal_Int32 nTopLen = rTop.getLength();

            rTop.realloc( nTopLen + nMergeLen );
            if ( bAppend )
            {
                // append the rMerge to the rTop
                memcpy( rTop.getArray() + nTopLen, rMerge.getConstArray(), nMergeLen );
            }
            else
            {
                // prepend the rMerge to the rTop
                memmove( rTop.getArray() + nMergeLen, rTop.getConstArray(), nTopLen );
                memcpy( rTop.getArray(), rMerge.getConstArray(), nMergeLen );
            }
        }
    }

    void FastSaxSerializer::ForMerge::resetData( )
    {
        maData = Int8Sequence();
    }

    void FastSaxSerializer::ForSort::setCurrentElement( sal_Int32 nElement )
    {
        SequenceAsVector< sal_Int32 > aOrder( maOrder );
        if( std::find( aOrder.begin(), aOrder.end(), nElement ) != aOrder.end() )
        {
            mnCurrentElement = nElement;
            if ( maData.find( nElement ) == maData.end() )
                maData[ nElement ] = Int8Sequence();
        }
    }

    void FastSaxSerializer::ForSort::prepend( const Int8Sequence &rWhat )
    {
        append( rWhat );
    }

    void FastSaxSerializer::ForSort::append( const Int8Sequence &rWhat )
    {
        merge( maData[mnCurrentElement], rWhat, true );
    }

    void FastSaxSerializer::ForSort::sort()
    {
        // Clear the ForMerge data to avoid duplicate items
        resetData();

        // Sort it all
        std::map< sal_Int32, Int8Sequence >::iterator iter;
        for ( sal_Int32 i=0, len=maOrder.getLength(); i < len; i++ )
        {
            iter = maData.find( maOrder[i] );
            if ( iter != maData.end() )
                ForMerge::append( iter->second );
        }
    }

    FastSaxSerializer::Int8Sequence& FastSaxSerializer::ForSort::getData()
    {
        sort( );
        return ForMerge::getData();
    }

#if DEBUG
    void FastSaxSerializer::ForSort::print( )
    {
        std::map< sal_Int32, Int8Sequence >::iterator iter = maData.begin();
        while ( iter != maData.end( ) )
        {
            std::cerr << "pair: " << iter->first;
            for ( sal_Int32 i=0, len=iter->second.getLength(); i < len; ++i )
                std::cerr << iter->second[i];
            std::cerr << "\n";
            ++iter;
        }

        sort( );
        ForMerge::print();
    }
#endif

} // namespace sax_fastparser

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
