/*************************************************************************
 *
 *  $RCSfile: testsax.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 14:01:52 $
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <osl/time.h>
#include <osl/diagnose.h>


#include <com/sun/star/test/XSimpleTest.hpp>

#include <com/sun/star/io/XOutputStream.hpp>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;

#include "factory.hxx"

/****
* test szenarios :
*
*
*
****/

namespace sax_test {

class OSaxParserTest : public WeakImplHelper1< XSimpleTest >
{
public:
    OSaxParserTest( const Reference < XMultiServiceFactory > & rFactory ) : m_rFactory( rFactory )
    {
    }
public:
    virtual void SAL_CALL testInvariant(
        const OUString& TestName,
        const Reference < XInterface >& TestObject)
        throw ( IllegalArgumentException, RuntimeException);

    virtual sal_Int32 SAL_CALL test(
        const OUString& TestName,
        const Reference < XInterface >& TestObject,
        sal_Int32 hTestHandle)
        throw ( IllegalArgumentException,RuntimeException);

    virtual sal_Bool SAL_CALL testPassed(void) throw (  RuntimeException);
    virtual Sequence< OUString > SAL_CALL getErrors(void)               throw (RuntimeException);
    virtual Sequence< Any > SAL_CALL getErrorExceptions(void)       throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings(void)                 throw (RuntimeException);

private:
    void testSimple( const Reference < XParser > &r );
    void testNamespaces( const Reference < XParser > &r );
    void testFile(  const Reference < XParser > &r );
    void testEncoding( const Reference < XParser > &rParser );
    void testPerformance( const Reference < XParser > &rParser );

private:
    Sequence<Any>       m_seqExceptions;
    Sequence<OUString>      m_seqErrors;
    Sequence<OUString>      m_seqWarnings;
    Reference < XMultiServiceFactory > m_rFactory;
};



/**
* for external binding
*
*
**/
Reference < XInterface > SAL_CALL OSaxParserTest_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    OSaxParserTest *p = new OSaxParserTest( rSMgr );
    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}


OUString     OSaxParserTest_getServiceName( ) throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.xml.sax.Parser" ));
}

OUString    OSaxParserTest_getImplementationName( ) throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.extensions.xml.sax.Parser"));
}

Sequence<OUString> OSaxParserTest_getSupportedServiceNames( ) throw ()
{
    Sequence<OUString> aRet(1);

    aRet.getArray()[0] = OSaxParserTest_getImplementationName( );

    return aRet;
}




void OSaxParserTest::testInvariant(
    const OUString& TestName,
    const Reference < XInterface >& TestObject )
    throw ( IllegalArgumentException, RuntimeException)
{
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser")) == TestName ) {
        Reference < XParser > parser( TestObject , UNO_QUERY );

        ERROR_ASSERT( parser.is() , "XDataInputStream cannot be queried" );
    }
}


sal_Int32 OSaxParserTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)
    throw ( IllegalArgumentException, RuntimeException)
{
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser")) == TestName )  {
        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else {

                Reference < XParser > parser( TestObject , UNO_QUERY );

                if( 1 == hTestHandle ) {
                    testSimple( parser );
                }
                else if( 2 == hTestHandle ) {
                    testNamespaces( parser );
                }
                else if( 3 == hTestHandle ) {
                    testEncoding( parser );
                }
                else if( 4 == hTestHandle ) {
                    testFile( parser );
                }
                else if( 5 == hTestHandle ) {
                    testPerformance( parser );
                }
            }
        }
        catch( Exception & e )
        {
            OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US);
            BUILD_ERROR( 0 , o.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( hTestHandle >= 6) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        BUILD_ERROR( 0 , "service not supported by test." );
    }
    return hTestHandle;
}



