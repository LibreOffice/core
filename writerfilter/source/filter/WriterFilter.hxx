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

#ifndef INCLUDED_WRITERFILTER_SOURCE_FILTER_WRITERFILTER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_FILTER_WRITERFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <cppuhelper/implbase5.hxx>
#include <comphelper/sequenceashashmap.hxx>

/// Common DOC/DOCX filter, calls DocxExportFilter via UNO or does the DOCX import.
class WriterFilter : public cppu::WeakImplHelper5
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter,
    com::sun::star::document::XExporter,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xSrcDoc, m_xDstDoc;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > m_xHandler;


public:
   WriterFilter( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext) {}
   virtual ~WriterFilter() {}

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancel(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    void putPropertiesToDocumentGrabBag(const comphelper::SequenceAsHashMap& rProperties);

};


OUString WriterFilter_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

bool SAL_CALL WriterFilter_supportsService( const OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< OUString > SAL_CALL WriterFilter_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
