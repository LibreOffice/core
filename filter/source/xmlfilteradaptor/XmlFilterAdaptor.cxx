/*************************************************************************
 *
 *  $RCSfile: XmlFilterAdaptor.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:03:35 $
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
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

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

#ifndef _COM_SUN_STAR_XML_XIMPORTFILTER_HPP_
#include <com/sun/star/xml/XImportFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_XEXPORTFILTER_HPP_
#include <com/sun/star/xml/XExportFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef  _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLELOADER_HPP_
#include <com/sun/star/style/XStyleLoader.hpp>
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

using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;

Reference< com::sun::star::frame::XModel > xModel;

sal_Bool SAL_CALL XmlFilterAdaptor::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    OUString udConvertClass=msUserData[0];
    OUString udImport =msUserData[2];
    sal_Int32 nSteps= 4;
    sal_Int32 nProgressRange(nSteps);

    //
    Sequence < PropertyValue > aAnySeq;
    Reference < XStatusIndicator >xStatusIndicator;
    sal_Int32 nLength = aDescriptor.getLength();
    sal_Int32 newLength;
    if ( nLength  )
    {
        const Sequence< ::com::sun::star::beans::PropertyValue >pValue = aDescriptor ;
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            //OSL_ENSURE( sal_False, ::rtl::OUStringToOString( pValue[i].Name, RTL_TEXTENCODING_ASCII_US).getStr());

            if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "StatusIndicator" ) ) )
            {
                 pValue[i].Value >>= xStatusIndicator;

            }
        }
    }
//
    if (xStatusIndicator.is()){
       xStatusIndicator->start(OUString(  RTL_CONSTASCII_USTRINGPARAM( "Loading :" )),nProgressRange);
       nSteps= 0;
       xStatusIndicator->setValue(nSteps);
    }

    OUString sXMLImportService (  udImport  );
    const OUString sSaxParser ( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser") );
    Reference < XParser > xSaxParser( mxMSF->createInstance( sSaxParser ), UNO_QUERY );
    Reference < XDocumentHandler > xHandler( mxMSF->createInstance( sXMLImportService ), UNO_QUERY );
    try{
      if(! xHandler.is())
        fprintf (stderr, "XMLReader::Read: %s Unable to create service instance xHandler\n" );
    }
    catch( Exception& e){
      fprintf (stderr, "XMLReader::Read: %s Unable to create service instance\n" );
    }
    Reference < XImporter > xImporter( xHandler, UNO_QUERY );
    xImporter->setTargetDocument ( mxDoc );
    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }

    //*********************
    // Creating a ConverterBridge instance
    //*********************
    Reference< XInterface > xConvBridge;
     xConvBridge = mxMSF->createInstance( udConvertClass );
    try{
      if(! xConvBridge.is()){
         OSL_ENSURE( sal_False,"XMLReader::Read: %s service missing\n" );
         //fprintf (stderr, "XMLReader::Read: %s service missing\n",udConvertClass );
        return sal_False;
      }
    }
    catch( Exception& e){
        //fprintf (stderr, "XMLReader::Read: %s Unable to create service instance\n" );
         OSL_ENSURE( sal_False,"XMLReader::Read: Unable to create service instance\n" );
        return sal_False;
     }
    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }
    Reference< com::sun::star::xml::XImportFilter > xConverter;
    try {
      xConverter = Reference< com::sun::star::xml::XImportFilter > ( xConvBridge, UNO_QUERY );
    }
    catch( Exception& e)
      {
        fprintf(stderr, "Fell into the catch block!: %s\n",::rtl::OUStringToOString(  e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
        OSL_ENSURE( sal_False, ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        fprintf(stderr, "Casting failure!\n");
      }

        sal_Bool xconv_ret = sal_True;
     if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
     }
    //*********************
    // Calling Filtering Component
    //*********************
       xconv_ret =xConverter->importer(aDescriptor,xHandler,msUserData);

     if (!xconv_ret) {
       //return sal_False;
        return sal_True;
     }
     if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
     }
     //********************
     //Template Loading if Required
     //********************
     if (!msTemplateName.equalsAscii("")){
             com::sun::star::uno::Reference< XModel >xModel ( com::sun::star::uno::Reference< XModel >::query( mxDoc ) );
         com::sun::star::uno::Reference< com::sun::star::style::XStyleFamiliesSupplier > xstylefamiliessupplier ( com::sun::star::uno::Reference< com::sun::star::style::XStyleFamiliesSupplier >::query( mxDoc ) );

         Reference< com::sun::star::container::XNameAccess >xName;
         if(xstylefamiliessupplier.is()){
               //fprintf (stderr, "\n\nCreated xstylefamiliessupplier\n");
               xName=xstylefamiliessupplier->getStyleFamilies();
         }
         com::sun::star::uno::Reference< com::sun::star::style::XStyleLoader > xstyleLoader ( com::sun::star::uno::Reference< com::sun::star::style::XStyleLoader >::query( xstylefamiliessupplier->getStyleFamilies()) );


         if(xstyleLoader.is()){
                //fprintf (stderr, "\n\nCreated xstyleLoader\n");
            xName=xstylefamiliessupplier->getStyleFamilies();
         }

         Sequence < OUString > elementNames = xName->getElementNames();
         Sequence<com::sun::star::beans::PropertyValue> pValue=xstyleLoader->getStyleLoaderOptions();

         //Load the Styles from the Template URL Supplied in the TypeDetection file
         xstyleLoader->loadStylesFromURL(msTemplateName,pValue);

          //*********************
         // Debug Info
         //*********************
         /*
         sal_Int32 nLength = pValue.getLength();
         OUString temp =OUString::createFromAscii(" ");
         for ( sal_Int32 i = 0 ; i < nLength; i++)
         {
                fprintf(stderr,"UserData :  %s",OUStringToOString(  pValue[i].Name, RTL_TEXTENCODING_ASCII_US ).getStr());
            pValue[i].Value>>=temp;
            fprintf(stderr,"UserData :  %s",OUStringToOString(  temp, RTL_TEXTENCODING_ASCII_US ).getStr());

         }

         nLength = elementNames.getLength();
         for ( sal_Int32 x = 0 ; x < nLength; x++)
         {
                fprintf(stderr,"UserData :  %s",OUStringToOString(  elementNames[x], RTL_TEXTENCODING_ASCII_US ).getStr());

         }

         fprintf(stderr,"UserData :  %s",OUStringToOString(  xModel->getURL(), RTL_TEXTENCODING_ASCII_US ).getStr());
         */
     }


    //*********************
    //End of template stuff
    //*********************
      if (xStatusIndicator.is()){
         xStatusIndicator->setValue(nSteps++);
         xStatusIndicator->end();
     }
     return sal_True;
}

