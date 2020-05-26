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

#ifndef INCLUDED_SVX_LBOXCTRL_HXX
#define INCLUDED_SVX_LBOXCTRL_HXX

#include <rtl/ustring.hxx>
#include <vector>
#include <svtools/popupwindowcontroller.hxx>
#include <svx/svxdllapi.h>

class ToolBox;
class SvxPopupWindowListBox;

class SvxUndoRedoControl final : public svt::PopupWindowController
{
    std::vector< OUString > aUndoRedoList;
    OUString                aDefaultTooltip;

public:
    SvxUndoRedoControl(const css::uno::Reference<css::uno::XComponentContext>& rContext);
    virtual ~SvxUndoRedoControl() override;

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& rArguments ) override;

    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    void Do(sal_Int16 nCount);

    void SetText(const OUString& rText);

    void SetInfo(sal_Int32 nCount);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
