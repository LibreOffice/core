/*************************************************************************
 *
 *  $RCSfile: filterdetect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 17:57:54 $
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
 *  Contributor(s): Aidan Butler (aidan.butler@sun.com)
 *
 *
 *
 ************************************************************************/
#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#ifndef _FILTERDETECT_HXX
#include "filterdetect.hxx"
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


#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXTENDEDFILTERDETECTION_HPP_
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNamesAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif


#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif

#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
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
using com::sun::star::document::XExtendedFilterDetection;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;
using com::sun::star::task::XInteractionHandler;

using namespace ::com::sun::star::frame;
using namespace ::com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


Reference< com::sun::star::frame::XModel > xModel;

::rtl::OUString SAL_CALL supportedByType( const ::com::sun::star::uno::Sequence< ::rtl::OUString > urlPattern ,  const ::rtl::OString resultString, const ::rtl::OUString checkType);


::rtl::OUString SAL_CALL FilterDetect::detect( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& aArguments ) throw( com::sun::star::uno::RuntimeException )
{
        ::rtl::OUString sTypeName = OUString::createFromAscii("");
        ::rtl::OUString sUrl;
        ::rtl::OUString originalTypeName;
        Sequence<PropertyValue > lProps ;

        com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInStream;
        ::rtl::OUString temp;
        //OSL_ENSURE( sal_False, " starting Detect" );
        const PropertyValue * pValue = aArguments.getConstArray();
        sal_Int32 nLength;
        ::rtl::OString resultString;

        sal_Int32 location=0;

        nLength = aArguments.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {

            if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "TypeName" ) ) )
            {
                  //pValue[i].Value >>= originalTypeName;
                    location=i;
                   // OSL_ENSURE( sal_False, ::rtl::OUStringToOString(sTypeName,RTL_TEXTENCODING_ASCII_US).getStr() );

            }
            else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "URL" ) ) )
            {

                pValue[i].Value >>= sUrl;
                   //OSL_ENSURE( sal_False, ::rtl::OUStringToOString(sUrl,RTL_TEXTENCODING_ASCII_US).getStr() );

            }


        }
        try{
            Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;

            ::ucb::Content aContent(sUrl,xEnv);
            xInStream = aContent.openStream();
             com::sun::star::uno::Sequence< sal_Int8 > aData;
             if (!xInStream.is())
             {
                 return sTypeName;
             }
            long nBytesToRead=xInStream->available();
            xInStream->skipBytes (0);
            long bytestRead =xInStream->readBytes (aData,  1000);
            resultString=::rtl::OString((const sal_Char *)aData.getConstArray(),bytestRead) ;


             // test typedetect code


            Reference <XNameAccess> xTypeCont(mxMSF->createInstance(OUString::createFromAscii("com.sun.star.document.TypeDetection")),UNO_QUERY);
            Sequence < ::rtl::OUString > myTypes= xTypeCont->getElementNames();
            nLength = myTypes.getLength();


            sal_Int32 new_nlength=0;
            sal_Int32 i = 0 ;
             while(  (i < nLength) && (sTypeName.equalsAscii("")))
            {

                Any elem = xTypeCont->getByName(myTypes[i]);
                elem >>=lProps;
                new_nlength = lProps.getLength();
                sal_Int32 j =0;
                while( j < new_nlength && sTypeName.equalsAscii(""))
                {

                    Sequence< ::rtl::OUString > tmpStr;
                    lProps[j].Value >>=tmpStr;

                    if((lProps[j].Name.equalsAscii("URLPattern")) && (tmpStr.getLength()>0) )
                    {

                        sTypeName = supportedByType(tmpStr,resultString, myTypes[i]);

                    }
                 j++;
                }
            i++;
        }
        //end test

        }
        catch(Exception &)
        {
                 OSL_ENSURE( sal_False, "An Exception occured while opening File stream" );
        }

        if(sTypeName.equalsAscii(""))
        {
            //sTypeName=::rtl::OUString::createFromAscii("writer_Flat_XML_File");
        }
        else
           {
             aArguments[location].Value <<=sTypeName;

        }
       // OSL_ENSURE( sal_False, ::rtl::OUStringToOString(sTypeName,RTL_TEXTENCODING_ASCII_US).getStr() );


    return sTypeName;
}



::rtl::OUString SAL_CALL supportedByType( const Sequence< ::rtl::OUString > urlPattern ,  const ::rtl::OString resultString, const ::rtl::OUString checkType)
{
    sal_Int32 i=0;
    sal_Int32 checked =0;
    ::rtl::OUString sTypeName= OUString::createFromAscii("");
    while ( (i<urlPattern.getLength()) && (sTypeName.equalsAscii("")))
    {
    //OSL_ENSURE( sal_False, ::rtl::OUStringToOString(myTypes[i],RTL_TEXTENCODING_ASCII_US).getStr() );
    // OSL_ENSURE( sal_False, ::rtl::OUStringToOString(tmpStr[k].copy(0,7),RTL_TEXTENCODING_ASCII_US).getStr() );

        if((urlPattern[i].match(OUString::createFromAscii("doctype:")))&&(sTypeName.equalsAscii("")))
        {
            ::rtl::OString tryStr = ::rtl::OUStringToOString(urlPattern[i].copy(8),RTL_TEXTENCODING_ASCII_US).getStr();
            // OSL_ENSURE( sal_False, tryStr);
            while((checked <=resultString.getLength())&& (sTypeName.equalsAscii("")))
            {
                if( resultString.match(tryStr,checked))
                {
                    sTypeName = checkType;
                    break;
                }
                checked++;
            }
        }
       i++;
     }
    return sTypeName;
}



// XInitialization

void SAL_CALL FilterDetect::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
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
            else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "TemplateName" ) ) )
            {

              pValue[i].Value>>=msTemplateName;
            }

        }
    }
}



OUString FilterDetect_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.filters.XMLFilterDetect" ) );
}
#define SERVICE_NAME1 "com.sun.star.document.ExtendedTypeDetection"

sal_Bool SAL_CALL FilterDetect_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) );
}
Sequence< OUString > SAL_CALL FilterDetect_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
    return aRet;
}
#undef SERVICE_NAME1
#undef SERVICE_NAME2

Reference< XInterface > SAL_CALL FilterDetect_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new FilterDetect( rSMgr );
}

// XServiceInfo
OUString SAL_CALL FilterDetect::getImplementationName(  )
    throw (RuntimeException)
{
    return FilterDetect_getImplementationName();
}
sal_Bool SAL_CALL FilterDetect::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return FilterDetect_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return FilterDetect_getSupportedServiceNames();
}
