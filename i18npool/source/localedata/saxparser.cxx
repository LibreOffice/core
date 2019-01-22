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

#include <sal/config.h>

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stack>

#include <sal/main.h>

#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/implbase.hxx>


#include "LocaleNode.hxx"

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;


/************
 * Sequence of bytes -> InputStream
 ************/
class OInputStream : public WeakImplHelper < XInputStream >
{
public:
    explicit OInputStream( const Sequence< sal_Int8 >&seq )
        : nPos(0)
        , m_seq(seq)
    {}

public:
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override
        {
            nBytesToRead = (nBytesToRead > m_seq.getLength() - nPos ) ?
                m_seq.getLength() - nPos :
                nBytesToRead;
            aData = Sequence< sal_Int8 > ( &(m_seq.getConstArray()[nPos]) , nBytesToRead );
            nPos += nBytesToRead;
            return nBytesToRead;
        }
    virtual sal_Int32 SAL_CALL readSomeBytes(
        css::uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nMaxBytesToRead ) override
        {
            return readBytes( aData, nMaxBytesToRead );
        }
    virtual void SAL_CALL skipBytes( sal_Int32 /*nBytesToSkip*/ ) override
        {
            // not implemented
        }
    virtual sal_Int32 SAL_CALL available(  ) override
        {
            return m_seq.getLength() - nPos;
        }
    virtual void SAL_CALL closeInput(  ) override
        {
            // not needed
        }
    sal_Int32 nPos;
    Sequence< sal_Int8> m_seq;
};


// Helper : create an input stream from a file

static Reference< XInputStream > createStreamFromFile(
    const char *pcFile )
{
    Reference<  XInputStream >  r;

    FILE *f = fopen( pcFile , "rb" );

    if (!f)
    {
        fprintf(stderr, "failure opening %s\n", pcFile);
        return r;
    }

    if (fseek( f , 0 , SEEK_END ) == -1)
    {
        fprintf(stderr, "failure fseeking %s\n", pcFile);
        fclose(f);
        return r;
    }

    long nLength = ftell( f );
    if (nLength == -1)
    {
        fprintf(stderr, "failure ftelling %s\n", pcFile);
        fclose(f);
        return r;
    }

    if (fseek( f , 0 , SEEK_SET ) == -1)
    {
        fprintf(stderr, "failure fseeking %s\n", pcFile);
        fclose(f);
        return r;
    }

    Sequence<sal_Int8> seqIn(nLength);
    if (fread( seqIn.getArray(), nLength , 1 , f ) == 1)
        r.set( new OInputStream( seqIn ) );
    else
        fprintf(stderr, "failure reading %s\n", pcFile);
    fclose( f );
    return r;
}


