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

#ifndef INCLUDED_SVTOOLS_POPUPWINDOWCONTROLLER_HXX
#define INCLUDED_SVTOOLS_POPUPWINDOWCONTROLLER_HXX

#include <memory>
#include <svtools/svtdllapi.h>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }

namespace svt
{
class PopupWindowControllerImpl;

class SVT_DLLPUBLIC PopupWindowController : public cppu::ImplInheritanceHelper< svt::ToolboxController,
                                                                                css::lang::XServiceInfo >
{
public:
    PopupWindowController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                           const css::uno::Reference< css::frame::XFrame >& xFrame,
                           const OUString& aCommandURL );
    virtual ~PopupWindowController() override;

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) = 0;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override = 0;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override = 0;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;

private:
    std::unique_ptr< PopupWindowControllerImpl >  mxImpl;
};

} // namespace svt

#endif // INCLUDED_SVTOOLS_POPUPWINDOWCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
