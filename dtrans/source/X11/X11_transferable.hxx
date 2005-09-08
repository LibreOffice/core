/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: X11_transferable.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:03:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DTRANS_X11_TRANSFERABLE_HXX_
#define _DTRANS_X11_TRANSFERABLE_HXX_

#ifndef _DTRANS_X11_SELECTION_HXX_
#include <X11_selection.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HDL_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace x11 {

    class X11Transferable : public ::cppu::WeakImplHelper1 <
        ::com::sun::star::datatransfer::XTransferable >
    {
        ::osl::Mutex m_aMutex;

        SelectionManager&               m_rManager;
        Reference< XInterface >         m_xCreator;
        Atom                m_aSelection;
    public:
        X11Transferable( SelectionManager& rManager, const Reference< XInterface >& xCreator, Atom selection = None );
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
