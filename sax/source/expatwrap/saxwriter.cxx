/*************************************************************************
 *
 *  $RCSfile: saxwriter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-25 08:13:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <string.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

#include <com/sun/star/io/XActiveDataSource.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase3.hxx>

#include <rtl/strbuf.hxx>

#include <assert.h>

using namespace ::rtl;
using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;

#include "factory.hxx"
#include "xml2utf.hxx"

#define LINEFEED 10

/******
*
*
* Character conversion functions
*
*
*****/

namespace sax_expatwrap {
/*****
*
* Does special conversions (beside encoding) that is needed for xml. E.g. &<>"' plus some more are
* special characters in XML that need to be transformed
*
* @param bConvertAll For Attributes it is necessary to convert every symbol (including line feed and tab)
*                    Set this to true, if you want to perform this special conversion
*
****/
sal_Int32 CalcXMLLen( const Sequence<sal_Int8> & seq , sal_Bool bConvertAll )
{
    sal_Int32 nLen = 0;
    const sal_Int8 *pArray = seq.getConstArray();


    for( int i = 0 ; i < seq.getLength() ; i ++ ) {

        sal_Int8 c = pArray[i];
        if( '&' == c ) {  // resemble to &amp;
            nLen += 5;
        }
        else if( '<' == c ) {
            nLen += 4;        // &lt;
        }
        else if( '>' == c ) {
            nLen += 4;        // &gt;
        }
        else if( 39 == c ) {    // 39 == '''
            nLen += 6;        // &apos;
        }
        else if( '"' == c ) {
            nLen += 6;        // &quot;
        }
        else if( 13 == c ) {
            nLen += 6;         // &#x0d;
        }
        else if( bConvertAll && LINEFEED == c ) {
            nLen += 6;
        }
        else if( bConvertAll &&  9 == c ) {
            nLen += 6;
        }
        else {
            nLen ++;
        }
    }

    return nLen;
}


inline sal_Int32 getFirstLineBreak( const Sequence<sal_Int8> & seq)
{
    const sal_Int8 *pSource = seq.getConstArray();

    sal_Int32 nLen  = seq.getLength();
    for( int n = 0; n < nLen ; n ++ )
    {
        if( LINEFEED == pSource[n] ) {
            return n;
        }
    }
    return -1;
}

inline sal_Int32 getLastLineBreak( const Sequence<sal_Int8>  & seq)
{
    const sal_Int8 *pSource = seq.getConstArray();
    sal_Int32 nLen  = seq.getLength();

    for( int n = nLen-1; n >= 0 ; n -- )
    {
        if( LINEFEED == pSource[n] ) {
            return n;
        }
    }
    return -1;
}


class SAXWriter :
    public WeakImplHelper3<
            XActiveDataSource,
            XExtendedDocumentHandler,
              XServiceInfo >
{
public:
    SAXWriter( ) :
        m_nMaxColumn(72),
        m_bForceLineBreak(sal_False),
        m_bAllowLineBreak(sal_False),
        m_unicode2utf8( RTL_TEXTENCODING_UTF8 )
        {}

public: // XActiveDataSource
    virtual void SAL_CALL setOutputStream(const Reference< XOutputStream > & aStream)
        throw (RuntimeException)
            {
                m_out = aStream;
                m_bDocStarted = sal_False;
                m_nLevel = 0;
                m_bIsCDATA = sal_False;
                m_nColumn = 0;
            }
    virtual Reference< XOutputStream >  SAL_CALL getOutputStream(void)
        throw(RuntimeException)
            { return m_out; }

public: // XDocumentHandler
    virtual void SAL_CALL startDocument(void)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL endDocument(void)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL startElement(const OUString& aName,
                                       const Reference< XAttributeList > & xAttribs)
        throw (SAXException, RuntimeException);

    virtual void SAL_CALL endElement(const OUString& aName)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL characters(const OUString& aChars)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                const OUString& aData)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator > & xLocator)
        throw(SAXException, RuntimeException);

public: // XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA(void) throw(SAXException, RuntimeException);
    virtual void SAL_CALL endCDATA(void) throw(RuntimeException);
    virtual void SAL_CALL comment(const OUString& sComment)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL unknown(const OUString& sString)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL allowLineBreak(void)
        throw(SAXException,RuntimeException);

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw();
    sal_Bool                    SAL_CALL supportsService(const OUString& ServiceName) throw();

