/*************************************************************************
 *
 *  $RCSfile: testsax.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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
#include <smart/com/sun/star/test/XSimpleTest.hxx>

#include <rtl/wstring.hxx>
#include <osl/time.h>
//#include <vos/dynload.hxx>
//#include <vos/diagnose.hxx>
#include <usr/weak.hxx>
#include <tools/string.hxx>
#include <vos/conditn.hxx>
#include <assert.h>

#include <smart/com/sun/star/io/XOutputStream.hxx>
#include <smart/com/sun/star/xml/sax/SAXParseException.hxx>
#include <smart/com/sun/star/xml/sax/XParser.hxx>
#include <smart/com/sun/star/xml/sax/XExtendedDocumentHandler.hxx>

#include <smart/com/sun/star/lang/XMultiServiceFactory.hxx>  // for the multiservice-factories
#include <usr/factoryhlp.hxx>

#include <usr/reflserv.hxx>  // for EXTERN_SERVICE_CALLTYPE

#include "factory.hxx"

#ifdef _USE_NAMESPACE
using namespace rtl;
using namespace vos;
using namespace usr;
#endif


#define BUILD_ERROR(expr, Message)\
        {\
            m_seqErrors.realloc( m_seqErrors.getLen() + 1 ); \
        m_seqExceptions.realloc(  m_seqExceptions.getLen() + 1 ); \
        String str; \
        str += __FILE__;\
        str += " "; \
        str += "(" ; \
        str += __LINE__ ;\
        str += ")\n";\
        str += "[ " ; \
        str += #expr; \
        str += " ] : " ; \
        str += Message; \
        m_seqErrors.getArray()[ m_seqErrors.getLen()-1] = StringToUString( str , CHARSET_SYSTEM ); \
        }\
        ((void)0)


#define WARNING_ASSERT(expr, Message) \
        if( ! (expr) ) { \
            m_seqWarnings.realloc( m_seqErrors.getLen() +1 ); \
            String str;\
            str += __FILE__;\
            str += " "; \
            str += "(" ; \
            str += __LINE__ ;\
            str += ")\n";\
            str += "[ " ; \
            str += #expr; \
            str += " ] : " ; \
            str += Message; \
            m_seqWarnings.getArray()[ m_seqWarnings.getLen()-1] = StringToUString( str , CHARSET_SYSTEM ); \
            return; \
        }\
        ((void)0)

#define ERROR_ASSERT(expr, Message) \
        if( ! (expr) ) { \
            BUILD_ERROR(expr, Message );\
            return; \
        }\
        ((void)0)

#define ERROR_EXCEPTION_ASSERT(expr, Message, Exception) \
    if( !(expr)) { \
        BUILD_ERROR(expr,Message);\
        m_seqExceptions.getArray()[ m_seqExceptions.getLen()-1] = UsrAny( Exception );\
        return; \
    } \
    ((void)0)

/****
* test szenarios :
*
*
*
****/



class OSaxParserTest :
        public XSimpleTest,
        public OWeakObject
{
public:
    OSaxParserTest( const XMultiServiceFactoryRef & rFactory ) : m_rFactory( rFactory )
    {

    }

public: // refcounting
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                        acquire()                        { OWeakObject::acquire(); }
    void                        release()                        { OWeakObject::release(); }
    void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

public:
    virtual void testInvariant(const UString& TestName, const XInterfaceRef& TestObject)
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );

    virtual INT32 test( const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );

    virtual BOOL testPassed(void)                               THROWS( (   UsrSystemException) );
    virtual Sequence< UString > getErrors(void)                 THROWS( (UsrSystemException) );
    virtual Sequence< UsrAny > getErrorExceptions(void)         THROWS( (UsrSystemException) );
    virtual Sequence< UString > getWarnings(void)               THROWS( (UsrSystemException) );

private:
    void testSimple( const XParserRef &r );
    void testNamespaces( const XParserRef &r );
    void testFile(  const XParserRef &r );
    void testEncoding( const XParserRef &rParser );
    void testPerformance( const XParserRef &rParser );

private:
    Sequence<UsrAny>        m_seqExceptions;
    Sequence<UString>       m_seqErrors;
    Sequence<UString>       m_seqWarnings;
    XMultiServiceFactoryRef m_rFactory;
};



