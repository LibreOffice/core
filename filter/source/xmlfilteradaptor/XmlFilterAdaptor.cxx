/*************************************************************************
 *
 *  $RCSfile: XmlFilterAdaptor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aidan $ $Date: 2002-04-10 13:49:48 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *                  Aidan Butler (aidan.butler@sun.com)
 *
 *
 ************************************************************************/
#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#ifndef _XMLFILTERADAPTOR_HXX
#include "XmlFilterAdaptor.hxx"
//#include <genericxmlfilter/GenericXMLFilter.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
/*
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
*/
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENTCONVERSION_XCONVERTERBRIDGE_HPP_
//#include <com/sun/star/documentconversion/XConverterBridge.hpp>
#include "XConverterBridge.hpp"
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif

using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::io::XActiveDataSource;
using com::sun::star::io::XOutputStream;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XExporter;
using com::sun::star::document::XFilter;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;
using com::sun::star::frame::XConfigManager;

sal_Bool SAL_CALL XmlFilterAdaptor::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) 
    throw (RuntimeException)
{
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    OUString sFileName= OUString::createFromAscii("");
    OUString sDirectory = OUString::createFromAscii("");
    OUString sURL = OUString::createFromAscii("");
    OUString udConvertClass=msUserData[0];
    OUString udJarPath=msUserData[1];
    OUString udImport =msUserData[2];
    OUString udExport =msUserData[3];   
    Reference < XInputStream > xInputStream;
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
    Reference<XConfigManager > xConfMgr( mxMSF->createInstance(
                      OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")),UNO_QUERY );
    
    if(! xConfMgr.is()){
      fprintf (stderr, "::com::sun::star::frame::XConfigManager service missing\n" );
      return sal_False;
    }
    else
      fprintf (stderr, "::com::sun::star::frame::XConfigManager service Created\n" );
    
    OUString installPath = xConfMgr->substituteVariables(OUString::createFromAscii("$(progurl)")); 
    installPath= installPath.concat(OUString::createFromAscii("/"));     // Extra slash onto end of URL

    
    /*
    nLength = msUserData.getLength();
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        fprintf(stderr,"UserData :  %s",::rtl::OUStringToOString( msUserData[i], RTL_TEXTENCODING_ASCII_US ).getStr());
        fprintf(stderr,"\n");
        
    }
    */
    const OUString sSaxParser ( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser") );
    
    /* The idea is that we could invoke a different export component based on the filter name.
     * we will default to importing writer documents*/

    OUString sXMLImportService (  udImport  );
    

    
    Reference < XParser > xSaxParser( mxMSF->createInstance( sSaxParser ), UNO_QUERY );
    Reference < XDocumentHandler > xHandler( mxMSF->createInstance( sXMLImportService ), UNO_QUERY );
    Reference < XImporter > xImporter( xHandler, UNO_QUERY );
    xImporter->setTargetDocument ( mxDoc );
    
    // Creating a ConverterBridge instance
    fprintf(stderr, "now creating the xConverter instance \n");
    Reference< XInterface > xConvBridge;
    
    xConvBridge = mxMSF->createInstance( udConvertClass );
    try{
      if(! xConvBridge.is()){
        fprintf (stderr, "XMLReader::Read: %s service missing\n",msUserData[0] );
        return sal_False;
      }
    }
    catch( Exception& e)
      {
        fprintf (stderr, "XMLReader::Read: %s Unable to create service instance\n" );
        return sal_False;
     }
    fprintf(stderr, "now creating the ConverterBridge instance\n");
    

    Reference< com::sun::star::documentconversion::XConverterBridge > xConverter;
    
    try {
      xConverter = Reference< com::sun::star::documentconversion::XConverterBridge > ( xConvBridge, UNO_QUERY );
    }
    catch( Exception& e)
      {
        fprintf(stderr, "Fell into the catch block!: %s\n",::rtl::OUStringToOString( 
                                         e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
        fprintf(stderr, "Casting failure!\n");
      }
    fprintf(stderr, "Created ConverterBridge instance\n"); 

    //pipe creation //
    Reference <XInterface> xPipe;
    xPipe=mxMSF->createInstance(OUString::createFromAscii("com.sun.star.io.Pipe"));
    if(!xPipe.is()){
      fprintf(stderr,"XMLReader::Read:com.sun.star.io.Pipe service missing\n");
      return sal_False;
    }
    else
      fprintf(stderr,"pipe created\n");
    
    
    Reference< XInputStream > xPipeInput (xPipe,UNO_QUERY);

    //inputStream from pipe
    
    Reference< XOutputStream > xTmpOutputStream (xPipe,UNO_QUERY);

    sal_Bool bRet = sal_True;
       

    //Call the ConverterBridge
    fprintf(stderr,"Calling converter!!");
    xConverter->convert(xInputStream, xTmpOutputStream,false,installPath.concat(udJarPath),sFileName);

    InputSource aInput;
    aInput.sSystemId = sFileName;
    aInput.aInputStream =  xPipeInput;
    
    xSaxParser->setDocumentHandler ( xHandler );
    try
    {
         
        xSaxParser->parseStream ( aInput );
    }
    catch( Exception &exc)
    {
            fprintf(stderr,"\nParseStream Exception\n");
        fprintf(stderr, "Fell into the catch block!: %s\n",::rtl::OUStringToOString( 
                exc.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
        bRet = sal_False;
    }
    
    return bRet;
}

sal_Bool SAL_CALL XmlFilterAdaptor::exportImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) 
    throw (RuntimeException)
{
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    OUString sFileName=OUString::createFromAscii("");
    OUString sDirectory= OUString::createFromAscii("");
    OUString sURL =OUString::createFromAscii("") ;
    OUString udConvertClass=msUserData[0];
    OUString udJarPath=msUserData[1];
    OUString udImport =msUserData[2];
    OUString udExport =msUserData[3];   
    Reference < XOutputStream > xOutputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
      
        if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "OutputStream" ) ) )
        {
            pValue[i].Value >>= xOutputStream;
            break;
        }
         else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "URL" ) ) )
        {   pValue[i].Value >>= sURL;
                sal_Int32 stringIndex = sURL.lastIndexOf(OUString::createFromAscii("/"));
            sFileName = sURL.copy(stringIndex+1, sURL.getLength()-stringIndex);
            sDirectory = sURL.copy(7,sURL.getLength()-7);
        }
    }
    if ( !xOutputStream.is() )
    {
        OSL_ASSERT ( 0 );
        return sal_False;
    }
    Reference<XConfigManager > xConfMgr( mxMSF->createInstance(
                      OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")),UNO_QUERY );
    
    if(! xConfMgr.is()){
      fprintf (stderr, "::com::sun::star::frame::XConfigManager service missing\n" );
      return sal_False;
    }
    else
      fprintf (stderr, "::com::sun::star::frame::XConfigManager service Created\n" );
    
    OUString installPath = xConfMgr->substituteVariables(OUString::createFromAscii("$(progurl)")); 
    installPath= installPath.concat(OUString::createFromAscii("/"));
    
    
    
    
    const OUString sSaxWriter ( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer") );

    /* The idea is that we could invoke a different export component based on the filter name.
     * we will default to exporting writer documents*/

    //OUString sXMLExportService ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.XMLExporter" ) );
    OUString sXMLExportService (  udExport  );
    
    Reference < XActiveDataSource > xSaxWriter( mxMSF->createInstance( sSaxWriter ), UNO_QUERY );

        // Set up converter bridge.
    //
    fprintf(stderr, "going to create converterbridge!\n");
    Reference< XInterface > xConvBridge;
    
    xConvBridge = mxMSF->createInstance(
                        udConvertClass );
    if(! xConvBridge.is()){
      fprintf (stderr, "XMLReader::Read: com.sun.star.documentconversion.ConverterBridge service missing\n" );
      return sal_False;
    }
        fprintf(stderr, "now creating the xConverter instance\n");
 
    Reference< com::sun::star::documentconversion::XConverterBridge > xConverter;
    
    try {
      xConverter = Reference< com::sun::star::documentconversion::XConverterBridge > ( xConvBridge, UNO_QUERY );
    }
    catch( Exception& e){
      fprintf(stderr, "Fell into the catch block!: %s\n",::rtl::OUStringToOString( 
                                           e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
      fprintf(stderr, "Casting failure!\n");
    }
    
    fprintf(stderr, "created converterbridge\n");
    
    // Set up pipe and setOutputStream to the pipe.
    //
    Reference< XInterface > xPipe;
    xPipe = mxMSF->createInstance(
                      OUString::createFromAscii("com.sun.star.io.Pipe") );
    if (! xPipe.is()){
      fprintf(stderr, "XMLReader::Read: com.sun.star.io.Pipe service missing\n" );
      return sal_False;
    }
    else
      fprintf(stderr, "pipe created\n");
       
    
    Reference< XOutputStream > xPipeOutput( xPipe, UNO_QUERY );
    
    // Get InputStream from pipe.
    //
        Reference<XInputStream> xTmpInputStream ( xPipe, UNO_QUERY );

    xSaxWriter->setOutputStream( xPipeOutput );
    Sequence < Any > aAnys ( 1 );
    aAnys[0] <<= xSaxWriter;
    Reference < XExporter > xExporter( mxMSF->createInstanceWithArguments ( sXMLExportService, aAnys ), UNO_QUERY );
    Reference < XFilter > xFilter( xExporter, UNO_QUERY );
    xExporter->setSourceDocument ( mxDoc );
    xFilter->filter ( aDescriptor );
    
    // Make the call to the bridge.
    //
    xConverter->convert(xTmpInputStream, xOutputStream,true,installPath.concat(udJarPath),sFileName);
    xOutputStream->closeOutput();
    xTmpInputStream->closeInput();
    
    return(sal_True);
}
sal_Bool SAL_CALL XmlFilterAdaptor::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) 
  throw (RuntimeException)
{
    return meType == FILTER_EXPORT ? exportImpl ( aDescriptor ) : importImpl ( aDescriptor );
}
void SAL_CALL XmlFilterAdaptor::cancel(  ) 
    throw (RuntimeException)
{
}
// XExporter
void SAL_CALL XmlFilterAdaptor::setSourceDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc ) 
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    meType = FILTER_EXPORT;
    mxDoc = xDoc;
}

// XImporter
void SAL_CALL XmlFilterAdaptor::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc ) 
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    meType = FILTER_IMPORT;
    mxDoc = xDoc;
}
// XInitialization
void SAL_CALL XmlFilterAdaptor::initialize( const Sequence< Any >& aArguments ) 
    throw (Exception, RuntimeException)
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    sal_Int32 newLength;
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue * pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
          
            if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Type" ) ) )
            {
                 pValue[i].Value >>= msFilterName;
                 
            }
            else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "UserData" ) ) )
            {
                
                pValue[i].Value >>= msUserData;
                
            }
            
        }
    }
}
OUString XmlFilterAdaptor_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.XmlFilterAdaptor" ) );
}
#define SERVICE_NAME1 "com.sun.star.document.ExportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ImportFilter"
sal_Bool SAL_CALL XmlFilterAdaptor_supportsService( const OUString& ServiceName ) 
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) ||
           ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME2 ) );
}
Sequence< OUString > SAL_CALL XmlFilterAdaptor_getSupportedServiceNames(  ) 
    throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
    pArray[1] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
    return aRet;
}
#undef SERVICE_NAME1
#undef SERVICE_NAME2

Reference< XInterface > SAL_CALL XmlFilterAdaptor_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new XmlFilterAdaptor( rSMgr );
}

// XServiceInfo
OUString SAL_CALL XmlFilterAdaptor::getImplementationName(  ) 
    throw (RuntimeException)
{
    return XmlFilterAdaptor_getImplementationName();
}
sal_Bool SAL_CALL XmlFilterAdaptor::supportsService( const OUString& rServiceName ) 
    throw (RuntimeException)
{
    return XmlFilterAdaptor_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL XmlFilterAdaptor::getSupportedServiceNames(  ) 
    throw (RuntimeException)
{
    return XmlFilterAdaptor_getSupportedServiceNames();
}
