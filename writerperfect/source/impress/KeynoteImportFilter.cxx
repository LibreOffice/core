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
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <iostream>
#include <libetonyek/libetonyek.h>
#include <libodfgen/libodfgen.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include <ucbhelper/content.hxx>

#include <DirectoryStream.hxx>
#include <DocumentHandler.hxx>
#include <WPXSvInputStream.hxx>

#include <xmloff/attrlist.hxx>

#include "KeynoteImportFilter.hxx"

using std::shared_ptr;

using com::sun::star::io::XInputStream;
using com::sun::star::uno::Any;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

using writerperfect::DocumentHandler;
using writerperfect::WPXSvInputStream;

namespace beans = com::sun::star::beans;
namespace container = com::sun::star::container;
namespace ucb = com::sun::star::ucb;

bool KeynoteImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdpGenerator &rGenerator, utl::MediaDescriptor &)
{
    return libetonyek::EtonyekDocument::parse(&rInput, &rGenerator);
}

bool KeynoteImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    if (libetonyek::EtonyekDocument::isSupported(&rInput))
    {
        rTypeName = "impress_Keynote_Document";
        return true;
    }

    return false;
}

// XExtendedFilterDetection
OUString SAL_CALL KeynoteImportFilter::detect(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &Descriptor)
throw(com::sun::star::uno::RuntimeException, std::exception)
{
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 nNewLength = nLength + 2;
    sal_Int32 nComponentDataLocation = -1;
    sal_Int32 nTypeNameLocation = -1;
    sal_Int32 nUCBContentLocation = -1;
    bool bIsPackage = false;
    bool bUCBContentChanged = false;
    const beans::PropertyValue *pValue = Descriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    Reference < ucb::XContent > xContent;
    Sequence < beans::NamedValue > lComponentDataNV;
    Sequence < beans::PropertyValue > lComponentDataPV;
    bool bComponentDataNV = true;

    for (sal_Int32 i = 0 ; i < nLength; i++)
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

    shared_ptr< librevenge::RVNGInputStream > input(new WPXSvInputStream(xInputStream));

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
        ucbhelper::Content aContent(xContent, Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext());
        try
        {
            if (aContent.isFolder())
            {
                input.reset(new writerperfect::DirectoryStream(xContent));
                bIsPackage = true;
            }
        }
        catch (...)
        {
            return OUString();
        }
    }

    libetonyek::EtonyekDocument::Type type = libetonyek::EtonyekDocument::TYPE_UNKNOWN;
    const libetonyek::EtonyekDocument::Confidence confidence = libetonyek::EtonyekDocument::isSupported(input.get(), &type);
    if ((libetonyek::EtonyekDocument::CONFIDENCE_NONE == confidence) || (libetonyek::EtonyekDocument::TYPE_KEYNOTE != type))
        return OUString();

    if (confidence == libetonyek::EtonyekDocument::CONFIDENCE_SUPPORTED_PART)
    {
        if (bIsPackage)   // we passed a directory stream, but the filter claims it's APXL file?
            return OUString();

        const Reference < container::XChild > xChild(xContent, UNO_QUERY);
        if (xChild.is())
        {
            const Reference < ucb::XContent > xPackageContent(xChild->getParent(), UNO_QUERY);
            if (xPackageContent.is())
            {
                input.reset(new writerperfect::DirectoryStream(xPackageContent));
                if (libetonyek::EtonyekDocument::CONFIDENCE_EXCELLENT == libetonyek::EtonyekDocument::isSupported(input.get()))
                {
                    xContent = xPackageContent;
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
            aValue.Value = comphelper::makeBoolAny(true);
            lComponentDataNV[nCDSize] = aValue;
            Descriptor[nComponentDataLocation].Value <<= lComponentDataNV;
        }
        else
        {
            const sal_Int32 nCDSize = lComponentDataPV.getLength();
            lComponentDataPV.realloc(nCDSize + 1);
            beans::PropertyValue aProp;
            aProp.Name = "IsPackage";
            aProp.Value = comphelper::makeBoolAny(true);
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

OUString KeynoteImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("org.libreoffice.comp.Impress.KeynoteImportFilter");
}

Sequence< OUString > SAL_CALL KeynoteImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL KeynoteImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new KeynoteImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL KeynoteImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return KeynoteImportFilter_getImplementationName();
}

sal_Bool SAL_CALL KeynoteImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL KeynoteImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return KeynoteImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
