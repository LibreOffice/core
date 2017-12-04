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
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
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

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XExtendedFilterDetection;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::XDocumentHandler;

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

bool WordPerfectImportFilter::importImpl(const Sequence< css::beans::PropertyValue > &aDescriptor)
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
            if (unsuccessfulAttempts == 3) // timeout after 3 password attempts
                return false;
        }
    }

    // An XML import service: what we push sax messages to.
    Reference < XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.comp.Writer.XMLOasisImporter", mxContext),
        css::uno::UNO_QUERY_THROW);

    // The XImporter sets up an empty target document for XDocumentHandler to write to.
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    // OO Document Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler aHandler(xInternalHandler);

    OdtGenerator collector;
    collector.addDocumentHandler(&aHandler, ODF_FLAT_XML);
    collector.registerEmbeddedObjectHandler("image/x-wpg", &handleEmbeddedWPGObject);
    collector.registerEmbeddedImageHandler("image/x-wpg", &handleEmbeddedWPGImage);
    return libwpd::WPD_OK == libwpd::WPDocument::parse(&input, &collector, aUtf8Passwd.isEmpty() ? nullptr : aUtf8Passwd.getStr());
}

sal_Bool SAL_CALL WordPerfectImportFilter::filter(const Sequence< css::beans::PropertyValue > &aDescriptor)
{
    return importImpl(aDescriptor);
}
void SAL_CALL WordPerfectImportFilter::cancel()
{
}

// XImporter
void SAL_CALL WordPerfectImportFilter::setTargetDocument(const Reference< css::lang::XComponent > &xDoc)
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL WordPerfectImportFilter::detect(Sequence< PropertyValue > &Descriptor)
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
void SAL_CALL WordPerfectImportFilter::initialize(const Sequence< Any > &/*aArguments*/)
{
}

// XServiceInfo
OUString SAL_CALL WordPerfectImportFilter::getImplementationName()
{
    return OUString("com.sun.star.comp.Writer.WordPerfectImportFilter");
}

sal_Bool SAL_CALL WordPerfectImportFilter::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL WordPerfectImportFilter::getSupportedServiceNames()
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}


extern "C"
SAL_DLLPUBLIC_EXPORT css::uno::XInterface *SAL_CALL
com_sun_star_comp_Writer_WordPerfectImportFilter_get_implementation(
    css::uno::XComponentContext *const context,
    const css::uno::Sequence<css::uno::Any> &)
{
    return cppu::acquire(new WordPerfectImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
