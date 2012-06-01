/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (C) 2000 by Sun Microsystems, Inc.
 * Copyright (C) 2002-2004 William Lachance (wlach@interlog.com)
 * Copyright (C) 2004 Net Integration Technologies (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 */
/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <osl/diagnose.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <comphelper/componentcontext.hxx>
#include <xmloff/attrlist.hxx>
#include <sfx2/passwd.hxx>
#include <ucbhelper/content.hxx>

#include <libwpd/libwpd.h>

#include "filter/FilterInternal.hxx"
#include "filter/DocumentHandler.hxx"
#include "filter/OdgGenerator.hxx"
#include "filter/OdtGenerator.hxx"
#include "stream/WPXSvStream.h"
#include "WordPerfectImportFilter.hxx"

using namespace ::com::sun::star;

using rtl::OString;
using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;
using com::sun::star::ucb::XCommandEnvironment;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

void callHandler(uno::Reference < XDocumentHandler > xDocHandler);


static bool handleEmbeddedWPGObject(const WPXBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdgGenerator exporter(pHandler, streamType);

    libwpg::WPGFileFormat fileFormat = libwpg::WPG_AUTODETECT;

    if (!libwpg::WPGraphics::isSupported(const_cast<WPXInputStream *>(data.getDataStream())))
        fileFormat = libwpg::WPG_WPG1;

    return libwpg::WPGraphics::parse(const_cast<WPXInputStream *>(data.getDataStream()), &exporter, fileFormat);
}

static bool handleEmbeddedWPGImage(const WPXBinaryData &input, WPXBinaryData &output)
{
    WPXString svgOutput;
    libwpg::WPGFileFormat fileFormat = libwpg::WPG_AUTODETECT;

    if (!libwpg::WPGraphics::isSupported(const_cast<WPXInputStream *>(input.getDataStream())))
        fileFormat = libwpg::WPG_WPG1;

    if (!libwpg::WPGraphics::generateSVG(const_cast<WPXInputStream *>(input.getDataStream()), svgOutput, fileFormat))
        return false;

    output.clear();
    output.append((unsigned char *)svgOutput.cstr(), strlen(svgOutput.cstr()));
    return true;
}

sal_Bool SAL_CALL WordPerfectImportFilter::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException)
{
    WRITER_DEBUG_MSG(("WordPerfectImportFilter::importImpl: Got here!\n"));

    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    OUString sURL;
    uno::Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name == "URL" )
            pValue[i].Value >>= sURL;
    }
    if ( !xInputStream.is() )
    {
        OSL_ASSERT( 0 );
        return sal_False;
    }

    WPXSvInputStream input( xInputStream );

    OString aUtf8Passwd;

    WPDConfidence confidence = WPDocument::isFileFormatSupported(&input);

    if (WPD_CONFIDENCE_SUPPORTED_ENCRYPTION == confidence)
    {
        int unsuccessfulAttempts = 0;
        while (true )
        {
            SfxPasswordDialog aPasswdDlg( 0 );
            aPasswdDlg.SetMinLen(0);
            if(!aPasswdDlg.Execute())
                return sal_False;
            String aPasswd = aPasswdDlg.GetPassword();
            OUString aUniPasswd(aPasswd.GetBuffer() /*, aPasswd.Len(), RTL_TEXTENCODING_UCS2 */);
            aUtf8Passwd = OUStringToOString(aUniPasswd, RTL_TEXTENCODING_UTF8);
            if (WPD_PASSWORD_MATCH_OK == WPDocument::verifyPassword(&input, aUtf8Passwd.getStr()))
                break;
            else
                unsuccessfulAttempts++;
            if (unsuccessfulAttempts == 3) // timeout after 3 password atempts
                return sal_False;
        }
    }

    // An XML import service: what we push sax messages to..
    OUString sXMLImportService (  "com.sun.star.comp.Writer.XMLOasisImporter"  );
    Reference < XDocumentHandler > xInternalHandler( comphelper::ComponentContext( mxContext ).createComponent( sXMLImportService ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    uno::Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    // OO Document Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    OdtGenerator collector(&xHandler, ODF_FLAT_XML);
	collector.registerEmbeddedObjectHandler("image/x-wpg", &handleEmbeddedWPGObject);
	collector.registerEmbeddedImageHandler("image/x-wpg", &handleEmbeddedWPGImage);
    if (WPD_OK == WPDocument::parse(&input, &collector, aUtf8Passwd.isEmpty() ? 0 : aUtf8Passwd.getStr()))
        return sal_True;
    return sal_False;
}

sal_Bool SAL_CALL WordPerfectImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException)
{
    WRITER_DEBUG_MSG(("WordPerfectImportFilter::filter: Got here!\n"));
    return importImpl ( aDescriptor );
}
void SAL_CALL WordPerfectImportFilter::cancel(  )
throw (RuntimeException)
{
    WRITER_DEBUG_MSG(("WordPerfectImportFilter::cancel: Got here!\n"));
}