/**
* for external binding
*
*
**/
XInterfaceRef OSaxParserTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception))
{
    OSaxParserTest *p = new OSaxParserTest( rSMgr );
    XInterfaceRef xService = *p;
    return xService;
}


UString     OSaxParserTest_getServiceName( ) THROWS( () )
{
    return L"test.com.sun.star.xml.sax.Parser";
}

UString     OSaxParserTest_getImplementationName( ) THROWS( () )
{
    return L"test.extensions.xml.sax.Parser";
}

Sequence<UString> OSaxParserTest_getSupportedServiceNames( ) THROWS( () )
{
    Sequence<UString> aRet(1);

    aRet.getArray()[0] = OSaxParserTest_getImplementationName( );

    return aRet;
}


BOOL OSaxParserTest::queryInterface( Uik uik , XInterfaceRef &rOut )
{
    if( XSimpleTest::getSmartUik() == uik ) {
        rOut = (XSimpleTest *) this;
    }
    else {
        return OWeakObject::queryInterface( uik , rOut );
    }
    return TRUE;
}


void OSaxParserTest::testInvariant( const UString& TestName, const XInterfaceRef& TestObject )
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.xml.sax.Parser" == TestName ) {
        XParserRef parser( TestObject , USR_QUERY );

        ERROR_ASSERT( parser.is() , "XDataInputStream cannot be queried" );
    }
}


