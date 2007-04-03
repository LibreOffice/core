/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewTabBarModule.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:57:06 $
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

#ifndef SD_FRAMEWORK_VIEW_TAB_BAR_MODULE_HXX
#define SD_FRAMEWORK_VIEW_TAB_BAR_MODULE_HXX

#include "MutexOwner.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCHANGELISTENER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XTOOLBARCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XToolBarController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif


namespace {

typedef ::cppu::WeakComponentImplHelper1 <
    ::com::sun::star::drawing::framework::XConfigurationChangeListener
    > ViewTabBarModuleInterfaceBase;

} // end of anonymous namespace.




namespace sd { namespace framework {

/** This module is responsible for showing the ViewTabBar above the view in
    the center pane.
*/
class ViewTabBarModule
    : private sd::MutexOwner,
      public ViewTabBarModuleInterfaceBase
{
public:
    /** Create a new module that controlls the view tab bar above the view
        in the specified pane.
        @param rxController
            This is the access point to the drawing framework.
        @param rxViewTabBarId
            This ResourceId specifies which tab bar is to be managed by the
            new module.
    */
    ViewTabBarModule (
        const ::com::sun::star::uno::Reference<com::sun::star::frame::XController>& rxController,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxViewTabBarId);
    virtual ~ViewTabBarModule (void);

    virtual void SAL_CALL disposing (void);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (com::sun::star::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XToolBarController>
        mxToolBarController;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId> mxViewTabBarId;

    /** This is the place where the view tab bar is filled.  Only missing
        buttons are added, so it is safe to call this method multiple
        times.
    */
    void UpdateViewTabBar (void);
};

} } // end of namespace sd::framework

#endif
