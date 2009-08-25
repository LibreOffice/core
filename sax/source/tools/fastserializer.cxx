/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contexthandler2.cxx,v $
 *
 *  $Revision: 1.1.2.2 $
 *
 *  last change: $Author: dr $ $Date: 2008/02/11 10:43:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "fastserializer.hxx"
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/xml/Attribute.hpp>
#include <com/sun/star/xml/FastAttribute.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include <string.h>

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringToOString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
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

static Sequence< sal_Int8 > aClosingBracket((sal_Int8 *)">", 1);
static Sequence< sal_Int8 > aSlashAndClosingBracket((sal_Int8 *)"/>", 2);
static Sequence< sal_Int8 > aColon((sal_Int8 *)":", 1);
static Sequence< sal_Int8 > aOpeningBracket((sal_Int8 *)"<", 1);
static Sequence< sal_Int8 > aOpeningBracketAndSlash((sal_Int8 *)"</", 2);
static Sequence< sal_Int8 > aQuote((sal_Int8 *)"\"", 1);
static Sequence< sal_Int8 > aEqualSignAndQuote((sal_Int8 *)"=\"", 2);
static Sequence< sal_Int8 > aSpace((sal_Int8 *)" ", 1);
static Sequence< sal_Int8 > aXmlHeader((sal_Int8*) "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n", 56);

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
        writeBytes(aXmlHeader);
    }

    OUString FastSaxSerializer::escapeXml( const OUString& s )
    {
        ::rtl::OUStringBuffer sBuf( s.getLength() );
        const sal_Unicode* pStr = s;
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
            writeBytes(aColon);
            writeBytes(mxFastTokenHandler->getUTF8Identifier(TOKEN(nElement)));
        } else
            writeBytes(mxFastTokenHandler->getUTF8Identifier(nElement));
    }

    void SAL_CALL FastSaxSerializer::startFastElement( ::sal_Int32 Element, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(aOpeningBracket);

        writeId(Element);
        writeFastAttributeList(Attribs);

        writeBytes(aClosingBracket);
    }

    void SAL_CALL FastSaxSerializer::startUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(aOpeningBracket);

        if (Namespace.getLength())
        {
            write(Namespace);
            writeBytes(aColon);
        }

        write(Name);

        writeFastAttributeList(Attribs);

        writeBytes(aClosingBracket);
    }

    void SAL_CALL FastSaxSerializer::endFastElement( ::sal_Int32 Element )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(aOpeningBracketAndSlash);

        writeId(Element);

        writeBytes(aClosingBracket);
    }

    void SAL_CALL FastSaxSerializer::endUnknownElement( const OUString& Namespace, const OUString& Name )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(aOpeningBracketAndSlash);

        if (Namespace.getLength())
        {
            write(Namespace);
            writeBytes(aColon);
        }

        write(Name);

        writeBytes(aClosingBracket);
    }

    void SAL_CALL FastSaxSerializer::singleFastElement( ::sal_Int32 Element, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(aOpeningBracket);

        writeId(Element);
        writeFastAttributeList(Attribs);

        writeBytes(aSlashAndClosingBracket);
    }

    void SAL_CALL FastSaxSerializer::singleUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs )
        throw (SAXException, RuntimeException)
    {
        if (!mxOutputStream.is())
            return;

        writeBytes(aOpeningBracket);

        if (Namespace.getLength())
        {
            write(Namespace);
            writeBytes(aColon);
        }

        write(Name);

        writeFastAttributeList(Attribs);

        writeBytes(aSlashAndClosingBracket);
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
            writeBytes(aSpace);

            write(pAttr[i].Name);
            writeBytes(aEqualSignAndQuote);
            write(escapeXml(pAttr[i].Value));
            writeBytes(aQuote);
        }

        Sequence< FastAttribute > aFastAttrSeq = Attribs->getFastAttributes();
        const FastAttribute *pFastAttr = aFastAttrSeq.getConstArray();
        sal_Int32 nFastAttrLength = aFastAttrSeq.getLength();
        for (sal_Int32 j = 0; j < nFastAttrLength; j++)
        {
            writeBytes(aSpace);

            sal_Int32 nToken = pFastAttr[j].Token;
            writeId(nToken);

            writeBytes(aEqualSignAndQuote);

            write(escapeXml(Attribs->getValue(pFastAttr[j].Token)));

            writeBytes(aQuote);
        }
    }

    // XServiceInfo
    OUString FastSaxSerializer::getImplementationName() throw (RuntimeException)
    {
        return OUString::createFromAscii( SERIALIZER_IMPLEMENTATION_NAME );
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
        seq.getArray()[0] = OUString::createFromAscii( SERIALIZER_SERVICE_NAME );
        return seq;
    }

    OUString FastSaxSerializer::getImplementationName_Static()
    {
        return OUString::createFromAscii( SERIALIZER_IMPLEMENTATION_NAME );
    }

    Sequence< OUString > FastSaxSerializer::getSupportedServiceNames_Static(void)
    {
        Sequence<OUString> aRet(1);
        aRet.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(SERIALIZER_SERVICE_NAME) );
        return aRet;
    }

    void FastSaxSerializer::mark()
    {
        maMarkStack.push( ForMerge() );
    }

    void FastSaxSerializer::mergeTopMarks( sax_fastparser::MergeMarksEnum eMergeType )
    {
        if ( maMarkStack.empty() )
            return;

        if ( maMarkStack.size() == 1 )
        {
            mxOutputStream->writeBytes( maMarkStack.top().getData() );
            maMarkStack.pop();
            return;
        }

        const Int8Sequence aMerge( maMarkStack.top().getData() );
        maMarkStack.pop();

        switch ( eMergeType )
        {
            case MERGE_MARKS_APPEND:   maMarkStack.top().append( aMerge );   break;
            case MERGE_MARKS_PREPEND:  maMarkStack.top().prepend( aMerge );  break;
            case MERGE_MARKS_POSTPONE: maMarkStack.top().postpone( aMerge ); break;
        }
    }

    void FastSaxSerializer::writeBytes( const Sequence< ::sal_Int8 >& aData ) throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
    {
        if ( maMarkStack.empty() )
            mxOutputStream->writeBytes( aData );
        else
            maMarkStack.top().append( aData );
    }

    FastSaxSerializer::Int8Sequence& FastSaxSerializer::ForMerge::getData()
    {
        merge( maData, maPostponed, true );
        maPostponed.realloc( 0 );

        return maData;
    }

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

} // namespace sax_fastparser

