/*************************************************************************
 *
 *  $RCSfile: testwriter.cxx,v $
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

//#include <tools/presys.h>
#include <vector>
//#include <tools/postsys.h>
#include <smart/com/sun/star/test/XSimpleTest.hxx>
#include <smart/com/sun/star/lang/XMultiServiceFactory.hxx>  // for the multiservice-factories

#include <stdio.h>

#include <smart/com/sun/star/io/XActiveDataSource.hxx>
#include <smart/com/sun/star/io/XOutputStream.hxx>
#include <smart/com/sun/star/xml/sax/SAXParseException.hxx>
#include <smart/com/sun/star/xml/sax/XParser.hxx>
#include <smart/com/sun/star/xml/sax/XExtendedDocumentHandler.hxx>

#include <rtl/wstring.hxx>
#include <osl/time.h>
#include <usr/weak.hxx>
#include <tools/string.hxx>
#include <assert.h>

#include <usr/factoryhlp.hxx>

#include <usr/reflserv.hxx>  // for EXTERN_SERVICE_CALLTYPE


#ifdef _USE_NAMESPACE
using namespace std;
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


class OFileWriter :
        public XOutputStream,
        public OWeakObject
{
public:
    OFileWriter( char *pcFile ) { strcpy( m_pcFile , pcFile ); m_f = 0; }


public: // refcounting
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut )
    {
        if( XOutputStream::getSmartUik() == aUik ) {
            rOut = (XOutputStream *) this;
        }
        else return OWeakObject::queryInterface( aUik , rOut );

        return TRUE;
    }
    void                        acquire()                        { OWeakObject::acquire(); }
    void                        release()                        { OWeakObject::release(); }
    void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

public:
    virtual void writeBytes(const Sequence< BYTE >& aData)
                        THROWS( (NotConnectedException, BufferSizeExceededException, UsrSystemException) );
    virtual void flush(void)
                        THROWS( (NotConnectedException, BufferSizeExceededException, UsrSystemException) );
    virtual void closeOutput(void)
                        THROWS( (NotConnectedException, BufferSizeExceededException, UsrSystemException) );


private:
    char m_pcFile[256];
    FILE *m_f;
};


void OFileWriter::writeBytes(const Sequence< BYTE >& aData)
            THROWS( (NotConnectedException, BufferSizeExceededException, UsrSystemException) )
{
    if( ! m_f ) {
        m_f = fopen( m_pcFile , "w" );
    }

    fwrite( aData.getConstArray() , 1 , aData.getLen() , m_f );

}


void OFileWriter::flush(void)
            THROWS( (NotConnectedException, BufferSizeExceededException, UsrSystemException) )
{
    fflush( m_f );
}

void OFileWriter::closeOutput(void)
                        THROWS( (NotConnectedException, BufferSizeExceededException, UsrSystemException) )
{
    fclose( m_f );
    m_f = 0;
}


class OSaxWriterTest :
        public XSimpleTest,
        public OWeakObject
{
public:
    OSaxWriterTest( const XMultiServiceFactoryRef & rFactory ) : m_rFactory( rFactory )
    {

    }
    ~OSaxWriterTest() {}

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
    void testSimple( const XExtendedDocumentHandlerRef &r );
    void testExceptions( const XExtendedDocumentHandlerRef &r );
    void testDTD( const XExtendedDocumentHandlerRef &r );
    void testPerformance( const XExtendedDocumentHandlerRef &r );
    void writeParagraph( const XExtendedDocumentHandlerRef &r , const UString & s);

private:
    Sequence<UsrAny>        m_seqExceptions;
    Sequence<UString>       m_seqErrors;
    Sequence<UString>       m_seqWarnings;
    XMultiServiceFactoryRef m_rFactory;

};



/*----------------------------------------
*
*   Attributlist implementation
*
*----------------------------------------*/
struct AttributeListImpl_impl;
class AttributeListImpl :
        public XAttributeList,
        public OWeakObject
{
public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );
    ~AttributeListImpl();

public:
    BOOL                    queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                    acquire()                        { OWeakObject::acquire(); }
    void                    release()                        { OWeakObject::release(); }
    void*                   getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