sal_Bool OSaxParserTest::testPassed(void)                                       throw (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OSaxParserTest::getErrors(void)                            throw (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OSaxParserTest::getErrorExceptions(void)                    throw (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OSaxParserTest::getWarnings(void)                      throw (RuntimeException)
{
    return m_seqWarnings;
}

Reference < XInputStream > createStreamFromSequence(
    const Sequence<sal_Int8> seqBytes ,
    const Reference < XMultiServiceFactory > &xSMgr )
{
    Reference < XInterface > xOutStreamService =
        xSMgr->createInstance( OUString::createFromAscii("com.sun.star.io.Pipe") );
    assert( xOutStreamService.is() );
    Reference< XOutputStream >  rOutStream( xOutStreamService , UNO_QUERY );
    assert( rOutStream.is() );

    Reference< XInputStream > rInStream( xOutStreamService , UNO_QUERY );
    assert( rInStream.is() );

    rOutStream->writeBytes( seqBytes );
    rOutStream->flush();
    rOutStream->closeOutput();

    return rInStream;
}

Reference< XInputStream > createStreamFromFile(
    const char *pcFile ,
    const Reference < XMultiServiceFactory > &xSMgr )
{
    FILE *f = fopen( pcFile , "rb" );
    Reference<  XInputStream >  r;

    if( f ) {
        fseek( f , 0 , SEEK_END );
        int nLength = ftell( f );
        fseek( f , 0 , SEEK_SET );

        Sequence<sal_Int8> seqIn(nLength);
        fread( seqIn.getArray() , nLength , 1 , f );

        r = createStreamFromSequence( seqIn , xSMgr );
        fclose( f );
    }
    return r;
}









//  #define PCHAR_TO_OUSTRING(x) OStringToOUString(x,CHARSET_PC_1252)
//  #define USTRING_TO_PCHAR(x) UStringToString(x,CHARSET_PC_437).GetStr()



class TestDocumentHandler :
    public WeakImplHelper3< XExtendedDocumentHandler , XEntityResolver , XErrorHandler >
{
public:
    TestDocumentHandler( const Reference < XMultiServiceFactory >  &r , sal_Bool bPrint )
    {
        m_xSMgr = r;
        m_bPrint = bPrint;
    }

public: // Error handler
    virtual void SAL_CALL error(const Any& aSAXParseException) throw (SAXException, RuntimeException)
    {
        printf( "Error !\n" );
        throw  SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("error from error handler")) ,
            Reference < XInterface >() ,
            aSAXParseException );
    }
    virtual void SAL_CALL fatalError(const Any& aSAXParseException) throw (SAXException, RuntimeException)
    {
        printf( "Fatal Error !\n" );
    }
    virtual void SAL_CALL warning(const Any& aSAXParseException) throw (SAXException, RuntimeException)
    {
        printf( "Warning !\n" );
    }


public: // ExtendedDocumentHandler

    virtual void SAL_CALL startDocument(void) throw (SAXException, RuntimeException)
    {
         m_iLevel = 0;
        m_iElementCount = 0;
        m_iAttributeCount = 0;
        m_iWhitespaceCount =0;
        m_iCharCount=0;
        if( m_bPrint ) {
            printf( "document started\n" );
        }
    }
    virtual void SAL_CALL endDocument(void) throw (SAXException, RuntimeException)
    {
        if( m_bPrint ) {
            printf( "document finished\n" );
            printf( "(ElementCount %d),(AttributeCount %d),(WhitespaceCount %d),(CharCount %d)\n",
                m_iElementCount, m_iAttributeCount, m_iWhitespaceCount , m_iCharCount );
        }
    }
    virtual void SAL_CALL startElement(const OUString& aName,
                              const Reference< XAttributeList > & xAttribs)
        throw (SAXException,RuntimeException)
    {

        if( m_rLocator.is() ) {
            if( m_bPrint )
            {
                OString o = OUStringToOString( m_rLocator->getSystemId() , RTL_TEXTENCODING_UTF8 );
                printf( "%s(%d):" , o.getStr() , m_rLocator->getLineNumber() );
            }
        }
        if( m_bPrint ) {
            int i;
            for( i = 0; i < m_iLevel ; i ++ ) {
                printf( "  " );
            }
            OString o = OUStringToOString(aName , RTL_TEXTENCODING_UTF8 );
            printf( "<%s> " , aName.getStr() );

            for( i = 0 ; i < xAttribs->getLength() ; i ++ )
            {
                OString o1 = OUStringToOString(xAttribs->getNameByIndex( i ), RTL_TEXTENCODING_UTF8 );
                OString o2 = OUStringToOString(xAttribs->getTypeByIndex( i ), RTL_TEXTENCODING_UTF8 );
                OString o3 = OUStringToOString(xAttribs->getValueByIndex( i ) , RTL_TEXTENCODING_UTF8 );
                printf( "(%s,%s,'%s')" , o1.getStr(), o2.getStr(), o3.getStr() );
            }
            printf( "\n" );
        }
        m_iLevel ++;
        m_iElementCount ++;
        m_iAttributeCount += xAttribs->getLength();
    }

    virtual void SAL_CALL endElement(const OUString& aName) throw (SAXException,RuntimeException)
    {
        assert( m_iLevel );
        m_iLevel --;
        if( m_bPrint ) {
            int i;
            for( i = 0; i < m_iLevel ; i ++ ) {
                printf( "  " );
            }
            OString o = OUStringToOString(aName , RTL_TEXTENCODING_UTF8 );
            printf( "</%s>\n" , o.getStr() );
        }
    }

    virtual void SAL_CALL characters(const OUString& aChars) throw (SAXException,RuntimeException)
    {
        if( m_bPrint ) {
            int i;
            for( i = 0; i < m_iLevel ; i ++ ) {
                printf( "  " );
            }
            OString o = OUStringToOString(aChars , RTL_TEXTENCODING_UTF8 );
            printf( "%s\n" , o.getStr() );
        }
        m_iCharCount += aChars.len();
    }
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) throw (SAXException,RuntimeException)
    {
        m_iWhitespaceCount += aWhitespaces.len();
    }

    virtual void SAL_CALL processingInstruction(const OUString& aTarget, const OUString& aData) throw (SAXException,RuntimeException)
    {
        if( m_bPrint )
        {
            OString o1 = OUStringToOString(aTarget, RTL_TEXTENCODING_UTF8 );
            OString o2 = OUStringToOString(aData, RTL_TEXTENCODING_UTF8 );
            printf( "PI : %s,%s\n" , o1.getStr() , o2.getStr() );
        }
    }

    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator> & xLocator)
        throw (SAXException,RuntimeException)
    {
        m_rLocator = xLocator;
    }

    virtual InputSource SAL_CALL resolveEntity(
        const OUString& sPublicId,
        const OUString& sSystemId)
        throw (SAXException,RuntimeException)
    {
        InputSource source;
        source.sSystemId = sSystemId;
        source.sPublicId = sPublicId;

        source.aInputStream = createStreamFromFile(
            OUStringToOString( sSystemId , RTL_TEXTENCODING_ASCII_US) , m_xSMgr );

        return source;
    }

    virtual void SAL_CALL startCDATA(void) throw (SAXException,RuntimeException)
    {
        if( m_bPrint ) {
            printf( "CDataStart :\n" );
        }
    }
    virtual void SAL_CALL endCDATA(void) throw (SAXException,RuntimeException)
    {
        if( m_bPrint ) {
            printf( "CEndStart :\n" );
        }
    }
    virtual void SAL_CALL comment(const OUString& sComment) throw (SAXException,RuntimeException)
    {
        if( m_bPrint ) {
            OString o1 = OUStringToOString(sComment, RTL_TEXTENCODING_UTF8 );
            printf( "<!--%s-->\n" , o1.getStr() );
        }
    }
    virtual void SAL_CALL unknown(const OUString& sString) throw (SAXException,RuntimeException)
    {
        if( m_bPrint )
        {
            OString o1 = OUStringToOString(sString, RTL_TEXTENCODING_UTF8 );
            printf( "UNKNOWN : {%s}\n" , o1.getStr() );
        }
    }

    virtual void SAL_CALL allowLineBreak( void) throw (SAXException, RuntimeException )
    {

    }