// XImporter
void SAL_CALL WordPerfectImportFilter::setTargetDocument( const uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    WRITER_DEBUG_MSG(("WordPerfectImportFilter::getTargetDocument: Got here!\n"));
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL WordPerfectImportFilter::detect( uno::Sequence< PropertyValue >& Descriptor )
throw( uno::RuntimeException )
{
    WRITER_DEBUG_MSG(("WordPerfectImportFilter::detect: Got here!\n"));

    WPDConfidence confidence = WPD_CONFIDENCE_NONE;
    OUString sTypeName;
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 location = nLength;
    OUString sURL;
    const PropertyValue *pValue = Descriptor.getConstArray();
    uno::Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "TypeName" )
            location=i;
        else if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name == "URL" )
            pValue[i].Value >>= sURL;
    }

    uno::Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;
    if (!xInputStream.is())
    {
        try
        {
            ::ucbhelper::Content aContent(sURL, xEnv);
            xInputStream = aContent.openStream();
        }
        catch ( ... )
        {
            return ::rtl::OUString();
        }

        if (!xInputStream.is())
            return ::rtl::OUString();
    }

    WPXSvInputStream input( xInputStream );

    if (input.atEOS())
        return ::rtl::OUString();

    confidence = WPDocument::isFileFormatSupported(&input);

    if (confidence == WPD_CONFIDENCE_EXCELLENT || confidence == WPD_CONFIDENCE_SUPPORTED_ENCRYPTION)
        sTypeName = OUString(  "writer_WordPerfect_Document"  );

    if (!sTypeName.isEmpty())
    {
        if ( location == Descriptor.getLength() )
        {
            Descriptor.realloc(nLength+1);
            Descriptor[location].Name = ::rtl::OUString("TypeName");
        }

        Descriptor[location].Value <<=sTypeName;
    }

    return sTypeName;
}


// XInitialization
void SAL_CALL WordPerfectImportFilter::initialize( const Sequence< Any >& aArguments )
throw (Exception, RuntimeException)
{
    WRITER_DEBUG_MSG(("WordPerfectImportFilter::initialize: Got here!\n"));
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue *pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue[i].Name == "Type" )
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}
OUString WordPerfectImportFilter_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "com.sun.star.comp.Writer.WordPerfectImportFilter"  );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
sal_Bool SAL_CALL WordPerfectImportFilter_supportsService( const OUString &ServiceName )
throw (RuntimeException)
{
    return ( ServiceName == SERVICE_NAME1 || ServiceName == SERVICE_NAME2 );
}
Sequence< OUString > SAL_CALL WordPerfectImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  OUString (  SERVICE_NAME1  );
    pArray[1] =  OUString (  SERVICE_NAME2  );
    return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

