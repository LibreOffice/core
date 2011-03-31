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

#ifndef _RTFIMPORTFILTER_HXX_
#define _RTFIMPORTFILTER_HXX_

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>

/// The physical access to the RTF document (for reading).
class RtfImportFilter : public cppu::WeakImplHelper2
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter
>
{
protected:
    ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xCtx;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xDstDoc;
public:
    RtfImportFilter( const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xCtx );
    virtual ~RtfImportFilter();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};

::rtl::OUString RtfImport_getImplementationName();
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL RtfImport_getSupportedServiceNames()
    throw();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL RtfImport_createInstance(
                                                                        const ::com::sun::star::uno::Reference<
                                                                        com::sun::star::uno::XComponentContext > &xCtx)
    throw( ::com::sun::star::uno::Exception );

#define IMPL_NAME_RTFIMPORT "com.sun.star.comp.Writer.RtfImport"

#endif // _RTFIMPORTFILTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
