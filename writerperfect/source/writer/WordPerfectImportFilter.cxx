/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
#include <cppuhelper/supportsservice.hxx>

#include <DocumentHandler.hxx>
#include <WPXSvInputStream.hxx>

#include <xmloff/attrlist.hxx>
#include <sfx2/passwd.hxx>
#include <ucbhelper/content.hxx>

#include <libodfgen/libodfgen.hxx>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

#include "WordPerfectImportFilter.hxx"

using ::ucbhelper::Content;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
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

using writerperfect::DocumentHandler;
using writerperfect::WPXSvInputStream;

static bool handleEmbeddedWPGObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdgGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);

    libwpg::WPGFileFormat fileFormat = libwpg::WPG_AUTODETECT;

    if (!libwpg::WPGraphics::isSupported(data.getDataStream()))
        fileFormat = libwpg::WPG_WPG1;

    return libwpg::WPGraphics::parse(data.getDataStream(), &exporter, fileFormat);
}

static bool handleEmbeddedWPGImage(const librevenge::RVNGBinaryData &input, librevenge::RVNGBinaryData &output)
{
    libwpg::WPGFileFormat fileFormat = libwpg::WPG_AUTODETECT;

    if (!libwpg::WPGraphics::isSupported(input.getDataStream()))
        fileFormat = libwpg::WPG_WPG1;

    librevenge::RVNGStringVector svgOutput;
    librevenge::RVNGSVGDrawingGenerator aSVGGenerator(svgOutput, "");

    if (!libwpg::WPGraphics::parse(input.getDataStream(), &aSVGGenerator, fileFormat))
        return false;

    if (svgOutput.empty())
        return false;

    assert(1 == svgOutput.size());

    output.clear();
    output.append(reinterpret_cast<const unsigned char *>(svgOutput[0].cstr()), svgOutput[0].size());
    return true;
}

bool SAL_CALL WordPerfectImportFilter::importImpl(const Sequence< css::beans::PropertyValue > &aDescriptor)
throw (RuntimeException, std::exception)
{
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    for (sal_Int32 i = 0 ; i < nLength; i++)
    {
        if (pValue[i].Name == "InputStream")
            pValue[i].Value >>= xInputStream;
    }
    if (!xInputStream.is())
    {
        OSL_ASSERT(false);
        return false;
    }

    WPXSvInputStream input(xInputStream);

    OString aUtf8Passwd;

    libwpd::WPDConfidence confidence = libwpd::WPDocument::isFileFormatSupported(&input);

    if (libwpd::WPD_CONFIDENCE_SUPPORTED_ENCRYPTION == confidence)
    {
        int unsuccessfulAttempts = 0;
        while (true)
        {
            ScopedVclPtrInstance< SfxPasswordDialog > aPasswdDlg(nullptr);
            aPasswdDlg->SetMinLen(0);
            if (!aPasswdDlg->Execute())
                return false;
            OUString aPasswd = aPasswdDlg->GetPassword();
            aUtf8Passwd = OUStringToOString(aPasswd, RTL_TEXTENCODING_UTF8);
            if (libwpd::WPD_PASSWORD_MATCH_OK == libwpd::WPDocument::verifyPassword(&input, aUtf8Passwd.getStr()))
                break;
            else
                unsuccessfulAttempts++;
            if (unsuccessfulAttempts == 3) // timeout after 3 password atempts
                return false;
        }
    }

    // An XML import service: what we push sax messages to..
    Reference < XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.comp.Writer.XMLOasisImporter", mxContext),
        css::uno::UNO_QUERY_THROW);

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    // OO Document Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    OdtGenerator collector;
    collector.addDocumentHandler(&xHandler, ODF_FLAT_XML);
    collector.registerEmbeddedObjectHandler("image/x-wpg", &handleEmbeddedWPGObject);
    collector.registerEmbeddedImageHandler("image/x-wpg", &handleEmbeddedWPGImage);
    if (libwpd::WPD_OK == libwpd::WPDocument::parse(&input, &collector, aUtf8Passwd.isEmpty() ? nullptr : aUtf8Passwd.getStr()))
        return true;
    return false;
}

sal_Bool SAL_CALL WordPerfectImportFilter::filter(const Sequence< css::beans::PropertyValue > &aDescriptor)
throw (RuntimeException, std::exception)
{
    return importImpl(aDescriptor);
}
void SAL_CALL WordPerfectImportFilter::cancel()
throw (RuntimeException, std::exception)
{
}

