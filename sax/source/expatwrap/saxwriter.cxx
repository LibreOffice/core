/*************************************************************************
 *
 *  $RCSfile: saxwriter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-01 12:46:53 $
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
#include <rtl/byteseq.hxx>

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
#define SAXWRITER_CHECK_FOR_INVALID_CHARS

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
* Calculates the length of the sequence after conversion, but the conversion is not done.
* .g. &<>"' plus some more are
* special characters in XML that need to be transformed
*
* @param bConvertAll For Attributes it is necessary to convert every symbol (including line feed and tab)
*                    Set this to true, if you want to perform this special conversion
* @return The returned value is equal to the length of the incoming sequence, when no
+         conversion is necessary, otherwise it is larger than the length of the sequence.
****/
//  inline sal_Int32 CalcXMLLen( const Sequence<sal_Int8> & seq , sal_Bool bConvertAll ) throw()
//  {
//      sal_Int32 nLen = 0;
//      const sal_Int8 *pArray = seq.getConstArray();

//      for( int i = 0 ; i < seq.getLength() ; i ++ ) {

//          sal_Int8 c = pArray[i];
//          switch( c )
//          {
//          case '&':       // resemble to &amp;
//              nLen +=5;
//              break;
//          case '<':       // &lt;
//          case '>':       // &gt;
//              nLen +=4;
//              break;
//          case 39:        // 39 == ''', &apos;
//          case '"':       // &quot;
//          case 13:        // &#x0d;
//              nLen += 6;
//              break;

//          case 10:        // &#x0a;
//          case 9:         // &#x09;
//              if( bConvertAll )
//              {
//                  nLen += 6;        //
//              }
//              break;
//          default:
//              nLen ++;
//          }
//      }

//      return nLen;
//  }

const sal_Bool g_bValidCharsBelow32[31] =
{
//  0 1 2 3 4 5 6 7
    0,0,0,0,0,0,0,0,  //0
    0,1,1,0,0,1,0,0,  //8
    0,0,0,0,0,0,0,0,  //16
    0,0,0,0,0,0,0
};

inline sal_Int32 calcXMLByteLength( const sal_Unicode *pStr, sal_Int32 nStrLen,
                                    sal_Bool bDoNormalization,
                                    sal_Bool bNormalizeWhitespace ) throw( SAXException )
{
    sal_Int32 nOutputLength = 0;

    for( sal_Int32 i = 0 ; i < nStrLen ; i++ )
    {
        sal_uInt16 c = pStr[i];
        if( (c >= 0x0001) && (c <= 0x007F) )
        {
            if( bDoNormalization )
            {
                switch( c )
                {
                case '&':       // resemble to &amp;
                    nOutputLength +=5;
                    break;
                case '<':       // &lt;
                case '>':       // &gt;
                    nOutputLength +=4;
                    break;
                case 39:        // 39 == ''', &apos;
                case '"':       // &quot;
                case 13:        // &#x0d;
                    nOutputLength += 6;
                    break;

                case 10:        // &#x0a;
                case 9:         // &#x09;
                    if( bNormalizeWhitespace )
                    {
                        nOutputLength += 6;       //
                    }
                    else
                    {
                        nOutputLength ++;
                    }
                    break;
                default:
                    nOutputLength ++;
                }
            }
            else
            {
                nOutputLength ++;
            }
        }
        else if( c > 0x07FF )
        {
            nOutputLength += 3;
        }
        else
        {
            nOutputLength += 2;
        }
#ifdef SAXWRITER_CHECK_FOR_INVALID_CHARS
        // check first for the most common characters
        if( c < 32 || c >= 0xd800 )
        {
            if( (c < 32 && ! g_bValidCharsBelow32[c]) ||
                (c >= 0xd800 && c <= 0xdfff) ||
                c == 0xffff ||
                c == 0xfffe )
            {
                SAXException except;
                except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export: " ) );
                except.Message += OUString::valueOf( (sal_Int32) c );
                throw except;
            }
        }
#endif
    }

    return nOutputLength;
}

/** Converts an UTF16 string to UTF8 and does XML normalization

    @param pTarget
           Pointer to a piece of memory, to where the output should be written. The caller
           must call calcXMLByteLength on the same string, to ensure,
           that there is enough memory for converting.
 */
