/*************************************************************************
 *
 *  $RCSfile: testwriter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-13 06:49:11 $
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
#include <vector>
#include <stdio.h>
#include <assert.h>

#include <com/sun/star/test/XSimpleTest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>  // for the multiservice-factories

#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <osl/time.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>


using namespace ::std;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;

#include "factory.hxx"

namespace sax_test {

class OFileWriter :
        public WeakImplHelper1< XOutputStream >
{
public:
    OFileWriter( char *pcFile ) { strcpy( m_pcFile , pcFile ); m_f = 0; }


public:
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL flush(void)
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL closeOutput(void)
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


void OFileWriter::flush(void)
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    fflush( m_f );
}

void OFileWriter::closeOutput(void)
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    fclose( m_f );
    m_f = 0;
}


class OSaxWriterTest :
        public WeakImplHelper1< XSimpleTest >
{
public:
    OSaxWriterTest( const Reference < XMultiServiceFactory > & rFactory ) : m_rFactory( rFactory )
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

    virtual sal_Bool SAL_CALL testPassed(void)
        throw  (    RuntimeException);
    virtual Sequence< OUString > SAL_CALL getErrors(void)               throw  (RuntimeException);
    virtual Sequence< Any > SAL_CALL getErrorExceptions(void)       throw  (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings(void)             throw  (RuntimeException);

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
*   Attributlist implementation
*
*----------------------------------------*/
struct AttributeListImpl_impl;
class AttributeListImpl : public WeakImplHelper1< XAttributeList >
{
public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );
    ~AttributeListImpl();

public:
    virtual sal_Int16 SAL_CALL getLength(void) throw  (RuntimeException);
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
        this->sName     = sName;
        this->sType     = sType;
        this->sValue    = sValue;
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



sal_Int16 AttributeListImpl::getLength(void) throw  (RuntimeException)
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
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString AttributeListImpl::getValueByName(const OUString& sName) throw  (RuntimeException)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
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
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.xml.sax.Writer"));
}

OUString    OSaxWriterTest_getImplementationName( ) throw  ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.extensions.xml.sax.Writer"));
}

Sequence<OUString> OSaxWriterTest_getSupportedServiceNames( ) throw  ()
{
    Sequence<OUString> aRet(1);

    aRet.getArray()[0] = OSaxWriterTest_getImplementationName( );

    return aRet;
}



void OSaxWriterTest::testInvariant( const OUString& TestName,
                                    const Reference < XInterface >& TestObject )
    throw  (    IllegalArgumentException, RuntimeException)
{
    if( OUString::createFromAscii("com.sun.star.xml.sax.Writer") == TestName ) {
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
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer")) == TestName )
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



sal_Bool OSaxWriterTest::testPassed(void)                   throw  (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OSaxWriterTest::getErrors(void) throw  (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OSaxWriterTest::getErrorExceptions(void)                    throw  (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OSaxWriterTest::getWarnings(void)                      throw  (RuntimeException)
{
    return m_seqWarnings;
}

void OSaxWriterTest::writeParagraph(
    const Reference< XExtendedDocumentHandler > &r ,
    const OUString & s)
{
    int nMax = s.len();
    int nStart = 0;

    Sequence<sal_uInt16> seq( s.len() );
    memcpy( seq.getArray() , s.getStr() , s.len() * sizeof( sal_uInt16 ) );

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
    OUString testParagraph = OUString( RTL_CONSTASCII_USTRINGPARAM(
        "Dies ist ein bloeder Test um zu uberpruefen, ob der SAXWriter "
        "wohl Zeilenumbrueche halbwegs richtig macht oder ob er die Zeile "
        "bis zum bitteren Ende schreibt." ));

    OFileWriter *pw = new OFileWriter("output.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    Reference< XAttributeList > rList( (XAttributeList *) pList , UNO_QUERY );
    Reference< XOutputStream > ref( ( XOutputStream * ) pw , UNO_QUERY );

    Reference< XActiveDataSource > source( r , UNO_QUERY );

    ERROR_ASSERT( ref.is() , "no output stream" );
    ERROR_ASSERT( source.is() , "no active data source" );

    source->setOutputStream( ref );

    r->startDocument();

    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("Arg1" )),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("CDATA")) ,
                         OUString( RTL_CONSTASCII_USTRINGPARAM("bla\n   u")) );
    pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("Arg2")) ,
                         OUString( RTL_CONSTASCII_USTRINGPARAM("CDATA")) ,
                         OUString( RTL_CONSTASCII_USTRINGPARAM("blub")) );

    r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("tag1"))  , rList );
    r->ignorableWhitespace( OUString() );

    r->characters( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) );
    r->ignorableWhitespace( OUString() );

    r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("hi")) , rList );
    r->ignorableWhitespace( OUString() );

    // the enpassant must be converted & -> &amp;
    r->characters( OUString( RTL_CONSTASCII_USTRINGPARAM("&#252;")) );

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
    r->characters( OUString( RTL_CONSTASCII_USTRINGPARAM(">fsfsdf<"))  );
    r->endCDATA();
    r->ignorableWhitespace( OUString() );

    writeParagraph( r , testParagraph );


    r->ignorableWhitespace( OUString() );
    r->comment( OUString( RTL_CONSTASCII_USTRINGPARAM("Dies ist ein Kommentar !")) );
    r->ignorableWhitespace( OUString() );

    r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("emptytagtest"))  , rList );
    r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("emptytagtest")) );

    r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("hi")) );
    r->ignorableWhitespace( OUString() );

    r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("tag1")) );
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
            r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) , rList );
            bException = sal_False;
        }
        catch( SAXException &e )
        {

        }
        ERROR_ASSERT( bException , "expected exception not thrown !" );
    }

    r->startDocument();

    r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) , rList );
    r->startCDATA();

    {
        sal_Bool bException = sal_True;
        try{
            r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) , rList );
            bException = sal_False;
        }
        catch( SAXException &e ) {

        }
        ERROR_ASSERT( bException , "expected exception not thrown !" );
    }

    r->endCDATA();
    r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("hi")) );

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
    r->unknown( OUString( RTL_CONSTASCII_USTRINGPARAM("<!DOCTYPE iCalendar >\n")) );
    r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) , rList );

    r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) );
    r->endDocument();
}

void OSaxWriterTest::testPerformance(const  Reference< XExtendedDocumentHandler > &r )
{
    OFileWriter *pw = new OFileWriter("testPerformance.xml");
    AttributeListImpl *pList = new AttributeListImpl;

    OUString testParagraph =
        OUString( RTL_CONSTASCII_USTRINGPARAM(
            "Dies ist ein bloeder Test um zu uberpruefen, ob der SAXWriter "
            "wohl Zeilenumbrueche halbwegs richtig macht oder ob er die Zeile "
            "bis zum bitteren Ende schreibt." ));


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
    for( i2 = 0 ; i2 < 75 ; i2 ++ )
    {
        r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("tag") ) +
                         OUString::valueOf( i2 ), rList );
        for( sal_Int32 i = 0 ; i < 450 ; i ++ )
        {
            r->ignorableWhitespace( OUString());
            r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) , rList );
            r->characters( testParagraph );

            r->ignorableWhitespace( OUString() );
            r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) );
        }
    }
    for( i2 = 74 ; i2 >= 0  ; i2-- )
    {
        r->ignorableWhitespace( OUString() );
        r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("tag") ) + OUString::valueOf( i2 ) );
    }

    r->endDocument();

    osl_getSystemTime( &aEndTime );

    double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
    double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);

    printf( "Performance writing : %g s\n" , fEnd - fStart );
}
}