private:

    void doIndent( Sequence<sal_Int8> &);
    void writeSequence( const Sequence<sal_Int8> & seq );

    inline void pushStartElement()
        {
            if( m_seqStartElement.getLength() )
            {
                writeSequence( m_seqStartElement );
                m_seqStartElement = Sequence < sal_Int8 > ();
            }
        }

    Sequence < sal_Int8 > ustring2UTF8( const OUString &sValue )
        {
            return m_unicode2utf8.convert( sValue );
        }

    Sequence < sal_Int8 > utf8ToXML( const Sequence< sal_Int8 > & , sal_Bool bConvertAll );

    /****
     * @param bConvertAll  TRUE,  when LINEFEED plus tab shall also be normalized
     *                     sal_False  otherwise
     *
     ****/
    Sequence < sal_Int8 > ustring2XML( const OUString &sValue , sal_Bool bConvertAll )
        {
            return utf8ToXML( ustring2UTF8( sValue ) , bConvertAll );
        }


    Unicode2TextConverter m_unicode2utf8;
    Reference< XOutputStream > m_out;
    Sequence < sal_Int8 > m_seqStartElement;

    // Status information
    sal_Bool m_bDocStarted;
    sal_Bool m_bIsCDATA;
    sal_Bool m_bForceLineBreak;
    sal_Bool m_bAllowLineBreak;
    sal_Int32 m_nLevel;
    sal_Int32 m_nColumn;
    sal_Int32 m_nMaxColumn;
};


//--------------------------------------
// the extern interface
//---------------------------------------
Reference < XInterface > SAL_CALL SaxWriter_CreateInstance(
    const Reference < XMultiServiceFactory >  & rSMgr )
    throw (Exception)
{
    SAXWriter *p = new SAXWriter;
    return Reference< XInterface > ( SAL_STATIC_CAST(OWeakObject *, p ) );
}

OUString SaxWriter_getServiceName()
{
    return OUString::createFromAscii( "com.sun.star.xml.sax.Writer" );
}

OUString SaxWriter_getImplementationName()
{
    return OUString::createFromAscii( "com.sun.star.extensions.xml.sax.Writer" );
}

Sequence< OUString >    SaxWriter_getSupportedServiceNames(void) throw()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = SaxWriter_getImplementationName();
    return aRet;
}


Sequence < sal_Int8 >  SAXWriter::utf8ToXML( const Sequence<sal_Int8> & seqSource ,  sal_Bool bConvertAll )
{
    Sequence< sal_Int8 > seqTarget( CalcXMLLen( seqSource , bConvertAll ) );
    sal_Int32 nMaxSource = seqSource.getLength();
    const sal_Int8 *pSource = seqSource.getConstArray();
    sal_Int8  *pTarget = seqTarget.getArray();
    sal_Int32 nTarget = 0;

    for( int nSource = 0 ; nSource < nMaxSource ; nSource ++ ) {
        sal_Int8 c = pSource[nSource];
        if( '&' == c ) {  // resemble to &amp;
            memcpy( &(pTarget[nTarget]) , "&amp;" , 5 );
            nTarget += 5;
        }
        else if( '<' == c ) {
            memcpy( &(pTarget[nTarget]) , "&lt;" , 4 );
            nTarget += 4;        // &lt;
        }
        else if( '>' == c ) {
            memcpy( &(pTarget[nTarget]) , "&gt;" , 4 );
            nTarget += 4;        // &gt;
        }
        else if( 39 == c ) {    // 39 == '''
            memcpy( &(pTarget[nTarget]) , "&apos;" , 6 );
            nTarget += 6;        // &apos;
        }
        else if( '"' == c ) {
            memcpy( &(pTarget[nTarget]) , "&quot;" , 6 );
            nTarget += 6;         // &quot;
        }
        else if( 13 == c ) {
            memcpy( &(pTarget[nTarget]) , "&#x0d;" , 6 );
            nTarget += 6;
        }
        else if( LINEFEED == c && bConvertAll ) {
            memcpy( &(pTarget[nTarget]) , "&#x0a;" , 6 );
            nTarget += 6;
        }
        else if( 9  == c && bConvertAll ) {
            memcpy( &(pTarget[nTarget]) , "&#x09;" , 6 );
            nTarget += 6;
        }
        else {
            pTarget[nTarget] = c;
            nTarget ++;
        }
    }
    return seqTarget;
}