INT32 OSaxParserTest::test( const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.xml.sax.Parser" == TestName )  {
        TRY {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else {

                XParserRef parser( TestObject , USR_QUERY );

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
        CATCH( Exception , e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }
        END_CATCH;

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



BOOL OSaxParserTest::testPassed(void)                                       THROWS( (UsrSystemException) )
{
    return m_seqErrors.getLen() == 0;
}


Sequence< UString > OSaxParserTest::getErrors(void)                             THROWS( (UsrSystemException) )
{
    return m_seqErrors;
}


Sequence< UsrAny > OSaxParserTest::getErrorExceptions(void)                     THROWS( (UsrSystemException) )
{
    return m_seqExceptions;
}


Sequence< UString > OSaxParserTest::getWarnings(void)                       THROWS( (UsrSystemException) )
{
    return m_seqWarnings;
}

XInputStreamRef createStreamFromSequence( const Sequence<BYTE> seqBytes , XMultiServiceFactoryRef &xSMgr )
{
    XInterfaceRef xOutStreamService = xSMgr->createInstance( L"com.sun.star.io.Pipe" );
    assert( xOutStreamService.is() );
    XOutputStreamRef rOutStream( xOutStreamService , USR_QUERY );
    assert( rOutStream.is() );

    XInputStreamRef rInStream( xOutStreamService , USR_QUERY );
    assert( rInStream.is() );

    rOutStream->writeBytes( seqBytes );
    rOutStream->flush();
    rOutStream->closeOutput();

    return rInStream;
}

XInputStreamRef createStreamFromFile( const char *pcFile , XMultiServiceFactoryRef &xSMgr )
{
    FILE *f = fopen( pcFile , "rb" );
    XInputStreamRef r;

    if( f ) {
        fseek( f , 0 , SEEK_END );
        int nLength = ftell( f );
        fseek( f , 0 , SEEK_SET );

        Sequence<BYTE> seqIn(nLength);
        fread( seqIn.getArray() , nLength , 1 , f );

        r = createStreamFromSequence( seqIn , xSMgr );
        fclose( f );
    }
    return r;
}









#define PCHAR_TO_USTRING(x) StringToUString(String(x),CHARSET_PC_1252)
#define USTRING_TO_PCHAR(x) UStringToString(x,CHARSET_PC_437).GetStr()



class TestDocumentHandler :
            public XExtendedDocumentHandler,
            public XEntityResolver,
            public XErrorHandler,
            public OWeakObject
{
public:
    TestDocumentHandler( XMultiServiceFactoryRef  &r , BOOL bPrint )
    {
        m_xSMgr = r;
        m_bPrint = bPrint;
    }


public:
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                        acquire()                        { OWeakObject::acquire(); }
    void                        release()                        { OWeakObject::release(); }
    void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }


public: // Error handler
    virtual void error(const UsrAny& aSAXParseException) THROWS( (SAXException, UsrSystemException) )
    {
        printf( "Error !\n" );
        THROW( SAXException( L"error from error handler" , XInterfaceRef() , aSAXParseException ) );
    }
    virtual void fatalError(const UsrAny& aSAXParseException) THROWS( (SAXException, UsrSystemException) )
    {
        printf( "Fatal Error !\n" );
    }
    virtual void warning(const UsrAny& aSAXParseException) THROWS( (SAXException, UsrSystemException) )
    {
        printf( "Warning !\n" );
    }


public: // ExtendedDocumentHandler

    virtual void startDocument(void) THROWS( (SAXException, UsrSystemException) )
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
    virtual void endDocument(void) THROWS( (SAXException, UsrSystemException) )
    {
        if( m_bPrint ) {
            printf( "document finished\n" );
            printf( "(ElementCount %d),(AttributeCount %d),(WhitespaceCount %d),(CharCount %d)\n",
                m_iElementCount, m_iAttributeCount, m_iWhitespaceCount , m_iCharCount );
        }
    }
    virtual void startElement(const UString& aName, const XAttributeListRef& xAttribs)
                                    THROWS( (SAXException,UsrSystemException) )
    {

        if( m_rLocator.is() ) {
            if( m_bPrint )
                printf( "%s(%d):" , USTRING_TO_PCHAR( m_rLocator->getSystemId() ) ,
                                    m_rLocator->getLineNumber() );
        }
        if( m_bPrint ) {
            int i;
            for( i = 0; i < m_iLevel ; i ++ ) {
                printf( "  " );
            }
            printf( "<%s> " , USTRING_TO_PCHAR( aName ) );

            for( i = 0 ; i < xAttribs->getLength() ; i ++ ) {
                printf( "(%s,%s,'%s')" ,    USTRING_TO_PCHAR( xAttribs->getNameByIndex( i ) ) ,
                                            USTRING_TO_PCHAR( xAttribs->getTypeByIndex( i ) ) ,
                                            USTRING_TO_PCHAR( xAttribs->getValueByIndex( i ) ) );
            }
            printf( "\n" );
        }
        m_iLevel ++;
        m_iElementCount ++;
        m_iAttributeCount += xAttribs->getLength();
    }
    virtual void endElement(const UString& aName) THROWS( (SAXException,UsrSystemException) )
    {
        assert( m_iLevel );
        m_iLevel --;
        if( m_bPrint ) {
            int i;
            for( i = 0; i < m_iLevel ; i ++ ) {
                printf( "  " );
            }
            printf( "</%s>\n" , USTRING_TO_PCHAR( aName ) );
        }
    }

    virtual void characters(const UString& aChars) THROWS( (SAXException,UsrSystemException) )
    {
        if( m_bPrint ) {
            int i;
            for( i = 0; i < m_iLevel ; i ++ ) {
                printf( "  " );
            }
            printf( "%s\n" , USTRING_TO_PCHAR( aChars ) );
        }
        m_iCharCount += aChars.len();
    }
    virtual void ignorableWhitespace(const UString& aWhitespaces) THROWS( (SAXException,UsrSystemException) )
    {
        m_iWhitespaceCount += aWhitespaces.len();
    }

    virtual void processingInstruction(const UString& aTarget, const UString& aData) THROWS( (SAXException,UsrSystemException) )
    {
        if( m_bPrint )
                    printf( "PI : %s,%s\n" , USTRING_TO_PCHAR( aTarget ) , USTRING_TO_PCHAR( aData ) );
    }

    virtual void setDocumentLocator(const XLocatorRef& xLocator) THROWS( (SAXException,UsrSystemException) )
    {
        m_rLocator = xLocator;
    }

    virtual InputSource resolveEntity(const UString& sPublicId, const UString& sSystemId)
                                                                        THROWS( (SAXException,UsrSystemException) )
    {
        InputSource source;
        source.sSystemId = sSystemId;
        source.sPublicId = sPublicId;
        source.aInputStream = createStreamFromFile( USTRING_TO_PCHAR( sSystemId ) , m_xSMgr );

        return source;
    }

    virtual void startCDATA(void) THROWS( (SAXException,UsrSystemException) )
    {
        if( m_bPrint ) {
            printf( "CDataStart :\n" );
        }
    }
    virtual void endCDATA(void) THROWS( (SAXException,UsrSystemException) )
    {
        if( m_bPrint ) {
            printf( "CEndStart :\n" );
        }
    }
    virtual void comment(const UString& sComment) THROWS( (SAXException,UsrSystemException) )
    {
        if( m_bPrint ) {
            printf( "<!--%s-->\n" , USTRING_TO_PCHAR( sComment ) );
        }
    }
    virtual void unknown(const UString& sString) THROWS( (SAXException,UsrSystemException) )
    {
        if( m_bPrint ) {
            printf( "UNKNOWN : {%s}\n" , USTRING_TO_PCHAR( sString ) );
        }
    }

    virtual void allowLineBreak( void) THROWS( (SAXException, UsrSystemException ) )
    {

    }


public:
    int m_iLevel;
    int m_iElementCount;
    int m_iAttributeCount;
    int m_iWhitespaceCount;
    int m_iCharCount;
    BOOL m_bPrint;

    XMultiServiceFactoryRef m_xSMgr;
    XLocatorRef m_rLocator;
};

BOOL TestDocumentHandler::queryInterface( Uik aUik , XInterfaceRef & rOut )
{
    if( aUik == XDocumentHandler::getSmartUik() ) {
        rOut = (XDocumentHandler * )this;
    }
    else if ( aUik == XExtendedDocumentHandler::getSmartUik() ) {
        rOut = (XExtendedDocumentHandler *) this;
    }
    else if ( aUik == XEntityResolver::getSmartUik() ) {
        rOut = (XEntityResolver *) this;
    }
    else if ( aUik == XErrorHandler::getSmartUik() ) {
        rOut = (XErrorHandler * ) this;
    }
    else {
        return OWeakObject::queryInterface( aUik , rOut );
    }
    return TRUE;
}




void OSaxParserTest::testSimple(    const XParserRef &rParser )
{

    char TestString[] =
                        "<!DOCTYPE personnel [\n"
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

    Sequence<BYTE> seqBytes( strlen( TestString ) );
    memcpy( seqBytes.getArray() , TestString , strlen( TestString ) );


    XInputStreamRef rInStream;
    UString sInput;
    rInStream = createStreamFromSequence( seqBytes , m_rFactory );
    sInput = UString( L"internal" );

    if( rParser.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , FALSE );
        XDocumentHandlerRef rDocHandler( (XDocumentHandler *) pDocHandler , USR_QUERY );
        XEntityResolverRef  rEntityResolver( (XEntityResolver *) pDocHandler , USR_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );

        TRY {
            rParser->parseStream( source );
            ERROR_ASSERT( pDocHandler->m_iElementCount      == 4 , "wrong element count"    );
            ERROR_ASSERT( pDocHandler->m_iAttributeCount    == 2 , "wrong attribut count"   );
            ERROR_ASSERT( pDocHandler->m_iCharCount         == 130 , "wrong char count"     );
            ERROR_ASSERT( pDocHandler->m_iWhitespaceCount   == 0, "wrong whitespace count" );
        }
        CATCH( SAXParseException , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );
        }
        AND_CATCH( SAXException , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );

        }
        AND_CATCH( Exception , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 1 , "unknown exception" );
        }
        END_CATCH;

    }


}

