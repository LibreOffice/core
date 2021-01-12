/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#ifndef INCLUDED_VCL_IOS_IOSTRANSFERABLE_HXX
#define INCLUDED_VCL_IOS_IOSTRANSFERABLE_HXX

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>

#include "DataFlavorMapping.hxx"

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

#include <memory>
#include <vector>

class iOSTransferable : public ::cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
public:
    explicit iOSTransferable(
        css::uno::Reference<css::datatransfer::XMimeContentTypeFactory> const& rXMimeCntFactory,
        std::shared_ptr<DataFlavorMapper> pDataFlavorMapper);

    virtual ~iOSTransferable() override;
    iOSTransferable(const iOSTransferable&) = delete;
    iOSTransferable& operator=(const iOSTransferable&) = delete;

    // XTransferable

    virtual css::uno::Any SAL_CALL
    getTransferData(const css::datatransfer::DataFlavor& aFlavor) override;

    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;

    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& aFlavor) override;

    // Helper functions not part of the XTransferable interface

    void initClipboardItemList();

    bool compareDataFlavors(const css::datatransfer::DataFlavor& lhs,
                            const css::datatransfer::DataFlavor& rhs);

private:
    css::uno::Sequence<css::datatransfer::DataFlavor> mFlavorList;
    css::uno::Reference<css::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
    std::shared_ptr<DataFlavorMapper> mDataFlavorMapper;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
