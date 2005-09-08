/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: X11_dndcontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:02:04 $
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

#ifndef _DTRANS_X11_DNDCONTEXT_HXX
#define _DTRANS_X11_DNDCONTEXT_HXX

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCECONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDROPCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDRAGCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <X11/Xlib.h>

using namespace com::sun::star::uno;

namespace x11 {

    class SelectionManager;

    class DropTargetDropContext :
        public ::cppu::WeakImplHelper1<
    ::com::sun::star::datatransfer::dnd::XDropTargetDropContext
    >
    {
        Window                      m_aDropWindow;
        Time                        m_nTimestamp;
        SelectionManager&           m_rManager;
        Reference< XInterface >     m_xManagerRef;
    public:
        DropTargetDropContext( Window, Time, SelectionManager& );
        virtual ~DropTargetDropContext();

        // XDropTargetDropContext
        virtual void SAL_CALL acceptDrop( sal_Int8 dragOperation ) throw();
        virtual void SAL_CALL rejectDrop() throw();
        virtual void SAL_CALL dropComplete( sal_Bool success ) throw();
    };

    class DropTargetDragContext :
        public ::cppu::WeakImplHelper1<
    ::com::sun::star::datatransfer::dnd::XDropTargetDragContext
    >
    {
        Window                      m_aDropWindow;
        Time                        m_nTimestamp;
        SelectionManager&           m_rManager;
        Reference< XInterface >     m_xManagerRef;
    public:
        DropTargetDragContext( Window, Time, SelectionManager& );
        virtual ~DropTargetDragContext();

        // XDropTargetDragContext
        virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw();
        virtual void SAL_CALL rejectDrag() throw();
    };

    class DragSourceContext :
        public ::cppu::WeakImplHelper1<
    ::com::sun::star::datatransfer::dnd::XDragSourceContext
    >
    {
        Window                      m_aDropWindow;
        Time                        m_nTimestamp;
        SelectionManager&           m_rManager;
        Reference< XInterface >     m_xManagerRef;
    public:
        DragSourceContext( Window, Time, SelectionManager& );
        virtual ~DragSourceContext();

        // XDragSourceContext
        virtual sal_Int32   SAL_CALL getCurrentCursor() throw();
        virtual void        SAL_CALL setCursor( sal_Int32 cursorId ) throw();
        virtual void        SAL_CALL setImage( sal_Int32 imageId ) throw();
        virtual void        SAL_CALL transferablesFlavorsChanged() throw();
    };
} // namespace

#endif // _DTRANS_X11_DNDCONTEXT_HXX
