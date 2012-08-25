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

#ifndef BASCTL_DLGEDCLIP_HXX
#define BASCTL_DLGEDCLIP_HXX

#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <cppuhelper/implbase2.hxx>

namespace basctl
{

//============================================================================
// DlgEdTransferableImpl
//============================================================================

typedef ::cppu::WeakImplHelper2< ::com::sun::star::datatransfer::XTransferable, ::com::sun::star::datatransfer::clipboard::XClipboardOwner > DlgEdTransferableHelper;

class DlgEdTransferableImpl : public DlgEdTransferableHelper
{
private:
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > m_SeqFlavors;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > m_SeqData;

protected:
    virtual sal_Bool compareDataFlavors( const ::com::sun::star::datatransfer::DataFlavor& lFlavor, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

public:
    DlgEdTransferableImpl( const ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >& aSeqFlavors, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aSeqData );
    virtual ~DlgEdTransferableImpl();

    // XTransferable
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) throw(::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) throw(::com::sun::star::uno::RuntimeException);

    // XClipboardOwner
    virtual void SAL_CALL lostOwnership( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& xClipboard, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans ) throw(::com::sun::star::uno::RuntimeException);
};

} // namespace basctl

#endif // BASCTL_DLGEDCLIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
