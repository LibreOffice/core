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
        ::com::sun::star::document::XFilter,
        ::com::sun::star::document::XExporter,
        ::com::sun::star::document::XImporter,
        ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit XMLFilter( ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext > const & xContext );
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
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XImporter ____
    virtual void SAL_CALL setTargetDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XExporter ____
    virtual void SAL_CALL setSourceDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

    inline void setDocumentHandler(const OUString& _sDocumentHandler) { m_sDocumentHandler = _sDocumentHandler; }

    virtual OUString getMediaType(bool _bOasis);

    /** fills the oasis flag only when a filtername was set
    *
    * \param _rMediaDescriptor
    * \param _rOutOASIS
    */
    virtual void isOasisFormat(const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& _rMediaDescriptor, bool & _rOutOASIS );

private:
    // methods

    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_Import( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::lang::XComponent > & xDocumentComp,
                           const ::com::sun::star::uno::Sequence<
                               ::com::sun::star::beans::PropertyValue > & aMediaDescriptor );
    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_ImportStream(
        const OUString & rStreamName,
        const OUString & rServiceName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage > & xStorage,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XParser > & xParser,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiComponentFactory > & xFactory,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XGraphicObjectResolver > & xGraphicObjectResolver,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xPropSet );

    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_Export( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::lang::XComponent > & xDocumentComp,
                           const ::com::sun::star::uno::Sequence<
                               ::com::sun::star::beans::PropertyValue > & aMediaDescriptor );
    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_ExportStream(
        const OUString & rStreamName,
        const OUString & rServiceName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage > & xStorage,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XWriter >& xActiveDataSource,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xFactory,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any > & rFilterProperties );

    // members
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XComponent >       m_xTargetDoc;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XComponent >       m_xSourceDoc;

    css::uno::Sequence<css::beans::PropertyValue> m_aMediaDescriptor;

    OUString                                m_sDocumentHandler; // when set it will be set as doc handler

    volatile bool                                  m_bCancelOperation;
    ::osl::Mutex                                   m_aMutex;
};

class XMLReportFilterHelper : public XMLFilter
{
    virtual void isOasisFormat(const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& _rMediaDescriptor, bool & _rOutOASIS ) override;
public:
    explicit XMLReportFilterHelper( ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext > const & _xContext )
                            :XMLFilter(_xContext)
    {}
    static OUString getImplementationName_Static()
    {
        return OUString( "com.sun.star.comp.chart2.report.XMLFilter" );
    }
protected:
    virtual OUString SAL_CALL
        getImplementationName()
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override
    {
        return getImplementationName_Static();
    }
    // ____ XImporter ____
    virtual void SAL_CALL setTargetDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override
    {
        setDocumentHandler( "com.sun.star.comp.report.ImportDocumentHandler" );
        XMLFilter::setTargetDocument(Document);
    }

    // ____ XExporter ____
    virtual void SAL_CALL setSourceDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override
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
