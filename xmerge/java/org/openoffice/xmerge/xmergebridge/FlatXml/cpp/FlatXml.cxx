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
#include "precompiled_xmerge.hxx"
#include <stdio.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/uno/XNamingService.hpp>

#include <com/sun/star/registry/XImplementationRegistration.hpp>

#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>

#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <rtl/textenc.h>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::xml::sax::XDocumentHandler;
using rtl::OUString;

namespace XFlatXml {

void mygetchar()
{
#ifdef SAL_W32
    _fgetchar();
#else
    getchar();
#endif
}



//,com::sun::star::xml::sax::XDocumentHandler,com::sun::star::xml::XExportFilter
class XFlatXml : public WeakImplHelper3< com::sun::star::xml::XImportFilter,com::sun::star::xml::XExportFilter,com::sun::star::xml::sax::XDocumentHandler>
{
private:
  Reference< XMultiServiceFactory > xMSF;
  int indentation;
  bool followsEndNode;
  bool followsStartNode;


public:

  XFlatXml( const Reference< XMultiServiceFactory > &r ) : xMSF( r )
    {}

    virtual sal_Bool SAL_CALL importer(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aSourceData, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XDocumentHandler>& xHandler, const com::sun::star::uno::Sequence<rtl::OUString>& msUserData) throw(RuntimeException);

     virtual sal_Bool SAL_CALL exporter(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aSourceData, const com::sun::star::uno::Sequence<rtl::OUString>& msUserData) throw(RuntimeException);
    Reference < com::sun::star::io::XOutputStream > xOutputStream;

    virtual void SAL_CALL startDocument() throw (com::sun::star::xml::sax::SAXException,RuntimeException) ;
    virtual void SAL_CALL endDocument() throw (com::sun::star::xml::sax::SAXException,RuntimeException);
    virtual void SAL_CALL startElement(const rtl::OUString& str, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& attriblist) throw (com::sun::star::xml::sax::SAXException,RuntimeException);
    virtual void SAL_CALL endElement(const rtl::OUString& str)  throw (com::sun::star::xml::sax::SAXException,RuntimeException);
    virtual void SAL_CALL characters(const rtl::OUString& str)  throw (com::sun::star::xml::sax::SAXException,RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(const rtl::OUString& str) throw (com::sun::star::xml::sax::SAXException,RuntimeException);
    virtual void  SAL_CALL processingInstruction(const rtl::OUString& str, const rtl::OUString& str2) throw (com::sun::star::xml::sax::SAXException,RuntimeException) ;
    virtual void SAL_CALL setDocumentLocator(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XLocator>& doclocator) throw (com::sun::star::xml::sax::SAXException,RuntimeException) ;

};

