/*************************************************************************
 *
 *  $RCSfile: filterdetect.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-24 18:27:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "filterdetect.hxx"

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
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
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
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif


using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::document;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::beans;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::xml;
using namespace com::sun::star::task;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::ucb;


OUString SAL_CALL FilterDetect::detect(Sequence< PropertyValue >& aArguments )
    throw( RuntimeException )
{
    // type name to return
    OUString sOriginalTypeName;
    OUString sTypeName;
    OUString sURL;
    // stream of the document to be detected
    Reference< XInputStream > xInStream;
    for ( sal_Int32 i = 0 ; i < aArguments.getLength(); i++)
    {
        OUString aName = aArguments[i].Name;
        if (aName.equalsAscii("TypeName" ) )
            aArguments[i].Value >>= sOriginalTypeName;
        if (aName.equalsAscii("URL" ) )
            aArguments[i].Value >>= sURL;
        if (aName.equalsAscii("InputStream" ) )
            aArguments[i].Value >>= xInStream;
    }

    if (!xInStream.is())
    {
        // open the stream if it was not suplied by the framework
        Reference< XSimpleFileAccess > xSFI(mxMSF->createInstance(
            OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess" )), UNO_QUERY);
        if (sURL.getLength() > 0 && xSFI.is())
        {
            try
            {
                xInStream = xSFI->openFileRead( sURL);
            }
            catch( Exception& )
            {
                return sTypeName;
            }
        } else {
            // failed to access UCB
            return sTypeName;
        }
    }

    // flatxml starts with an office:document element. this element
    // conatains a clas="..." attribut by which we can deduct the
    // type of document that is to be loaded
    //
    // WARNING:
    // parsing the plain text of the document is an easy way to do this
    // but not the purest solution, since namespaces and other xml details
    // may lead to another syntactic expression of the same document.
    // this example works for the way the office serializes it's XML stream
    // but might need extension for other data sources...
    static OString aDocToken("office:document");
    // static OString aClassToken("office:class=\"");
    static OString aMimeTypeToken("office:mimetype=\"");

    sal_Int32 nHeadSize = 4096;
    Sequence< sal_Int8 > aHeadData(nHeadSize);

    // rewind seekable stream
    Reference< XSeekable > xSeek(xInStream, UNO_QUERY);
    if (xSeek.is())
        xSeek->seek(0);

    long bytestRead = xInStream->readBytes(aHeadData, nHeadSize);

    OString aHead = OString((const sal_Char *)aHeadData.getConstArray(), bytestRead).toAsciiLowerCase();

    // check for document element of flatxml format
    if (aHead.indexOf(aDocToken) >= 0)
    {
        // read document class
        sal_Int32 n = aHead.indexOf(aMimeTypeToken);
        if (n >= 0)
        {
            n += aMimeTypeToken.getLength();
            OString aMimeType = aHead.copy(n, aHead.indexOf('\"', n) - n);
            // return type for class found
            if      (aMimeType.equals("application/x-vnd.oasis.opendocument.text") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.text"))
                sTypeName = OUString::createFromAscii("devguide_FlatXMLType_Cpp_writer");
            else if (aMimeType.equals("application/x-vnd.oasis.opendocument.text-master") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.text-master"))
                sTypeName = OUString::createFromAscii("devguide_FlatXMLType_Cpp_master");
           else if (aMimeType.equals("application/x-vnd.oasis.openoffice.text-global") ||
                      aMimeType.equals("application/vnd.oasis.openoffice.text-global"))
                sTypeName = OUString::createFromAscii("devguide_FlatXMLType_Cpp_master");
           else if (aMimeType.equals("application/x-vnd.oasis.opendocument.spreadsheet") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.spreadsheet"))
                sTypeName = OUString::createFromAscii("devguide_FlatXMLType_Cpp_calc");
            else if (aMimeType.equals("application/x-vnd.oasis.opendocument.drawing") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.drawing"))
                sTypeName = OUString::createFromAscii("devguide_FlatXMLType_Cpp_draw");
            else if (aMimeType.equals("application/x-vnd.oasis.opendocument.presentation") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.presentation"))
                sTypeName = OUString::createFromAscii("devguide_FlatXMLType_Cpp_impress");
            else if (aMimeType.equals("application/x-vnd.oasis.opendocument.presentation") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.presentation"))
                sTypeName = OUString::createFromAscii("devguide_FlatXMLType_Cpp_impress");
        }
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
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "devguide.officedev.samples.filter.FlatXmlDetect" ) );
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
