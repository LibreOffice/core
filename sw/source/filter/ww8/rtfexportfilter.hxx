/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Miklos Vajna.
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

#ifndef _RTFEXPORTFILTER_HXX_
#define _RTFEXPORTFILTER_HXX_

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <tools/stream.hxx>
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

::rtl::OUString RtfExport_getImplementationName();
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL RtfExport_getSupportedServiceNames()
    throw();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL RtfExport_createInstance(
                                                                        const ::com::sun::star::uno::Reference<
                                                                        com::sun::star::uno::XComponentContext > &xCtx)
    throw( ::com::sun::star::uno::Exception );

#define IMPL_NAME_RTFEXPORT "com.sun.star.comp.Writer.RtfExport"

#endif // _RTFEXPORTFILTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
