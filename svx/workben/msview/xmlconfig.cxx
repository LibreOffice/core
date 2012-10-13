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


#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "rtl/ref.hxx"

#include <svx/msdffimp.hxx>

#include "xmlconfig.hxx"

#include <stdio.h>
#include <ctype.h>
#include <stack>

using ::rtl::OUString;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::IOException;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

///////////////////////////////////////////////////////////////////////

AtomConfigMap gAtomConfigMap;

///////////////////////////////////////////////////////////////////////

class ConfigHandler : public ::cppu::WeakAggImplHelper1<XDocumentHandler>
{
public:
    // XDocumentHandler
    virtual void SAL_CALL startDocument(void) throw( SAXException, RuntimeException );
    virtual void SAL_CALL endDocument(void) throw( SAXException, RuntimeException );
    virtual void SAL_CALL startElement(const OUString& aName, const Reference< XAttributeList > & xAttribs) throw( SAXException, RuntimeException );
    virtual void SAL_CALL endElement(const OUString& aName) throw( SAXException, RuntimeException );
    virtual void SAL_CALL characters(const OUString& aChars) throw( SAXException, RuntimeException );
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) throw( SAXException, RuntimeException );
    virtual void SAL_CALL processingInstruction(const OUString& aTarget, const OUString& aData) throw( SAXException, RuntimeException );
    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator > & xLocator) throw( SAXException, RuntimeException );

private:
    void errorThrow( const OUString& rErrorMessage ) throw (SAXException );
    ElementConfigType parseType( const OUString& rErrorMessage ) throw ( SAXException );
    void addElement( ElementConfigPtr& rElementConfig ) throw ( SAXException );
    OUString getAttribute( const Reference< XAttributeList > & xAttribs, const sal_Char* pName ) throw( SAXException );

    ElementConfigPtr importAtomConfig( const Reference< XAttributeList > & xAttribs, bool bIsContainer ) throw( SAXException );
    ElementConfigPtr importElementConfig( const Reference< XAttributeList > & xAttribs ) throw( SAXException );
    ElementConfigPtr importSwitchConfig( const Reference< XAttributeList > & xAttribs ) throw( SAXException );
    ElementConfigPtr importCaseConfig( const Reference< XAttributeList > & xAttribs ) throw( SAXException );
    ElementConfigPtr importValueElementConfig( const Reference< XAttributeList > & xAttribs ) throw( SAXException );

    std::stack< ElementConfigPtr > maElementStack;
};

void ConfigHandler::errorThrow( const OUString& rErrorMessage ) throw (SAXException )
{
    Reference< XInterface > aContext;
    Any aWrappedException;
    throw SAXException(rErrorMessage, aContext, aWrappedException);
}

ElementConfigType ConfigHandler::parseType( const OUString& sType ) throw (SAXException )
{
    if ( sType == "uint" )
    {
        return ECT_UINT;
    }
    else if ( sType == "byte" )
    {
        return ECT_BYTE;
    }
    else if ( sType == "unistring" )
    {
        return ECT_UNISTRING;
    }
    else if ( sType == "float" )
    {
        return ETC_FLOAT;
    }
    else if ( sType == "hexdump" )
    {
    }
    else
    {
        OUString aMessage( "unknown type: "  );
        aMessage += sType;
        errorThrow( aMessage );
    }

    return ECT_HEXDUMP;
}

void ConfigHandler::addElement( ElementConfigPtr& rElementConfig ) throw ( SAXException )
{
    ElementConfigContainer* pParent = dynamic_cast< ElementConfigContainer* >( maElementStack.top().get() );

    if( !pParent )
        errorThrow( OUString( "illegal parent for element"  ) );


    pParent->addElementConfig( rElementConfig );
}

OUString ConfigHandler::getAttribute( const Reference< XAttributeList > & xAttribs, const sal_Char* pName ) throw( SAXException )
{
    OUString aName( OUString::createFromAscii( pName ) );

    const sal_Int16 nAttrCount = xAttribs.is() ? xAttribs->getLength() : 0;
    sal_Int16 i;
    for(i=0; i < nAttrCount; i++)
    {
        if( xAttribs->getNameByIndex( i ) == aName )
            return xAttribs->getValueByIndex( i );
    }

    OUString aMessage( "missing required attribute: " );
    aMessage += aName;
    errorThrow( aMessage );

    return OUString();
}

