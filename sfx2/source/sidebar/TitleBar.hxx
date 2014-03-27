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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_TITLEBAR_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_TITLEBAR_HXX

#include "Paint.hxx"

#include <vcl/fixed.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>


namespace sfx2 { namespace sidebar {

class TitleBar
    : public Window
{
public:
    TitleBar (
        const ::rtl::OUString& rsTitle,
        Window* pParentWindow,
        const sidebar::Paint& rInitialBackgroundPaint);
    virtual ~TitleBar (void);

    void SetTitle (const ::rtl::OUString& rsTitle);
    void SetIcon (const Image& rIcon);

    virtual void Paint (const Rectangle& rUpdateArea) SAL_OVERRIDE;
    virtual void DataChanged (const DataChangedEvent& rEvent) SAL_OVERRIDE;
    virtual void setPosSizePixel (
        long nX,
        long nY,
        long nWidth,
        long nHeight,
        sal_uInt16 nFlags = WINDOW_POSSIZE_ALL) SAL_OVERRIDE;

    ToolBox& GetToolBox (void);
    const ToolBox& GetToolBox (void) const;

protected:
    SidebarToolBox maToolBox;
    ::rtl::OUString msTitle;

    virtual Rectangle GetTitleArea (const Rectangle& rTitleBarBox) = 0;
    virtual void PaintDecoration (const Rectangle& rTitleBarBox) = 0;
    virtual void PaintFocus (const Rectangle& rFocusBox);
    virtual sidebar::Paint GetBackgroundPaint (void) = 0;
    virtual Color GetTextColor (void) = 0;
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex);
    virtual cssu::Reference<css::accessibility::XAccessible> CreateAccessible (void) SAL_OVERRIDE;

private:
    Image maIcon;

    void PaintTitle (const Rectangle& rTitleBox);
    DECL_LINK(SelectionHandler, ToolBox*);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
