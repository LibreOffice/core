/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "fastserializer.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.hxx>

#include <comphelper/sequenceasvector.hxx>

#include <com/sun/star/xml/Attribute.hpp>
#include <com/sun/star/xml/FastAttribute.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include <string.h>

#if DEBUG
#include <cstdio>
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
using ::com::sun::star::xml::sax::XFastTokenHandler;
using ::com::sun::star::xml::sax::XFastSerializer;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::NotConnectedException;
using ::com::sun::star::io::IOException;
using ::com::sun::star::io::BufferSizeExceededException;

static rtl::ByteSequence aClosingBracket((const sal_Int8 *)">", 1);
static rtl::ByteSequence aSlashAndClosingBracket((const sal_Int8 *)"/>", 2);
static rtl::ByteSequence aColon((const sal_Int8 *)":", 1);
static rtl::ByteSequence aOpeningBracket((const sal_Int8 *)"<", 1);
static rtl::ByteSequence aOpeningBracketAndSlash((const sal_Int8 *)"</", 2);
static rtl::ByteSequence aQuote((const sal_Int8 *)"\"", 1);
static rtl::ByteSequence aEqualSignAndQuote((const sal_Int8 *)"=\"", 2);
static rtl::ByteSequence aSpace((const sal_Int8 *)" ", 1);
static rtl::ByteSequence aXmlHeader((const sal_Int8*) "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n", 56);

#define HAS_NAMESPACE(x) ((x & 0xffff0000) != 0)
#define NAMESPACE(x) (x >> 16)
#define TOKEN(x) (x & 0xffff)

namespace sax_fastparser {
    FastSaxSerializer::FastSaxSerializer( ) : mxOutputStream(), mxFastTokenHandler(), maMarkStack() {}
    FastSaxSerializer::~FastSaxSerializer() {}

    void SAL_CALL FastSaxSerializer::startDocument(  ) throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;
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
            writeBytes(toUnoSequence(aColon));
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

        writeBytes(toUnoSequence(aOpeningBracket));

        writeId(Element);
        writeFastAttributeList(Attribs);

        writeBytes(toUnoSequence(aClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::startUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(toUnoSequence(aOpeningBracket));

        if (Namespace.getLength())
        {
            write(Namespace);
            writeBytes(toUnoSequence(aColon));
        }

        write(Name);

        writeFastAttributeList(Attribs);

        writeBytes(toUnoSequence(aClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::endFastElement( ::sal_Int32 Element )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(toUnoSequence(aOpeningBracketAndSlash));

        writeId(Element);

        writeBytes(toUnoSequence(aClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::endUnknownElement( const OUString& Namespace, const OUString& Name )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(toUnoSequence(aOpeningBracketAndSlash));

        if (Namespace.getLength())
        {
            write(Namespace);
            writeBytes(toUnoSequence(aColon));
        }

        write(Name);

        writeBytes(toUnoSequence(aClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::singleFastElement( ::sal_Int32 Element, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        if ( !maMarkStack.empty() )
            maMarkStack.top()->setCurrentElement( Element );

        writeBytes(toUnoSequence(aOpeningBracket));

        writeId(Element);
        writeFastAttributeList(Attribs);

        writeBytes(toUnoSequence(aSlashAndClosingBracket));
    }

    void SAL_CALL FastSaxSerializer::singleUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(toUnoSequence(aOpeningBracket));

        if (Namespace.getLength())
        {
            write(Namespace);
            writeBytes(toUnoSequence(aColon));
        }

        write(Name);

        writeFastAttributeList(Attribs);

        writeBytes(toUnoSequence(aSlashAndClosingBracket));
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
            writeBytes(toUnoSequence(aSpace));

            write(pAttr[i].Name);
            writeBytes(toUnoSequence(aEqualSignAndQuote));
            write(escapeXml(pAttr[i].Value));
            writeBytes(toUnoSequence(aQuote));
        }

        Sequence< FastAttribute > aFastAttrSeq = Attribs->getFastAttributes();
        const FastAttribute *pFastAttr = aFastAttrSeq.getConstArray();
        sal_Int32 nFastAttrLength = aFastAttrSeq.getLength();
        for (sal_Int32 j = 0; j < nFastAttrLength; j++)
        {
            writeBytes(toUnoSequence(aSpace));

            sal_Int32 nToken = pFastAttr[j].Token;
            writeId(nToken);

            writeBytes(toUnoSequence(aEqualSignAndQuote));

            write(escapeXml(Attribs->getValue(pFastAttr[j].Token)));

            writeBytes(toUnoSequence(aQuote));
        }
    }

    // XServiceInfo
    OUString FastSaxSerializer::getImplementationName() throw (RuntimeException)
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM( SERIALIZER_IMPLEMENTATION_NAME ));
    }

    // XServiceInfo
    sal_Bool FastSaxSerializer::supportsService(const OUString& ServiceName) throw (RuntimeException)
    {
        Sequence< OUString > aSNL = getSupportedServiceNames();
        const OUString * pArray = aSNL.getConstArray();

        for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

        return sal_False;
    }

    // XServiceInfo
    Sequence< OUString > FastSaxSerializer::getSupportedServiceNames(void) throw (RuntimeException)
    {
        Sequence<OUString> seq(1);
        seq.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM( SERIALIZER_SERVICE_NAME ));
        return seq;
    }

    OUString FastSaxSerializer::getImplementationName_Static()
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM( SERIALIZER_IMPLEMENTATION_NAME ));
    }

    Sequence< OUString > FastSaxSerializer::getSupportedServiceNames_Static(void)
    {
        Sequence<OUString> aRet(1);
        aRet.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(SERIALIZER_SERVICE_NAME) );
        return aRet;
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

#if DEBUG
    void FastSaxSerializer::printMarkStack( )
    {
        ::std::stack< boost::shared_ptr< ForMerge > > aCopy( maMarkStack );
        int nSize = aCopy.size();
        int i = 0;
        while ( !aCopy.empty() )
        {
            fprintf( stderr, "%d\n", nSize - i );

            aCopy.top( )->print( );

            fprintf( stderr, "\n" );

            aCopy.pop( );
            i++;
        }
    }
#endif

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
        fprintf( stderr, "Data: " );
        for ( sal_Int32 i=0, len=maData.getLength(); i < len; i++ )
        {
            fprintf( stderr, "%c", maData[i] );
        }

        fprintf( stderr, "\nPostponed: " );
        for ( sal_Int32 i=0, len=maPostponed.getLength(); i < len; i++ )
        {
            fprintf( stderr, "%c", maPostponed[i] );
        }

        fprintf( stderr, "\n" );
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
            fprintf( stderr, "pair: %d, ", iter->first );
            for ( sal_Int32 i=0, len=iter->second.getLength(); i < len; ++i )
                fprintf( stderr, "%c", iter->second[i] );
            fprintf( stderr, "\n" );
            ++iter;
        }

        sort( );
        ForMerge::print();
    }
#endif

} // namespace sax_fastparser

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
