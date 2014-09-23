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

#ifndef INCLUDED_SD_SOURCE_UI_CONTROLLER_SLIDELAYOUTCONTROLLER_HXX
#define INCLUDED_SD_SOURCE_UI_CONTROLLER_SLIDELAYOUTCONTROLLER_HXX

#include <svtools/popupwindowcontroller.hxx>

namespace sd
{

class SlideLayoutController : public svt::PopupWindowController
{
public:
    SlideLayoutController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                           const OUString& sCommandURL,
                           bool bInsertPage );

    virtual vcl::Window* createPopupWindow( vcl::Window* pParent ) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    using svt::PopupWindowController::createPopupWindow;
private:
    bool mbInsertPage;
};

}

#endif // INCLUDED_SD_SOURCE_UI_CONTROLLER_SLIDELAYOUTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
