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

#ifndef INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_TRANSFERABLE_HXX
#define INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_TRANSFERABLE_HXX

#include <X11_selection.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/implbase.hxx>

namespace x11 {

    class X11Transferable : public ::cppu::WeakImplHelper <
        ::com::sun::star::datatransfer::XTransferable >
    {
        SelectionManager&               m_rManager;
        Atom                m_aSelection;
    public:
        X11Transferable( SelectionManager& rManager, Atom selection = None );
        virtual ~X11Transferable();

        /*
         * XTransferable
         */

        virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
            throw(::com::sun::star::datatransfer::UnsupportedFlavorException,
                  ::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException, std::exception
                  ) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
