/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef XMLSCRIPT_XMLBAS_EXPORT_HXX
#define XMLSCRIPT_XMLBAS_EXPORT_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase4.hxx>
#include <osl/mutex.hxx>


//.........................................................................
namespace xmlscript
{
//.........................................................................

    // =============================================================================
    // class XMLBasicExporterBase
    // =============================================================================

    typedef ::cppu::WeakImplHelper4<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::document::XExporter,
        ::com::sun::star::document::XFilter > XMLBasicExporterBase_BASE;

    class XMLBasicExporterBase : public XMLBasicExporterBase_BASE
    {
    private:
        ::osl::Mutex                                                                        m_aMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
        sal_Bool                                                                            m_bOasis;

    public:
        XMLBasicExporterBase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, sal_Bool bOasis );
        virtual ~XMLBasicExporterBase();

        // XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XExporter
        virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxDoc )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XFilter
        virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
            throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancel()
            throw (::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class XMLBasicExporter
    // =============================================================================

    class XMLBasicExporter : public XMLBasicExporterBase
    {
    public:
        XMLBasicExporter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~XMLBasicExporter();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class XMLOasisBasicExporter
    // =============================================================================

    class XMLOasisBasicExporter : public XMLBasicExporterBase
    {
    public:
        XMLOasisBasicExporter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~XMLOasisBasicExporter();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace xmlscript
//.........................................................................

#endif // XMLSCRIPT_XMLBAS_EXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