class TestDocumentHandler :
    public WeakImplHelper< XExtendedDocumentHandler , XEntityResolver , XErrorHandler >
{
public:
    TestDocumentHandler(const char* locale, const char* outFile )
        : rootNode(nullptr)
        , nError(0)
        , theLocale(locale)
        , of(outFile, locale)
    {
    }

    virtual ~TestDocumentHandler(  ) override
    {
        of.closeOutput();
        delete rootNode;
    }


public: // Error handler
    virtual void SAL_CALL error(const Any& aSAXParseException) override
    {
        ++nError;
        printf( "Error !\n" );
        throw  SAXException(
            "error from error handler",
            Reference < XInterface >() ,
            aSAXParseException );
    }
    virtual void SAL_CALL fatalError(const Any& /*aSAXParseException*/) override
    {
        ++nError;
        printf( "Fatal Error !\n" );
    }
    virtual void SAL_CALL warning(const Any& /*aSAXParseException*/) override
    {
        printf( "Warning !\n" );
    }


public: // ExtendedDocumentHandler


    stack<LocaleNode *> currentNode ;
    LocaleNode * rootNode;

    virtual void SAL_CALL startDocument() override
    {
    printf( "parsing document %s started\n", theLocale.c_str());
    of.writeAsciiString("#include <sal/types.h>\n\n\n");
    of.writeAsciiString("#include <stdio.h>\n\n");
    of.writeAsciiString("extern \"C\" {\n\n");
    }

    virtual void SAL_CALL endDocument() override
    {
        if (rootNode)
        {
            rootNode->generateCode(of);
            int err = rootNode->getError();
            if (err)
            {
                printf( "Error: in data for %s: %d\n", theLocale.c_str(), err);
                nError += err;
            }
        }
        else
        {
            ++nError;
            printf( "Error: no data for %s\n", theLocale.c_str());
        }
        printf( "parsing document %s finished\n", theLocale.c_str());

        of.writeAsciiString("} // extern \"C\"\n\n");
        of.closeOutput();
    }

    virtual void SAL_CALL startElement(const OUString& aName,
                              const Reference< XAttributeList > & xAttribs) override
    {

        LocaleNode * l =  LocaleNode::createNode (aName, xAttribs);
        if (!currentNode.empty() ) {
            LocaleNode * ln = currentNode.top();
            ln->addChild(l);
        } else {
            rootNode = l;
        }
        currentNode.push (l);
    }


    virtual void SAL_CALL endElement(const OUString& /*aName*/) override
    {
        currentNode.pop();
    }

    virtual void SAL_CALL characters(const OUString& aChars) override
    {

        LocaleNode * l = currentNode.top();
        l->setValue (aChars);
    }

    virtual void SAL_CALL ignorableWhitespace(const OUString& /*aWhitespaces*/) override
    {
    }

    virtual void SAL_CALL processingInstruction(const OUString& /*aTarget*/, const OUString& /*aData*/) override
    {
        // ignored
    }

    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator> & /*xLocator*/) override
    {
        // ignored
    }

    virtual InputSource SAL_CALL resolveEntity(
        const OUString& sPublicId,
        const OUString& sSystemId) override
    {
        InputSource source;
        source.sSystemId = sSystemId;
        source.sPublicId = sPublicId;

        source.aInputStream = createStreamFromFile(
            OUStringToOString(sSystemId, RTL_TEXTENCODING_ASCII_US).getStr() );

        return source;
    }

    virtual void SAL_CALL startCDATA() override
    {
    }
    virtual void SAL_CALL endCDATA() override
    {
    }
    virtual void SAL_CALL comment(const OUString& /*sComment*/) override
    {
    }
    virtual void SAL_CALL unknown(const OUString& /*sString*/) override
    {
    }

    virtual void SAL_CALL allowLineBreak() override
    {

    }

public:
    int nError;
    std::string theLocale;
    OFileWriter of;
};


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    try {
        if( argc < 4) {
            printf( "usage : %s <locale> <XML inputfile> <destination file>\n", argv[0] );
            exit( 1 );
        }

        Reference< XComponentContext > xContext(
            defaultBootstrap_InitialComponentContext());


        // parser demo
        // read xml from a file and count elements

        Reference< XParser > rParser = Parser::create(xContext);

        int nError = 0;
        // create and connect the document handler to the parser
        TestDocumentHandler *pDocHandler = new TestDocumentHandler( argv[1], argv[3]);

        Reference < XDocumentHandler >  rDocHandler( static_cast<XDocumentHandler *>(pDocHandler) );
        Reference< XEntityResolver > rEntityResolver( static_cast<XEntityResolver *>(pDocHandler) );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );

        // create the input stream
        InputSource source;
        source.aInputStream = createStreamFromFile( argv[2] );
        source.sSystemId    = OUString::createFromAscii( argv[2] );

        // start parsing
        rParser->parseStream( source );

        nError = pDocHandler->nError;
        css::uno::Reference<css::lang::XComponent>(
            xContext, css::uno::UNO_QUERY_THROW)->dispose();
        return nError;
    } catch (css::uno::Exception & e) {
        std::cerr << "ERROR: " << e.Message << '\n';
        return EXIT_FAILURE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