public:
    int m_iLevel;
    int m_iElementCount;
    int m_iAttributeCount;
    int m_iWhitespaceCount;
    int m_iCharCount;
    sal_Bool m_bPrint;

    Reference < XMultiServiceFactory > m_xSMgr;
    Reference < XLocator > m_rLocator;
};


void OSaxParserTest::testSimple(    const Reference < XParser > &rParser )
{

    char TestString[] = "<!DOCTYPE personnel [\n"
                        "<!ENTITY testInternal \"internal Test!\">\n"
                        "<!ENTITY test SYSTEM \"external_entity.xml\">\n"
                        "]>\n"
                        "<personnel>\n"
                        "<person> fjklsfdklsdfkl\n"
                        "fjklsfdklsdfkl\n"
                        "<?testpi pidata?>\n"
                        "&testInternal;\n"
                        "<HUHU x='5' y='kjfd'> blahuhu\n"
                        "<HI> blahi\n"
                        "     <![CDATA[<greeting>Hello, '+1+12world!</greeting>]]>\n"
                        "   <!-- huhu <jdk> -->\n"
                        "<?testpi pidata?>\n"
                        "</HI>\n"
                        "aus XMLTest\n"
                        "</HUHU>\n"
                        "</person>\n"
                        "</personnel>\n\n\n";

    Sequence< sal_Int8> seqBytes( strlen( TestString ) );
    memcpy( seqBytes.getArray() , TestString , strlen( TestString ) );


    Reference< XInputStream > rInStream;
    OUString sInput;
    rInStream = createStreamFromSequence( seqBytes , m_rFactory );
    sInput = OUString( OUString( RTL_CONSTASCII_USTRINGPARAM("internal")) );

    if( rParser.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , sal_False );
        Reference < XDocumentHandler > rDocHandler( (XDocumentHandler *) pDocHandler , UNO_QUERY );
        Reference< XEntityResolver >
            rEntityResolver( (XEntityResolver *) pDocHandler , UNO_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );

        try
        {
            rParser->parseStream( source );
            ERROR_ASSERT( pDocHandler->m_iElementCount      == 4 , "wrong element count"    );
            ERROR_ASSERT( pDocHandler->m_iAttributeCount    == 2 , "wrong attribut count"   );
            ERROR_ASSERT( pDocHandler->m_iCharCount         == 130 , "wrong char count"     );
            ERROR_ASSERT( pDocHandler->m_iWhitespaceCount   == 0, "wrong whitespace count" );
        }
        catch( SAXParseException & e )
        {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            BUILD_ERROR( 1 , o1.getStr() );
        }
        catch( SAXException & e )
        {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            BUILD_ERROR( 1 , o1.getStr() );
        }
        catch( Exception & e )
        {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            BUILD_ERROR( 1 , o1.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 1 , "unknown exception" );
        }
    }
}