sal_Int32 convertToXML( const sal_Unicode * pStr, sal_Int32 nStrLen,
                        sal_Bool bDoNormalization,
                        sal_Bool bNormalizeWhitespace,
                        sal_Int8 *pTarget )
{
    sal_Int32 nOutputLength = 0;
    sal_Int32 nPos = 0;

    for( sal_Int32 i = 0 ; i < nStrLen ; i ++ )
    {
        sal_uInt16 c = pStr[i];
        if( (c >= 0x0001) && (c <= 0x007F) )
        {
            if( bDoNormalization )
            {
                switch( c )
                {
                case '&':  // resemble to &amp;
                    memcpy( &(pTarget[nPos]) , "&amp;" , 5 );
                    nPos += 5;
                    break;
                case '<':
                    memcpy( &(pTarget[nPos]) , "&lt;" , 4 );
                    nPos += 4;        // &lt;
                    break;
                case '>':
                    memcpy( &(pTarget[nPos]) , "&gt;" , 4 );
                    nPos += 4;        // &gt;
                    break;
                case 39:                 // 39 == '''
                    memcpy( &(pTarget[nPos]) , "&apos;" , 6 );
                    nPos += 6;        // &apos;
                    break;
                case '"':
                    memcpy( &(pTarget[nPos]) , "&quot;" , 6 );
                    nPos += 6;        // &quot;
                    break;
                case 13:
                    memcpy( &(pTarget[nPos]) , "&#x0d;" , 6 );
                    nPos += 6;
                    break;
                case LINEFEED:
                    if( bNormalizeWhitespace )
                    {
                        memcpy( &(pTarget[nPos]) , "&#x0a;" , 6 );
                        nPos += 6;
                    }
                    else
                    {
                        pTarget[nPos] = LINEFEED;
                        nPos ++;
                    }
                    break;
                case 9:
                    if( bNormalizeWhitespace )
                    {
                        memcpy( &(pTarget[nPos]) , "&#x09;" , 6 );
                        nPos += 6;
                    }
                    else
                    {
                        pTarget[nPos] = 9;
                        nPos ++;
                    }
                    break;
                default:
                    pTarget[nPos] = (sal_Int8)c;
                    nPos ++;
                }
            }
            else
            {
                pTarget[nPos] = (sal_Int8)c;
                nPos ++;
            }
        }
        else if( c > 0x07FF )
        {
            pTarget[nPos] = sal_Int8(0xE0 | ((c >> 12) & 0x0F));
            nPos ++;
            pTarget[nPos] = sal_Int8(0x80 | ((c >>  6) & 0x3F));
            nPos ++;
            pTarget[nPos] = sal_Int8(0x80 | ((c >>  0) & 0x3F));
            nPos ++;
        }
        else
        {
            pTarget[nPos] = sal_Int8(0xC0 | ((c >>  6) & 0x1F));
            nPos ++;
            pTarget[nPos] = sal_Int8(0x80 | ((c >>  0) & 0x3F));
            nPos ++;
        }
    }
    return nPos;
}


/** returns position of first ascii 10 within the string, -1 when no 10 in string.
 */
static inline sal_Int32 getFirstLineBreak( const OUString & str ) throw ()
{
    const sal_Unicode *pSource = str.getStr();
    sal_Int32 nLen  = str.getLength();

    for( int n = 0; n < nLen ; n ++ )
    {
        if( LINEFEED == pSource[n] ) {
            return n;
        }
    }
    return -1;
}

/** returns position of last ascii 10 within sequence, -1 when no 10 in string.
 */
static inline sal_Int32 getLastLineBreak( const Sequence<sal_Int8>  & seq) throw ()
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
        m_bAllowLineBreak(sal_False)
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

    void writeSequence( const Sequence<sal_Int8> & seq );
    void insertIndentation( sal_Int8 *pTarget ) throw();
    sal_Int32 getIndentPrefixLength( sal_Int32 nFirstLineBreakOccurence ) throw();

    inline void pushStartElement()
        {
            if( m_seqStartElement.getLength() )
            {
                writeSequence( m_seqStartElement );
                m_seqStartElement = Sequence < sal_Int8 > ();
            }
        }



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