// XImporter
void SAL_CALL WordPerfectImportFilter::setTargetDocument(const Reference< css::lang::XComponent > &xDoc)
throw (css::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL WordPerfectImportFilter::detect(Sequence< PropertyValue > &Descriptor)
throw(RuntimeException, std::exception)
{
    libwpd::WPDConfidence confidence = libwpd::WPD_CONFIDENCE_NONE;
    OUString sTypeName;
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 location = nLength;
    const PropertyValue *pValue = Descriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    for (sal_Int32 i = 0 ; i < nLength; i++)
    {
        if (pValue[i].Name == "TypeName")
            location=i;
        else if (pValue[i].Name == "InputStream")
            pValue[i].Value >>= xInputStream;
    }

    if (!xInputStream.is())
        return OUString();

    WPXSvInputStream input(xInputStream);

    confidence = libwpd::WPDocument::isFileFormatSupported(&input);

    if (confidence == libwpd::WPD_CONFIDENCE_EXCELLENT || confidence == libwpd::WPD_CONFIDENCE_SUPPORTED_ENCRYPTION)
        sTypeName = "writer_WordPerfect_Document";

    if (!sTypeName.isEmpty())
    {
        if (location == nLength)
        {
            Descriptor.realloc(nLength+1);
            Descriptor[location].Name = "TypeName";
        }

        Descriptor[location].Value <<=sTypeName;
    }

    return sTypeName;
}


// XInitialization
void SAL_CALL WordPerfectImportFilter::initialize(const Sequence< Any > &aArguments)
throw (Exception, RuntimeException, std::exception)
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if (nLength && (aArguments[0] >>= aAnySeq))
    {
        const PropertyValue *pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for (sal_Int32 i = 0 ; i < nLength; i++)
        {
            if (pValue[i].Name == "Type")
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}
OUString WordPerfectImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Writer.WordPerfectImportFilter");
}

Sequence< OUString > SAL_CALL WordPerfectImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL WordPerfectImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new WordPerfectImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL WordPerfectImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return WordPerfectImportFilter_getImplementationName();
}
sal_Bool SAL_CALL WordPerfectImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL WordPerfectImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return WordPerfectImportFilter_getSupportedServiceNames();
}


WordPerfectImportFilterDialog::WordPerfectImportFilterDialog(const Reference< XComponentContext > &rContext) :
    mxContext(rContext) {}

WordPerfectImportFilterDialog::~WordPerfectImportFilterDialog()
{
}

void SAL_CALL WordPerfectImportFilterDialog::setTitle(const OUString &)
throw (RuntimeException, std::exception)
{
}

sal_Int16 SAL_CALL WordPerfectImportFilterDialog::execute()
throw (RuntimeException, std::exception)
{
    WPXSvInputStream input(mxInputStream);

    OString aUtf8Passwd;

    libwpd::WPDConfidence confidence = libwpd::WPDocument::isFileFormatSupported(&input);

    if (libwpd::WPD_CONFIDENCE_SUPPORTED_ENCRYPTION == confidence)
    {
        int unsuccessfulAttempts = 0;
        while (true)
        {
            ScopedVclPtrInstance< SfxPasswordDialog > aPasswdDlg(nullptr);
            aPasswdDlg->SetMinLen(0);
            if (!aPasswdDlg->Execute())
                return css::ui::dialogs::ExecutableDialogResults::CANCEL;
            msPassword = aPasswdDlg->GetPassword().getStr();
            aUtf8Passwd = OUStringToOString(msPassword, RTL_TEXTENCODING_UTF8);
            if (libwpd::WPD_PASSWORD_MATCH_OK == libwpd::WPDocument::verifyPassword(&input, aUtf8Passwd.getStr()))
                break;
            else
                unsuccessfulAttempts++;
            if (unsuccessfulAttempts == 3) // timeout after 3 password atempts
                return css::ui::dialogs::ExecutableDialogResults::CANCEL;
        }
    }
    return css::ui::dialogs::ExecutableDialogResults::OK;
}

Sequence<PropertyValue> SAL_CALL WordPerfectImportFilterDialog::getPropertyValues() throw(RuntimeException, std::exception)
{
    Sequence<PropertyValue> aRet(1);
    PropertyValue *pArray = aRet.getArray();

    pArray[0].Name = "Password";
    pArray[0].Value <<= msPassword;

    return aRet;
}

void SAL_CALL WordPerfectImportFilterDialog::setPropertyValues(const Sequence<PropertyValue> &aProps)
throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
      css::lang::IllegalArgumentException, css::lang::WrappedTargetException, RuntimeException, std::exception)
{
    const PropertyValue *pPropArray = aProps.getConstArray();
    long nPropCount = aProps.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const PropertyValue &rProp = pPropArray[i];
        OUString aPropName = rProp.Name;

        if (aPropName == "Password")
            rProp.Value >>= msPassword;
        else if (aPropName == "InputStream")
            rProp.Value >>= mxInputStream;
    }
}


// XServiceInfo
OUString SAL_CALL WordPerfectImportFilterDialog::getImplementationName()
throw (RuntimeException, std::exception)
{
    return WordPerfectImportFilterDialog_getImplementationName();
}

sal_Bool SAL_CALL WordPerfectImportFilterDialog::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL WordPerfectImportFilterDialog::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return WordPerfectImportFilterDialog_getSupportedServiceNames();
}

OUString WordPerfectImportFilterDialog_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Writer.WordPerfectImportFilterDialog");
}

Sequence< OUString > SAL_CALL WordPerfectImportFilterDialog_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet { "com.sun.star.ui.dialogs.FilterOptionsDialog" };
    return aRet;
}

Reference< XInterface > SAL_CALL WordPerfectImportFilterDialog_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new WordPerfectImportFilterDialog(rContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
