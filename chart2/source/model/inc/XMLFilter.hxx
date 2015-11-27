/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CHART2_SOURCE_MODEL_INC_XMLFILTER_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_INC_XMLFILTER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>
#include <osl/mutex.hxx>


namespace com { namespace sun { namespace star {
namespace embed
{
    class XStorage;
}
namespace xml { namespace sax
{
    class XParser;
}}
namespace document
{
    class XGraphicObjectResolver;
}
}}}

namespace chart
{

class XMLFilter : public
    ::cppu::WeakImplHelper<
        css::document::XFilter,
        css::document::XExporter,
        css::document::XImporter,
        css::lang::XServiceInfo >
{
public:
    explicit XMLFilter( css::uno::Reference< css::uno::XComponentContext > const & xContext );
    virtual ~XMLFilter();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

protected:
    // ____ XFilter ____
    virtual sal_Bool SAL_CALL filter(
        const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XImporter ____
    virtual void SAL_CALL setTargetDocument(
        const css::uno::Reference< css::lang::XComponent >& Document )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XExporter ____
    virtual void SAL_CALL setSourceDocument(
        const css::uno::Reference< css::lang::XComponent >& Document )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;

    inline void setDocumentHandler(const OUString& _sDocumentHandler) { m_sDocumentHandler = _sDocumentHandler; }

    virtual OUString getMediaType(bool _bOasis);

    /** fills the oasis flag only when a filtername was set
    *
    * \param _rMediaDescriptor
    * \param _rOutOASIS
    */
    virtual void isOasisFormat(const css::uno::Sequence< css::beans::PropertyValue >& _rMediaDescriptor, bool & _rOutOASIS );

private:
    // methods

    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_Import( const css::uno::Reference< css::lang::XComponent > & xDocumentComp,
                           const css::uno::Sequence< css::beans::PropertyValue > & aMediaDescriptor );
    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_ImportStream(
        const OUString & rStreamName,
        const OUString & rServiceName,
        const css::uno::Reference< css::embed::XStorage > & xStorage,
        const css::uno::Reference< css::xml::sax::XParser > & xParser,
        const css::uno::Reference< css::lang::XMultiComponentFactory > & xFactory,
        const css::uno::Reference< css::document::XGraphicObjectResolver > & xGraphicObjectResolver,
        css::uno::Reference< css::beans::XPropertySet >& xPropSet );

    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_Export( const css::uno::Reference< css::lang::XComponent > & xDocumentComp,
                           const css::uno::Sequence< css::beans::PropertyValue > & aMediaDescriptor );
    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_ExportStream(
        const OUString & rStreamName,
        const OUString & rServiceName,
        const css::uno::Reference< css::embed::XStorage > & xStorage,
        const css::uno::Reference< css::xml::sax::XWriter >& xActiveDataSource,
        const css::uno::Reference< css::lang::XMultiServiceFactory > & xFactory,
        const css::uno::Sequence< css::uno::Any > & rFilterProperties );

    // members
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XComponent >       m_xTargetDoc;
    css::uno::Reference< css::lang::XComponent >       m_xSourceDoc;

    css::uno::Sequence<css::beans::PropertyValue> m_aMediaDescriptor;

    OUString                                m_sDocumentHandler; // when set it will be set as doc handler

    volatile bool                                  m_bCancelOperation;
    ::osl::Mutex                                   m_aMutex;
};

class XMLReportFilterHelper : public XMLFilter
{
    virtual void isOasisFormat(const css::uno::Sequence< css::beans::PropertyValue >& _rMediaDescriptor,
                               bool & _rOutOASIS ) override;
public:
    explicit XMLReportFilterHelper( css::uno::Reference< css::uno::XComponentContext > const & _xContext )
                            :XMLFilter(_xContext)
    {}
    static OUString getImplementationName_Static()
    {
        return OUString( "com.sun.star.comp.chart2.report.XMLFilter" );
    }
protected:
    virtual OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override
    {
        return getImplementationName_Static();
    }
    // ____ XImporter ____
    virtual void SAL_CALL setTargetDocument(
        const css::uno::Reference< css::lang::XComponent >& Document )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override
    {
        setDocumentHandler( "com.sun.star.comp.report.ImportDocumentHandler" );
        XMLFilter::setTargetDocument(Document);
    }

    // ____ XExporter ____
    virtual void SAL_CALL setSourceDocument(
        const css::uno::Reference< css::lang::XComponent >& Document )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override
    {
        setDocumentHandler( "com.sun.star.comp.report.ExportDocumentHandler" );
        XMLFilter::setSourceDocument(Document);
    }

    virtual OUString getMediaType(bool _bOasis) override;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_INC_XMLFILTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