OUString SaxWriter_getServiceName() throw()
{
    return OUString::createFromAscii( "com.sun.star.xml.sax.Writer" );
}

OUString SaxWriter_getImplementationName() throw()
{
    return OUString::createFromAscii( "com.sun.star.extensions.xml.sax.Writer" );
}

Sequence< OUString >    SaxWriter_getSupportedServiceNames(void) throw()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = SaxWriter_getImplementationName();
    return aRet;
}



sal_Int32 SAXWriter::getIndentPrefixLength( sal_Int32 nFirstLineBreakOccurence ) throw()
{
    sal_Int32 nLength =-1;
    if( m_bForceLineBreak ||
        m_bAllowLineBreak && nFirstLineBreakOccurence + m_nColumn > m_nMaxColumn )
    {
        nLength = m_nLevel +1;
    }
    m_bForceLineBreak = sal_False;
    m_bAllowLineBreak = sal_False;
    return nLength;
}

void SAXWriter::insertIndentation( sal_Int8 *pTarget ) throw()
{
    pTarget[0] = 10;
    memset( &(pTarget[1]) , 32 , m_nLevel );
}

static inline sal_Bool isFirstCharWhitespace( const sal_Unicode *p ) throw()
{
    return *p == ' ';
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
        throw SAXException(
            OUString::createFromAscii( "endDocument called before startDocument" ),
            Reference< XInterface >() , Any() );
    }
    if( m_nLevel ) {
        throw SAXException(
            OUString::createFromAscii( "unexpected end of document" ),
            Reference< XInterface >() , Any() );
    }
    try
    {
        m_out->closeOutput();
    }
    catch( IOException & e )
    {
        Any a;
        a <<= e;
        throw SAXException(
            OUString::createFromAscii( "IO exception during closing the IO Stream" ),
            Reference< XInterface > (),
            a );
    }
}


void SAXWriter::startElement(const OUString& aName, const Reference< XAttributeList >& xAttribs)
    throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "startElement called before startDocument" ));
        throw except;
    }
    if( m_bIsCDATA )
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "startElement call not allowed with CDATA sections" ));
        throw except;
    }
    pushStartElement();

    // first calculate the sequence length
    sal_Int32 nLength = 0;
    sal_Int32 nAttribCount = xAttribs.is() ? xAttribs->getLength() : 0;

    nLength ++; // "<"
    nLength += calcXMLByteLength( aName.getStr() , aName.getLength(),
                                  sal_False, sal_False ); // the tag name

    int n;
    for( n = 0 ; n < nAttribCount ; n ++ ) {
        nLength ++; // " "
        OUString tmp =  xAttribs->getNameByIndex( n );

        nLength += calcXMLByteLength( tmp.getStr() , tmp.getLength() , sal_False, sal_False );

        nLength += 2; // ="

        tmp = xAttribs->getValueByIndex( n );

        nLength += calcXMLByteLength( tmp.getStr(), tmp.getLength(), sal_True, sal_True );

        nLength += 1; // "
    }

    nLength ++;  // '>'

    // Is there a new indentation necesarry ?
    sal_Int32 nPrefix = getIndentPrefixLength( nLength  );
    if( nPrefix >= 0 )
        nLength += nPrefix;

    // now create the sequence, store it ( maybe later an empty tag ? )
    m_seqStartElement = Sequence< sal_Int8 > ( nLength );

    sal_Int8 *pTarget = (sal_Int8*) m_seqStartElement.getConstArray();  // we OWN the sequence
    sal_Int32 nPos =0;

    // write into sequence
    if( nPrefix >= 0 )
    {
        insertIndentation( pTarget );
        nPos += nPrefix;
    }
    pTarget[nPos] = '<';
    nPos ++;

    nPos += convertToXML( aName.getStr(), aName.getLength(), sal_False, sal_False, &(pTarget[nPos]) );

    for( n = 0 ; n < nAttribCount ; n ++ )
    {
        pTarget[nPos] = ' ';
        nPos ++;

        OUString tmp = xAttribs->getNameByIndex( n );
        nPos += convertToXML( tmp.getStr(), tmp.getLength(), sal_False, sal_False, &(pTarget[nPos]) );

        pTarget[nPos] = '=';
        nPos ++;
        pTarget[nPos] = '"';
        nPos ++;

        tmp = xAttribs->getValueByIndex( n );
        nPos += convertToXML( tmp.getStr(), tmp.getLength(), sal_True, sal_True , &(pTarget[nPos] ) );

        pTarget[nPos] = '"';
        nPos ++;
    }

    pTarget[nPos] = '>';
    nPos ++;
    OSL_ASSERT( nPos == nLength );

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
        sal_Int32 nLength = 3 + calcXMLByteLength( aName.getStr(), aName.getLength(), sal_False, sal_False );
        sal_Int32 nPrefix = getIndentPrefixLength( nLength );

        if( nPrefix >= 0 )
        {
            nLength += nPrefix;
        }
        Sequence< sal_Int8 > seqWrite( nLength );

        sal_Int8 *pTarget = ( sal_Int8 * ) seqWrite.getConstArray(); // we OWN it

        sal_Int32 nPos = 0;
        if( nPrefix >= 0 )
        {
            nPos += nPrefix;
            insertIndentation( pTarget );
        }
        pTarget[nPos] = '<';
        nPos ++;
        pTarget[nPos] = '/';
        nPos ++;

        nPos += convertToXML( aName.getStr(), aName.getLength(), sal_False, sal_False, &(pTarget[nPos] ) );

        pTarget[nPos] = '>';
        nPos ++;

        OSL_ASSERT( nPos == nLength );
        writeSequence( seqWrite );
    }
}

