/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DUMPFILTER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DUMPFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

namespace sw {

    /** Implementation of UNO export service to dump the layout of the
        document as XML. This filter should be mostly be used for testing
        purpose.
      */
    class LayoutDumpFilter : public cppu::WeakImplHelper
                             <
                               css::document::XFilter,
                               css::document::XExporter,
                               css::lang::XInitialization,
                               css::lang::XServiceInfo
                             >
    {
    protected:
        css::uno::Reference< css::lang::XComponent > m_xSrcDoc;

    public:
       LayoutDumpFilter();
       virtual ~LayoutDumpFilter();

        // XFilter
        virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL cancel(  )
            throw (css::uno::RuntimeException, std::exception) override;

        // XExporter
        virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
