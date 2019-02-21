/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_IMPORTFILTER_HXX
#define INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_IMPORTFILTER_HXX

#include <libodfgen/libodfgen.hxx>

#include <librevenge/librevenge.h>

#include <librevenge-stream/librevenge-stream.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>

#include <unotools/mediadescriptor.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include "DocumentHandler.hxx"
#include "WPXSvInputStream.hxx"

#include <xmloff/attrlist.hxx>

#include "DocumentHandlerFor.hxx"

namespace writerperfect
{
namespace detail
{
template <class Generator>
class ImportFilterImpl
    : public cppu::WeakImplHelper<css::document::XFilter, css::document::XImporter,
                                  css::document::XExtendedFilterDetection,
                                  css::lang::XInitialization>
{
public:
    ImportFilterImpl(const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        : mxContext(rxContext)
    {
    }

    const css::uno::Reference<css::uno::XComponentContext>& getXContext() const
    {
        return mxContext;
    }

    // XFilter
    virtual sal_Bool SAL_CALL
    filter(const css::uno::Sequence<css::beans::PropertyValue>& rDescriptor) override
    {
        utl::MediaDescriptor aDescriptor(rDescriptor);
        css::uno::Reference<css::io::XInputStream> xInputStream;
        aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
        if (!xInputStream.is())
        {
            OSL_ASSERT(false);
            return false;
        }

        css::uno::Reference<css::awt::XWindow> xDialogParent;
        aDescriptor["ParentWindow"] >>= xDialogParent;

        // An XML import service: what we push sax messages to..
        css::uno::Reference<css::xml::sax::XDocumentHandler> xInternalHandler(
            mxContext->getServiceManager()->createInstanceWithContext(
                DocumentHandlerFor<Generator>::name(), mxContext),
            css::uno::UNO_QUERY_THROW);

        // The XImporter sets up an empty target document for XDocumentHandler to write to..
        css::uno::Reference<css::document::XImporter> xImporter(xInternalHandler,
                                                                css::uno::UNO_QUERY);
        xImporter->setTargetDocument(mxDoc);

        // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
        // writes to in-memory target doc
        DocumentHandler aHandler(xInternalHandler);

        WPXSvInputStream input(xInputStream);

        Generator exporter;
        exporter.addDocumentHandler(&aHandler, ODF_FLAT_XML);

        doRegisterHandlers(exporter);

        return doImportDocument(Application::GetFrameWeld(xDialogParent), input, exporter,
                                aDescriptor);
    }

    virtual void SAL_CALL cancel() override {}

    // XImporter
    const css::uno::Reference<css::lang::XComponent>& getTargetDocument() const { return mxDoc; }
    virtual void SAL_CALL
    setTargetDocument(const css::uno::Reference<css::lang::XComponent>& xDoc) override
    {
        mxDoc = xDoc;
    }

    //XExtendedFilterDetection
    virtual OUString SAL_CALL
    detect(css::uno::Sequence<css::beans::PropertyValue>& Descriptor) override
    {
        OUString sTypeName;
        sal_Int32 nLength = Descriptor.getLength();
        sal_Int32 location = nLength;
        const css::beans::PropertyValue* pValue = Descriptor.getConstArray();
        css::uno::Reference<css::io::XInputStream> xInputStream;
        for (sal_Int32 i = 0; i < nLength; i++)
        {
            if (pValue[i].Name == "TypeName")
                location = i;
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
                Descriptor.realloc(nLength + 1);
                Descriptor[location].Name = "TypeName";
            }

            Descriptor[location].Value <<= sTypeName;
        }

        return sTypeName;
    }

    // XInitialization
    virtual void SAL_CALL
    initialize(const css::uno::Sequence<css::uno::Any>& /*aArguments*/) override
    {
    }

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName) = 0;
    virtual bool doImportDocument(weld::Window* pParent, librevenge::RVNGInputStream& rInput,
                                  Generator& rGenerator, utl::MediaDescriptor& rDescriptor)
        = 0;
    virtual void doRegisterHandlers(Generator&){};

    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::lang::XComponent> mxDoc;
};
}

/** A base class for import filters.
 */
template <class Generator>
struct ImportFilter : public cppu::ImplInheritanceHelper<detail::ImportFilterImpl<Generator>,
                                                         css::lang::XServiceInfo>
{
    ImportFilter(const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        : cppu::ImplInheritanceHelper<detail::ImportFilterImpl<Generator>, css::lang::XServiceInfo>(
              rxContext)
    {
    }
};
}

#endif // INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_IMPORTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
