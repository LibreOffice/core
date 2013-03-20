/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    PopupMenuController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );
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