public:
    virtual INT16 getLength(void) THROWS( (UsrSystemException) );
    virtual UString getNameByIndex(INT16 i) THROWS( (UsrSystemException) );
    virtual UString getTypeByIndex(INT16 i) THROWS( (UsrSystemException) );
    virtual UString getTypeByName(const UString& aName) THROWS( (UsrSystemException) );
    virtual UString getValueByIndex(INT16 i) THROWS( (UsrSystemException) );
    virtual UString getValueByName(const UString& aName) THROWS( (UsrSystemException) );

public:
    void addAttribute( const UString &sName , const UString &sType , const UString &sValue );
    void clear();

private:
    struct AttributeListImpl_impl *m_pImpl;
};


struct TagAttribute
{
    TagAttribute(){}
    TagAttribute( const UString &sName, const UString &sType , const UString &sValue )
    {
        this->sName     = sName;
        this->sType     = sType;
        this->sValue    = sValue;
    }

    UString sName;
    UString sType;
    UString sValue;
};

struct AttributeListImpl_impl
{
    AttributeListImpl_impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }
    vector<struct TagAttribute> vecAttribute;
};



INT16 AttributeListImpl::getLength(void) THROWS( (UsrSystemException) )
{
    return m_pImpl->vecAttribute.size();
}


AttributeListImpl::AttributeListImpl( const AttributeListImpl &r )
{
    m_pImpl = new AttributeListImpl_impl;
    *m_pImpl = *(r.m_pImpl);
}

UString AttributeListImpl::getNameByIndex(INT16 i) THROWS( (UsrSystemException) )
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return UString();
}


UString AttributeListImpl::getTypeByIndex(INT16 i) THROWS( (UsrSystemException) )
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return UString();
}

UString AttributeListImpl::getValueByIndex(INT16 i) THROWS( (UsrSystemException) )
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return UString();

}

UString AttributeListImpl::getTypeByName( const UString& sName ) THROWS( (UsrSystemException) )
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return UString();
}

UString AttributeListImpl::getValueByName(const UString& sName) THROWS( (UsrSystemException) )
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return UString();
}


BOOL AttributeListImpl::queryInterface( Uik aUik, XInterfaceRef & rOut )
{
    if( aUik == XAttributeList::getSmartUik() ) {
        rOut = (XAttributeList * )this;
    }
    else {
        return OWeakObject::queryInterface( aUik , rOut );
    }
    return TRUE;
}


AttributeListImpl::AttributeListImpl()
{
    m_pImpl = new AttributeListImpl_impl;
}



AttributeListImpl::~AttributeListImpl()
{
    delete m_pImpl;
}


void AttributeListImpl::addAttribute(   const UString &sName ,
                                        const UString &sType ,
                                        const UString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}

void AttributeListImpl::clear()
{
    vector<struct TagAttribute> dummy;
    m_pImpl->vecAttribute.swap( dummy );

    assert( ! getLength() );
}











/**
* for external binding
*
*
**/
XInterfaceRef OSaxWriterTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception))
{
    OSaxWriterTest *p = new OSaxWriterTest( rSMgr );
    XInterfaceRef xService = *p;
    return xService;
}

UString     OSaxWriterTest_getServiceName( ) THROWS( () )
{
    return L"test.com.sun.star.xml.sax.Writer";
}

UString     OSaxWriterTest_getImplementationName( ) THROWS( () )
{
    return L"test.extensions.xml.sax.Writer";
}

Sequence<UString> OSaxWriterTest_getSupportedServiceNames( ) THROWS( () )
{
    Sequence<UString> aRet(1);

    aRet.getArray()[0] = OSaxWriterTest_getImplementationName( );

    return aRet;
}


BOOL OSaxWriterTest::queryInterface( Uik uik , XInterfaceRef &rOut )
{
    if( XSimpleTest::getSmartUik() == uik ) {
        rOut = (XSimpleTest *) this;
    }
    else {
        return OWeakObject::queryInterface( uik , rOut );
    }
    return TRUE;
}


