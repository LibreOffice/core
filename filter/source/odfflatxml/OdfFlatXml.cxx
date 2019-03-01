/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <sax/tools/documenthandleradapter.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <tools/diagnose_ex.h>

using namespace ::cppu;
using namespace ::osl;
using namespace ::sax;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;

namespace filter {
    namespace odfflatxml {
        /*
         * OdfFlatXml export and imports ODF flat XML documents by plugging a pass-through
         * filter implementation into XmlFilterAdaptor.
         */
        class OdfFlatXml : public WeakImplHelper<XImportFilter,
                                                  XExportFilter, DocumentHandlerAdapter, css::lang::XServiceInfo>
        {
        private:
            Reference< XComponentContext > m_xContext;

        public:

            explicit OdfFlatXml(const Reference<XComponentContext> &r) :
                m_xContext(r)
            {
            }

            // XImportFilter
            virtual sal_Bool SAL_CALL
            importer(const Sequence< PropertyValue >& sourceData,
                     const Reference< XDocumentHandler >& docHandler,
                     const Sequence< OUString >& userData) override;

            // XExportFilter
            virtual sal_Bool SAL_CALL
            exporter(
                     const Sequence< PropertyValue >& sourceData,
                     const Sequence< OUString >& userData) override;

            OUString SAL_CALL getImplementationName() override
            { return OUString("com.sun.star.comp.filter.OdfFlatXml"); }

            sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
            { return cppu::supportsService(this, ServiceName); }

            css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
            {
                return css::uno::Sequence<OUString>{
                    "com.sun.star.document.ImportFilter",
                        "com.sun.star.document.ExportFilter"};
            }

            // UNO component helper methods

            static OUString impl_getImplementationName();

            static Sequence< OUString > impl_getSupportedServiceNames();

            static Reference< XInterface > SAL_CALL impl_createInstance(const Reference< XMultiServiceFactory >& fact);
        };
    }
}

using namespace ::filter::odfflatxml;

sal_Bool
OdfFlatXml::importer(
                     const Sequence< PropertyValue >& sourceData,
                     const Reference< XDocumentHandler >& docHandler,
                     const Sequence< OUString >& /* userData */)
{
    // Read InputStream to read from and an URL used for the system id
    // of the InputSource we create from the given sourceData sequence
    Reference<XInputStream> inputStream;
    OUString paramName;
    OUString url;

    sal_Int32 paramCount = sourceData.getLength();
    for (sal_Int32 paramIdx = 0; paramIdx < paramCount; paramIdx++)
        {
            paramName = sourceData[paramIdx].Name;
            if ( paramName == "InputStream" )
                sourceData[paramIdx].Value >>= inputStream;
            else if ( paramName == "URL" )
                sourceData[paramIdx].Value >>= url;
        }

    OSL_ASSERT(inputStream.is());
    if (!inputStream.is())
        return false;

    Reference<XParser> saxParser = Parser::create(m_xContext);

    InputSource inputSource;
    inputSource.sSystemId = url;
    inputSource.sPublicId = url;
    inputSource.aInputStream = inputStream;
    css::uno::Reference< css::xml::sax::XFastParser > xFastParser = dynamic_cast<
                            css::xml::sax::XFastParser* >( docHandler.get() );
    saxParser->setDocumentHandler(docHandler);
    try
    {
        css::uno::Reference< css::io::XSeekable > xSeekable( inputStream, css::uno::UNO_QUERY );
        if ( xSeekable.is() )
            xSeekable->seek( 0 );

        if( xFastParser.is() )
            xFastParser->parseStream( inputSource );
        else
            saxParser->parseStream(inputSource);
    }
    catch (const Exception &)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("filter.odfflatxml", exceptionToString(ex));
        return false;
    }
    catch (const std::exception &exc)
    {
        SAL_WARN("filter.odfflatxml", exc.what());
        return false;
    }
    return true;
}

sal_Bool
OdfFlatXml::exporter(const Sequence< PropertyValue >& sourceData,
                     const Sequence< OUString >& /*msUserData*/)
{
    OUString paramName;
    Reference<XOutputStream> outputStream;

    // Read output stream and target URL from the parameters given in sourceData.
    sal_Int32 paramCount = sourceData.getLength();
    for (sal_Int32 paramIdx = 0; paramIdx < paramCount; paramIdx++)
        {
            paramName = sourceData[paramIdx].Name;
            if ( paramName == "OutputStream" )
                sourceData[paramIdx].Value >>= outputStream;
        }

    if (!getDelegate().is())
        {
            Reference< XDocumentHandler > saxWriter = Writer::create(m_xContext);
            setDelegate(saxWriter);
        }
    // get data source interface ...
    Reference<XActiveDataSource> dataSource(getDelegate(), UNO_QUERY);
    OSL_ASSERT(dataSource.is());
    if (!dataSource.is())
        return false;
    OSL_ASSERT(outputStream.is());
    if (!outputStream.is())
        return false;
    dataSource->setOutputStream(outputStream);

    return true;
}


OUString OdfFlatXml::impl_getImplementationName()
{
    return OUString("com.sun.star.comp.filter.OdfFlatXml");
}

Sequence< OUString > OdfFlatXml::impl_getSupportedServiceNames()
{
    Sequence< OUString > lServiceNames(2);
    lServiceNames[0] = "com.sun.star.document.ImportFilter";
    lServiceNames[1] = "com.sun.star.document.ExportFilter";
    return lServiceNames;
}

Reference< XInterface > SAL_CALL OdfFlatXml::impl_createInstance(const Reference< XMultiServiceFactory >& fact)
{
    return Reference<XInterface> (static_cast<OWeakObject *>(new OdfFlatXml( comphelper::getComponentContext(fact) )));

}

extern "C" SAL_DLLPUBLIC_EXPORT void*
odfflatxml_component_getFactory( const sal_Char* pImplementationName,
                      void* pServiceManager,
                      void* /* pRegistryKey */ )
{
    if ((!pImplementationName) || (!pServiceManager))
        return nullptr;

    css::uno::Reference< css::lang::XMultiServiceFactory >
        xSMGR = static_cast< css::lang::XMultiServiceFactory* >(pServiceManager);
    css::uno::Reference< css::lang::XSingleServiceFactory > xFactory;
    OUString sImplName = OUString::createFromAscii(pImplementationName);

    if (OdfFlatXml::impl_getImplementationName() == sImplName)
        xFactory = cppu::createOneInstanceFactory( xSMGR,
                                                   OdfFlatXml::impl_getImplementationName(),
                                                   OdfFlatXml::impl_createInstance,
                                                   OdfFlatXml::impl_getSupportedServiceNames() );

    if (xFactory.is())
    {
        xFactory->acquire();
        return xFactory.get();
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
