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

#ifndef INCLUDED_PDFIADAPTOR_HXX
#define INCLUDED_PDFIADAPTOR_HXX

#include "xmlemitter.hxx"
#include "treevisitorfactory.hxx"

#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>


namespace pdfi
{
    typedef ::cppu::WeakComponentImplHelper2<
        com::sun::star::document::XFilter,
        com::sun::star::document::XImporter > PDFIHybridAdaptorBase;

    class PDFIHybridAdaptor : private cppu::BaseMutex,
                              public PDFIHybridAdaptorBase
    {
    private:
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext >  m_xContext;
        com::sun::star::uno::Reference<
            com::sun::star::frame::XModel >           m_xModel;

    public:
        explicit PDFIHybridAdaptor( const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::uno::XComponentContext >& xContext );

        // XFilter
        virtual sal_Bool SAL_CALL filter( const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rFilterData ) throw(com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancel() throw();

        // XImporter
        virtual void SAL_CALL setTargetDocument( const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xDocument )
            throw( com::sun::star::lang::IllegalArgumentException );

    };

    typedef ::cppu::WeakComponentImplHelper2<
        com::sun::star::xml::XImportFilter,
        com::sun::star::document::XImporter > PDFIAdaptorBase;

    /** Adapts raw pdf import to XImportFilter interface
     */
    class PDFIRawAdaptor : private cppu::BaseMutex,
                           public PDFIAdaptorBase
    {
    private:
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext >  m_xContext;
        com::sun::star::uno::Reference<
            com::sun::star::frame::XModel >           m_xModel;
        TreeVisitorFactorySharedPtr                   m_pVisitorFactory;
        bool                                          m_bEnableToplevelText;

        bool parse( const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>&       xInput,
                    const com::sun::star::uno::Reference<com::sun::star::task::XInteractionHandler>& xIHdl,
                    const rtl::OUString&                                                          rPwd,
                    const com::sun::star::uno::Reference<com::sun::star::task::XStatusIndicator>& xStatus,
                    const XmlEmitterSharedPtr&                                                    rEmitter,
                    const rtl::OUString&                                                          rURL );

    public:
        explicit PDFIRawAdaptor( const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::uno::XComponentContext >& xContext );

        /** Set factory object used to create the tree visitors

            Used for customizing the tree to the specific output
            format (writer, draw, etc)
         */
        void setTreeVisitorFactory(const TreeVisitorFactorySharedPtr& rVisitorFactory);

        /// TEMP - enable writer-like text:p on doc level
        void enableToplevelText() { m_bEnableToplevelText=true; }

        /** Export pdf document to ODG

            @param xOutput
            Stream to write the flat xml file to

            @param xStatus
            Optional status indicator
         */
        bool odfConvert( const rtl::OUString&                                                          rURL,
                         const com::sun::star::uno::Reference<com::sun::star::io::XOutputStream>&      xOutput,
                         const com::sun::star::uno::Reference<com::sun::star::task::XStatusIndicator>& xStatus );

        // XImportFilter
        virtual sal_Bool SAL_CALL importer( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rSourceData,
                                            const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& rHdl,
                                            const com::sun::star::uno::Sequence< rtl::OUString >& rUserData ) throw( com::sun::star::uno::RuntimeException );

        // XImporter
        virtual void SAL_CALL setTargetDocument( const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xDocument )
            throw( com::sun::star::lang::IllegalArgumentException );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
