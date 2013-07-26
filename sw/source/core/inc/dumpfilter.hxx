/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _LAYOUTDUMP_HXX
#define _LAYOUTDUMP_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>

namespace sw {

    /** Implementation of UNO export service to dump the layout of the
        document as XML. This filter should be mostly be used for testing
        purpose.
      */
    class LayoutDumpFilter : public cppu::WeakImplHelper4
                             <
                               com::sun::star::document::XFilter,
                               com::sun::star::document::XExporter,
                               com::sun::star::lang::XInitialization,
                               com::sun::star::lang::XServiceInfo
                             >
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xSrcDoc;

    public:
       LayoutDumpFilter();
       virtual ~LayoutDumpFilter();

        // XFilter
        virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
            throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancel(  )
            throw (::com::sun::star::uno::RuntimeException);

        // XExporter
        virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw (::com::sun::star::uno::RuntimeException);

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
