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

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/supportsservice.hxx>

#include <DocumentHandler.hxx>
#include <WPXSvInputStream.hxx>

#include <sfx2/passwd.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/svapp.hxx>
#include <xmloff/xmlimp.hxx>

#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

#include "WordPerfectImportFilter.hxx"

using com::sun::star::uno::Reference;

using com::sun::star::awt::XWindow;
using com::sun::star::document::XImporter;
using com::sun::star::io::XInputStream;
using com::sun::star::xml::sax::XFastDocumentHandler;

using writerperfect::DocumentHandler;
using writerperfect::WPXSvInputStream;

static bool handleEmbeddedWPGObject(const librevenge::RVNGBinaryData& data,
                                    OdfDocumentHandler* pHandler, const OdfStreamType streamType)
{
    OdgGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);

    libwpg::WPGFileFormat fileFormat = libwpg::WPG_AUTODETECT;

    if (!libwpg::WPGraphics::isSupported(data.getDataStream()))
        fileFormat = libwpg::WPG_WPG1;

    return libwpg::WPGraphics::parse(data.getDataStream(), &exporter, fileFormat);
}

static bool handleEmbeddedWPGImage(const librevenge::RVNGBinaryData& input,
                                   librevenge::RVNGBinaryData& output)
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
    output.append(reinterpret_cast<const unsigned char*>(svgOutput[0].cstr()), svgOutput[0].size());
    return true;
}

bool WordPerfectImportFilter::importImpl(
    const css::uno::Sequence<css::beans::PropertyValue>& aDescriptor)
{
    Reference<XInputStream> xInputStream;
    Reference<XWindow> xDialogParent;
    for (const auto& rValue : aDescriptor)
    {
        if (rValue.Name == "InputStream")
            rValue.Value >>= xInputStream;
        else if (rValue.Name == "ParentWindow")
            rValue.Value >>= xDialogParent;
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
            SfxPasswordDialog aPasswdDlg(Application::GetFrameWeld(xDialogParent));
            aPasswdDlg.SetMinLen(0);
            if (!aPasswdDlg.run())
                return false;
            OUString aPasswd = aPasswdDlg.GetPassword();
            aUtf8Passwd = OUStringToOString(aPasswd, RTL_TEXTENCODING_UTF8);
            if (libwpd::WPD_PASSWORD_MATCH_OK
                == libwpd::WPDocument::verifyPassword(&input, aUtf8Passwd.getStr()))
                break;
            else
                unsuccessfulAttempts++;
            if (unsuccessfulAttempts == 3) // timeout after 3 password attempts
                return false;
        }
    }

    // An XML import service: what we push sax messages to.
    Reference<XInterface> xInternalFilter
        = mxContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.comp.Writer.XMLOasisImporter"_ustr, mxContext);
    assert(xInternalFilter);
    css::uno::Reference<css::xml::sax::XFastDocumentHandler> xInternalHandler(xInternalFilter,
                                                                              css::uno::UNO_QUERY);
    assert(xInternalHandler);

    // The XImporter sets up an empty target document for XDocumentHandler to write to.
    Reference<XImporter> xImporter(xInternalHandler, css::uno::UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    // OO Document Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler aHandler(
        new SvXMLLegacyToFastDocHandler(static_cast<SvXMLImport*>(xInternalHandler.get())));

    OdtGenerator collector;
    collector.addDocumentHandler(&aHandler, ODF_FLAT_XML);
    collector.registerEmbeddedObjectHandler("image/x-wpg", &handleEmbeddedWPGObject);
    collector.registerEmbeddedImageHandler("image/x-wpg", &handleEmbeddedWPGImage);
    return libwpd::WPD_OK
           == libwpd::WPDocument::parse(&input, &collector,
                                        aUtf8Passwd.isEmpty() ? nullptr : aUtf8Passwd.getStr());
}

sal_Bool SAL_CALL
WordPerfectImportFilter::filter(const css::uno::Sequence<css::beans::PropertyValue>& aDescriptor)
{
    return importImpl(aDescriptor);
}
void SAL_CALL WordPerfectImportFilter::cancel() {}

// XImporter
void SAL_CALL
WordPerfectImportFilter::setTargetDocument(const Reference<css::lang::XComponent>& xDoc)
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL
WordPerfectImportFilter::detect(css::uno::Sequence<css::beans::PropertyValue>& Descriptor)
{
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 location = nLength;
    Reference<XInputStream> xInputStream;
    for (sal_Int32 i = 0; i < nLength; i++)
    {
        if (Descriptor[i].Name == "TypeName")
            location = i;
        else if (Descriptor[i].Name == "InputStream")
            Descriptor[i].Value >>= xInputStream;
    }

    if (!xInputStream.is())
        return OUString();

    WPXSvInputStream input(xInputStream);

    OUString sTypeName;
    libwpd::WPDConfidence confidence = libwpd::WPDocument::isFileFormatSupported(&input);
    if (confidence == libwpd::WPD_CONFIDENCE_EXCELLENT
        || confidence == libwpd::WPD_CONFIDENCE_SUPPORTED_ENCRYPTION)
    {
        if (location == nLength)
        {
            Descriptor.realloc(nLength + 1);
            Descriptor.getArray()[location].Name = "TypeName";
        }

        sTypeName = "writer_WordPerfect_Document";
        Descriptor.getArray()[location].Value <<= sTypeName;
    }

    return sTypeName;
}

// XInitialization
void SAL_CALL
WordPerfectImportFilter::initialize(const css::uno::Sequence<css::uno::Any>& /*aArguments*/)
{
}

// XServiceInfo
OUString SAL_CALL WordPerfectImportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Writer.WordPerfectImportFilter"_ustr;
}

sal_Bool SAL_CALL WordPerfectImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL WordPerfectImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_WordPerfectImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new WordPerfectImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