void OSaxParserTest::testNamespaces( const XParserRef &rParser )
{

    char TestString[] =
    "<?xml version='1.0'?>\n"
      "<!-- all elements here are explicitly in the HTML namespace -->\n"
      "<html:html xmlns:html='http://www.w3.org/TR/REC-html40'>\n"
        "<html:head><html:title>Frobnostication</html:title></html:head>\n"
        "<html:body><html:p>Moved to \n"
          "<html:a href='http://frob.com'>here.</html:a></html:p></html:body>\n"
      "</html:html>\n";

    Sequence<BYTE> seqBytes( strlen( TestString ) );
    memcpy( seqBytes.getArray() , TestString , strlen( TestString ) );


    XInputStreamRef rInStream;
    UString sInput;

    rInStream = createStreamFromSequence( seqBytes , m_rFactory );
    sInput = UString( L"internal" );

    if( rParser.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , FALSE );
        XDocumentHandlerRef rDocHandler( (XDocumentHandler *) pDocHandler , USR_QUERY );
        XEntityResolverRef  rEntityResolver( (XEntityResolver *) pDocHandler , USR_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );

        TRY {
            rParser->parseStream( source );
            ERROR_ASSERT( pDocHandler->m_iElementCount      == 6 , "wrong element count"    );
            ERROR_ASSERT( pDocHandler->m_iAttributeCount    == 2 , "wrong attribut count"   );
            ERROR_ASSERT( pDocHandler->m_iCharCount         == 33, "wrong char count"       );
            ERROR_ASSERT( pDocHandler->m_iWhitespaceCount   == 0 , "wrong whitespace count" );
        }
        CATCH( SAXParseException , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );
        }
        AND_CATCH( SAXException , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );

        }
        AND_CATCH( Exception , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 1 , "unknown exception" );
        }
        END_CATCH;
    }
}