sal_Bool SAL_CALL XmlFilterAdaptor::exportImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{

    OUString udConvertClass=msUserData[0];
    OUString udExport =msUserData[3];


    //*********************
        // Set up converter bridge.
    //*********************

    //fprintf(stderr, "going to create converterbridge!\n");
    Reference< XInterface > xConvBridge;

    xConvBridge = mxMSF->createInstance(
                        udConvertClass );
    if(! xConvBridge.is()){
      //fprintf (stderr, "XMLReader::Read: %s service missing\n",::rtl::OUStringToOString( msUserData[0], RTL_TEXTENCODING_ASCII_US).getStr() );
      OSL_ENSURE( sal_False,"XMLReader::Read: %s service missing\n" );
      return sal_False;
    }

        //fprintf(stderr, "now creating the xConverter instance\n");

    Reference< com::sun::star::xml::XExportFilter > xConverter;

    //*********************
    // Creating Status Bar
    //*********************
    Reference<com::sun::star::task::XStatusIndicator> xStatusIndicator;
    Reference< com::sun::star::frame::XController >xController(xModel->getCurrentController());
    if(xController.is()){
      //fprintf (stderr, "\nCreated xController\n");
      Reference < com::sun::star::frame::XFrame >xFrame (xController->getFrame());
      if(xFrame.is()){
        //fprintf (stderr, "\nCreated xFrame\n");
        Reference<com::sun::star::task::XStatusIndicatorFactory> xFactory(xFrame,UNO_QUERY);
        if (xFactory.is()) xStatusIndicator = xFactory->createStatusIndicator();
      }
    }

    sal_Int32 nSteps= 3;
    sal_Int32 nProgressRange(nSteps);
    if (xStatusIndicator.is()){
       xStatusIndicator->start(OUString(  RTL_CONSTASCII_USTRINGPARAM( "Saving :" )),nProgressRange);
        nSteps= 0;
        xStatusIndicator->setValue(nSteps);
    }

    //

    try {
        xConverter = Reference< com::sun::star::xml::XExportFilter > ( xConvBridge, UNO_QUERY );
          if (xStatusIndicator.is()){
            xStatusIndicator->setValue(nSteps++);
        }
    }
    catch( Exception& e){
      //fprintf(stderr, "Fell into the catch block!: %s\n",::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
      OSL_ENSURE( sal_False, ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
      //fprintf(stderr, "Casting failure!\n");
    }

    //fprintf(stderr, "created converterbridge\n");
    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }
    //*********************
    //Calling Filtering Component
    //*********************
    sal_Bool xconv_ret=xConverter->exporter(aDescriptor,msUserData);
    if (!xconv_ret) {
      return sal_False;
    }
    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }
    OUString sXMLExportService (  udExport  );
    Sequence < Any > aAnys ( 1 );
    aAnys[0] <<= xConverter;
    Reference < XExporter > xExporter( mxMSF->createInstanceWithArguments (
                       sXMLExportService, aAnys ), UNO_QUERY );
    Reference < XFilter > xFilter( xExporter, UNO_QUERY );
    xExporter->setSourceDocument ( mxDoc );
     xStatusIndicator->setValue(nSteps++);
    xFilter->filter ( aDescriptor );
    xStatusIndicator->end();
    return sal_True;
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
        com::sun::star::uno::Reference< com::sun::star::frame::XModel >rModel ( com::sun::star::uno::Reference< com::sun::star::frame::XModel >::query( xDoc ) );
    xModel=rModel;

}

// XImporter
void SAL_CALL XmlFilterAdaptor::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    meType = FILTER_IMPORT;
    mxDoc = xDoc;
    //xModel = uno::Reference< frame::XModel >::query( xDoc );
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
             //OSL_ENSURE( sal_False, ::rtl::OUStringToOString( pValue[i].Name, RTL_TEXTENCODING_ASCII_US).getStr());

            if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Type" ) ) )
            {
                 pValue[i].Value >>= msFilterName;

            }
            else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "UserData" ) ) )
            {

                pValue[i].Value >>= msUserData;

            }
            else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "TemplateName" ) ) )
            {

              pValue[i].Value>>=msTemplateName;
              //fprintf(stderr, "\n%s",::rtl::OUStringToOString(msFilterName , RTL_TEXTENCODING_ASCII_US).getStr());

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
