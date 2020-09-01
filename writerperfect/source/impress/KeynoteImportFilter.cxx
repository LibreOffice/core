/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* KeynoteImportFilter: Sets up the filter, and calls OdpExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <libetonyek/libetonyek.h>
#include <ucbhelper/content.hxx>
#include <unotools/ucbhelper.hxx>

#include <DirectoryStream.hxx>
#include <WPXSvInputStream.hxx>

#include "KeynoteImportFilter.hxx"

using writerperfect::WPXSvInputStream;

namespace beans = com::sun::star::beans;
namespace ucb = com::sun::star::ucb;

bool KeynoteImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                           OdpGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libetonyek::EtonyekDocument::parse(&rInput, &rGenerator);
}

bool KeynoteImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libetonyek::EtonyekDocument::isSupported(&rInput))
    {
        rTypeName = "impress_Keynote_Document";
        return true;
    }

    return false;
}

// XExtendedFilterDetection
OUString SAL_CALL
KeynoteImportFilter::detect(css::uno::Sequence<css::beans::PropertyValue>& Descriptor)
{
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 nNewLength = nLength + 2;
    sal_Int32 nComponentDataLocation = -1;
    sal_Int32 nTypeNameLocation = -1;
    sal_Int32 nUCBContentLocation = -1;
    bool bIsPackage = false;
    bool bUCBContentChanged = false;
    const beans::PropertyValue* pValue = Descriptor.getConstArray();
    css::uno::Reference<com::sun::star::io::XInputStream> xInputStream;
    css::uno::Reference<ucb::XContent> xContent;
    css::uno::Sequence<beans::NamedValue> lComponentDataNV;
    css::uno::Sequence<beans::PropertyValue> lComponentDataPV;
    bool bComponentDataNV = true;

    for (sal_Int32 i = 0; i < nLength; i++)
    {
        if (pValue[i].Name == "TypeName")
        {
            nTypeNameLocation = i;
            --nNewLength;
        }
        if (pValue[i].Name == "ComponentData")
        {
            bComponentDataNV = pValue[i].Value >>= lComponentDataNV;
            if (!bComponentDataNV)
                pValue[i].Value >>= lComponentDataPV;
            nComponentDataLocation = i;
            --nNewLength;
        }
        else if (pValue[i].Name == "InputStream")
        {
            pValue[i].Value >>= xInputStream;
        }
        else if (pValue[i].Name == "UCBContent")
        {
            pValue[i].Value >>= xContent;
            nUCBContentLocation = i;
        }
    }

    assert(nNewLength >= nLength);

    if (!xInputStream.is())
        return OUString();

    std::unique_ptr<librevenge::RVNGInputStream> input
        = std::make_unique<WPXSvInputStream>(xInputStream);

    /* Apple Keynote documents come in two variants:
     * * actual files (zip), only produced by Keynote 5 (at least with
     *   default settings)
     * * packages (IOW, directories), produced by Keynote 1-4 and again
     *   starting with 6.
     * But since the libetonyek import only works with a stream, we need
     * to pass it one for the whole package. Here we determine if that
     * is needed.
     *
     * Note: for convenience, we also recognize that the main XML file
     * from a package was passed and pass the whole package to the
     * filter instead.
     */
    if (xContent.is())
    {
        ucbhelper::Content aContent(xContent, utl::UCBContentHelper::getDefaultCommandEnvironment(),
                                    comphelper::getProcessComponentContext());
        try
        {
            if (aContent.isFolder())
            {
                input = std::make_unique<writerperfect::DirectoryStream>(xContent);
                bIsPackage = true;
            }
        }
        catch (...)
        {
            return OUString();
        }
    }

    libetonyek::EtonyekDocument::Type type = libetonyek::EtonyekDocument::TYPE_UNKNOWN;
    const libetonyek::EtonyekDocument::Confidence confidence
        = libetonyek::EtonyekDocument::isSupported(input.get(), &type);
    if ((libetonyek::EtonyekDocument::CONFIDENCE_NONE == confidence)
        || (libetonyek::EtonyekDocument::TYPE_KEYNOTE != type))
        return OUString();

    if (confidence == libetonyek::EtonyekDocument::CONFIDENCE_SUPPORTED_PART)
    {
        if (bIsPackage) // we passed a directory stream, but the filter claims it's APXL file?
            return OUString();

        std::unique_ptr<writerperfect::DirectoryStream> xDir
            = writerperfect::DirectoryStream::createForParent(xContent);
        auto pDir = xDir.get();
        input = std::move(xDir);
        if (bool(input))
        {
            if (libetonyek::EtonyekDocument::CONFIDENCE_EXCELLENT
                == libetonyek::EtonyekDocument::isSupported(input.get()))
            {
                xContent = pDir->getContent();
                bUCBContentChanged = true;
                bIsPackage = true;
            }
            else
            {
                // The passed stream has been detected as APXL file, but its parent dir is not a valid Keynote
                // package? Something is wrong here...
                return OUString();
            }
        }
    }

    // we do not need to insert ComponentData if this is not a package
    if (!bIsPackage && (nComponentDataLocation == -1))
        --nNewLength;

    if (nNewLength > nLength)
        Descriptor.realloc(nNewLength);

    if (nTypeNameLocation == -1)
    {
        assert(nLength < nNewLength);
        nTypeNameLocation = nLength++;
        Descriptor[nTypeNameLocation].Name = "TypeName";
    }

    if (bIsPackage && (nComponentDataLocation == -1))
    {
        assert(nLength < nNewLength);
        nComponentDataLocation = nLength++;
        Descriptor[nComponentDataLocation].Name = "ComponentData";
    }

    if (bIsPackage)
    {
        if (bComponentDataNV)
        {
            const sal_Int32 nCDSize = lComponentDataNV.getLength();
            lComponentDataNV.realloc(nCDSize + 1);
            beans::NamedValue aValue;
            aValue.Name = "IsPackage";
            aValue.Value <<= true;
            lComponentDataNV[nCDSize] = aValue;
            Descriptor[nComponentDataLocation].Value <<= lComponentDataNV;
        }
        else
        {
            const sal_Int32 nCDSize = lComponentDataPV.getLength();
            lComponentDataPV.realloc(nCDSize + 1);
            beans::PropertyValue aProp;
            aProp.Name = "IsPackage";
            aProp.Value <<= true;
            aProp.Handle = -1;
            aProp.State = beans::PropertyState_DIRECT_VALUE;
            lComponentDataPV[nCDSize] = aProp;
            Descriptor[nComponentDataLocation].Value <<= lComponentDataPV;
        }
    }

    if (bUCBContentChanged)
        Descriptor[nUCBContentLocation].Value <<= xContent;

    const OUString sTypeName("impress_AppleKeynote");
    Descriptor[nTypeNameLocation].Value <<= sTypeName;

    return sTypeName;
}

// XServiceInfo
OUString SAL_CALL KeynoteImportFilter::getImplementationName()
{
    return "org.libreoffice.comp.Impress.KeynoteImportFilter";
}

sal_Bool SAL_CALL KeynoteImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL KeynoteImportFilter::getSupportedServiceNames()
{
    return { "com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Impress_KeynoteImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new KeynoteImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
