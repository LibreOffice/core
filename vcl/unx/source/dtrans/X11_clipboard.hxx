/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _DTRANS_X11_CLIPBOARD_HXX_
#define _DTRANS_X11_CLIPBOARD_HXX_

#include <X11_selection.hxx>

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif
#include <cppuhelper/compbase4.hxx>

// ------------------------------------------------------------------------

#define X11_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.datatransfer.X11ClipboardSupport"

namespace x11 {

    class X11Clipboard :
        public ::cppu::WeakComponentImplHelper4 <
        ::com::sun::star::datatransfer::clipboard::XClipboardEx,
        ::com::sun::star::datatransfer::clipboard::XClipboardNotifier,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization
        >,
        public SelectionAdaptor
    {
        com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > m_aContents;
        com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;

        SelectionManager&                                       m_rSelectionManager;
        com::sun::star::uno::Reference< ::com::sun::star::lang::XInitialization >   m_xSelectionManager;
        ::std::list< com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > > m_aListeners;
        Atom                                                    m_aSelection;

    protected:


        friend class SelectionManager;
        friend class X11_Transferable;

        void fireChangedContentsEvent();
        void clearContents();

    public:

        X11Clipboard( SelectionManager& rManager, Atom aSelection );
        virtual ~X11Clipboard();

        static X11Clipboard* get( const ::rtl::OUString& rDisplayName, Atom aSelection );

        /*
         *  XInitialization
         */
        virtual void SAL_CALL initialize( const Sequence< Any >& arguments ) throw(  ::com::sun::star::uno::Exception );

        /*
         * XServiceInfo
         */

        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw(RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw(RuntimeException);

        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw(RuntimeException);

        /*
         * XClipboard
         */

        virtual com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
            throw(RuntimeException);

        virtual void SAL_CALL setContents(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans,
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
            throw(RuntimeException);

        virtual ::rtl::OUString SAL_CALL getName()
            throw(RuntimeException);

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(RuntimeException);

        /*
         * XClipboardNotifier
         */
        virtual void SAL_CALL addClipboardListener(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(RuntimeException);

        virtual void SAL_CALL removeClipboardListener(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(RuntimeException);

        /*
         *  SelectionAdaptor
         */
        virtual com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > getTransferable();
        virtual void clearTransferable();
        virtual void fireContentsChanged();
        virtual com::sun::star::uno::Reference< XInterface > getReference() throw();
    };

// ------------------------------------------------------------------------

    Sequence< ::rtl::OUString > SAL_CALL X11Clipboard_getSupportedServiceNames();
    com::sun::star::uno::Reference< XInterface > SAL_CALL X11Clipboard_createInstance(
        const com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

// ------------------------------------------------------------------------

} // namepspace

#endif