void OSaxParserTest::testNamespaces( const Reference < XParser > &rParser )
{

    char TestString[] =
    "<?xml version='1.0'?>\n"
      "<!-- all elements here are explicitly in the HTML namespace -->\n"
      "<html:html xmlns:html='http://www.w3.org/TR/REC-html40'>\n"
        "<html:head><html:title>Frobnostication</html:title></html:head>\n"
        "<html:body><html:p>Moved to \n"
          "<html:a href='http://frob.com'>here.</html:a></html:p></html:body>\n"
      "</html:html>\n";

    Sequence<sal_Int8> seqBytes( strlen( TestString ) );
    memcpy( seqBytes.getArray() , TestString , strlen( TestString ) );


    Reference< XInputStream >  rInStream;
    OUString sInput;

    rInStream = createStreamFromSequence( seqBytes , m_rFactory );
    sInput = OUString( RTL_CONSTASCII_USTRINGPARAM( "internal" ));

    if( rParser.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , sal_False );
        Reference < XDocumentHandler > rDocHandler( (XDocumentHandler *) pDocHandler , UNO_QUERY );
        Reference< XEntityResolver >    rEntityResolver(
            (XEntityResolver *) pDocHandler , UNO_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );

        try
        {
            rParser->parseStream( source );
            ERROR_ASSERT( pDocHandler->m_iElementCount      == 6 , "wrong element count"    );
            ERROR_ASSERT( pDocHandler->m_iAttributeCount    == 2 , "wrong attribut count"   );
            ERROR_ASSERT( pDocHandler->m_iCharCount         == 33, "wrong char count"       );
            ERROR_ASSERT( pDocHandler->m_iWhitespaceCount   == 0 , "wrong whitespace count" );
        }
        catch( Exception & e ) {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            BUILD_ERROR( 1 , o1.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 1 , "unknown exception" );
        }
    }
}

void OSaxParserTest::testEncoding( const Reference < XParser > &rParser )
{
    char TestString[] =
    "<?xml version='1.0' encoding=\"iso-8859-1\"?>\n"
      "<!-- all elements here are explicitly in the HTML namespace -->\n"
      "<html:html xmlns:html='http://www.w3.org/TR/REC-html40'>\n"
        "<html:head><html:title>Frobnostication</html:title></html:head>\n"
        "<html:body><html:p>Moved to ß\n"
          "<html:a href='http://frob.com'>here.</html:a></html:p></html:body>\n"
      "</html:html>\n";

    Sequence<sal_Int8> seqBytes( strlen( TestString ) );
    memcpy( seqBytes.getArray() , TestString , strlen( TestString ) );


    Reference< XInputStream > rInStream;
    OUString sInput;

    rInStream = createStreamFromSequence( seqBytes , m_rFactory );
    sInput = OUString( RTL_CONSTASCII_USTRINGPARAM("internal") );

    if( rParser.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , sal_False );
        Reference < XDocumentHandler > rDocHandler( (XDocumentHandler *) pDocHandler , UNO_QUERY );
        Reference< XEntityResolver >  rEntityResolver( (XEntityResolver *) pDocHandler , UNO_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );
        try
        {
            rParser->parseStream( source );
        }
        catch( Exception & e )
        {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            BUILD_ERROR( 1 , o1.getStr() );
        }
        catch ( ... )
        {
            BUILD_ERROR( 1 , "unknown exception" );
        }
    }
}

