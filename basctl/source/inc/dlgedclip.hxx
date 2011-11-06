/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