void SAXWriter::characters(const OUString& aChars)  throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "characters method called before startDocument" ) );
        throw except;
    }

    if( aChars.getLength() )
    {
        pushStartElement();

        sal_Int32 nLength = calcXMLByteLength( aChars.getStr(), aChars.getLength(),
                                               ! m_bIsCDATA , sal_False );
        if( m_bIsCDATA )
        {
            // no indentation, no normalization
            Sequence<sal_Int8> seqWrite( nLength );
            convertToXML( aChars.getStr(), aChars.getLength() , sal_False, sal_False ,
                          (sal_Int8*)seqWrite.getConstArray() );
            writeSequence( seqWrite );
        }
        else
        {
            // Note : nFirstLineBreakOccurence is not exact, because we don't know, how
            //        many 2 and 3 byte chars are inbetween. However this whole stuff
            //        is eitherway for pretty printing only, so it does not need to be exact.
            sal_Int32 nFirstLineBreakOccurence = getFirstLineBreak( aChars );

            sal_Int32 nIdentPrefix = getIndentPrefixLength(
                nFirstLineBreakOccurence >= 0 ? nFirstLineBreakOccurence : nLength );

            if( nIdentPrefix >= 0 )
            {
                nLength += nIdentPrefix;
                if( isFirstCharWhitespace( aChars.getStr() ) )
                {
                    nLength --;
                }
            }

            // create the sequence
            Sequence< sal_Int8 > seqWrite( nLength );
            sal_Int8 *pTarget = (sal_Int8 * ) seqWrite.getConstArray();

            // insert indentation
            sal_Int32 nPos = 0;
            if( nIdentPrefix >= 0 )
            {
                insertIndentation( pTarget );
                nPos += nIdentPrefix;
                if( isFirstCharWhitespace( aChars.getStr() ) )
                {
                    nPos += convertToXML(
                        &(aChars.getStr()[1]) , aChars.getLength() -1 ,
                        sal_True , sal_False , &(pTarget[nPos]) );
                }
                else
                {
                    nPos += convertToXML(
                        aChars.getStr() , aChars.getLength() , sal_True, sal_False,
                        &(pTarget[nPos]));
                }
            }
            else
            {
                nPos += convertToXML(
                    aChars.getStr() , aChars.getLength() , sal_True, sal_False, &(pTarget[nPos]) );
            }

            OSL_ASSERT( nPos == nLength );
            writeSequence( seqWrite );
        }
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

    sal_Int32 nLength = 2;  // "<?"
    nLength += calcXMLByteLength( aTarget.getStr(), aTarget.getLength(), sal_False, sal_False );

    nLength += 1;  // " "

    nLength += calcXMLByteLength( aData.getStr(), aData.getLength(), sal_False, sal_False );

    nLength += 2; // "?>"

    pushStartElement();

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix > 0 )
    {
        nLength += nPrefix;
    }

    Sequence< sal_Int8 > seqWrite( nLength );

    sal_Int32 nPos = 0;
    sal_Int8 * pTarget = ( sal_Int8 * ) seqWrite.getConstArray();

    if( nPrefix >= 0 )
    {
        insertIndentation( pTarget );
        nPos += nPos;
    }

    pTarget[nPos] = '<';
    nPos ++;
    pTarget[nPos] = '?';
    nPos ++;

    nPos += convertToXML( aTarget.getStr(), aTarget.getLength(), sal_False, sal_False, &(pTarget[nPos] ) );

    pTarget[nPos] = ' ';
    nPos ++;

    nPos += convertToXML( aData.getStr(), aData.getLength(), sal_False, sal_False, &(pTarget[nPos]) );
    pTarget[nPos] = '?';
    nPos ++;
    pTarget[nPos] = '>';
    nPos ++;

    OSL_ASSERT( nPos == nLength );

    writeSequence( seqWrite );
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


    sal_Int32 nLength = 9;
    sal_Int32 nPrefix = getIndentPrefixLength( 9 );
    if( nPrefix >= 0)
    {
        nLength += nPrefix;
    }

    Sequence< sal_Int8 > seq( nLength );
    sal_Int8 *pTarget = ( sal_Int8* )seq.getConstArray();
    sal_Int32 nPos = 0;

    if( nPrefix >= 0 )
    {
        insertIndentation( pTarget );
        nPos += nPrefix;
    }

    memcpy( &(pTarget[nPos]), "<![CDATA[" , 9 );

    writeSequence( seq );

    m_bIsCDATA = sal_True;
}