 OUString replace(OUString origString, OUString origChar, OUString replaceChar){
           OUString tmp=rtl::OUString::createFromAscii("");
           int index=origString.indexOf(origChar);
           if(index !=-1){
           while (index !=-1){
               OUString first =origString.copy(0,index);
               first=first.concat(replaceChar);
               tmp=tmp.concat(first);
               origString=origString.copy(index+1,origString.getLength());
               index=origString.indexOf(origChar);
               if(index==-1) {
               tmp=tmp.concat(origString);
               }

           }

           }

           return tmp;
    }

OUString needsMask(OUString origString){

        if (origString.indexOf(rtl::OUString::createFromAscii("&"))!=-1){
        origString=replace(origString,rtl::OUString::createFromAscii("&"),rtl::OUString::createFromAscii("&amp;"));
        }
        if (origString.indexOf(rtl::OUString::createFromAscii("\""))!=-1){
        origString=replace(origString,rtl::OUString::createFromAscii("\""),rtl::OUString::createFromAscii("&quot;"));
        }
        if (origString.indexOf(rtl::OUString::createFromAscii("<"))!=-1){
        origString=replace(origString,rtl::OUString::createFromAscii("<"),rtl::OUString::createFromAscii("&lt;"));
        }
        if (origString.indexOf(rtl::OUString::createFromAscii(">"))!=-1){
        origString=replace(origString,rtl::OUString::createFromAscii(">"),rtl::OUString::createFromAscii("&gt;"));
        }
        return origString;

    }


sal_Bool XFlatXml::importer(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aSourceData, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XDocumentHandler>& xHandler, const com::sun::star::uno::Sequence<rtl::OUString>& msUserData) throw (RuntimeException)
{
  /*
  fprintf (stderr,"\nFound the C++ importer");

  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[0], RTL_TEXTENCODING_ASCII_US ).getStr());

  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[1], RTL_TEXTENCODING_ASCII_US ).getStr());

  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[2], RTL_TEXTENCODING_ASCII_US ).getStr());
  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[3], RTL_TEXTENCODING_ASCII_US ).getStr());
  */
   OUString udConvertClass=msUserData[0];
   OUString udJarPath=msUserData[1];
   OUString udImport =msUserData[2];
   OUString udExport =msUserData[3];
   sal_Int32 nLength = aSourceData.getLength();
   OUString sFileName= OUString::createFromAscii("");
   OUString sDirectory = OUString::createFromAscii("");
   OUString sURL = OUString::createFromAscii("");
   Reference<com::sun::star::io::XInputStream> xInputStream;
   Sequence<com::sun::star::beans::PropertyValue> pValue=aSourceData;
   for ( sal_Int32 i = 0 ; i < nLength; i++)
    {

      if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "InputStream" ) ) )
            pValue[i].Value >>= xInputStream;

