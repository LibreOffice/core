/*************************************************************************
 *
 *  $RCSfile: XmlFilterAdaptor.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 15:57:42 $
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
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
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

#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
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
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/conainer/XNameAccess.hpp>
#endif
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/mediadescriptor.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::style;
using namespace com::sun::star::xml;
using namespace com::sun::star::xml::sax;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;

Reference< com::sun::star::frame::XModel > xModel;

sal_Bool SAL_CALL XmlFilterAdaptor::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    OUString udConvertClass=msUserData[0];
    OUString udImport =msUserData[2];
    sal_Int32 nSteps= 0;
    sal_Int32 nProgressRange = 4;

    comphelper::MediaDescriptor aMediaMap(aDescriptor);
    Reference< XStatusIndicator > xStatusIndicator(aMediaMap.getUnpackedValueOrDefault(
        comphelper::MediaDescriptor::PROP_STATUSINDICATOR(), Reference< XStatusIndicator >()));

    if (xStatusIndicator.is()){
        xStatusIndicator->start(OUString(  RTL_CONSTASCII_USTRINGPARAM( "Loading :" )),nProgressRange);
    }

    OUString sXMLImportService (  udImport  );
    const OUString sSaxParser ( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser") );
    Reference < XParser > xSaxParser( mxMSF->createInstance( sSaxParser ), UNO_QUERY );
    Reference < XDocumentHandler > xHandler( mxMSF->createInstance( sXMLImportService ), UNO_QUERY );
    if(! xHandler.is()) {
        OSL_ENSURE(sal_False, "XMLReader::Read: %s Unable to create service instance xHandler\n" );
        return sal_False;
    }
    Reference < XImporter > xImporter( xHandler, UNO_QUERY );
    xImporter->setTargetDocument ( mxDoc );

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }

    //*********************
    // Creating a ConverterBridge instance
    //*********************
    Reference< XInterface > xConvBridge(mxMSF->createInstance( udConvertClass ), UNO_QUERY);
    if(! xConvBridge.is()){
        OSL_ENSURE( sal_False,"XMLReader::Read: %s service missing\n" );
        return sal_False;
    }
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    Reference< XImportFilter > xConverter( xConvBridge, UNO_QUERY );

     //********************
    //Template Loading if Required
    //********************
    if (!msTemplateName.equalsAscii("")){
        Reference< XModel > xModel(mxDoc, UNO_QUERY);
        Reference< XStyleFamiliesSupplier > xstylefamiliessupplier(mxDoc, UNO_QUERY);

        Reference< XNameAccess >xName;
        if(xstylefamiliessupplier.is()){
            xName=xstylefamiliessupplier->getStyleFamilies();
        }
        Reference< XStyleLoader > xstyleLoader (xstylefamiliessupplier->getStyleFamilies(), UNO_QUERY);


        if(xstyleLoader.is()){
            xName=xstylefamiliessupplier->getStyleFamilies();
        }

        Sequence < OUString > elementNames = xName->getElementNames();
        Sequence<com::sun::star::beans::PropertyValue> pValue=xstyleLoader->getStyleLoaderOptions();

        //Load the Styles from the Template URL Supplied in the TypeDetection file
        if(msTemplateName.indexOf(OUString::createFromAscii("file:"))==-1)
        {
            Reference< XConfigManager >xCfgMgr ( mxMSF->createInstance(
                OUString::createFromAscii("com.sun.star.config.SpecialConfigManager") ), UNO_QUERY );
            OUString PathString=xCfgMgr->substituteVariables(OUString::createFromAscii("$(progurl)"));
            PathString=PathString.concat(OUString::createFromAscii("/"));
            msTemplateName=PathString.concat(msTemplateName);
        }

        xstyleLoader->loadStylesFromURL(msTemplateName,pValue);
    }

    sal_Bool xconv_ret = sal_True;

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }
    //*********************
    // Calling Filtering Component
    //*********************
    try {
        if (!xConverter->importer(aDescriptor,xHandler,msUserData)) {
            if (xStatusIndicator.is())
                   xStatusIndicator->end();
            return sal_False;
        }
    }
    catch( Exception& e){
        if (xStatusIndicator.is())
               xStatusIndicator->end();

        OSL_ENSURE( sal_False, ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        return sal_False;
    }
    if (xStatusIndicator.is()) {
        xStatusIndicator->setValue(nSteps++);
        xStatusIndicator->end();
    }
    return sal_True;
}

sal_Bool SAL_CALL XmlFilterAdaptor::exportImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{

    OUString udConvertClass = msUserData[0];
    OUString udExport = msUserData[3];

    // Status Bar
    sal_Int32 nSteps= 1;
    sal_Int32 nProgressRange(3);
    comphelper::MediaDescriptor aMediaMap(aDescriptor);
    Reference< XStatusIndicator > xStatusIndicator(aMediaMap.getUnpackedValueOrDefault(
        comphelper::MediaDescriptor::PROP_STATUSINDICATOR(), Reference< XStatusIndicator >()));

    if (xStatusIndicator.is())
       xStatusIndicator->start(OUString(  RTL_CONSTASCII_USTRINGPARAM( "Saving :" )),nProgressRange);

    // Set up converter bridge.
    Reference< com::sun::star::xml::XExportFilter > xConverter(mxMSF->createInstance(udConvertClass ), UNO_QUERY);
    if(! xConverter.is()){
      OSL_ENSURE( sal_False, "xml export sub service missing" );
      return sal_False;
    }

    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    //put filter component into exporting state
    if (!xConverter->exporter(aDescriptor, msUserData)) {
        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return sal_False;
    }
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    try{
        // create the xml exporter service and supply the converter component
        // which implements the document handler
        Sequence < Any > aAnys ( 1 );
        aAnys[0] <<= xConverter;
        Reference< XExporter > xExporter( mxMSF->createInstanceWithArguments (
                       udExport, aAnys ), UNO_QUERY_THROW );
        xExporter->setSourceDocument( mxDoc );
        // get XFilter interface
        Reference< XFilter > xFilter( xExporter, UNO_QUERY_THROW );

        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        // call the actual filtering component
        if (!xFilter->filter(aDescriptor))
        {
            if (xStatusIndicator.is())
                   xStatusIndicator->end();
            return sal_False;
        }
    }
    catch( Exception& exE){
        OSL_ENSURE( sal_False, ::rtl::OUStringToOString( exE.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return sal_False;
    }

    // done
    if (xStatusIndicator.is())
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
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        comphelper::SequenceAsHashMap aMap(aAnySeq);
        msFilterName = aMap.getUnpackedValueOrDefault(
            OUString::createFromAscii("Type"), OUString());
        msUserData = aMap.getUnpackedValueOrDefault(
            OUString::createFromAscii("UserData"), Sequence< OUString >());
        msTemplateName = aMap.getUnpackedValueOrDefault(
            OUString::createFromAscii("TemplateName"), OUString());
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
