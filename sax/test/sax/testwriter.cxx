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
#include <vector>
#include <stdio.h>

#include <com/sun/star/test/XSimpleTest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <osl/time.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>


using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;

namespace sax_test {

class OFileWriter :
        public WeakImplHelper< XOutputStream >
{
public:
    explicit OFileWriter( char *pcFile ) { strncpy( m_pcFile, pcFile, 256 - 1 ); m_f = 0; }


public:
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL flush()
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL closeOutput()
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
private:
    char m_pcFile[256];
    FILE *m_f;
};


void OFileWriter::writeBytes(const Sequence< sal_Int8 >& aData)
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    if( ! m_f ) {
        m_f = fopen( m_pcFile , "w" );
    }

       fwrite( aData.getConstArray() , 1 , aData.getLength() , m_f );
}


void OFileWriter::flush()
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    fflush( m_f );
}

void OFileWriter::closeOutput()
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    fclose( m_f );
    m_f = 0;
}


class OSaxWriterTest :
        public WeakImplHelper< XSimpleTest >
{
public:
    explicit OSaxWriterTest( const Reference < XMultiServiceFactory > & rFactory ) : m_rFactory( rFactory )
    {

    }
    ~OSaxWriterTest() {}


public:
    virtual void SAL_CALL testInvariant(
        const OUString& TestName,
        const Reference < XInterface >& TestObject)
        throw  (    IllegalArgumentException,
                    RuntimeException);

    virtual sal_Int32 SAL_CALL test(
        const OUString& TestName,
        const Reference < XInterface >& TestObject,
        sal_Int32 hTestHandle)
        throw  (    IllegalArgumentException,RuntimeException);

    virtual sal_Bool SAL_CALL testPassed()
        throw  (    RuntimeException);
    virtual Sequence< OUString > SAL_CALL getErrors()               throw  (RuntimeException);
    virtual Sequence< Any > SAL_CALL getErrorExceptions()       throw  (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings()             throw  (RuntimeException);

private:
    void testSimple( const Reference< XExtendedDocumentHandler > &r );
    void testExceptions( const Reference< XExtendedDocumentHandler > &r );
    void testDTD( const Reference< XExtendedDocumentHandler > &r );
    void testPerformance( const Reference< XExtendedDocumentHandler > &r );
    void writeParagraph( const Reference< XExtendedDocumentHandler > &r , const OUString & s);

private:
    Sequence<Any>       m_seqExceptions;
    Sequence<OUString>      m_seqErrors;
    Sequence<OUString>      m_seqWarnings;
    Reference < XMultiServiceFactory > m_rFactory;

};


/*----------------------------------------
*
*   Attributelist implementation
*
*----------------------------------------*/
struct AttributeListImpl_impl;
class AttributeListImpl : public WeakImplHelper< XAttributeList >
{
public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );
    ~AttributeListImpl();

public:
    virtual sal_Int16 SAL_CALL getLength() throw  (RuntimeException);
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i) throw  (RuntimeException);
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw  (RuntimeException);
    virtual OUString SAL_CALL getTypeByName(const OUString& aName) throw  (RuntimeException);
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i) throw  (RuntimeException);
    virtual OUString SAL_CALL getValueByName(const OUString& aName) throw  (RuntimeException);

public:
    void addAttribute( const OUString &sName ,
                       const OUString &sType ,
                       const OUString &sValue );
    void clear();

private:
    struct AttributeListImpl_impl *m_pImpl;
};


