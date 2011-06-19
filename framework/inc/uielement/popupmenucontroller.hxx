/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __FRAMEWORK_POPUPMENUCONTROLLER_HXX_
#define __FRAMEWORK_POPUPMENUCONTROLLER_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>

#include "svtools/toolboxcontroller.hxx"
#include "boost/scoped_ptr.hpp"

#include <macros/xserviceinfo.hxx>

class Window;

namespace framework
{
class PopupMenuControllerImpl;

class PopupMenuController : public svt::ToolboxController, public ::com::sun::star::lang::XServiceInfo
{
public:
    PopupMenuController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );
    ~PopupMenuController();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    DECLARE_XSERVICEINFO

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    // XStatusListener
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);

    bool CreatePopupMenuController() throw (::com::sun::star::uno::Exception);

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XPopupMenuController > mxPopupMenuController;
    ::com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu > mxPopupMenu;
};

} // namespace framework

#endif // __FRAMEWORK_POPUPMENUCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