void SAXWriter::doIndent( Sequence<sal_Int8> &seq )
{
    sal_Int32 nLength = getFirstLineBreak( seq );
    nLength = ( nLength >= 0 ) ? nLength : seq.getLength();
    if( ( m_bForceLineBreak ) ||
        ( m_bAllowLineBreak && nLength + m_nColumn > m_nMaxColumn  )
        ) {

        // write the linebreaks !
        Sequence<sal_Int8> seqIndent( m_nLevel + 1 );
        seqIndent.getArray()[0] = 10;
        memset( &(seqIndent.getArray()[1] ) , 32 , m_nLevel );
        writeSequence( seqIndent );

        // remove one leading space in the sequence
        if( seq.getLength() && 32 == seq.getArray()[0] ) {
            memmove( seq.getArray() , &(seq.getArray()[1]) , seq.getLength() -1 );
            seq.realloc( seq.getLength()-1 );
        }

    }

    m_bForceLineBreak = sal_False;
    m_bAllowLineBreak = sal_False;
}


/********
* write through to the output stream and counts columns
*
*****/
void SAXWriter::writeSequence( const Sequence<sal_Int8> & seq )
{

    sal_Int32 nPos = getLastLineBreak( seq );
    try
    {
        m_out->writeBytes( seq );
    }
    catch( IOException & e )
    {
        Any a;
        a <<= e;
        throw SAXException(
            OUString::createFromAscii( "io exception during writing" ),
            Reference< XInterface > (),
            a );
    }

    if( nPos >= 0 ) {
        m_nColumn = seq.getLength() - (nPos+1);
    }
    else {
        m_nColumn += seq.getLength();
    }
}




// XServiceInfo
OUString SAXWriter::getImplementationName() throw()
{
    return SaxWriter_getImplementationName();
}

// XServiceInfo
sal_Bool SAXWriter::supportsService(const OUString& ServiceName) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > SAXWriter::getSupportedServiceNames(void) throw ()
{
    Sequence<OUString> seq(1);
    seq.getArray()[0] = SaxWriter_getServiceName();
    return seq;
}



void SAXWriter::startDocument()                     throw(SAXException, RuntimeException )
{
    if( m_bDocStarted || ! m_out.is() ) {
        throw SAXException();
    }
    m_bDocStarted = sal_True;
    const char pc[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    const int nLen = strlen( pc );
    Sequence<sal_Int8> seqWrite( nLen+1 );
    memcpy( seqWrite.getArray() , pc , nLen );
    seqWrite.getArray()[nLen] = LINEFEED;
    writeSequence( seqWrite );
}


void SAXWriter::endDocument(void)                   throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        throw SAXException();
    }
    if( m_nLevel ) {
        throw SAXException(
            OUString::createFromAscii( "unexpected end of document" ),
            Reference< XInterface >() , Any() );
    }
    m_out->closeOutput();
}


void SAXWriter::startElement(const OUString& aName, const Reference< XAttributeList >& xAttribs)
    throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        throw SAXException();
    }
    if( m_bIsCDATA )
    {
        throw SAXException();
    }
    pushStartElement();

    sal_Int32 nAttribCount = xAttribs.is() ? xAttribs->getLength() : 0;

    OStringBuffer str(64 *( nAttribCount+1) );
    str.append( "<" );

    // Tags may only contain ascii chars !
    str.append( OUStringToOString( aName.getStr() , RTL_TEXTENCODING_UTF8 ) );

    for( int n = 0 ; n < nAttribCount ; n ++ ) {
        str.append( " " );
        str.append( OUStringToOString( xAttribs->getNameByIndex( n ) , RTL_TEXTENCODING_UTF8 ) );
        str.append( "=\"" );

        Sequence<sal_Int8> seq = ustring2XML( xAttribs->getValueByIndex( n ) , sal_True );
        str.append( ( const sal_Char * ) seq.getConstArray() , seq.getLength() );
        str.append( "\"" );
    }

    // preparing for empty tag
    str.append( ">" );

    Sequence<sal_Int8> seqWrite( str.getLength() );
    memcpy( seqWrite.getArray() , str.getStr() , str.getLength()  );

    doIndent( seqWrite );

    m_seqStartElement = seqWrite;

    m_nLevel ++;
}