void OSaxParserTest::testFile( const Reference < XParser > & rParser )
{

    Reference< XInputStream > rInStream = createStreamFromFile( "testsax.xml" , m_rFactory );
    OUString sInput = OUString( RTL_CONSTASCII_USTRINGPARAM( "testsax.xml" ) );


    if( rParser.is() && rInStream.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , sal_True );
        Reference < XDocumentHandler > rDocHandler( (XDocumentHandler *) pDocHandler , UNO_QUERY );
        Reference < XEntityResolver >   rEntityResolver( (XEntityResolver *) pDocHandler , UNO_QUERY );
        Reference < XErrorHandler > rErrorHandler( ( XErrorHandler * )pDocHandler , UNO_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );
        rParser->setErrorHandler( rErrorHandler );

        try
        {
            rParser->parseStream( source );
        }
        catch( SAXParseException & e ) {
            Any any;
            any <<= e;

            while(sal_True) {
                SAXParseException *pEx;
                if( any.getValueType() == getCppuType( &e ) ) {
                    pEx = ( SAXParseException * ) any.getValue();
                    OString o1 = OUStringToOString(pEx->Message, RTL_TEXTENCODING_UTF8 );
                    printf( "%s\n" , o1.getStr()  );
                    any = pEx->WrappedException;
                }
                else {
                    break;
                }
            }
        }
        catch( SAXException & e )
        {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            BUILD_ERROR( 1 , o1.getStr() );

        }
        catch( Exception & e ) {
            printf( "normal exception ! %s\n", e.Message );
        }
        catch ( ... )
        {
            printf( "any exception !!!!\n" );
        }
    }
}

void OSaxParserTest::testPerformance( const Reference < XParser > & rParser )
{

    Reference < XInputStream > rInStream =
        createStreamFromFile( "testPerformance.xml" , m_rFactory );
    OUString sInput = OUString( RTL_CONSTASCII_USTRINGPARAM( "testperformance.xml") );

    if( rParser.is() && rInStream.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , sal_False );
        Reference < XDocumentHandler > rDocHandler( (XDocumentHandler *) pDocHandler , UNO_QUERY );
        Reference < XEntityResolver > rEntityResolver( (XEntityResolver *) pDocHandler , UNO_QUERY );
        Reference < XErrorHandler > rErrorHandler( ( XErrorHandler * )pDocHandler , UNO_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );
        rParser->setErrorHandler( rErrorHandler );

        try
        {
            TimeValue aStartTime, aEndTime;
            osl_getSystemTime( &aStartTime );
            rParser->parseStream( source );
            osl_getSystemTime( &aEndTime );

            double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
            double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);

            printf( "Performance reading : %g s\n" , fEnd - fStart );

        }
        catch( SAXParseException &e ) {
            Any any;
            any <<= e;
            while(sal_True) {
                if( any.getValueType() == getCppuType( &e ) ) {
                    SAXParseException ex;
                    any >>= ex;
                    OString o = OUStringToOString( ex.Message , RTL_TEXTENCODING_ASCII_US );
                    printf( "%s\n" , o.getStr()  );
                    any <<= ex.WrappedException;
                }
                else {
                    break;
                }
            }
        }
        catch( SAXException &e  ) {
            OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n" , o.getStr()  );

        }
        catch( ... )
        {
            printf( "any exception !!!!\n" );
        }
    }
}
}
using namespace sax_test;

extern "C"
{


void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

            OUString str =
                OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OSaxParserTest_getImplementationName() +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            Reference< XRegistryKey > xNewKey = xKey->createKey( str );
            xNewKey->createKey( OSaxParserTest_getServiceName() );

            str =
                OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OSaxWriterTest_getImplementationName() +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );

            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OSaxWriterTest_getServiceName() );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return sal_False;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr =
            reinterpret_cast< XMultiServiceFactory * > ( pServiceManager );

        OUString aImplementationName = OUString::createFromAscii( pImplName );


        if (aImplementationName == OSaxWriterTest_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        OSaxWriterTest_CreateInstance,
                                        OSaxWriterTest_getSupportedServiceNames() );
        }
        else if (aImplementationName == OSaxParserTest_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        OSaxParserTest_CreateInstance,
                                        OSaxParserTest_getSupportedServiceNames() );
        }
        if (xRet.is())
        {
            xRet->acquire();
            pRet = xRet.get();
        }
    }

    return pRet;
}

}