void OSaxParserTest::testEncoding( const XParserRef &rParser )
{
    char TestString[] =
    "<?xml version='1.0' encoding=\"iso-8859-1\"?>\n"
      "<!-- all elements here are explicitly in the HTML namespace -->\n"
      "<html:html xmlns:html='http://www.w3.org/TR/REC-html40'>\n"
        "<html:head><html:title>Frobnostication</html:title></html:head>\n"
        "<html:body><html:p>Moved to ß\n"
          "<html:a href='http://frob.com'>here.</html:a></html:p></html:body>\n"
      "</html:html>\n";

    Sequence<BYTE> seqBytes( strlen( TestString ) );
    memcpy( seqBytes.getArray() , TestString , strlen( TestString ) );


    XInputStreamRef rInStream;
    UString sInput;

    rInStream = createStreamFromSequence( seqBytes , m_rFactory );
    sInput = UString( L"internal" );

    if( rParser.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , FALSE );
        XDocumentHandlerRef rDocHandler( (XDocumentHandler *) pDocHandler , USR_QUERY );
        XEntityResolverRef  rEntityResolver( (XEntityResolver *) pDocHandler , USR_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );
        TRY {
            rParser->parseStream( source );
        }
        CATCH( SAXParseException , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );
        }
        AND_CATCH( SAXException , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );

        }
        AND_CATCH( Exception , e ) {
            BUILD_ERROR( 1 , USTRING_TO_PCHAR( e.Message ) );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 1 , "unknown exception" );
        }
        END_CATCH;

    }

}

void OSaxParserTest::testFile( const XParserRef & rParser )
{

    XInputStreamRef rInStream = createStreamFromFile( "testsax.xml" , m_rFactory );
    UString sInput = UString( PCHAR_TO_USTRING( "testsax.xml" ) );


    if( rParser.is() && rInStream.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , TRUE );
        XDocumentHandlerRef rDocHandler( (XDocumentHandler *) pDocHandler , USR_QUERY );
        XEntityResolverRef  rEntityResolver( (XEntityResolver *) pDocHandler , USR_QUERY );
        XErrorHandlerRef    rErrorHandler( ( XErrorHandler * )pDocHandler , USR_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );
        rParser->setErrorHandler( rErrorHandler );

        TRY {
            rParser->parseStream( source );
        }
        CATCH( SAXParseException , e ) {
            UsrAny any;
            any.set( &e , SAXParseException_getReflection() );
            while(TRUE) {
                SAXParseException *pEx;
                if( any.getReflection() == SAXParseException_getReflection() ) {
                    pEx = ( SAXParseException * ) any.get();
                    printf( "%s\n" , UStringToString( pEx->Message , CHARSET_SYSTEM ).GetStr()  );
                    any = pEx->WrappedException;
                }
                else {
                    break;
                }
            }
        }
        AND_CATCH( SAXException , e ) {
            printf( "%s\n" , UStringToString( e.Message , CHARSET_SYSTEM ).GetStr()  );

        }
        AND_CATCH( Exception , e ) {
            printf( "normal exception ! %s\n", e.getName() );
        }
        AND_CATCH_ALL() {
            printf( "any exception !!!!\n" );
        }
        END_CATCH;
    }
}