void SAXWriter::endElement(const OUString& aName)   throw (SAXException, RuntimeException)
{
    if( ! m_bDocStarted ) {
        throw SAXException ();
    }
    m_nLevel --;

    if( m_nLevel < 0 ) {
        throw SAXException();
    }

    if( m_seqStartElement.getLength() )
    {
        m_seqStartElement.realloc( m_seqStartElement.getLength() + 1 );

        sal_Int8 *p = m_seqStartElement.getArray();
        p[m_seqStartElement.getLength()-2] = '/';
        p[m_seqStartElement.getLength()-1] = '>';
        writeSequence( m_seqStartElement );
        m_seqStartElement = Sequence< sal_Int8 > ();
    }
    else {
        // only ascii chars allowed
        sal_Int32 nLen = aName.getLength();
        Sequence< sal_Int8 > seqWrite( nLen + 3 );

        sal_Int8 *p = seqWrite.getArray();
        sal_Unicode *pStr = (sal_Unicode * )aName.getStr();

        p[0] = '<';
        p[1] = '/';
        for( sal_Int32 i = 0 ; i < nLen ; i ++ )
        {
            p[2+i] = (sal_Int8) pStr[i];
        }
        p[nLen+2] = '>';

        doIndent( seqWrite );
        writeSequence( seqWrite );
    }
}

void SAXWriter::characters(const OUString& aChars)  throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        throw SAXException();
    }
    pushStartElement();

    if( m_bIsCDATA ) {
        Sequence<sal_Int8> seqWrite = ustring2UTF8( aChars);
        writeSequence( seqWrite );
    }
    else {
        Sequence<sal_Int8> seqWrite =  ustring2XML( aChars  , sal_False );
        doIndent( seqWrite );
        writeSequence( seqWrite );
    }
}


void SAXWriter::ignorableWhitespace(const OUString& aWhitespaces) throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        throw SAXException ();
    }

    m_bForceLineBreak = sal_True;
}

void SAXWriter::processingInstruction(const OUString& aTarget, const OUString& aData)
    throw (SAXException, RuntimeException)
{
    if( ! m_bDocStarted || m_bIsCDATA )
    {
        throw SAXException();
    }

    pushStartElement();

    OStringBuffer str( 128 );
    str.append( "<?" );
    str.append( OUStringToOString( aTarget , RTL_TEXTENCODING_UTF8 ) );
    str.append( " " );
    str.append( OUStringToOString( aData , RTL_TEXTENCODING_UTF8 ) );   // only ascii chars allowed
    str.append( "?>" );

    Sequence<sal_Int8> seq( str.getLength() );
    memcpy( seq.getArray() , str.getStr() , str.getLength() );

    doIndent( seq );
    writeSequence( seq );
}


void SAXWriter::setDocumentLocator(const Reference< XLocator >& xLocator)
        throw (SAXException, RuntimeException)
{

}

void SAXWriter::startCDATA(void) throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted || m_bIsCDATA)
    {
        throw SAXException ();
    }

    pushStartElement();

    Sequence < sal_Int8 > seq( ( sal_Int8 * ) "<![CDATA[" ,  9 );

    doIndent( seq );
    writeSequence( seq );
    m_bIsCDATA = sal_True;
}

void SAXWriter::endCDATA(void) throw (RuntimeException)
{
    if( ! m_bDocStarted | ! m_bIsCDATA)
    {
        throw SAXException();
    }

    pushStartElement();
    Sequence<sal_Int8> seq( ( sal_Int8 * ) "]]>" , 3 );

    doIndent( seq );
    writeSequence( seq );
    m_bIsCDATA = sal_False;
}


void SAXWriter::comment(const OUString& sComment) throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted || m_bIsCDATA )
    {
        throw SAXException();
    }

    pushStartElement();

    Sequence<sal_Int8> seq = ustring2XML( sComment , sal_False );

    Sequence<sal_Int8> seqWrite( seq.getLength() + 7 );
    sal_Int8 *p = seqWrite.getArray();
    p[0] = '<';
    p[1] = '!';
    p[2] = '-';
    p[3] = '-';
    memcpy( &(p[4]) , seq.getConstArray() , seq.getLength() );
    p[4+seq.getLength()] = '-';
    p[5+seq.getLength()] = '-';
    p[6+seq.getLength()] = '>';

    doIndent( seqWrite );
    writeSequence( seqWrite );
}


void SAXWriter::allowLineBreak( )   throw ( SAXException , RuntimeException)
{
    if( ! m_bDocStarted || m_bAllowLineBreak ) {
        throw SAXException();
    }

     m_bAllowLineBreak = sal_True;
}

void SAXWriter::unknown(const OUString& sString) throw (SAXException, RuntimeException)
{

    if( ! m_bDocStarted )
    {
        throw SAXException ();
    }
    if( m_bIsCDATA )
    {
        throw SAXException();
    }

    pushStartElement();

    OString str = OUStringToOString( sString , RTL_TEXTENCODING_UTF8 );

    Sequence<sal_Int8> seq( str.getLength() );
    memcpy( seq.getArray() , str.getStr() , str.getLength() );

    doIndent( seq );
    writeSequence( seq );
}

}

