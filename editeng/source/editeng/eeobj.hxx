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

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <tools/stream.hxx>

class EditDataObject :  public ::cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
private:
    SvMemoryStream  maRTFData;
    SvMemoryStream  maODFData;
    OUString        maText;
    OUString        maOfficeBookmark;

public:
                    EditDataObject();
                    virtual ~EditDataObject() override;

    SvMemoryStream& GetRTFStream() { return maRTFData; }
    SvMemoryStream& GetODFStream() { return maODFData; }
    OUString&       GetString() { return maText; }
    OUString&       GetURL()    { return maOfficeBookmark; }

    // css::datatransfer::XTransferable
    css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;
    css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) override;
    sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
