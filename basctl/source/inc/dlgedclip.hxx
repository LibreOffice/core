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

#ifndef INCLUDED_BASCTL_SOURCE_INC_DLGEDCLIP_HXX
#define INCLUDED_BASCTL_SOURCE_INC_DLGEDCLIP_HXX

#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <cppuhelper/implbase.hxx>

namespace basctl
{


class DlgEdTransferableImpl : public ::cppu::WeakImplHelper< css::datatransfer::XTransferable,
                                                             css::datatransfer::clipboard::XClipboardOwner >
{
private:
    css::uno::Sequence< css::datatransfer::DataFlavor > m_SeqFlavors;
    css::uno::Sequence< css::uno::Any > m_SeqData;

protected:
    static bool compareDataFlavors( const css::datatransfer::DataFlavor& lFlavor, const css::datatransfer::DataFlavor& rFlavor );

public:
    DlgEdTransferableImpl( const css::uno::Sequence< css::datatransfer::DataFlavor >& aSeqFlavors, const css::uno::Sequence< css::uno::Any >& aSeqData );
    virtual ~DlgEdTransferableImpl();

    // XTransferable
    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& rFlavor ) throw(css::datatransfer::UnsupportedFlavorException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& rFlavor ) throw(css::uno::RuntimeException, std::exception) override;

    // XClipboardOwner
    virtual void SAL_CALL lostOwnership( const css::uno::Reference< css::datatransfer::clipboard::XClipboard >& xClipboard, const css::uno::Reference< css::datatransfer::XTransferable >& xTrans ) throw(css::uno::RuntimeException, std::exception) override;
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_DLGEDCLIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
