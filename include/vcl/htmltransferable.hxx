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

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <cppuhelper/weak.hxx>
#include <vcl/dllapi.h>

namespace vcl::unohelper
{
// Helper class for passing HTML string as XTransferable to TransferableDataHelper object
class VCL_DLLPUBLIC HtmlTransferable final : public css::datatransfer::XTransferable,
                                             public ::cppu::OWeakObject
{
private:
    OString data;

public:
    HtmlTransferable(OString sData);
    SAL_DLLPRIVATE virtual ~HtmlTransferable() override;

    // css::uno::XInterface
    SAL_DLLPRIVATE css::uno::Any SAL_CALL queryInterface(const css::uno::Type& rType) override;
    void SAL_CALL acquire() noexcept override { OWeakObject::acquire(); }
    void SAL_CALL release() noexcept override { OWeakObject::release(); }

    // css::datatransfer::XTransferable
    SAL_DLLPRIVATE css::uno::Any SAL_CALL
    getTransferData(const css::datatransfer::DataFlavor& aFlavor) override;
    SAL_DLLPRIVATE css::uno::Sequence<css::datatransfer::DataFlavor>
        SAL_CALL getTransferDataFlavors() override;
    SAL_DLLPRIVATE sal_Bool SAL_CALL
    isDataFlavorSupported(const css::datatransfer::DataFlavor& aFlavor) override;
};

} // namespace vcl::unohelper
