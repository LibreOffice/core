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

#ifndef SD_FRAMEWORK_CHILD_WINDOW_PANE_HXX
#define SD_FRAMEWORK_CHILD_WINDOW_PANE_HXX

#include "framework/Pane.hxx"
#include "PaneShells.hxx"

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XRESOURCEID_HPP_
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#include <tools/link.hxx>
#include <memory>

namespace {

typedef ::cppu::ImplInheritanceHelper1 <
    ::sd::framework::Pane,
    ::com::sun::star::lang::XEventListener
    > ChildWindowPaneInterfaceBase;

} // end of anonymous namespace.


class SfxViewFrame;

namespace sd { class ViewShellBase; }

namespace sd { namespace framework {

/** The ChildWindowPane listens to the child window and disposes itself when
    the child window becomes inaccessible.  This happens for instance when a
    document is made read-only and the task pane is turned off.
*/
class ChildWindowPane
    : public ChildWindowPaneInterfaceBase
{
public:
    ChildWindowPane (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxPaneId,
        sal_uInt16 nChildWindowId,
        ViewShellBase& rViewShellBase,
        ::std::auto_ptr<SfxShell> pShell);
    virtual ~ChildWindowPane (void) throw();

    /** Hide the pane.  To make the pane visible again, call GetWindow().
    */
    void Hide (void);

    virtual void SAL_CALL disposing (void);

    /** This returns the content window when the child window is already
        visible.  Otherwise <NULL/> is returned.  In that case a later call
        may return the requested window (making a child window visible is an
        asynchronous process.)
        Note that GetWindow() may return different Window pointers when
        Hide() is called in between.
    */
    virtual ::Window* GetWindow (void);

    /** The local getWindow() first calls GetWindow() to provide a valid
        window pointer before forwarding the call to the base class.
    */
    virtual ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow>
        SAL_CALL getWindow (void)
        throw (::com::sun::star::uno::RuntimeException);

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()


    // XEventListener

    virtual void SAL_CALL disposing(
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId> mxPaneId;
    sal_uInt16 mnChildWindowId;
    ViewShellBase& mrViewShellBase;
    ::std::auto_ptr<SfxShell> mpShell;

    /** This flag is set when the pane shell has been activated at least
        once.  It is used to optimize the start-up performance (by not
        showing the window too early) and by not delaying its creation at
        later times.
    */
    bool mbHasBeenActivated;
};

} } // end of namespace sd::framework

#endif