void OSaxWriterTest::testInvariant( const UString& TestName, const XInterfaceRef& TestObject )
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.xml.sax.Writer" == TestName ) {
        XDocumentHandlerRef doc( TestObject , USR_QUERY );
        XExtendedDocumentHandlerRef ext( TestObject , USR_QUERY );
        XActiveDataSourceRef source( TestObject , USR_QUERY );

        ERROR_ASSERT( doc.is() , "XDocumentHandler cannot be queried" );
        ERROR_ASSERT( ext.is() , "XExtendedDocumentHandler cannot be queried" );
        ERROR_ASSERT( source.is() , "XActiveDataSource cannot be queried" );
    }
    else {
        BUILD_ERROR( 0 , "wrong test" );
    }
}


INT32 OSaxWriterTest::test( const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.xml.sax.Writer" == TestName )  {
        TRY {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else {

                XExtendedDocumentHandlerRef writer( TestObject , USR_QUERY );

                if( 1 == hTestHandle ) {
                    testSimple( writer );
                }
                else if( 2 == hTestHandle ) {
                    testExceptions( writer );
                }
                else if( 3 == hTestHandle ) {
                    testDTD( writer );
                }
                else if( 4 == hTestHandle ) {
                    testPerformance( writer );
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

        if( hTestHandle >= 5) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        BUILD_ERROR( 0 , "service not supported by test." );
    }
    return hTestHandle;
}



BOOL OSaxWriterTest::testPassed(void)                                       THROWS( (UsrSystemException) )
{
    return m_seqErrors.getLen() == 0;
}


Sequence< UString > OSaxWriterTest::getErrors(void)                             THROWS( (UsrSystemException) )
{
    return m_seqErrors;
}


Sequence< UsrAny > OSaxWriterTest::getErrorExceptions(void)                     THROWS( (UsrSystemException) )
{
    return m_seqExceptions;
}


Sequence< UString > OSaxWriterTest::getWarnings(void)                       THROWS( (UsrSystemException) )
{
    return m_seqWarnings;
}

void OSaxWriterTest::writeParagraph( const XExtendedDocumentHandlerRef &r , const UString & s)
{
    int nMax = s.len();
    int nStart = 0;

    Sequence<UINT16> seq( s.len() );
    memcpy( seq.getArray() , s.getStr() , s.len() * sizeof( UINT16 ) );

    for( int n = 1 ; n < nMax ; n++ ){
        if( 32 == seq.getArray()[n] ) {
            r->allowLineBreak();
            r->characters( s.copy( nStart , n - nStart ) );
            nStart = n;
        }
    }
    r->allowLineBreak();
    r->characters( s.copy( nStart , n - nStart ) );


}



void OSaxWriterTest::testSimple( const XExtendedDocumentHandlerRef &r )
{
    UString testParagraph = L"Dies ist ein bloeder Test um zu uberpruefen, ob der SAXWriter "
                            L"wohl Zeilenumbrueche halbwegs richtig macht oder ob er die Zeile "
                            L"bis zum bitteren Ende schreibt.";

    OFileWriter *pw = new OFileWriter("output.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    XAttributeListRef rList( (XAttributeList *) pList , USR_QUERY );
    XOutputStreamRef ref( ( XOutputStream * ) pw , USR_QUERY );

    XActiveDataSourceRef source( r , USR_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );

    r->startDocument();

    pList->addAttribute( L"Arg1" , L"CDATA" , L"bla\n   u" );
    pList->addAttribute( L"Arg2" , L"CDATA" , L"blub" );

    r->startElement( L"tag1"  , rList );
    r->ignorableWhitespace( L"" );

    r->characters( L"huhu" );
    r->ignorableWhitespace( L"" );

    r->startElement( L"hi" , rList );
    r->ignorableWhitespace( L"" );

    // the enpassant must be converted & -> &amp;
    r->characters( L"&#252;" );

    // Test added for mib. Tests if errors during conversions occurs
    r->ignorableWhitespace( UString() );
    sal_Char array[256];
    for( sal_Int32 n = 32 ; n < 254 ; n ++ ) {
        array[n-32] = n;
    }
    array[254-32] = 0;
    r->characters(
        StringToUString( array , RTL_TEXTENCODING_SYMBOL )
        );
    r->ignorableWhitespace( UString() );

    // '>' must not be converted
    r->startCDATA();
    r->characters( L">fsfsdf<" );
    r->endCDATA();
    r->ignorableWhitespace( UString() );

    writeParagraph( r , testParagraph );


    r->ignorableWhitespace( UString() );
    r->comment( L"Dies ist ein Kommentar !" );
    r->ignorableWhitespace( UString() );

    r->startElement( L"emptytagtest"  , rList );
    r->endElement( L"emptytagtest" );

    r->endElement( L"hi" );
    r->ignorableWhitespace( L"" );

    r->endElement( L"tag1" );
    r->endDocument();

}

void OSaxWriterTest::testExceptions( const XExtendedDocumentHandlerRef & r )
{

    OFileWriter *pw = new OFileWriter("output2.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    XAttributeListRef rList( (XAttributeList *) pList , USR_QUERY );
    XOutputStreamRef ref( ( XOutputStream * ) pw , USR_QUERY );

    XActiveDataSourceRef source( r , USR_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );

    { // startDocument must be called before start element
        BOOL bException = TRUE;
        TRY {
            r->startElement( L"huhu" , rList );
            bException = FALSE;
        }
        CATCH( SAXException ,e ) {

        }
        END_CATCH;
        ERROR_ASSERT( bException , "expected exception not thrown !" );
    }

    r->startDocument();

    r->startElement( L"huhu" , rList );
    r->startCDATA();

    {
        BOOL bException = TRUE;
        TRY{
            r->startElement( L"huhu" , rList );
            bException = FALSE;
        }
        CATCH( SAXException ,e ) {

        }
        END_CATCH;
        ERROR_ASSERT( bException , "expected exception not thrown !" );
    }

    r->endCDATA();
    r->endElement( L"hi" );

    r->endDocument();
}


void OSaxWriterTest::testDTD(const  XExtendedDocumentHandlerRef &r )
{
    OFileWriter *pw = new OFileWriter("outputDTD.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    XAttributeListRef rList( (XAttributeList *) pList , USR_QUERY );
    XOutputStreamRef ref( ( XOutputStream * ) pw , USR_QUERY );

    XActiveDataSourceRef source( r , USR_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );


    r->startDocument();
    r->unknown( L"<!DOCTYPE iCalendar >\n" );
    r->startElement( L"huhu" , rList );

    r->endElement( L"huhu" );
    r->endDocument();
}

void OSaxWriterTest::testPerformance(const  XExtendedDocumentHandlerRef &r )
{
    OFileWriter *pw = new OFileWriter("testPerformance.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    UString testParagraph = L"Dies ist ein bloeder Test um zu uberpruefen, ob der SAXWriter "
                            L"wohl > Zeilenumbrueche halbwegs richtig macht oder ob er die Zeile "
                            L"bis zum bitteren Ende schreibt.";


    XAttributeListRef rList( (XAttributeList *) pList , USR_QUERY );
    XOutputStreamRef ref( ( XOutputStream * ) pw , USR_QUERY );

    XActiveDataSourceRef source( r , USR_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );

    TimeValue aStartTime, aEndTime;
    osl_getSystemTime( &aStartTime );


    r->startDocument();
    // just write a bunch of xml tags !
    // for performance testing
    sal_Int32 i2;
    for( i2 = 0 ; i2 < 15 ; i2 ++ )
    {
        r->startElement( UString( L"tag" ) + UString::valueOf( i2 ), rList );
        for( sal_Int32 i = 0 ; i < 450 ; i ++ )
        {
            r->ignorableWhitespace( L"");
            r->startElement( L"huhu" , rList );
            r->characters( testParagraph );
//          writeParagraph( r , testParagraph );

            r->ignorableWhitespace( L"");
            r->endElement( L"huhu" );
        }
    }
    for( i2 = 14 ; i2 >= 0  ; i2-- )
    {
        r->ignorableWhitespace( L"");
        r->endElement( UString( L"tag" ) + UString::valueOf( i2 ) );
    }

    r->endDocument();

    osl_getSystemTime( &aEndTime );

    double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
    double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);

    printf( "Performance writing : %g s\n" , fEnd - fStart );
}
