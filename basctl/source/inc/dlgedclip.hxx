/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgedclip.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BASCTL_DLGEDCLIP_HXX
#define _BASCTL_DLGEDCLIP_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>


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
    DlgEdTransferableImpl();
    DlgEdTransferableImpl( const ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >& aSeqFlavors, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aSeqData );
    virtual ~DlgEdTransferableImpl();

    // XTransferable
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) throw(::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) throw(::com::sun::star::uno::RuntimeException);

    // XClipboardOwner
    virtual void SAL_CALL lostOwnership( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& xClipboard, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans ) throw(::com::sun::star::uno::RuntimeException);
};

#endif // _BASCTL_DLGEDCLIP_HXX
