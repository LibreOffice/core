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

#pragma once

#include "X11_selection.hxx"
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <cppuhelper/implbase.hxx>

namespace x11 {

    class X11Transferable : public ::cppu::WeakImplHelper< css::datatransfer::XTransferable >
    {
        SelectionManager&               m_rManager;
        Atom                m_aSelection;
    public:
        X11Transferable( SelectionManager& rManager, Atom selection );
        virtual ~X11Transferable() override;

        /*
         * XTransferable
         */

        virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;

        virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) override;

        virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;
    };

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