void OSaxParserTest::testPerformance( const XParserRef & rParser )
{

    XInputStreamRef rInStream = createStreamFromFile( "testPerformance.xml" , m_rFactory );
    UString sInput = UString( PCHAR_TO_USTRING( "testperformance.xml" ) );

    if( rParser.is() && rInStream.is() ) {
        InputSource source;

        source.aInputStream = rInStream;
        source.sSystemId    = sInput;

        TestDocumentHandler *pDocHandler = new TestDocumentHandler( m_rFactory , FALSE );
        XDocumentHandlerRef rDocHandler( (XDocumentHandler *) pDocHandler , USR_QUERY );
        XEntityResolverRef  rEntityResolver( (XEntityResolver *) pDocHandler , USR_QUERY );
        XErrorHandlerRef    rErrorHandler( ( XErrorHandler * )pDocHandler , USR_QUERY );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );
        rParser->setErrorHandler( rErrorHandler );

        TRY {
            TimeValue aStartTime, aEndTime;
            osl_getSystemTime( &aStartTime );
            rParser->parseStream( source );
            osl_getSystemTime( &aEndTime );

            double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
            double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);

            printf( "Performance reading : %g s\n" , fEnd - fStart );

        }
        CATCH( SAXParseException , e ) {
            UsrAny any;
            any.set( &e , SAXParseException_getReflection() );
            while(TRUE) {
                SAXParseException *pEx;
                if( any.getReflection() == SAXParseException_getReflection() ) {
                    pEx = ( SAXParseException * ) any.get();
                    printf( "%s\n" , UStringToString( pEx->Message , CHARSET_SYSTEM ).GetStr()  );
                    any = pEx->WrappedException;
                }
                else {
                    break;
                }
            }
        }
        AND_CATCH( SAXException , e ) {
            printf( "%s\n" , UStringToString( e.Message , CHARSET_SYSTEM ).GetStr()  );

        }
        AND_CATCH( Exception , e ) {
            printf( "normal exception ! %s\n", e.getName() );
        }
        AND_CATCH_ALL() {
            printf( "any exception !!!!\n" );
        }
        END_CATCH;
    }

}


extern "C"
{
BOOL EXTERN_SERVICE_CALLTYPE exService_writeRegEntry(
        const UNO_INTERFACE(XRegistryKey)* xUnoKey)

{
    XRegistryKeyRef   xKey;
    uno2smart(xKey, *xUnoKey);

    UString str = UString( L"/" ) + OSaxParserTest_getImplementationName() + UString( L"/UNO/SERVICES" );
    XRegistryKeyRef xNewKey = xKey->createKey( str );
    xNewKey->createKey( OSaxParserTest_getServiceName() );

    str = UString( L"/" ) + OSaxWriterTest_getImplementationName() + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( OSaxWriterTest_getServiceName() );

    return TRUE;
}


UNO_INTERFACE(XInterface) EXTERN_SERVICE_CALLTYPE exService_getFactory
(
    const wchar_t* implementationName,
    const UNO_INTERFACE(XMultiServiceFactory)* xUnoFact,
    const UNO_INTERFACE(XRegistryKey)*
)
{
    UNO_INTERFACE(XInterface) xUnoRet = {0, 0};

    XInterfaceRef           xRet;
    XMultiServiceFactoryRef xSMgr;
    UString                 aImplementationName(implementationName);

    uno2smart(xSMgr, *xUnoFact);

    if (aImplementationName == OSaxWriterTest_getImplementationName() )
    {
        xRet = createSingleFactory( xSMgr, implementationName,
                                    OSaxWriterTest_CreateInstance,
                                    OSaxWriterTest_getSupportedServiceNames() );
    }
    else if (aImplementationName == OSaxParserTest_getImplementationName() )
    {
        xRet = createSingleFactory( xSMgr, implementationName,
                                    OSaxParserTest_CreateInstance,
                                    OSaxParserTest_getSupportedServiceNames() );
    }
    if (xRet.is())
    {
        smart2uno(xRet, xUnoRet);
    }

    return xUnoRet;
}

}


