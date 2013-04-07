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

#ifndef _RTFEXPORTFILTER_HXX_
#define _RTFEXPORTFILTER_HXX_

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <shellio.hxx>

/// Dummy Writer implementation to be able to use the string format methods of the base class
class RtfWriter : public Writer
{
protected:
    sal_uLong WriteStream() { return 0; }
};

/// The physical access to the RTF document (for writing).
class RtfExportFilter : public cppu::WeakImplHelper2
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XExporter
>
{
protected:
    ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xCtx;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xSrcDoc;
    SvStream* m_pStream;
public:
    RtfExportFilter( const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xCtx );
    virtual ~RtfExportFilter();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    RtfWriter m_aWriter;
};

OUString RtfExport_getImplementationName();
::com::sun::star::uno::Sequence< OUString > SAL_CALL RtfExport_getSupportedServiceNames()
    throw();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL RtfExport_createInstance(
                                                                        const ::com::sun::star::uno::Reference<
                                                                        com::sun::star::uno::XComponentContext > &xCtx)
    throw( ::com::sun::star::uno::Exception );

#define IMPL_NAME_RTFEXPORT "com.sun.star.comp.Writer.RtfExport"

#endif // _RTFEXPORTFILTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
