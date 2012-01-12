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



#ifndef _DTRANS_X11_TRANSFERABLE_HXX_
#define _DTRANS_X11_TRANSFERABLE_HXX_

#include <X11_selection.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HDL_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#include <cppuhelper/implbase1.hxx>

namespace x11 {

    class X11Transferable : public ::cppu::WeakImplHelper1 <
        ::com::sun::star::datatransfer::XTransferable >
    {
        ::osl::Mutex m_aMutex;

        SelectionManager&               m_rManager;
        com::sun::star::uno::Reference< XInterface >            m_xCreator;
        Atom                m_aSelection;
    public:
        X11Transferable( SelectionManager& rManager, const com::sun::star::uno::Reference< XInterface >& xCreator, Atom selection = None );
        virtual ~X11Transferable();

        /*
         * XTransferable
         */

        virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
            throw(::com::sun::star::datatransfer::UnsupportedFlavorException,
                  ::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException
                  );

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
            throw(::com::sun::star::uno::RuntimeException);
    };

} // namespace

#endif