struct TagAttribute
{
    TagAttribute(){}
    TagAttribute( const OUString &sName,
                  const OUString &sType ,
                  const OUString &sValue )
    {
        sName     = sName;
        sType     = sType;
        sValue    = sValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
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


sal_Int16 AttributeListImpl::getLength() throw  (RuntimeException)
{
    return m_pImpl->vecAttribute.size();
}


AttributeListImpl::AttributeListImpl( const AttributeListImpl &r )
{
    m_pImpl = new AttributeListImpl_impl;
    *m_pImpl = *(r.m_pImpl);
}

OUString AttributeListImpl::getNameByIndex(sal_Int16 i) throw  (RuntimeException)
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeListImpl::getTypeByIndex(sal_Int16 i) throw  (RuntimeException)
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString AttributeListImpl::getValueByIndex(sal_Int16 i) throw  (RuntimeException)
{
    if( i < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString AttributeListImpl::getTypeByName( const OUString& sName ) throw  (RuntimeException)
{
    auto ii = std::find_if(m_pImpl->vecAttribute.begin(), m_pImpl->vecAttribute.end(),
        [&sName](const struct TagAttribute& rAttr) { return rAttr.sName == sName; });
    if (ii != m_pImpl->vecAttribute.end())
        return (*ii).sType;
    return OUString();
}

OUString AttributeListImpl::getValueByName(const OUString& sName) throw  (RuntimeException)
{
    auto ii = std::find_if(m_pImpl->vecAttribute.begin(), m_pImpl->vecAttribute.end(),
        [&sName](const struct TagAttribute& rAttr) { return rAttr.sName == sName; });
    if (ii != m_pImpl->vecAttribute.end())
        return (*ii).sValue;
    return OUString();
}


AttributeListImpl::AttributeListImpl()
{
    m_pImpl = new AttributeListImpl_impl;
}


AttributeListImpl::~AttributeListImpl()
{
    delete m_pImpl;
}


void AttributeListImpl::addAttribute(   const OUString &sName ,
                                        const OUString &sType ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}

void AttributeListImpl::clear()
{
    m_pImpl->vecAttribute.clear();

}


/**
* for external binding
*
*
**/
Reference < XInterface > SAL_CALL OSaxWriterTest_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr ) throw (Exception)
{
    OSaxWriterTest *p = new OSaxWriterTest( rSMgr );
    Reference < XInterface > xService = *p;
    return xService;
}

OUString     OSaxWriterTest_getServiceName( ) throw  ()
{
    return OUString( "test.com.sun.star.xml.sax.Writer");
}

OUString    OSaxWriterTest_getImplementationName( ) throw  ()
{
    return OUString( "test.extensions.xml.sax.Writer");
}

Sequence<OUString> OSaxWriterTest_getSupportedServiceNames( ) throw  ()
{
    Sequence<OUString> aRet { OSaxWriterTest_getImplementationName( ) };
    return aRet;
}


void OSaxWriterTest::testInvariant( const OUString& TestName,
                                    const Reference < XInterface >& TestObject )
    throw  (    IllegalArgumentException, RuntimeException)
{
    if( OUString("com.sun.star.xml.sax.Writer") == TestName ) {
        Reference< XDocumentHandler > doc( TestObject , UNO_QUERY );
        Reference< XExtendedDocumentHandler > ext( TestObject , UNO_QUERY );
        Reference< XActiveDataSource > source( TestObject , UNO_QUERY );

        ERROR_ASSERT( doc.is() , "XDocumentHandler cannot be queried" );
        ERROR_ASSERT( ext.is() , "XExtendedDocumentHandler cannot be queried" );
        ERROR_ASSERT( source.is() , "XActiveDataSource cannot be queried" );
    }
    else {
        BUILD_ERROR( 0 , "wrong test" );
    }
}


sal_Int32 OSaxWriterTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)
    throw  (    IllegalArgumentException,RuntimeException)
{
    if( OUString( "com.sun.star.xml.sax.Writer") == TestName )
    {
        try
        {
            if( 0 == hTestHandle )
            {
                testInvariant( TestName , TestObject );
            }
            else
            {
                Reference< XExtendedDocumentHandler > writer( TestObject , UNO_QUERY );

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
        catch( Exception & e )  {
            OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            BUILD_ERROR( 0 , o.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

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


sal_Bool OSaxWriterTest::testPassed()                   throw  (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OSaxWriterTest::getErrors() throw  (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OSaxWriterTest::getErrorExceptions()                    throw  (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OSaxWriterTest::getWarnings()                      throw  (RuntimeException)
{
    return m_seqWarnings;
}

void OSaxWriterTest::writeParagraph(
    const Reference< XExtendedDocumentHandler > &r ,
    const OUString & s)
{
    int nMax = s.getLength();
    int nStart = 0;

    Sequence<sal_uInt16> seq( s.getLength() );
    memcpy( seq.getArray() , s.getStr() , s.getLength() * sizeof( sal_uInt16 ) );

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


void OSaxWriterTest::testSimple( const Reference< XExtendedDocumentHandler > &r )
{
    OUString testParagraph = OUString(
        "This is a stupid test to check whether the SAXWriter possibly makes "
        "line breaks halfway correctly or whether it writes the line to the "
        "bitter end." );

    OFileWriter *pw = new OFileWriter("output.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    Reference< XAttributeList > rList( (XAttributeList *) pList , UNO_QUERY );
    Reference< XOutputStream > ref( ( XOutputStream * ) pw , UNO_QUERY );

    Reference< XActiveDataSource > source( r , UNO_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );

    r->startDocument();

    pList->addAttribute( OUString( "Arg1" ),
                         OUString( "CDATA") ,
                         OUString( "bla\n   u") );
    pList->addAttribute( OUString( "Arg2") ,
                         OUString( "CDATA") ,
                         OUString( "blub") );

    r->startElement( OUString( "tag1")  , rList );
    r->ignorableWhitespace( OUString() );

    r->characters( OUString( "huhu") );
    r->ignorableWhitespace( OUString() );

    r->startElement( OUString( "hi") , rList );
    r->ignorableWhitespace( OUString() );

    // the ampersand must be converted & -> &amp;
    r->characters( OUString( "&#252;") );

    // Test added for mib. Tests if errors during conversions occurs
    r->ignorableWhitespace( OUString() );
    sal_Char array[256];
    for( sal_Int32 n = 32 ; n < 254 ; n ++ ) {
        array[n-32] = n;
    }
    array[254-32] = 0;
    r->characters(
        OStringToOUString( array , RTL_TEXTENCODING_SYMBOL )
        );
    r->ignorableWhitespace( OUString() );

    // '>' must not be converted
    r->startCDATA();
    r->characters( OUString( ">fsfsdf<")  );
    r->endCDATA();
    r->ignorableWhitespace( OUString() );

    writeParagraph( r , testParagraph );


    r->ignorableWhitespace( OUString() );
    r->comment( OUString( "This is a comment !") );
    r->ignorableWhitespace( OUString() );

    r->startElement( OUString( "emptytagtest")  , rList );
    r->endElement( OUString( "emptytagtest") );

    r->endElement( OUString( "hi") );
    r->ignorableWhitespace( OUString() );

    r->endElement( OUString( "tag1") );
    r->endDocument();

}

void OSaxWriterTest::testExceptions( const Reference< XExtendedDocumentHandler > & r )
{

    OFileWriter *pw = new OFileWriter("output2.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    Reference< XAttributeList > rList( (XAttributeList *) pList , UNO_QUERY );
    Reference< XOutputStream > ref( ( XOutputStream * ) pw , UNO_QUERY );

    Reference< XActiveDataSource > source( r , UNO_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );

    { // startDocument must be called before start element
        sal_Bool bException = sal_True;
        try
        {
            r->startElement( OUString( "huhu") , rList );
            bException = sal_False;
        }
        catch( SAXException &e )
        {

        }
        ERROR_ASSERT( bException , "expected exception not thrown !" );
    }

    r->startDocument();

    r->startElement( OUString( "huhu") , rList );
    r->startCDATA();

    {
        sal_Bool bException = sal_True;
        try{
            r->startElement( OUString( "huhu") , rList );
            bException = sal_False;
        }
        catch( SAXException &e ) {

        }
        ERROR_ASSERT( bException , "expected exception not thrown !" );
    }

    r->endCDATA();

    {
        sal_Unicode array[] = { 'a' , 'b' , 4 , 9 , 10 };
        OUString o( array , 5 );
        try
        {
            r->characters( o );
            ERROR_ASSERT( 0 , "Writer allowed to write forbidden characters" );
        }
        catch( SAXException & e )
        {

        }
    }
    r->endElement( OUString( "huhu") );

    r->endDocument();
}


void OSaxWriterTest::testDTD(const  Reference< XExtendedDocumentHandler > &r )
{
    OFileWriter *pw = new OFileWriter("outputDTD.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    Reference< XAttributeList > rList( (XAttributeList *) pList , UNO_QUERY );
    Reference< XOutputStream > ref( ( XOutputStream * ) pw , UNO_QUERY );

    Reference< XActiveDataSource > source( r , UNO_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );


    r->startDocument();
    r->unknown( OUString( "<!DOCTYPE iCalendar >\n") );
    r->startElement( OUString( "huhu") , rList );

    r->endElement( OUString( "huhu") );
    r->endDocument();
}

void OSaxWriterTest::testPerformance(const  Reference< XExtendedDocumentHandler > &r )
{
    OFileWriter *pw = new OFileWriter("testPerformance.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    OUString testParagraph =
        OUString(
            "This is a stupid test to check whether the SAXWriter possibly makes "
            "line breaks halfway correctly or whether it writes the line to the "
            "bitter end." );


    Reference< XAttributeList > rList( (XAttributeList *) pList , UNO_QUERY );
    Reference< XOutputStream > ref( ( XOutputStream * ) pw , UNO_QUERY );

    Reference< XActiveDataSource > source( r , UNO_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );

    TimeValue aStartTime, aEndTime;
    osl_getSystemTime( &aStartTime );


    r->startDocument();
    // just write a bunch of xml tags !
    // for performance testing
    sal_Int32 i2;
    OUString huhu( "huhu" );
    const int ITERATIONS = 125;
    for( i2 = 0 ; i2 < ITERATIONS ; i2 ++ )
    {
        r->startElement( OUString( "tag" ) +
                         OUString::valueOf( i2 ), rList );
        for( sal_Int32 i = 0 ; i < 450 ; i ++ )
        {
            r->ignorableWhitespace( "" );
            r->startElement( huhu , rList );
            r->characters( testParagraph );

            r->ignorableWhitespace( "" );
            r->endElement( huhu );
        }
    }
    for( i2 = ITERATIONS-1 ; i2 >= 0  ; i2-- )
    {
        r->ignorableWhitespace( "" );
        r->endElement( OUString( "tag" ) + OUString::valueOf( i2 ) );
    }

    r->endDocument();

    osl_getSystemTime( &aEndTime );

    double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
    double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);

    printf( "Performance writing : %g s\n" , fEnd - fStart );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