uno::Reference< XInterface > SAL_CALL WordPerfectImportFilter_createInstance( const uno::Reference< uno::XComponentContext > & rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new WordPerfectImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL WordPerfectImportFilter::getImplementationName(  )
throw (RuntimeException)
{
    return WordPerfectImportFilter_getImplementationName();
}
sal_Bool SAL_CALL WordPerfectImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException)
{
    return WordPerfectImportFilter_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL WordPerfectImportFilter::getSupportedServiceNames(  )
throw (RuntimeException)
{
    return WordPerfectImportFilter_getSupportedServiceNames();
}


WordPerfectImportFilterDialog::WordPerfectImportFilterDialog( const uno::Reference< uno::XComponentContext > & rContext) :
    mxContext( rContext ) {}

WordPerfectImportFilterDialog::~WordPerfectImportFilterDialog()
{
}

void SAL_CALL WordPerfectImportFilterDialog::setTitle( const ::rtl::OUString & )
throw (uno::RuntimeException)
{
}

sal_Int16 SAL_CALL WordPerfectImportFilterDialog::execute()
throw (uno::RuntimeException)
{
    WPXSvInputStream input( mxInputStream );

    OString aUtf8Passwd;

    WPDConfidence confidence = WPDocument::isFileFormatSupported(&input);

    if (WPD_CONFIDENCE_SUPPORTED_ENCRYPTION == confidence)
    {
        int unsuccessfulAttempts = 0;
        while (true )
        {
            SfxPasswordDialog aPasswdDlg(0);
            aPasswdDlg.SetMinLen(0);
            if(!aPasswdDlg.Execute())
                return com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
            msPassword = ::rtl::OUString(aPasswdDlg.GetPassword().GetBuffer());
            aUtf8Passwd = OUStringToOString(msPassword, RTL_TEXTENCODING_UTF8);
            if (WPD_PASSWORD_MATCH_OK == WPDocument::verifyPassword(&input, aUtf8Passwd.getStr()))
                break;
            else
                unsuccessfulAttempts++;
            if (unsuccessfulAttempts == 3) // timeout after 3 password atempts
                return com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
        }
    }
    return com::sun::star::ui::dialogs::ExecutableDialogResults::OK;
}

uno::Sequence<beans::PropertyValue> SAL_CALL WordPerfectImportFilterDialog::getPropertyValues() throw(uno::RuntimeException)
{
    uno::Sequence<beans::PropertyValue> aRet(1);
    beans::PropertyValue *pArray = aRet.getArray();

    pArray[0].Name = rtl::OUString( "Password" );
    pArray[0].Value <<= msPassword;

    return aRet;
}

void SAL_CALL WordPerfectImportFilterDialog::setPropertyValues( const uno::Sequence<beans::PropertyValue>& aProps)
throw(beans::UnknownPropertyException, beans::PropertyVetoException,
      lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    const beans::PropertyValue *pPropArray = aProps.getConstArray();
    long nPropCount = aProps.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue &rProp = pPropArray[i];
        ::rtl::OUString aPropName = rProp.Name;

        if ( aPropName == ::rtl::OUString("Password") )
            rProp.Value >>= msPassword;
        else if ( aPropName == "InputStream" )
            rProp.Value >>= mxInputStream;
    }
}


// XServiceInfo
OUString SAL_CALL WordPerfectImportFilterDialog::getImplementationName(  )
throw (RuntimeException)
{
    return WordPerfectImportFilterDialog_getImplementationName();
}

sal_Bool SAL_CALL WordPerfectImportFilterDialog::supportsService( const OUString &rServiceName )
throw (RuntimeException)
{
    return WordPerfectImportFilterDialog_supportsService( rServiceName );
}

Sequence< OUString > SAL_CALL WordPerfectImportFilterDialog::getSupportedServiceNames(  )
throw (RuntimeException)
{
    return WordPerfectImportFilterDialog_getSupportedServiceNames();
}

OUString WordPerfectImportFilterDialog_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "com.sun.star.comp.Writer.WordPerfectImportFilterDialog"  );
}

#define SERVICE_NAME "com.sun.star.ui.dialogs.FilterOptionsDialog"
sal_Bool SAL_CALL WordPerfectImportFilterDialog_supportsService( const OUString &ServiceName )
throw (RuntimeException)
{
    return ( ServiceName == SERVICE_NAME );
}

Sequence< OUString > SAL_CALL WordPerfectImportFilterDialog_getSupportedServiceNames(  )
throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString *pArray = aRet.getArray();
    pArray[0] =  OUString (  SERVICE_NAME  );
    return aRet;
}
#undef SERVICE_NAME

uno::Reference< XInterface > SAL_CALL WordPerfectImportFilterDialog_createInstance( const uno::Reference< uno::XComponentContext > & rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new WordPerfectImportFilterDialog( rContext );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
