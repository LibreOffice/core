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

#ifndef __DESKTOP_JOB_HXX__
#define __DESKTOP_JOB_HXX__

#include <glib.h>
#include <libdbusmenu-glib/menuitem.h>

#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>

#define LIBREOFFICE_BUSNAME            "org.libreoffice.dbusmenu"
#define DESKTOPJOB_IMPLEMENTATION_NAME "org.libreoffice.desktop.AppMenuJob"
#define DESKTOPJOB_SERVICE_NAME        "org.libreoffice.desktop.AppMenuJob"

namespace css = ::com::sun::star;
using css::uno::Reference;
using css::uno::Sequence;
using css::uno::Any;
using css::uno::Exception;
using css::uno::RuntimeException;
using css::lang::IllegalArgumentException;

class DesktopJob : public cppu::WeakImplHelper2 < css::task::XJob, css::lang::XServiceInfo >
{
 private:
    Reference< css::lang::XMultiServiceFactory >  mxMSF;

 public:
    DesktopJob( const Reference< css::lang::XMultiServiceFactory > &rxMSF)
          : mxMSF( rxMSF ) {}

    virtual ~DesktopJob() {}

    // XJob
    virtual Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& Arguments)
        throw (IllegalArgumentException, Exception, RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (RuntimeException);

    virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};


::rtl::OUString
DesktopJob_getImplementationName()
    throw ( RuntimeException );

sal_Bool SAL_CALL
DesktopJob_supportsService( const ::rtl::OUString& ServiceName )
    throw ( RuntimeException );

Sequence< ::rtl::OUString > SAL_CALL
DesktopJob_getSupportedServiceNames()
    throw ( RuntimeException );

Reference< css::uno::XInterface >
SAL_CALL DesktopJob_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
    throw ( Exception );

#endif //__DESKTOP_JOB_HXX__

