/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * A LibreOffice extension to send the menubar structure through DBusMenu
 *
 * Copyright 2011 Canonical, Ltd.
 * Authors:
 *     Alberto Ruiz <alberto.ruiz@codethink.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the the GNU Lesser General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR PURPOSE.  See the applicable
 * version of the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with this program. If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef __FRAME_JOB_HXX__
#define __MRAME_JOB_HXX__

#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/awt/XMenu.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>

#include <glib.h>
#include <libdbusmenu-glib/menuitem.h>

#define FRAMEJOB_IMPLEMENTATION_NAME "com.sun.star.comp.Office.MyJob"
#define FRAMEJOB_SERVICE_NAME "com.sun.star.task.Job"

namespace css = ::com::sun::star;
using css::uno::Reference;

class FrameJob : public cppu::WeakImplHelper2 < css::task::XJob, css::lang::XServiceInfo >
{
 private:
    Reference < css::lang::XMultiServiceFactory > m_xMSF;
    Reference < css::container::XNameAccess >     m_xUICommands;
    Reference < css::frame::XFrame >              m_xFrame;

    unsigned long xid;

    //Private methods
    unsigned long     getXID                       (Reference < css::frame::XFrame >);
    DbusmenuMenuitem* getRootMenuitem              (Reference < css::awt::XMenu >,
                                                    gpointer);

 public:
    FrameJob( const css::uno::Reference< css::lang::XMultiServiceFactory > &rxMSF)
          : m_xMSF( rxMSF ) {};

    void exportMenus (Reference < css::frame::XFrame > xFrame);

    virtual ~FrameJob() {}

    // XJob
    virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& Arguments)
        throw (css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException);
};


::rtl::OUString FrameJob_getImplementationName()
    throw ( css::uno::RuntimeException );

sal_Bool SAL_CALL FrameJob_supportsService( const ::rtl::OUString& ServiceName )
    throw ( css::uno::RuntimeException );

css::uno::Sequence< ::rtl::OUString > SAL_CALL FrameJob_getSupportedServiceNames()
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL FrameJob_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
    throw ( css::uno::Exception );

#endif

