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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <stdio.h>
#include <string.h>
#include <stack>

#include "sal/main.h"

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

#include <osl/diagnose.h>

#include "LocaleNode.hxx"

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;






/************
 * Sequence of bytes -> InputStream
 ************/
class OInputStream : public WeakImplHelper1 < XInputStream >
{
public:
    OInputStream( const Sequence< sal_Int8 >&seq ) :
        nPos( 0 ),
        m_seq( seq )
        {}

public:
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {
            nBytesToRead = (nBytesToRead > m_seq.getLength() - nPos ) ?
                m_seq.getLength() - nPos :
                nBytesToRead;
            aData = Sequence< sal_Int8 > ( &(m_seq.getConstArray()[nPos]) , nBytesToRead );
            nPos += nBytesToRead;
            return nBytesToRead;
        }
    virtual sal_Int32 SAL_CALL readSomeBytes(
        ::com::sun::star::uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {
            return readBytes( aData, nMaxBytesToRead );
        }
    virtual void SAL_CALL skipBytes( sal_Int32 /*nBytesToSkip*/ )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {
            // not implemented
        }
    virtual sal_Int32 SAL_CALL available(  )
        throw(NotConnectedException, IOException, RuntimeException)
        {
            return m_seq.getLength() - nPos;
        }
    virtual void SAL_CALL closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException)
        {
            // not needed
        }
    sal_Int32 nPos;
    Sequence< sal_Int8> m_seq;
};

//-------------------------------
// Helper : create an input stream from a file
//------------------------------
Reference< XInputStream > createStreamFromFile(
    const char *pcFile )
{
    FILE *f = fopen( pcFile , "rb" );
    Reference<  XInputStream >  r;

    if( f ) {
        fseek( f , 0 , SEEK_END );
        size_t nLength = ftell( f );
        fseek( f , 0 , SEEK_SET );

        Sequence<sal_Int8> seqIn(nLength);
        if (fread( seqIn.getArray() , nLength , 1 , f ) == 1)
            r = Reference< XInputStream > ( new OInputStream( seqIn ) );
        else
            fprintf(stderr, "failure reading %s\n", pcFile);
        fclose( f );
    }
    return r;
}


class TestDocumentHandler :
    public WeakImplHelper3< XExtendedDocumentHandler , XEntityResolver , XErrorHandler >
{
public:
    TestDocumentHandler(const char* locale, const char* outFile )
        : rootNode(0)
        , nError(0)
        , of(outFile, locale)
    {
        strncpy( theLocale, locale, sizeof(theLocale) );
        theLocale[sizeof(theLocale)-1] = 0;
    }

    ~TestDocumentHandler(  )
    {
        of.closeOutput();
        delete rootNode;
    }


public: // Error handler
    virtual void SAL_CALL error(const Any& aSAXParseException) throw (SAXException, RuntimeException)
    {
        ++nError;
        printf( "Error !\n" );
        throw  SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("error from error handler")) ,
            Reference < XInterface >() ,
            aSAXParseException );
    }
    virtual void SAL_CALL fatalError(const Any& /*aSAXParseException*/) throw (SAXException, RuntimeException)
    {
        ++nError;
        printf( "Fatal Error !\n" );
    }
    virtual void SAL_CALL warning(const Any& /*aSAXParseException*/) throw (SAXException, RuntimeException)
    {
        printf( "Warning !\n" );
    }


