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

#ifndef INCLUDED_VCL_UNOHELP2_HXX
#define INCLUDED_VCL_UNOHELP2_HXX

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

namespace com { namespace sun { namespace star { namespace datatransfer { namespace clipboard {
    class XClipboard;
} } } } }
#include <vcl/dllapi.h>

namespace vcl { namespace unohelper {

    class VCL_DLLPUBLIC TextDataObject :
                            public css::datatransfer::XTransferable,
                            public ::cppu::OWeakObject
    {
    private:
        OUString        maText;

    public:
                        TextDataObject( const OUString& rText );
                        virtual ~TextDataObject();

        OUString&       GetString() { return maText; }

        // css::uno::XInterface
        css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
        void          SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
        void          SAL_CALL release() throw() override  { OWeakObject::release(); }

        // css::datatransfer::XTransferable
        css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) throw(css::datatransfer::UnsupportedFlavorException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
        css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(css::uno::RuntimeException, std::exception) override;
        sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) throw(css::uno::RuntimeException, std::exception) override;

        /// copies a given string to a given clipboard
        static  void    CopyStringTo(
            const OUString& rContent,
            const css::uno::Reference< css::datatransfer::clipboard::XClipboard >& rxClipboard
        );
    };

    struct MutexHelper
    {
        private:
            ::osl::Mutex maMutex;
        public:
            ::osl::Mutex& GetMutex() { return maMutex; }
    };

}}  // namespace vcl::unohelper

#endif // INCLUDED_VCL_UNOHELP2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
