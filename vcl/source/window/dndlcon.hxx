/*************************************************************************
 *
 *  $RCSfile: dndlcon.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obr $ $Date: 2001-02-20 11:17:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DNDLCON_HXX_
#define _DNDLCON_HXX_

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGGESTURERECOGNIZER_HPP_
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

class DNDListenerContainer : public ::cppu::WeakComponentImplHelper2<
    ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer, \
    ::com::sun::star::datatransfer::dnd::XDropTarget >
{
    ::osl::Mutex m_aMutex;

    sal_Bool m_bActive;
    sal_Int8 m_nDefaultActions;

public:

    DNDListenerContainer( sal_Int8 nDefaultActions );
    virtual ~DNDListenerContainer();

    sal_uInt32 fireDropEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDropContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& transferable );

    sal_uInt32 fireDragExitEvent();

    sal_uInt32 fireDragOverEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions );

    sal_uInt32 fireDragEnterEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >& dataFlavor );

    sal_uInt32 fireDropActionChangedEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions );

    sal_uInt32 fireDragGestureEvent(
        sal_Int8 dragAction, sal_Int32 dragOriginX, sal_Int32 dragOriginY,
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource >& dragSource,
        const ::com::sun::star::uno::Any& triggerEvent );

    /*
     * XDragGestureRecognizer
     */

    virtual void SAL_CALL addDragGestureListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureListener >& dgl ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeDragGestureListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureListener >& dgl ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetRecognizer(  ) throw(::com::sun::star::uno::RuntimeException);

    /*
     * XDropTarget
     */

    virtual void SAL_CALL addDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isActive(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActive( sal_Bool active ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int8 SAL_CALL getDefaultActions(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultActions( sal_Int8 actions ) throw(::com::sun::star::uno::RuntimeException);
};


//==================================================================================================
//
//==================================================================================================

#endif
