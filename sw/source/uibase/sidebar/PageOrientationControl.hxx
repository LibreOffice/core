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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEORIENTATIONCONTROL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEORIENTATIONCONTROL_HXX

#include <memory>
#include <svx/tbxctl.hxx>
#include <svx/pageitem.hxx>
#include <svx/rulritem.hxx>
#include <editeng/sizeitem.hxx>

class Button;

namespace sw { namespace sidebar {

class PageOrientationControl : public SfxPopupWindow
{
public:
    explicit PageOrientationControl(sal_uInt16 nId, vcl::Window* pParent);
    virtual ~PageOrientationControl() override;
    virtual void dispose() override;

private:
    VclPtr<PushButton> m_pPortrait;
    VclPtr<PushButton> m_pLandscape;

    std::unique_ptr<SvxPageItem> mpPageItem;
    std::unique_ptr<SvxSizeItem> mpPageSizeItem;
    std::unique_ptr<SvxLongLRSpaceItem> mpPageLRMarginItem;
    std::unique_ptr<SvxLongULSpaceItem> mpPageULMarginItem;

    void ExecuteMarginULChange(const long nPageTopMargin, const long nPageBottomMargin);
    void ExecuteMarginLRChange(const long nPageLeftMargin, const long nPageRightMargin);
    void ExecuteOrientationChange(const bool bLandscape);

    DECL_LINK(ImplOrientationHdl, Button*, void);
};

} } // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
