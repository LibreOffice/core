/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <boost/shared_ptr.hpp>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include <osl/diagnose.h>

#include <writerperfect/DirectoryStream.hxx>
#include <writerperfect/DocumentHandler.hxx>
#include <writerperfect/WPXSvInputStream.hxx>

#include <xmloff/attrlist.hxx>

#include <libodfgen/libodfgen.hxx>

#include "ImportFilterBase.hxx"

using boost::shared_ptr;

namespace writerperfect
{
namespace presentation
{

using com::sun::star::uno::Reference;
using com::sun::star::io::XInputStream;
using com::sun::star::io::XSeekable;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

using writerperfect::DocumentHandler;
using writerperfect::WPXSvInputStream;

namespace
{

template<class T>
bool lcl_queryIsPackage(const Sequence<T> &lComponentData)
{
    bool bIsPackage = false;

    const sal_Int32 nLength = lComponentData.getLength();
    const T *pValue = lComponentData.getConstArray();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        if (pValue[i].Name == "IsPackage")
        {
            pValue[i].Value >>= bIsPackage;
            break;
        }
    }

    return bIsPackage;
}

bool lcl_isPackage(const Any &rComponentData)
{
    Sequence < ::com::sun::star::beans::NamedValue > lComponentDataNV;
    Sequence < ::com::sun::star::beans::PropertyValue > lComponentDataPV;

    if (rComponentData >>= lComponentDataNV)
        return lcl_queryIsPackage(lComponentDataNV);
    else if (rComponentData >>= lComponentDataPV)
        return lcl_queryIsPackage(lComponentDataPV);

    return false;
}
}

ImportFilterImpl::ImportFilterImpl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext)
    : mxContext(rxContext)
{
}

ImportFilterImpl::~ImportFilterImpl()
{
}

sal_Bool SAL_CALL ImportFilterImpl::filter(const Sequence< ::com::sun::star::beans::PropertyValue > &aDescriptor)
throw (RuntimeException, std::exception)
{
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    Reference < ::com::sun::star::ucb::XContent > xContent;
    bool bIsPackage = false;
    for (sal_Int32 i = 0 ; i < nLength; i++)
    {
        if (pValue[i].Name == "ComponentData")
            bIsPackage = lcl_isPackage(pValue[i].Value);
        else if (pValue[i].Name == "InputStream")
            pValue[i].Value >>= xInputStream;
        else if (pValue[i].Name == "UCBContent")
            pValue[i].Value >>= xContent;
    }

    if (!xInputStream.is())
    {
        OSL_ASSERT(false);
        return sal_False;
    }

    if (bIsPackage && !xContent.is())
    {
        SAL_WARN("writerperfect", "presentation::ImportFilterImpl::filter: the input claims to be a package, but does not have UCBContent");
        bIsPackage = false;
    }

    // An XML import service: what we push sax messages to..
    Reference < XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.comp.Impress.XMLOasisImporter", mxContext),
        css::uno::UNO_QUERY_THROW);

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    shared_ptr< librevenge::RVNGInputStream > input;
    if (bIsPackage)
        input.reset(new writerperfect::DirectoryStream(xContent));
    else
        input.reset(new WPXSvInputStream(xInputStream));

    OdpGenerator exporter;
    exporter.addDocumentHandler(&xHandler, ODF_FLAT_XML);

    doRegisterHandlers(exporter);

    bool result=doImportDocument(*input, exporter);
    return result;
}

void SAL_CALL ImportFilterImpl::cancel()
throw (RuntimeException, std::exception)
{
}

// XImporter
void SAL_CALL ImportFilterImpl::setTargetDocument(const Reference< ::com::sun::star::lang::XComponent > &xDoc)
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL ImportFilterImpl::detect(com::sun::star::uno::Sequence< PropertyValue > &Descriptor)
throw(com::sun::star::uno::RuntimeException, std::exception)
{
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

    if (doDetectFormat(input, sTypeName))
    {
        assert(!sTypeName.isEmpty());

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
void SAL_CALL ImportFilterImpl::initialize(const Sequence< Any > &aArguments)
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

bool ImportFilterImpl::doDetectFormat(librevenge::RVNGInputStream &, OUString &)
{
    SAL_WARN("writerperfect", "presentation::ImportFilterImpl::doDetectFormat must not be called");
    return false;
}
void ImportFilterImpl::doRegisterHandlers(OdpGenerator &)
{
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
