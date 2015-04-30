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

#ifndef INCLUDED_SFX2_SIDEBAR_SIDEBAR_HXX
#define INCLUDED_SFX2_SIDEBAR_SIDEBAR_HXX

#include <sfx2/dllapi.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XSidebarProvider.hpp>

#include <com/sun/star/awt/XWindow2.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/weakref.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>

#include <sfx2/sidebar/UnoDecks.hxx>
#include <com/sun/star/ui/XDecks.hpp>


using namespace css;

/** get the sidebar for a given frame
*/
class SfxUnoSidebar : public ::cppu::WeakImplHelper1< css::ui::XSidebarProvider >
{

private :

    const uno::Reference<frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

public:

    SfxUnoSidebar(const uno::Reference<frame::XFrame>&);

    virtual void SAL_CALL showDecks (const sal_Bool bVisible) SAL_OVERRIDE;

    virtual void SAL_CALL setVisible (const sal_Bool bVisible) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isVisible() SAL_OVERRIDE;

    virtual css::uno::Reference<frame::XFrame> SAL_CALL getFrame() throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual css::uno::Reference<ui::XDecks> SAL_CALL getDecks() SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
