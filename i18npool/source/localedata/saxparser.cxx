/*************************************************************************
 *
 *  $RCSfile: saxparser.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-22 16:33:45 $
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
#include <vector>
#include <stack>

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

#include <vos/diagnose.hxx>

#include <tools/string.hxx>
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
        m_seq( seq ),
        nPos( 0 )
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
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
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
        int nLength = ftell( f );
        fseek( f , 0 , SEEK_SET );

        Sequence<sal_Int8> seqIn(nLength);
        fread( seqIn.getArray() , nLength , 1 , f );

        r = Reference< XInputStream > ( new OInputStream( seqIn ) );
        fclose( f );
    }
    return r;
}


class TestDocumentHandler :
    public WeakImplHelper3< XExtendedDocumentHandler , XEntityResolver , XErrorHandler >
{
public:
    TestDocumentHandler(const char* locale, const char* outFile ) :
      of(outFile, locale), nbOfCurrencies(0), nbOfCalendars(0), nbOfCollations(0),
      nbOfFormatElements(0), nbOfDays(50), nbOfMonths(50), nbOfEras(10),
      nbOfTransliterations(0), isStartDayOfWeek(false), foundDefaultName(false),
      flag(-1), foundVarient(false), openElement(false), rootNode(0)
    {
        strncpy( theLocale, locale, sizeof(theLocale) );
        theLocale[sizeof(theLocale)-1] = 0;
    }

    ~TestDocumentHandler(  )
    {
        of.closeOutput();
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



    stack<LocaleNode *> currentNode ;
    sal_Bool  fElement ;
    LocaleNode * rootNode;

    virtual void SAL_CALL startDocument(void) throw (SAXException, RuntimeException)
    {
    printf( "parsing document %s started\n", theLocale);
    of.writeAsciiString("#include <sal/types.h>\n\n\n");
#if SUPD > 618
    of.writeAsciiString("#include <stdio.h> // debug printfs\n\n");
#endif // SUPD > 618
    of.writeAsciiString("extern \"C\" {\n\n");
    }

    virtual void SAL_CALL endDocument(void) throw (SAXException, RuntimeException)
    {
        if (rootNode)
            rootNode->generateCode(of);
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
            LocaleNode * ln = (LocaleNode *) currentNode . top();
            ln->addChild(l);
        } else {
            rootNode = l;
        }
        currentNode . push (l);
    }


    virtual void SAL_CALL endElement(const OUString& aName) throw (SAXException,RuntimeException)
    {
        currentNode . pop();
    }

    virtual void SAL_CALL characters(const OUString& aChars) throw (SAXException,RuntimeException)
    {

        LocaleNode * l = currentNode . top();
        l->setValue (aChars);
        ::rtl::OUString str(aChars);
        sal_Unicode nonBreakSPace[2]= {0xa, 0x0};
        if(!openElement || str.equals(nonBreakSPace))
          return;
    }

    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) throw (SAXException,RuntimeException)
    {
   }

    virtual void SAL_CALL processingInstruction(const OUString& aTarget, const OUString& aData) throw (SAXException,RuntimeException)
    {
        // ignored
    }

    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator> & xLocator)
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
    virtual void SAL_CALL comment(const OUString& sComment) throw (SAXException,RuntimeException)
    {
    }
    virtual void SAL_CALL unknown(const OUString& sString) throw (SAXException,RuntimeException)
    {
    }

    virtual void SAL_CALL allowLineBreak( void) throw (SAXException, RuntimeException )
    {

    }

public:
    ::rtl::OUString currentElement;
    sal_Int16 nbOfCurrencies;
    sal_Int16 nbOfCalendars;
    sal_Int16 nbOfFormatElements;
    sal_Int16 nbOfTransliterations;
    sal_Int16 nbOfCollations;
    Sequence<sal_Int16> nbOfDays;
    Sequence<sal_Int16> nbOfMonths;
    Sequence<sal_Int16> nbOfEras;
    sal_Char *elementTag;
    sal_Char theLocale[50];
    sal_Int16 flag;
    OFileWriter of;
    sal_Bool isStartDayOfWeek;
    sal_Bool foundDefaultName;
    sal_Bool foundVarient;
        sal_Bool openElement;
};





int SAL_CALL main (int argc, char **argv)
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
    catch ( Exception& )
    {
        printf( "Exception on createRegistryServiceFactory\n" );
        exit(1);
    }

    Reference < XImplementationRegistration > xReg;
    try
    {
        // Create registration service
        Reference < XInterface > x = xSMgr->createInstance(
            OUString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" ) );
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
#ifdef SAL_W32
        OUString aDllName = OUString::createFromAscii( "sax" );
#else
#ifdef MACOSX
        OUString aDllName = OUString::createFromAscii( "libsax.dylib.framework" );
#else
        OUString aDllName = OUString::createFromAscii( "libsax.so" );
#endif
#endif
        xReg->registerImplementation(
            OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
            aDllName,
            Reference< XSimpleRegistry > ()  );
    }
    catch( Exception &e ) {
        printf( "Couldn't reach sax dll\n" );
        printf( "%s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() );

        exit(1);
    }


    //--------------------------------
    // parser demo
    // read xml from a file and count elements
    //--------------------------------
    Reference< XInterface > x = xSMgr->createInstance(
        OUString::createFromAscii( "com.sun.star.xml.sax.Parser" ) );
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
    }
    else
    {
        printf( "couln't create sax-parser component\n" );
        exit(1);
    }

    return 0;
}
