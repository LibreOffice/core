/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Pane.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:03:33 $
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

#ifndef SD_FRAMEWORK_PANE_HXX
#define SD_FRAMEWORK_PANE_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/TabBarButton.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/compbase2.hxx>
#include <tools/link.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Window;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    ::com::sun::star::drawing::framework::XPane,
      ::com::sun::star::lang::XUnoTunnel
    > PaneInterfaceBase;

} // end of anonymous namespace.

namespace sd { namespace framework {

/** A pane is a wrapper for a window and possibly for a tab bar (for view
    switching).  Panes are unique resources.

    This class has two responsibilities:
    1. It implements the XPane interface.  This is the most important
    interface of this class for API based views (of which there not that
    many yet) because it gives access to the XWindow.
    2. It gives access to the underlying VCL Window by implementing the
    XUnoTunnel interface.  This is necessary at the moment and in the
    foreseeable future because many parts of the Draw and Impress views rely
    on direct access on the Window class.
*/
class Pane
    : protected MutexOwner,
      public PaneInterfaceBase
{
public:
    /** Create a new Pane object that wrapps the given window.
        @param rsPaneURL
            The URL that is used by the configuration to identify the pane.
            The given URL has to be valid.
        @param pWindow
            The VCL Window (usually this really is an sd::Window) that is
            wrapped by the new Pane object.  The given pointer must not be
            NULL.
    */
    Pane (
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XResourceId>& rxPaneId,
        ::Window* pWindow)
        throw ();
    virtual ~Pane (void) throw();

    virtual void SAL_CALL disposing (void);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId (void);

    /** This method is typically used together with the XUnoTunnel to obtain
        a Window pointer from an XPane object.
    */
    virtual ::Window* GetWindow (void);

    // XPane

    /** For a UNO API based implementation of a view this may the most
        important method of this class because the view is only interested
        in the window of the pane.
    */
    virtual ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow>
        SAL_CALL getWindow (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas>
        SAL_CALL getCanvas (void)
        throw (::com::sun::star::uno::RuntimeException);


    // XResource

    virtual ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>
        SAL_CALL getResourceId (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** For the typical pane it makes no sense to be dislayed without a
        view.  Therefore this default implementation returns always <TRUE/>.
    */
    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);


    // XUnoTunnel

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException);

protected:
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId> mxPaneId;
    ::Window* mpWindow;
    ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow> mxWindow;
    ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas> mxCanvas;

    /** Overload this method, not getCanvas(), when you want to provide a
        different canvas.
    */
    virtual ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas>
        CreateCanvas (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Throw DisposedException when the object has already been disposed or
        is currently being disposed.  Otherwise this method returns
        normally.
    */
    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
