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
