/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dndhelp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:51:38 $
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

#ifndef _VCL_DNDHELP_HXX
#define _VCL_DNDHELP_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGGESTURELISTENER_HPP_
#include <com/sun/star/datatransfer/dnd/XDragGestureListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCELISTENER_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETLISTENER_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#endif

namespace com {
namespace sun {
namespace star {

namespace datatransfer {
namespace dnd {
    struct DragGestureEvent;
    struct DragSourceDragEvent;
    struct DragSourceDropEvent;
    struct DragSourceEvent;
    struct DropTargetDragEvent;
    struct DropTargetDragEnterEvent;
    struct DropTargetDropEvent;
    struct DropTargetEvent;
}}

}}}

namespace vcl
{

namespace unohelper
{

class VCL_DLLPUBLIC DragAndDropClient
{
public:
    virtual ~DragAndDropClient();

    // ::com::sun::star::datatransfer::dnd::XDragGestureListener
    virtual void dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::datatransfer::dnd::XDragSourceListener
    virtual void dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& dse ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::datatransfer::dnd::XDropTargetListener
    virtual void drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
};

class VCL_DLLPUBLIC DragAndDropWrapper :
                            public ::com::sun::star::datatransfer::dnd::XDragGestureListener,
                            public ::com::sun::star::datatransfer::dnd::XDragSourceListener,
                            public ::com::sun::star::datatransfer::dnd::XDropTargetListener,
                            public ::cppu::OWeakObject
{
private:
    DragAndDropClient* mpClient;

public:
                    DragAndDropWrapper( DragAndDropClient* pClient );
                    ~DragAndDropWrapper();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::datatransfer::dnd::XDragGestureListener
    void SAL_CALL dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::datatransfer::dnd::XDragSourceListener
    void SAL_CALL dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& dse ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::datatransfer::dnd::XDropTargetListener
    void SAL_CALL drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
};

}}  // namespace vcl::unohelper

#endif  // _VCL_DNDHELP_HXX