void SAL_CALL ConfigHandler::startDocument(void) throw( SAXException, RuntimeException )
{
}

void SAL_CALL ConfigHandler::endDocument(void) throw( SAXException, RuntimeException )
{
}

void SAL_CALL ConfigHandler::startElement(const OUString& aName, const Reference< XAttributeList > & xAttribs) throw( SAXException, RuntimeException )
{
    ElementConfigPtr pElement;

    if ( aName == "config" )
    {
        return;
    }

    if ( aName == "container" )
    {
        pElement = importAtomConfig( xAttribs, true );
    }
    else if ( aName == "atom" )
    {
        pElement = importAtomConfig( xAttribs, false );
    }
    else if ( aName == "element" )
    {
        pElement = importElementConfig( xAttribs );
    }
    else if ( aName == "value" )
    {
        pElement = importValueElementConfig( xAttribs );
    }
    else if ( aName == "switch" )
    {
        pElement = importSwitchConfig( xAttribs );
    }
    else if ( aName == "case" )
    {
        pElement = importCaseConfig( xAttribs );
    }

    if( !pElement.get() )
    {
        OUString aMessage( OUString( "unknown config element: ") );
        aMessage += aName;
        errorThrow( aMessage  );
    }

    maElementStack.push( pElement );
}

sal_Int32 toInt( const OUString& rText )
{
    if( rText.compareToAscii("0x",2) == 0)
    {
        sal_Int32 nValue = 0;
        const sal_Unicode *p = rText;
        p += 2;
        sal_Int32 nLength = rText.getLength() - 2;
        while( (nLength--) > 0 )
        {
            nValue <<= 4;
            if( *p >= '0' && *p <= '9' )
            {
                nValue += *p - '0';
            }
            else if( *p >= 'a' && *p <= 'f' )
            {
                nValue += *p - ('a' - 10);
            }
            else if( *p >= 'A' && *p <= 'F' )
            {
                nValue += *p - ('A' - 10 );
            }
            p++;
        }

        return nValue;
    }
    else
    {
        return rText.toInt32();
    }
}

ElementConfigPtr ConfigHandler::importAtomConfig( const Reference< XAttributeList > & xAttribs, bool bIsContainer ) throw (SAXException)
{
    if( !maElementStack.empty() )
        errorThrow( OUString( "atom elements must be root"  ) );

    ElementConfigPtr aPtr( new AtomConfig( getAttribute(xAttribs,"name"), bIsContainer ) );
    gAtomConfigMap[ (UINT16)toInt(getAttribute(xAttribs,"id"))] = aPtr;
    return aPtr;
}

ElementConfigPtr ConfigHandler::importElementConfig( const Reference< XAttributeList > & xAttribs ) throw (SAXException)
{
    ElementConfigType nType = parseType( getAttribute( xAttribs, "type" ) );
    ElementConfigPtr pElementConfig( new ElementConfigContainer( getAttribute( xAttribs, "name" ), nType ) );
    addElement( pElementConfig );
    return pElementConfig;
}

ElementConfigPtr ConfigHandler::importValueElementConfig( const Reference< XAttributeList > & xAttribs ) throw (SAXException)
{
    ElementConfigPtr pElementConfig( new ElementValueConfig( getAttribute( xAttribs, "name" ), getAttribute( xAttribs, "value" ) ) );
    addElement( pElementConfig );
    return pElementConfig;
}

ElementConfigPtr ConfigHandler::importSwitchConfig( const Reference< XAttributeList > & xAttribs ) throw (SAXException)
{
    ElementConfigType nType = parseType( getAttribute( xAttribs, "type" ) );
    ElementConfigPtr pElementConfig( new SwitchElementConfig( nType ) );
    addElement( pElementConfig );
    return pElementConfig;
}

ElementConfigPtr ConfigHandler::importCaseConfig( const Reference< XAttributeList > & xAttribs ) throw (SAXException)
{
    ElementConfigPtr pElementConfig( new CaseElementConfig( getAttribute( xAttribs, "value" ) ) );
    addElement( pElementConfig );
    return pElementConfig;
}