void SAXWriter::endCDATA(void) throw (RuntimeException)
{
    if( ! m_bDocStarted | ! m_bIsCDATA)
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "endCDATA was called without startCDATA" ) );
        throw except;
    }

    sal_Int32 nLength = 3;
    sal_Int32 nPrefix = getIndentPrefixLength( 3 );
    if( nPrefix >= 0)
    {
        nLength += nPrefix;
    }

    Sequence< sal_Int8 > seq( nLength );
    sal_Int8 *pTarget = ( sal_Int8* )seq.getConstArray();
    sal_Int32 nPos = 0;

    if( nPrefix >= 0 )
    {
        insertIndentation( pTarget );
        nPos += nPrefix;
    }

    memcpy( &(pTarget[nPos]), "]]>" , 3 );

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

    sal_Int32 nLength = 4; // "<!--"
    nLength += calcXMLByteLength( sComment.getStr(), sComment.getLength(), sal_False, sal_False);

    nLength += 3;

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
    {
        nLength += nPrefix;
    }

    Sequence<sal_Int8> seq( nLength );

    sal_Int8 *pTarget = (sal_Int8*) seq.getConstArray();
    sal_Int32 nPos = 0;

    if( nPrefix >= 0 )
    {
        insertIndentation( pTarget );
        nPos += nPrefix;
    }
    pTarget[nPos] = '<';
    pTarget[nPos+1] = '!';
    pTarget[nPos+2] = '-';
    pTarget[nPos+3] = '-';
    nPos += 4;

    nPos += convertToXML( sComment.getStr(), sComment.getLength(), sal_False, sal_False,
                          &(pTarget[nPos]));

    pTarget[nPos] = '-';
    pTarget[nPos+1] = '-';
    pTarget[nPos+2] = '>';
    nPos += 3;

    OSL_ASSERT( nPos == nLength );

    writeSequence( seq );
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

    sal_Int32 nLength = calcXMLByteLength( sString.getStr(), sString.getLength(), sal_False, sal_False );

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
    {
        nLength += nPrefix;
    }
    Sequence< sal_Int8 > seq( nLength );

    sal_Int8 *pTarget = (sal_Int8*) seq.getConstArray();
    sal_Int32 nPos = 0;

    if( nPrefix >= 0 )
    {
        insertIndentation( pTarget );
        nPos += nPrefix;
    }

    nPos += convertToXML( sString.getStr(), sString.getLength(), sal_False, sal_False,
                          &(pTarget[nPos]));

    OSL_ASSERT( nPos == nLength );
    writeSequence( seq );
}

}