public: // ExtendedDocumentHandler



    stack<LocaleNode *> currentNode ;
    sal_Bool  fElement ;
    LocaleNode * rootNode;

    virtual void SAL_CALL startDocument(void) throw (SAXException, RuntimeException)
    {
    printf( "parsing document %s started\n", theLocale);
    of.writeAsciiString("#include <sal/types.h>\n\n\n");
    of.writeAsciiString("#include <stdio.h> // debug printfs\n\n");
    of.writeAsciiString("extern \"C\" {\n\n");
    }

    virtual void SAL_CALL endDocument(void) throw (SAXException, RuntimeException)
    {
        if (rootNode)
        {
            rootNode->generateCode(of);
            int err = rootNode->getError();
            if (err)
            {
                printf( "Error: in data for %s: %d\n", theLocale, err);
                nError += err;
            }
        }
        else
        {
            ++nError;
            printf( "Error: no data for %s\n", theLocale);
        }
        printf( "parsing document %s finished\n", theLocale);

        of.writeAsciiString("} // extern \"C\"\n\n");
        of.closeOutput();
    }

    virtual void SAL_CALL startElement(const OUString& aName,
                              const Reference< XAttributeList > & xAttribs)
        throw (SAXException,RuntimeException)
    {

        LocaleNode * l =  LocaleNode::createNode (aName, xAttribs);
        if (!currentNode.empty() ) {
            LocaleNode * ln = (LocaleNode *) currentNode.top();
            ln->addChild(l);
        } else {
            rootNode = l;
        }
        currentNode.push (l);
    }


    virtual void SAL_CALL endElement(const OUString& /*aName*/) throw (SAXException,RuntimeException)
    {
        currentNode.pop();
    }

    virtual void SAL_CALL characters(const OUString& aChars) throw (SAXException,RuntimeException)
    {

        LocaleNode * l = currentNode.top();
        l->setValue (aChars);
    }

    virtual void SAL_CALL ignorableWhitespace(const OUString& /*aWhitespaces*/) throw (SAXException,RuntimeException)
    {
    }

    virtual void SAL_CALL processingInstruction(const OUString& /*aTarget*/, const OUString& /*aData*/) throw (SAXException,RuntimeException)
    {
        // ignored
    }

    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator> & /*xLocator*/)
        throw (SAXException,RuntimeException)
    {
        // ignored
    }

    virtual InputSource SAL_CALL resolveEntity(
        const OUString& sPublicId,
        const OUString& sSystemId)
        throw (RuntimeException)
    {
        InputSource source;
        source.sSystemId = sSystemId;
        source.sPublicId = sPublicId;

        source.aInputStream = createStreamFromFile(
            OUStringToOString( sSystemId , RTL_TEXTENCODING_ASCII_US) );

        return source;
    }

    virtual void SAL_CALL startCDATA(void) throw (SAXException,RuntimeException)
    {
    }
    virtual void SAL_CALL endCDATA(void) throw (RuntimeException)
    {
    }
    virtual void SAL_CALL comment(const OUString& /*sComment*/) throw (SAXException,RuntimeException)
    {
    }
    virtual void SAL_CALL unknown(const OUString& /*sString*/) throw (SAXException,RuntimeException)
    {
    }

    virtual void SAL_CALL allowLineBreak( void) throw (SAXException, RuntimeException )
    {

    }

public:
    int nError;
    sal_Char theLocale[50];
    OFileWriter of;
};





SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{


    if( argc < 6) {
        printf( "usage : %s <locaLe> <XML inputfile> <destination file> <services.rdb location> <types.rdb location>\n", argv[0] );
        exit( 1 );
    }

    // create service manager
    Reference< XMultiServiceFactory > xSMgr;
    try
    {
        xSMgr = createRegistryServiceFactory(
            ::rtl::OUString::createFromAscii(argv[4]),
            ::rtl::OUString::createFromAscii(argv[5]) );
    }
    catch ( Exception &e )
    {
        printf( "Exception on createRegistryServiceFactory %s\n",
            OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() );
        exit(1);
    }

    Reference < XImplementationRegistration > xReg;
    try
    {
        // Create registration service
        Reference < XInterface > x = xSMgr->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration")) );
        xReg = Reference<  XImplementationRegistration > ( x , UNO_QUERY );
    }
    catch( Exception & ) {
        printf( "Couldn't create ImplementationRegistration service\n" );
        exit(1);
    }

    OString sTestName;
    try
    {
        // Load dll for the tested component
        OUString aDllName(RTL_CONSTASCII_USTRINGPARAM( "sax.uno" SAL_DLLEXTENSION ));
        xReg->registerImplementation(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")),
            aDllName,
            Reference< XSimpleRegistry > ()  );
    }
    catch( Exception &e ) {
        printf( "Couldn't raise sax.uno library!\n" );
        printf( "%s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() );

        exit(1);
    }


    //--------------------------------
    // parser demo
    // read xml from a file and count elements
    //--------------------------------
    Reference< XInterface > x = xSMgr->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser")) );
    int nError = 0;
    if( x.is() )
    {
        Reference< XParser > rParser( x , UNO_QUERY );

        // create and connect the document handler to the parser
        TestDocumentHandler *pDocHandler = new TestDocumentHandler( argv[1], argv[3]);

        Reference < XDocumentHandler >  rDocHandler( (XDocumentHandler *) pDocHandler );
        Reference< XEntityResolver > rEntityResolver( (XEntityResolver *) pDocHandler );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );

        // create the input stream
        InputSource source;
        source.aInputStream = createStreamFromFile( argv[2] );
        source.sSystemId    = OUString::createFromAscii( argv[2] );

        try
        {
            // start parsing
            rParser->parseStream( source );
        }

        catch( Exception & e )
        {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            printf( "Exception during parsing : %s\n" ,  o1.getStr() );
            exit(1);
        }
        nError = pDocHandler->nError;
    }
    else
    {
        printf( "couln't create sax-parser component\n" );
        exit(1);
    }

    return nError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