void SAL_CALL ConfigHandler::endElement(const OUString& aName) throw( SAXException, RuntimeException )
{
    if ( aName == "config" )
    {
        return;
    }

    maElementStack.pop();
}

void SAL_CALL ConfigHandler::characters(const OUString& aChars) throw( SAXException, RuntimeException )
{
}

void SAL_CALL ConfigHandler::ignorableWhitespace(const OUString& aWhitespaces) throw( SAXException, RuntimeException )
{
}

void SAL_CALL ConfigHandler::processingInstruction(const OUString& aTarget, const OUString& aData) throw( SAXException, RuntimeException )
{
}

void SAL_CALL ConfigHandler::setDocumentLocator(const Reference< XLocator > & xLocator) throw( SAXException, RuntimeException )
{
}

void load_config( const OUString& rPath )
{
    try
    {
        // create stream
        SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( rPath, STREAM_READ );
        Reference<XInputStream> xInputStream( new utl::OInputStreamWrapper( pIStm, sal_True ) );

        // prepare ParserInputSrouce
        InputSource aParserInput;
        aParserInput.sSystemId = rPath;
        aParserInput.aInputStream = xInputStream;

        // get parser
        Reference< XParser > xParser = Parser::create(comphelper::getProcessComponentContext());

        // get filter
        ConfigHandler* pConfigHandler = new ConfigHandler();
        Reference< XDocumentHandler > xFilter( pConfigHandler );

        // connect parser and filter
        xParser->setDocumentHandler( xFilter );

        // finally, parser the stream
        xParser->parseStream( aParserInput );
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (rtl::OString("load_config(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

///////////////////////////////////////////////////////////////////////

rtl::OUString ElementConfig::format( SvStream& rStream, sal_Size& nLength ) const
{
    OUString aRet;
    if( maName.getLength() )
    {
        aRet += maName;
        aRet += OUString( " = "  );
    }

    switch( mnType )
    {
    case ECT_BYTE:      aRet += dump_byte( rStream, nLength ); break;
    case ECT_UINT:      aRet += dump_uint( rStream, nLength ); break;
    case ECT_UNISTRING: aRet += dump_unistring( rStream, nLength ); break;
    case ETC_FLOAT:     aRet += dump_float( rStream, nLength ); break;
    case ECT_HEXDUMP:
    default:            aRet += dump_hex( rStream, nLength ); break;
    }

    return aRet;
}

rtl::OUString ElementConfig::dump_hex( SvStream& rStream, sal_Size& nLength )
{
    char buffer[128];
    OUString aOut, aEmpty;
    OUString aHex, aAscii;
    sal_Char c;
    int nRow = 0;
    while( nLength && (rStream.GetError() == 0) )
    {
        rStream >> c;
        nLength--;

        unsigned int i = c;
        i &= 0xff;
        sprintf( buffer, "%02x ", i );
        aHex += OUString::createFromAscii( buffer );

        if( !isprint( c ) )
            c = '.';

        aAscii += OUString( (sal_Unicode) c );
        nRow++;

        if( (nRow == 16) || (nLength==0) )
        {
            while( aHex.getLength() < (16*3) )
                aHex += OUString( " " );
            aOut += aHex;
            aOut += aAscii;
            aOut += OUString( "\n\r"  );
            aHex = aEmpty;
            aAscii = aEmpty;
            nRow = 0;
        }
    }

    aOut += aHex;
    aOut += aAscii;

    return aOut;
}

rtl::OUString ElementConfig::dump_byte( SvStream& rStream, sal_Size& nLength )
{
    OUString aRet;
    if( nLength >= sizeof(sal_Char) )
    {
        sal_Char c;
        rStream >> c;

        char buffer[128];
        sprintf( buffer, "%u", (int)c );
        aRet += OUString::createFromAscii( buffer );
        nLength -= sizeof(sal_Char);
    }

    return aRet;
}

rtl::OUString ElementConfig::dump_uint( SvStream& rStream, sal_Size& nLength )
{
    OUString aRet;
    if( nLength >= sizeof( sal_uInt32 ) )
    {
        sal_uInt32 c;
        rStream >> c;

        char buffer[128];
        sprintf( buffer, "%u", c );
        aRet += OUString::createFromAscii( buffer );
        nLength-= sizeof( sal_uInt32 );
    }

    return aRet;
}

rtl::OUString ElementConfig::dump_unistring( SvStream& rStream, sal_Size& nLength )
{
    String aString = SvxMSDffManager::MSDFFReadZString( rStream, nLength, sal_True );
    nLength = 0;
    return aString;
}

rtl::OUString ElementConfig::dump_float( SvStream& rStream, sal_Size& nLength )
{
    OUString aRet;
    if( nLength >= sizeof( float ) )
    {
        float c;
        rStream >> c;

        char buffer[128];
        sprintf( buffer, "%g", (double)c );
        aRet += OUString::createFromAscii( buffer );
        nLength-= sizeof( float );
    }

    return aRet;
}

///////////////////////////////////////////////////////////////////////

rtl::OUString ElementConfigContainer::format( SvStream& rStream, sal_Size& nLength ) const
{
    OUString aRet;

    if( getType() == ETC_CONTAINER )
    {

        ElementConfigList::const_iterator aIter( maElementConfigList.begin() );
        const ElementConfigList::const_iterator aEnd( maElementConfigList.end() );
        while( (aIter != aEnd) && (nLength > 0) )
        {
            aRet += (*aIter++)->format( rStream, nLength );
            if( (aIter != aEnd) || (nLength != 0) )
                aRet += OUString( "\n\r"  );
        }

        if( nLength )
            aRet += ElementConfig::dump_hex( rStream, nLength );
    }
    else
    {
        aRet = getName();
        if( aRet.getLength() )
            aRet += OUString( " = "  );

        OUString aValue;
        switch( getType() )
        {
        case ECT_BYTE:      aValue = dump_byte( rStream, nLength ); break;
        case ECT_UINT:      aValue = dump_uint( rStream, nLength ); break;
        case ECT_UNISTRING: aValue = dump_unistring( rStream, nLength ); break;
        case ETC_FLOAT:     aValue = dump_float( rStream, nLength ); break;
        case ECT_HEXDUMP:
        default:            aValue = dump_hex( rStream, nLength ); break;
        }

        if( aValue.getLength() )
        {
            if( !maElementConfigList.empty() )
            {
                ElementConfigList::const_iterator aIter( maElementConfigList.begin() );
                const ElementConfigList::const_iterator aEnd( maElementConfigList.end() );
                while( (aIter != aEnd) && (nLength > 0) )
                {
                    ElementValueConfig* pValue = dynamic_cast< ElementValueConfig* >( (*aIter++).get() );
                    if( pValue && pValue->getValue() == aValue )
                    {
                        aValue = pValue->getName();
                        break;
                    }
                }
            }
        }
        else
        {
            aValue = OUString( "<empty!?>" );
        }

        aRet += aValue;
    }

    return aRet;
}

///////////////////////////////////////////////////////////////////////

rtl::OUString SwitchElementConfig::format( SvStream& rStream, sal_Size& nLength ) const
{
    OUString aValue;

    switch( getType() )
    {
    case ECT_BYTE:          aValue = dump_byte( rStream, nLength ); break;
    case ECT_UINT:          aValue = dump_uint( rStream, nLength ); break;
    case ETC_FLOAT:         aValue = dump_float( rStream, nLength ); break;
    case ECT_UNISTRING:     aValue = dump_unistring( rStream, nLength ); break;
    }

    if( aValue.getLength()  )
    {
        ElementConfigList::const_iterator aIter( maElementConfigList.begin() );
        const ElementConfigList::const_iterator aEnd( maElementConfigList.end() );
        while( (aIter != aEnd) && (nLength > 0) )
        {
            CaseElementConfig* pCase = dynamic_cast< CaseElementConfig* >( (*aIter++).get() );
            if( pCase && pCase->getValue() == aValue )
                return pCase->format( rStream, nLength );
        }
    }

    return ElementConfig::dump_hex( rStream, nLength );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
