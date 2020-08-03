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
#pragma once

#include <com/sun/star/ui/XStatusbarItem.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <vcl/vclptr.hxx>

class StatusBar;

namespace framework
{

struct AddonStatusbarItemData;

typedef cppu::WeakComponentImplHelper< css::ui::XStatusbarItem > StatusbarItem_Base;

class StatusbarItem final : protected cppu::BaseMutex,
                      public StatusbarItem_Base
{
public:
    explicit StatusbarItem(
                            StatusBar              *pStatusBar,
                            sal_uInt16              nId,
                            const OUString&   aCommand );
    virtual ~StatusbarItem() override;

    void SAL_CALL disposing() override;

    // css::ui::XStatusbarItem Attributes
    virtual OUString SAL_CALL getCommand() override;
    virtual ::sal_uInt16 SAL_CALL getItemId() override;
    virtual ::sal_uInt32 SAL_CALL getWidth() override;
    virtual ::sal_uInt16 SAL_CALL getStyle() override;
    virtual ::sal_Int32 SAL_CALL getOffset() override;
    virtual css::awt::Rectangle SAL_CALL getItemRect() override;
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString& rText ) override;
    virtual OUString SAL_CALL getHelpText() override;
    virtual void SAL_CALL setHelpText( const OUString& rHelpText ) override;
    virtual OUString SAL_CALL getQuickHelpText() override;
    virtual void SAL_CALL setQuickHelpText( const OUString& rQuickHelpText ) override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual void SAL_CALL setAccessibleName( const OUString& rAccessibleName ) override;
    virtual sal_Bool SAL_CALL getVisible() override;
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) override;

    // css::ui::XStatusbarItem Methods
    virtual void SAL_CALL repaint(  ) override;

private:
    VclPtr<StatusBar>       m_pStatusBar;
    sal_uInt16              m_nId;
    sal_uInt16              m_nStyle;
    OUString           m_aCommand;

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
