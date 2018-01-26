/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
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
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/supportsservice.hxx>

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

using ::rtl::OUString;
using ::rtl::OString;


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
        if ( aName == "TypeName" )
            aArguments[i].Value >>= sOriginalTypeName;
        if ( aName == "URL" )
            aArguments[i].Value >>= sURL;
        if ( aName == "InputStream" )
            aArguments[i].Value >>= xInStream;
    }

    if (!xInStream.is())
    {
        // open the stream if it was not supplied by the framework
        Reference< XSimpleFileAccess3 > xSFI(SimpleFileAccess::create(mxContext));
        if (sURL.getLength() > 0)
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
    // contains a class="..." attribute by which we can deduct the
    // type of document that is to be loaded

    // WARNING:
    // parsing the plain text of the document is an easy way to do this
    // but not the purest solution, since namespaces and other xml details
    // may lead to another syntactic expression of the same document.
    // this example works for the way the office serializes its XML stream
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
                sTypeName = "devguide_FlatXMLType_Cpp_writer";
            else if (aMimeType.equals("application/x-vnd.oasis.opendocument.text-master") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.text-master"))
                sTypeName = "devguide_FlatXMLType_Cpp_master";
           else if (aMimeType.equals("application/x-vnd.oasis.openoffice.text-global") ||
                      aMimeType.equals("application/vnd.oasis.openoffice.text-global"))
                sTypeName = "devguide_FlatXMLType_Cpp_master";
           else if (aMimeType.equals("application/x-vnd.oasis.opendocument.spreadsheet") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.spreadsheet"))
                sTypeName = "devguide_FlatXMLType_Cpp_calc";
            else if (aMimeType.equals("application/x-vnd.oasis.opendocument.drawing") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.drawing"))
                sTypeName = "devguide_FlatXMLType_Cpp_draw";
            else if (aMimeType.equals("application/x-vnd.oasis.opendocument.presentation") ||
                       aMimeType.equals("application/vnd.oasis.opendocument.presentation"))
                sTypeName = "devguide_FlatXMLType_Cpp_impress";
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
            if ( pValue[i].Name == "Type" )
            {
                pValue[i].Value >>= msFilterName;
            }
            else if ( pValue[i].Name == "UserData" )
            {
                pValue[i].Value >>= msUserData;
            }
            else if ( pValue[i].Name == "TemplateName" )
            {
                pValue[i].Value>>=msTemplateName;
            }
        }
    }
}

OUString FilterDetect_getImplementationName ()
    throw (RuntimeException)
{
    return OUString( "devguide.officedev.samples.filter.FlatXmlDetect" );
}

#define SERVICE_NAME1 "com.sun.star.document.ExtendedTypeDetection"

Sequence< OUString > SAL_CALL FilterDetect_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( SERVICE_NAME1 );
    return aRet;
}
#undef SERVICE_NAME1
#undef SERVICE_NAME2

Reference< XInterface > SAL_CALL FilterDetect_createInstance( const Reference< XComponentContext > & rContext)
    throw( Exception )
{
    return (cppu::OWeakObject*) new FilterDetect( rContext );
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
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return FilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