      else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "FileName" ) ) )
            pValue[i].Value >>= sFileName;
      else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "URL" ) ) )
        {
            pValue[i].Value >>= sURL;
                    sal_Int32 stringIndex = sURL.lastIndexOf(OUString::createFromAscii("/"));
            sFileName = sURL.copy(stringIndex+1, sURL.getLength()-stringIndex);
            sDirectory = sURL.copy(7,sURL.getLength()-7);
        }
    }
   if ( !xInputStream.is() )
    {
        OSL_ASSERT( 0 );
        return sal_False;
    }
         Reference<XConfigManager > xConfMgr( XFlatXml::xMSF->createInstance(
                      OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")),UNO_QUERY );

    if(! xConfMgr.is()){
      fprintf (stderr, "::com::sun::star::frame::XConfigManager service missing\n" );
      return sal_False;
    }


    const OUString sSaxParser ( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser") );
    OUString sXMLImportService (  udImport  );
    Reference < com::sun::star::xml::sax::XParser > xSaxParser( XFlatXml::xMSF->createInstance( sSaxParser ), UNO_QUERY );
    Reference <XInterface> xPipe;
    xPipe= XFlatXml::xMSF->createInstance(OUString::createFromAscii("com.sun.star.io.Pipe"));
    if(!xPipe.is()){
      fprintf(stderr,"XMLReader::Read:com.sun.star.io.Pipe service missing\n");
      return sal_False;
    }
    //else fprintf(stderr,"pipe created\n");


    Reference< com::sun::star::io::XInputStream > xPipeInput (xPipe,UNO_QUERY);

    //inputStream from pipe

    Reference< com::sun::star::io::XOutputStream > xTmpOutputStream (xPipe,UNO_QUERY);

    com::sun::star::xml::sax::InputSource aInput;
    aInput.sSystemId = sFileName;
    //aInput.aInputStream =  xPipeInput;
    aInput.aInputStream =xInputStream;
    xSaxParser->setDocumentHandler ( xHandler );
    try
    {

        xSaxParser->parseStream ( aInput );
    }
    catch( Exception &exc)
    {
            fprintf(stderr,"\nParseStream Exception\n");
        fprintf(stderr, "Fell into the catch block!: %s\n",OUStringToOString(
                exc.Message, RTL_TEXTENCODING_ASCII_US).getStr() );

    }

  return sal_True;
}

sal_Bool XFlatXml::exporter(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aSourceData, const com::sun::star::uno::Sequence<rtl::OUString>& msUserData) throw (RuntimeException){
  /*
  fprintf (stderr,"\nFound the C++ exporter");

  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[0], RTL_TEXTENCODING_ASCII_US ).getStr());

  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[1], RTL_TEXTENCODING_ASCII_US ).getStr());

  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[2], RTL_TEXTENCODING_ASCII_US ).getStr());
  fprintf(stderr,"\nUserData :  %s",OUStringToOString( msUserData[3], RTL_TEXTENCODING_ASCII_US ).getStr());
  */

  sal_Int32 nLength = aSourceData.getLength();
  Sequence<com::sun::star::beans::PropertyValue> pValue = aSourceData;
  OUString sFileName=OUString::createFromAscii("");
  OUString sDirectory= OUString::createFromAscii("");
  OUString sURL =OUString::createFromAscii("") ;
  for ( sal_Int32 i = 0 ; i < nLength; i++)
    {

      if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "OutputStream" ) ) )
    {
      pValue[i].Value >>= XFlatXml::xOutputStream;
      break;
    }
      else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "URL" ) ) )
    {   pValue[i].Value >>= sURL;
                sal_Int32 stringIndex = sURL.lastIndexOf(OUString::createFromAscii("/"));
            sFileName = sURL.copy(stringIndex+1, sURL.getLength()-stringIndex);
            sDirectory = sURL.copy(7,sURL.getLength()-7);
    }
    }



  return sal_True;
}





    void XFlatXml::startDocument() throw (com::sun::star::xml::sax::SAXException,RuntimeException){
      //fprintf(stderr,"\nStartDocument ");
      indentation=0;
      followsEndNode=false;
      followsStartNode=false;
      // xExportHandler->startDocument();

    }

   void XFlatXml::endDocument() throw (com::sun::star::xml::sax::SAXException,RuntimeException){
     //fprintf(stderr,"\nEndDocument ");
     //xExportHandler->endDocument();
   }

   void XFlatXml::startElement(const rtl::OUString& str, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& attriblist) throw (com::sun::star::xml::sax::SAXException,RuntimeException)
   {
    OUString indent=OUString::createFromAscii("");
     if (followsStartNode){
       indent+=OUString::createFromAscii("\n");
       for (int i=0 ;i<indentation;i++){
     indent+=OUString::createFromAscii(" ");
       }
     }
     OUString lt =indent;
     lt+=OUString::createFromAscii("<");
     lt+= (str);

     if (attriblist !=NULL)
       {
     lt += OUString::createFromAscii(" ");
     int len=attriblist->getLength();
     for (short i=0;i<len;i++)
       {
         lt +=attriblist->getNameByIndex(i);
         lt +=OUString::createFromAscii("=\"");
         lt +=needsMask(attriblist->getValueByIndex(i));
         lt +=OUString::createFromAscii("\" ");
       }
       }
     lt+=OUString::createFromAscii(">");
     OString sStr= OUStringToOString(lt, RTL_TEXTENCODING_UTF8);
     Sequence <sal_Int8> b((const sal_Int8 *)sStr.getStr(),sStr.getLength());
     //Sequence <sal_Int8>b((const sal_Int8 *)OUStringToOString( lt, RTL_TEXTENCODING_UTF8 ).getStr(),(OUStringToOString( lt, RTL_TEXTENCODING_UTF8 ).getStr()).getLength());
     try{

       XFlatXml::xOutputStream->writeBytes(b);
     }
     catch ( Exception &exc){
        fprintf(stderr,"Exception");
     }
      indentation++;
   }

   void XFlatXml::endElement(const rtl::OUString& str) throw (com::sun::star::xml::sax::SAXException,RuntimeException) {
     OUString indent=OUString::createFromAscii("");
        indentation--;
     if (followsEndNode){
       indent+=OUString::createFromAscii("\n");
       for (int i=0 ;i<indentation;i++){
     indent+=OUString::createFromAscii(" ");
       }
     }
     OUString lt =indent;
     lt+=OUString::createFromAscii("</");
        //OUString lt =OUString::createFromAscii("</");
     lt+= (str);

     lt+=OUString::createFromAscii(">");
     //System.out.println(str);
     //fprintf(stderr,"%s",OUStringToOString( lt, RTL_TEXTENCODING_ASCII_US ).getStr());
     OString sStr= OUStringToOString(lt, RTL_TEXTENCODING_UTF8);
     Sequence <sal_Int8> b((const sal_Int8 *)sStr.getStr(),sStr.getLength());
     try{

       XFlatXml::xOutputStream->writeBytes(b);
     }
     catch ( Exception &exc){

     }
     followsEndNode=true;
     followsStartNode=true;
}

   void XFlatXml::characters(const rtl::OUString& str) throw (com::sun::star::xml::sax::SAXException,RuntimeException) {
     //fprintf(stderr,"\n %s ",OUStringToOString(str, RTL_TEXTENCODING_ASCII_US).getStr());
     OUString newStr = needsMask(str);
     OString sStr= OUStringToOString(newStr, RTL_TEXTENCODING_UTF8);
     Sequence <sal_Int8> b((const sal_Int8 *)sStr.getStr(),sStr.getLength());
     try{

       XFlatXml::xOutputStream->writeBytes(b);
     }
     catch ( Exception &exc){

     }
     followsEndNode=false;
     followsStartNode=false;
   }


   void XFlatXml::ignorableWhitespace(const rtl::OUString& str) throw (com::sun::star::xml::sax::SAXException,RuntimeException){
     //xExportHandler->ignorableWhitespace( str);

     //fprintf(stderr,"%s",OUStringToOString( str, RTL_TEXTENCODING_ASCII_US ).getStr());
     /*
     OString sStr= OUStringToOString(str, RTL_TEXTENCODING_UTF8);
     Sequence <sal_Int8> b((const sal_Int8 *)sStr.getStr(),sStr.getLength());
     try{

       XFlatXml::xOutputStream->writeBytes(b);
     }
     catch ( Exception &exc){

     }
     */

}

   void  XFlatXml::processingInstruction(const rtl::OUString& str, const rtl::OUString& str2) throw (com::sun::star::xml::sax::SAXException,RuntimeException) {
     //xExportHandler->processingInstruction( str, str2);
}

   void XFlatXml::setDocumentLocator(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XLocator>& doclocator) throw (com::sun::star::xml::sax::SAXException,RuntimeException) {
     //xExportHandler->setDocumentLocator( doclocator);
   }



Reference< XInterface > SAL_CALL CreateInstance( const Reference< XMultiServiceFactory > &r)
{
  //fprintf(stderr,"\nCreateInstance\n");
  //return Reference< XInterface > ( ( OWeakObject * ) new OfficeClientMain(r) );
  return Reference< XInterface > ( ( OWeakObject * ) new XFlatXml(r) );
}

Sequence< OUString > getSupportedServiceNames()
{
  //fprintf(stderr,"\ngetSupportedServiceNames\n");
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(2);
            seqNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.documentconversion.XFlatXml" );
            pNames = &seqNames;
        }
    }
    return *pNames;
}

}

using namespace XFlatXml;
#define IMPLEMENTATION_NAME "com.sun.star.documentconversion.XFlatXml"


extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
        // printf("\ncomponent_getImplementationEnvironment\n");
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;

}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{

        //fprintf(stderr,"\ncomponent_writeInfo\n");
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
        //fprintf(stderr,"\ncomponent_getFactory");
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            CreateInstance, getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}
